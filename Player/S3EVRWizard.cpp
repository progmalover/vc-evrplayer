#include "stdafx.h"
#include "S3EVRWizard.h"
#include "EVRPresenterInclude.h"
#include "S3RenderEngine.h"
#include "S3RenderMixer.h"
#include <multimon.h>
#ifdef _BUILD_FROM_VS
#include <Windows.h>
#endif

using namespace std;



/******************************Public*Routine******************************\
* S3EVRWizard
*
* constructor
\**************************************************************************/
S3EVRWizard::S3EVRWizard()
    :  CUnknown(NAME("S3EVR Wizard"), NULL), S3DShowWizard()

{

}


S3EVRWizard::~S3EVRWizard()
{
}

/******************************Public*Routine******************************\
* NonDelegatingQueryInterface
\**************************************************************************/
STDMETHODIMP
S3EVRWizard::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;


    hr = CUnknown::NonDelegatingQueryInterface(riid,ppv);

    return hr;
}




HRESULT S3EVRWizard::Attach(
    IBaseFilter* pEVR,
    DWORD_PTR* pdwID)
{
    HRESULT hr = S_OK;

    FILTER_INFO fiEVR;
    OAFilterState state;
    DWORD dwVMRMode = 0L;;

    S3VR_VideoSource*       pVideoSource    = NULL;
    IMediaControl*          pMediaControl   = NULL;
    IFilterGraph*           pFilterGraph    = NULL;
    IDirect3DDevice9*       pDevice         = NULL;
    IDirect3D9*             pd3d9			= NULL;

    // check that pointers are valid
    if( !pEVR || !pdwID )
    {
        ::DbgMsg(_T("S3EVRWizard::Attach received NULL pointer"));
        return E_POINTER;
    }

    if( !m_bInitialized )
    {
        ::DbgMsg(_T("S3EVRWizard::Attach: method Initialize() was not called!"));
        return VFW_E_WRONG_STATE;
    }

    try
    {
        // check that provided VMR is part of the graph
        hr = pEVR->QueryFilterInfo( &fiEVR );
        CHECK_HR(
            (NULL == fiEVR.pGraph) ? E_FAIL : S_OK,
            ::DbgMsg(_T("S3EVRWizard::Attach: provided VMR was not added to the graph")));

        pFilterGraph = fiEVR.pGraph;
        pFilterGraph->AddRef();


        pVideoSource = new S3VR_VideoSource;

        CHECK_HR(
            !pVideoSource ? E_OUTOFMEMORY : S_OK,
            ::DbgMsg(_T("S3EVRWizard::Attach: failed to allocate S3VR_VideoSource structure")));

        pVideoSource->dwID = (DWORD_PTR)pVideoSource;

        pVideoSource->pRender = pEVR;
        pVideoSource->pRender->AddRef();

        pVideoSource->pGraph = pFilterGraph;
        pVideoSource->pGraph->AddRef();

        hr = pEVR->QueryInterface(IID_IMFVideoRenderer, (void**)&pVideoSource->pVideoAllocator);

        CHECK_HR(
            pVideoSource->pGraph->QueryInterface( IID_IMediaControl,
                            (void**)&pMediaControl),
            ::DbgMsg(_T("S3EVRWizard::Attach: cannot QI IMediaControl")));

        CHECK_HR(
            hr = pMediaControl->GetState( 100, &state),
            ::DbgMsg(_T("S3EVRWizard::Attach: failed to get state of IMediaControl, hr = 0x%08x"), hr));

        CHECK_HR(
            ( state != State_Stopped ) ? VFW_E_NOT_STOPPED : S_OK,
            ::DbgMsg(_T("S3EVRWizard::Attach: graph is not stopped, state = %ld"), state));


        IEVRSamplePresenter *pEVRSamplePresenter = NULL;
        hr = GetInterface((IEVRSamplePresenter *)this, (void **)&pEVRSamplePresenter );

        EVRCustomPresenter *pPresenter = new EVRCustomPresenter(hr, m_pRenderEngine, pEVRSamplePresenter, (DWORD_PTR)pVideoSource);
        IMFVideoPresenter *pRenderLessPresenter = NULL;
        hr = pPresenter->QueryInterface(IID_IMFVideoPresenter,(void **)&pRenderLessPresenter);


        pVideoSource->pVideoPresenter = pPresenter;


        // try to advise 'this' custom allocator-presenter to the EVR
        CHECK_HR(
            hr = ((IMFVideoRenderer *)pVideoSource->pVideoAllocator)->InitializeRenderer(NULL, pRenderLessPresenter),
            ::DbgMsg(_T("S3EVRWizard::Attach: failed to advise A/P, hr = 0x%08x"), hr));

        pRenderLessPresenter->Release();

        // we have to be thread safe only here when we actually mess up with shared data
        CAutoLock Lock(&m_WizardObjectLock);


        // we successfully attached subgraph, last thing left is to save
        // pVideoSource in the list
        m_listVideoSources.push_back( pVideoSource );
        *pdwID = pVideoSource->dwID;

    } // try
    catch( HRESULT hr1 )
    {
        hr = hr1;
        if( pVideoSource )
        {
            delete pVideoSource;
            pVideoSource = NULL;
        }
    }
    RELEASE( pd3d9 );
    RELEASE( fiEVR.pGraph );
    RELEASE( pFilterGraph );
    RELEASE( pMediaControl );
    RELEASE( pDevice );

    return hr;
}

HRESULT S3EVRWizard::Detach(DWORD_PTR dwID)
{
    HRESULT hr = S_OK;
    OAFilterState state;

    bool bSourceWasDeleted = false;

    IMediaControl*     pMc             = NULL;
    S3RenderMixer*     pMixerControl   = NULL;

    S3VR_VideoSource*  pvideosource    = NULL;
    S3VR_VideoSource*  pcursource      = NULL;

    if( !m_bInitialized )
    {
        ::DbgMsg(_T("S3EVRWizard::Detach: method 'Initialize' was never called"));
        return VFW_E_WRONG_STATE;
    }

    hr = GetSourceInfo_( dwID, &pvideosource );
    if( FAILED(hr) || !pvideosource )
    {
        ::DbgMsg(_T("S3EVRWizard::Detach: Failed in GetSourceInfo_()"));
        return ( FAILED(hr) ? hr : VFW_E_NOT_FOUND );
    }

    if( !m_pRenderEngine )
    {
        ::DbgMsg(_T("S3EVRWizard::Detach: FATAL IS3VRRenderEngine pointer is NULL!"));
        return E_UNEXPECTED;
    }

    if( !pvideosource->pGraph )
    {
        ::DbgMsg(_T("S3EVRWizard::Detach: video source info does not contain pointer to IFilterGraph!"));
        return VFW_E_NOT_FOUND;
    }

    try
    {
        CHECK_HR(
            hr = (pvideosource->pGraph)->QueryInterface(
                                    IID_IMediaControl, (void**)&pMc ),
            ::DbgMsg(_T("S3EVRWizard::Detach: cannot QI IMediaControl of the graph, hr = 0x%08x"), hr));

        CHECK_HR(
            hr = pMc->GetState( 100, &state ),
            ::DbgMsg(_T("S3EVRWizard::Detach: cannot obtain state from IMediaControl, hr = 0x%08x"), hr));

        CHECK_HR(
            ( State_Stopped != state ) ? VFW_E_NOT_STOPPED : S_OK,
            ::DbgMsg(_T("S3EVRWizard::Detach: correspondent graph was not stopped")));

        // advise NULL as A/P to VMR9 (this will return VMR9 to its default A/P)
        CHECK_HR(
            ( !(pvideosource->pVideoAllocator)) ? VFW_E_NOT_FOUND : S_OK,
            ::DbgMsg(_T("S3EVRWizard::Detach: video source info does not contain pointer to IVMRSurfaceAllocatorNotify9")));

        CHECK_HR(
            hr = m_pRenderEngine->GetMixerControl( &pMixerControl ),
            ::DbgMsg(_T("S3EVRWizard::Detach: FATAL, cannot find currently active IS3VRMixerControl!")));

        // we have to be thread safe only here when we actually mess up with shared data

        
        CHECK_HR(
            pvideosource->DisconnectPins(),
            ::DbgMsg(_T("S3EVRWizard::Detach: FATAL, failed to disconnect pins of EVR")));

        CHECK_HR(
            hr = ((IMFVideoRenderer *)pvideosource->pVideoAllocator)->InitializeRenderer(NULL, NULL),
            ::DbgMsg(_T("S3EVRWizard::Attach: failed to advise A/P, hr = 0x%08x"), hr));

        ((EVRCustomPresenter*)pvideosource->pVideoPresenter)->Release();

        CAutoLock Lock(&m_WizardObjectLock);

        // we unadvised custom allocator-presenter successfully, let's delete
        // video source structure from the list
        list< S3VR_VideoSource*>::iterator start, end, it;
        start = m_listVideoSources.begin();
        end = m_listVideoSources.end();

        for( it=start; it!=end; it++)
        {
            pcursource = (S3VR_VideoSource*)(*it);

            CHECK_HR(
                ( NULL == pcursource ) ? E_UNEXPECTED : S_OK,
                ::DbgMsg(_T("S3EVRWizard::Detach: FATAL, m_listVideoSources contains NULL pointer")));

            if( dwID == pcursource->dwID )
            {

                m_listVideoSources.remove( pcursource );
                delete pcursource;
                pcursource = NULL;
                bSourceWasDeleted = true;
                break;
            }
        }// for
        CHECK_HR(
            ( false == bSourceWasDeleted ) ? VFW_E_NOT_FOUND : S_OK,
            ::DbgMsg(_T("S3EVRWizard::Detach: FATAL, failed to delete source from the list (source was not found)")));
    }// try
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }

    RELEASE( pMc );

    return hr;
}



HRESULT S3EVRWizard::EndDeviceLoss( IDirect3DDevice9* pDevice )
{
	HRESULT hr = S_OK;
	IDirect3D9* pd3d9 = NULL;
	HMONITOR hMon = NULL;

	if( !pDevice )
		return E_POINTER;

	hr = pDevice->GetDirect3D( &pd3d9 );
	if( pd3d9 )
	{
		hMon = pd3d9->GetAdapterMonitor( D3DADAPTER_DEFAULT );

	}
	list<S3VR_VideoSource*>::iterator start, end, it;
	start = m_listVideoSources.begin();
	end = m_listVideoSources.end();
	for( it=start; it!=end; it++)
	{
		S3VR_VideoSource* pSource = (S3VR_VideoSource*)(*it);
		if( pSource && pSource->pVideoPresenter )
		{
			((EVRCustomPresenter*)pSource->pVideoPresenter)->NotifyEvent(EC_DISPLAY_CHANGED, S_OK, 0);
		}
	}				
	RELEASE( pd3d9 );
	return hr;
}


const DWORD PRESENTER_BUFFER_COUNT = 6;

HRESULT GetFourCC(IMFMediaType *pType, DWORD *pFourCC);


void  S3EVRWizard::ReleaseResources(DWORD_PTR UserID)
{
    S3VR_VideoSource *pSrc = NULL;

    HRESULT hr;
    // check that dwUserID points to a known VMR
    hr = GetSourceInfo_( UserID, &pSrc );

    if(pSrc)
    {
        m_WizardObjectLock.Lock();
        pSrc->updateMutex.Lock();

        SafeRelease(&pSrc->pTexturePriv);
        m_WizardObjectLock.Unlock();
        pSrc->updateMutex.Unlock();
    }
}



HRESULT S3EVRWizard::CreateVideoSamples(IMFMediaType *pFormat, VideoSampleList& videoSampleQueue, DWORD_PTR UserID)
{
    if (pFormat == NULL)
    {
        return MF_E_UNEXPECTED;
    }

    HRESULT hr = S_OK;
    IMFSample *pVideoSample = NULL;            
    IDirect3DSurface9 *pVideoSurface = NULL;


    UINT32 width = 0, height = 0;

    hr = MFGetAttributeSize(pFormat, MF_MT_FRAME_SIZE, &width, &height);
    if (FAILED(hr))
    {
        goto done;
    }

    DWORD d3dFormat = 0;

    hr = GetFourCC(pFormat, &d3dFormat);
    if (FAILED(hr))
    {
        goto done;
    }


    IDirect3DDevice9 *pDevice;

    m_pRenderEngine->Get3DDevice(&pDevice);

    // Create the video samples.
    for (int i = 0; i < PRESENTER_BUFFER_COUNT; i++)
    {
        IDirect3DTexture9 *pTexture = NULL;

        hr = pDevice->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, (D3DFORMAT)d3dFormat, D3DPOOL_DEFAULT, &pTexture, NULL);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = pTexture->GetSurfaceLevel(0, &pVideoSurface);
        if (FAILED(hr))
        {
            goto done;
        }


        // Fill it with black.
        hr = pDevice->ColorFill(pVideoSurface, NULL, D3DCOLOR_ARGB(0xFF, 0x00, 0x00, 0x00));
        if (FAILED(hr))
        {
            goto done;
        }

        // Create the sample.
        hr = g_pfMFCreateVideoSampleFromSurface(pVideoSurface, &pVideoSample);
        if (FAILED(hr))
        {
            goto done;
        }

        // Add it to the list.
        hr = videoSampleQueue.InsertBack(pVideoSample);
        if (FAILED(hr))
        {
            goto done;
        }

        // Set the swap chain pointer as a custom attribute on the sample. This keeps
        // a reference count on the swap chain, so that the swap chain is kept alive
        // for the duration of the sample's lifetime.
        hr = pVideoSample->SetUnknown(MFSamplePresenter_OffScreen, pTexture);
        if (FAILED(hr))
        {
            goto done;
        }

        SafeRelease(&pTexture);
        SafeRelease(&pVideoSample);
        SafeRelease(&pVideoSurface);
    }

    S3VR_VideoSource *pSrc = NULL;

    // check that dwUserID points to a known VMR
    hr = GetSourceInfo_( UserID, &pSrc );

    pSrc->SetVideoSize(width, height);
done:
    SafeRelease(&pDevice);
    SafeRelease(&pVideoSurface);
    SafeRelease(&pVideoSample);
    return hr;
}

HRESULT S3EVRWizard::PresentSample(IMFSample* pSample, DWORD_PTR UserID)
{
    HRESULT hr = S_OK;

    IMFMediaBuffer* pBuffer = NULL;
    IDirect3DSurface9* pSurface = NULL;

    if (pSample)
    {
        // Get the buffer from the sample.
        hr = pSample->GetBufferByIndex(0, &pBuffer);
        if (FAILED(hr))
        {
            goto done;
        }

        // Get the surface from the buffer.
        hr = g_pfMFGetService(pBuffer, MR_BUFFER_SERVICE, IID_PPV_ARGS(&pSurface));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (pSurface)
    {
        S3VR_VideoSource *pSrc = NULL;

        // check that dwUserID points to a known EVR
        hr = GetSourceInfo_( UserID, &pSrc );



        DWORD waittimes = 0;
        // this is important to be thread safe here
        while(pSrc->updateCount)
        {
            //if previous frame is not presented by render enginge
            //need to wait
            if(pSrc->bStop)
            {
                return S_OK;
            }
            Sleep(2);
            waittimes++;
            if(waittimes >= 4)
            {
                //Rendering thread should have some problem. 
                //Can not wait too long anyway
                break;
            }
        }



        m_WizardObjectLock.Lock();
        pSrc->updateMutex.Lock();

        pSrc->updateCount++;

        try
        {
            IDirect3DTexture9 *pTexture = NULL;
            hr = pSurface->GetContainer(__uuidof(IDirect3DTexture9), (void **)&pTexture);

            SafeRelease(&pSrc->pTexturePriv);

            pSrc->pTexturePriv = pTexture;
            pSrc->pTexturePriv->AddRef();

            pSrc->bTextureInited = TRUE;
            SafeRelease(&pTexture);



        } // try
        catch( HRESULT hr1)
        {
            hr = hr1;
        }

        pSrc->updateMutex.Unlock();
        m_WizardObjectLock.Unlock();
    }
done:
    SafeRelease(&pSurface);
    SafeRelease(&pBuffer);

    return hr;
}
