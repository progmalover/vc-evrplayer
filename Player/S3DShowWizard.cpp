#include "stdafx.h"
#include "S3Signage.h"
#include "S3DShowWizard.h"
#include "S3RenderEngine.h"
#include "S3RenderMixer.h"
#include <multimon.h>
#ifdef _BUILD_FROM_VS
#include <Windows.h>
#endif



/******************************Public*Routine******************************\
* S3DShowWizard
*
* constructor
\**************************************************************************/
S3DShowWizard::S3DShowWizard()
    :m_dwConfigFlags( NULL )
    , m_pRenderEngine( NULL )
    , m_bInitialized (FALSE )
    , m_RenderThreadStatus( eNotStarted )
    , m_maxVideoFPS (30)
    , m_hwnd( NULL)
{
#if TRACE_VIDEO_PRESENT
    memset(m_PresentTimeHistory, 0, sizeof(PresentTimeInfo)*1024);
    m_CurPos = 0;
#endif
}

/******************************Public*Routine******************************\
* ~S3DShowWizard
*
* destructor
\**************************************************************************/
S3DShowWizard::~S3DShowWizard()
{
    HRESULT hr = Terminate();
#if TRACE_VIDEO_PRESENT
    FILE* pMyFile = NULL;
    char  pDstStr[46];
    char  CurrentPath[MAX_PATH+1];

    if(!GetModuleFileNameA(NULL, CurrentPath, MAX_PATH))
    {
        return;
    }
    int pathEnd, pathLen;
    pathLen = strlen(CurrentPath);
    for(pathEnd = pathLen; pathEnd >= 0; pathEnd--)
    {
        if(CurrentPath[pathEnd] == '\\')
        {
            break;
        }
    }
    pathEnd++;
    strcpy_s(CurrentPath + pathEnd, MAX_PATH - pathEnd, "PresentTime.txt");

    fopen_s(&pMyFile, CurrentPath, "w+");
    if(pMyFile)
    {
        DWORD i;
        for(i=0;i<1024;i++)
        {
            sprintf_s(pDstStr, 46, "%4d ID:%8d Begin:%8d End:%8d\n", 
                i,
                m_PresentTimeHistory[m_CurPos%1024].ID, 
                m_PresentTimeHistory[m_CurPos%1024].BTime,
                m_PresentTimeHistory[m_CurPos%1024].ETime);
            m_CurPos++;
            fwrite(pDstStr, 45, 1, pMyFile);
        }
        fclose(pMyFile);
    }
#endif
}


HRESULT S3DShowWizard::Initialize(
    DWORD dwFlags,
    HWND hWnd,
    S3RenderEngine *pRenderEngine)
{
    HRESULT hr = S_OK;
    HWND hwndRE = NULL;

    if( m_bInitialized )
    {
        ::DbgMsg(_T("S3DShowWizard::Initialize: Wizard is already initialized"));
        return VFW_E_WRONG_STATE;
    }

    if( FALSE == IsWindow( hWnd ))
    {
        ::DbgMsg(_T("S3DShowWizard::Initialize: Invalid handle to the video window"));
        return E_INVALIDARG;
    }
    if(!pRenderEngine)
    {
        ::DbgMsg(_T("S3DShowWizard::Initialize: pRenderEngine can not be NULL"));
        return E_INVALIDARG;
    }
    if(m_pRenderEngine)
    {
        ::DbgMsg(_T("S3DShowWizard::Initialize: m_pRenderEngine is not NULL!"));
        return VFW_E_WRONG_STATE;
    }
    m_hwnd = hWnd;

    // TODO: check flags
    m_dwConfigFlags = dwFlags;

    CAutoLock Lock(&m_WizardObjectLock);

    try
    {
        // initialize render engine. We assume that if custom render engine is provided,
        // it is already initialized

        // check that pRenderEngine was initialized and they point to the same window
        CHECK_HR(
            hr = pRenderEngine->GetVideoWindow( &hwndRE ),
            ::DbgMsg(_T("S3DShowWizard::Initialize: Failed to get window handler from the provided RenderEngine, hr = 0x%08x"), hr));

        CHECK_HR(
            hwndRE != m_hwnd ? E_FAIL : S_OK,
            ::DbgMsg(_T("S3DShowWizard::Initialize: specified render engine points to a different window than wizard")));

        m_pRenderEngine = pRenderEngine;
        m_pRenderEngine->SetWizardOwner(this);

        m_bInitialized = TRUE;

        CHECK_HR(
            hr = StartRenderingThread_(),
            ::DbgMsg(_T("S3DShowWizard::Initialize: failed in StartRenderingThread_, hr = 0x%08x"), hr));

    }// __try
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }
    return hr;
}

/******************************Public*Routine******************************\
* Terminate
*
* This method must be called before destroying the wizard
\**************************************************************************/
HRESULT S3DShowWizard::Terminate()
{
    HRESULT hr = S_OK;

    // check that we do not have connected subgraphs
    if( m_listVideoSources.size() > 0 )
    {
        return VFW_E_WRONG_STATE;
    }

    try
    {
        CHECK_HR(
            hr = StopRenderingThread_(),
            ::DbgMsg(_T("S3DShowWizard::Terminate: failed to stop rendering thread, hr = 0x%08x"), hr));

        // Unadvise render engine
        if( m_pRenderEngine )
        {
            CHECK_HR(
                hr = m_pRenderEngine->Terminate(),
                ::DbgMsg(_T("S3DShowWizard::Terminate: failed to terminate render engine, hr = 0x%08x"), hr));
        }
    }
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }
    return hr;
}

HRESULT S3DShowWizard::Attach(
    IBaseFilter* pVMR,
    DWORD_PTR* pdwID)
{
    HRESULT hr = S_OK;

    FILTER_INFO fiVMR;
    OAFilterState state;
    DWORD dwVMRMode = 0L;;

    S3VR_VideoSource*  pVideoSource    = NULL;
    IMediaControl*          pMediaControl   = NULL;
    IVMRFilterConfig9*      pFilterConfig   = NULL;
    IFilterGraph*           pFilterGraph    = NULL;
    IDirect3DDevice9*       pDevice         = NULL;
	IDirect3D9*             pd3d9			= NULL;

    // check that pointers are valid
    if( !pVMR || !pdwID )
    {
        ::DbgMsg(_T("S3DShowWizard::Attach received NULL pointer"));
        return E_POINTER;
    }

    if( !m_bInitialized )
    {
        ::DbgMsg(_T("S3DShowWizard::Attach: method Initialize() was not called!"));
        return VFW_E_WRONG_STATE;
    }

    try
    {
        // check that provided VMR is part of the graph
        hr = pVMR->QueryFilterInfo( &fiVMR );
        CHECK_HR(
            (NULL == fiVMR.pGraph) ? E_FAIL : S_OK,
            ::DbgMsg(_T("S3DShowWizard::Attach: provided VMR was not added to the graph")));

        pFilterGraph = fiVMR.pGraph;
        pFilterGraph->AddRef();



        pVideoSource = new S3VR_VideoSource;

        CHECK_HR(
            !pVideoSource ? E_OUTOFMEMORY : S_OK,
            ::DbgMsg(_T("S3DShowWizard::Attach: failed to allocate S3VR_VideoSource structure")));

        pVideoSource->dwID = (DWORD_PTR)pVideoSource;

        pVideoSource->pRender = pVMR;
        pVideoSource->pRender->AddRef();

        pVideoSource->pGraph = pFilterGraph;
        pVideoSource->pGraph->AddRef();


        CHECK_HR(
            pVideoSource->pGraph->QueryInterface( IID_IMediaControl,
                            (void**)&pMediaControl),
            ::DbgMsg(_T("S3DShowWizard::Attach: cannot QI IMediaControl")));

        CHECK_HR(
            hr = pMediaControl->GetState( 100, &state),
            ::DbgMsg(_T("S3DShowWizard::Attach: failed to get state of IMediaControl, hr = 0x%08x"), hr));

        CHECK_HR(
            ( state != State_Stopped ) ? VFW_E_NOT_STOPPED : S_OK,
            ::DbgMsg(_T("S3DShowWizard::Attach: graph is not stopped, state = %ld"), state));

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
    RELEASE( fiVMR.pGraph );
    RELEASE( pFilterGraph );
    RELEASE( pFilterConfig );
    RELEASE( pMediaControl );
    RELEASE( pDevice );

    return hr;
}

HRESULT S3DShowWizard::Detach(DWORD_PTR dwID)
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
        ::DbgMsg(_T("S3DShowWizard::Detach: method 'Initialize' was never called"));
        return VFW_E_WRONG_STATE;
    }

    hr = GetSourceInfo_( dwID, &pvideosource );
    if( FAILED(hr) || !pvideosource )
    {
        ::DbgMsg(_T("S3DShowWizard::Detach: Failed in GetSourceInfo_()"));
        return ( FAILED(hr) ? hr : VFW_E_NOT_FOUND );
    }

    if( !m_pRenderEngine )
    {
        ::DbgMsg(_T("S3DShowWizard::Detach: FATAL IS3VRRenderEngine pointer is NULL!"));
        return E_UNEXPECTED;
    }

    if( !pvideosource->pGraph )
    {
        ::DbgMsg(_T("S3DShowWizard::Detach: video source info does not contain pointer to IFilterGraph!"));
        return VFW_E_NOT_FOUND;
    }

    try
    {
        CHECK_HR(
            hr = (pvideosource->pGraph)->QueryInterface(
                                    IID_IMediaControl, (void**)&pMc ),
            ::DbgMsg(_T("S3DShowWizard::Detach: cannot QI IMediaControl of the graph, hr = 0x%08x"), hr));

        CHECK_HR(
            hr = pMc->GetState( 100, &state ),
            ::DbgMsg(_T("S3DShowWizard::Detach: cannot obtain state from IMediaControl, hr = 0x%08x"), hr));

        CHECK_HR(
            ( State_Stopped != state ) ? VFW_E_NOT_STOPPED : S_OK,
            ::DbgMsg(_T("S3DShowWizard::Detach: correspondent graph was not stopped")));


        CHECK_HR(
            hr = m_pRenderEngine->GetMixerControl( &pMixerControl ),
            ::DbgMsg(_T("S3DShowWizard::Detach: FATAL, cannot find currently active IS3VRMixerControl!")));

        // we have to be thread safe only here when we actually mess up with shared data
        CAutoLock Lock(&m_WizardObjectLock);

        //don't need to call this, the graph will call it when stop is called
        //CHECK_HR(
        //    hr = StopPresenting( pvideosource->dwID ),
        //    ::DbgMsg("S3DShowWizard::Detach: failed in StopPresenting(), hr = 0x%08x", hr));

        CHECK_HR(
            pvideosource->DisconnectPins(),
            ::DbgMsg(_T("S3DShowWizard::Detach: FATAL, failed to disconnect pins of VMR")));


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
                ::DbgMsg(_T("S3DShowWizard::Detach: FATAL, m_listVideoSources contains NULL pointer")));

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
            ::DbgMsg(_T("S3DShowWizard::Detach: FATAL, failed to delete source from the list (source was not found)")));
    }// try
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }

    RELEASE( pMc );

    return hr;
}

HRESULT S3DShowWizard::BeginDeviceLoss( void )
{
	HRESULT hr = S_OK;

    CAutoLock Lock(&m_WizardObjectLock);

	// go through all the connected sources and reset the device
	list<S3VR_VideoSource*>::iterator start, end, it;
	start = m_listVideoSources.begin();
	end = m_listVideoSources.end();
	for( it=start; it!=end; it++)
	{
		S3VR_VideoSource* pSource = (S3VR_VideoSource*)(*it);
		if( pSource )
		{
			pSource->DeleteSurfaces();
			RELEASE( pSource->pTexturePriv );
            RELEASE( pSource->pTempSurfacePriv );
		}
	}// for
	return hr;
}

HRESULT S3DShowWizard::EndDeviceLoss( IDirect3DDevice9* pDevice )
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

	}				
	RELEASE( pd3d9 );
	return hr;
}

HRESULT S3DShowWizard::VerifyID( DWORD_PTR dwID )
{
    HRESULT hr = S_OK;
    S3VR_VideoSource *pSrc = NULL;

    hr = this->GetSourceInfo_( dwID, &pSrc);

    if( SUCCEEDED( hr ) && pSrc )
    {
        return S_OK;
    }
    else
    {
        return VFW_E_NOT_FOUND;
    }
}

HRESULT S3DShowWizard::GetGraph(DWORD_PTR dwID, IFilterGraph** ppGraph)
{
    HRESULT hr = S_OK;
    S3VR_VideoSource *pSrc = NULL;

    if( !ppGraph )
    {
        ::DbgMsg(_T("S3DShowWizard::GetGraph: second argument is NULL"));
        return E_POINTER;
    }

    if( !m_bInitialized )
    {
        ::DbgMsg(_T("S3DShowWizard::GetGraph: Method 'Initialize' was never called"));
        return VFW_E_WRONG_STATE;
    }

    hr = this->GetSourceInfo_( dwID, &pSrc);

    if( SUCCEEDED(hr))
    {
        if( pSrc->pGraph )
        {
            *ppGraph = pSrc->pGraph;
            (*ppGraph)->AddRef();
            hr = S_OK;
        }
        else
        {
            ::DbgMsg(_T("S3DShowWizard::GetGraph: FATAL: member of the list of video sources contain NULL IFilterGraph pointer"));
            hr = VFW_E_NOT_FOUND;
        }
    }
    else
    {
        ::DbgMsg(_T("S3DShowWizard::GetGraph: Failed in GetSourceInfo_(), hr = 0x%08x"), hr);
    }
    return hr;
}

HRESULT S3DShowWizard::GetRenderEngine(S3RenderEngine** ppRenderEngine)
{
    HRESULT hr = S_OK;

    if( !ppRenderEngine )
    {
        ::DbgMsg(_T("S3DShowWizard::GetRenderEngine: first argument is NULL"));
        return E_POINTER;
    }

    if( !m_bInitialized )
    {
        ::DbgMsg(_T("S3DShowWizard::GetRenderEngine: Method 'Initialize' was never called"));
        return VFW_E_WRONG_STATE;
    }

    *ppRenderEngine = m_pRenderEngine;
    return S_OK;
}

HRESULT S3DShowWizard::GetMixerControl(S3RenderMixer** ppMixerControl)
{
    HRESULT hr = S_OK;
    if( !ppMixerControl )
    {
        ::DbgMsg(_T("S3DShowWizard::GetMixerControl: received NULL pointer"));
        return E_POINTER;
    }
    if( !m_bInitialized )
    {
        ::DbgMsg(_T("S3DShowWizard::GetMixerControl: Method 'Initialize' was never called"));
        return VFW_E_WRONG_STATE;
    }
    if( !m_pRenderEngine )
    {
        ::DbgMsg(_T("S3DShowWizard::GetMixerControl: FATAL, cannot find IS3VRRenderEngine"));
        return E_UNEXPECTED;
    }

    hr = m_pRenderEngine->GetMixerControl( ppMixerControl );

    return hr;
}

HRESULT S3DShowWizard::GetTexture(
        DWORD_PTR dwID,
        LPDIRECT3DTEXTURE9* ppTexture
        )
{
    HRESULT hr = E_FAIL;
    S3VR_VideoSource *pSrc = NULL;

    if( !ppTexture )
    {
        ::DbgMsg(_T("S3DShowWizard::GetRenderEngine: second argument is NULL"));
        return E_POINTER;
    }

    if( !m_bInitialized )
    {
        ::DbgMsg(_T("S3DShowWizard::GetRenderEngine: method 'Initialize' was never called"));
        return VFW_E_WRONG_STATE;
    }

    hr = GetSourceInfo_( dwID, &pSrc );
    if( FAILED(hr))
    {
        ::DbgMsg(_T("S3DShowWizard::GetRenderEngine: failed in GetSourceInfo_()  (video source was not found)"));
        return VFW_E_NOT_FOUND;
    }

    pSrc->updateMutex.Lock();
    if(!pSrc->bTextureInited)
    {
        *ppTexture = NULL;
        return S_FALSE;
    }

    *ppTexture = pSrc->pTexturePriv;

    return S_OK;
}

HRESULT S3DShowWizard::ReleaseTexture(
        DWORD_PTR dwID
        )
{
    HRESULT hr = S_OK;
    S3VR_VideoSource *pSrc = NULL;

    try
    {
        if( !m_bInitialized )
        {
            ::DbgMsg(_T("S3DShowWizard::GetRenderEngine: method 'Initialize' was never called"));
            hr = VFW_E_WRONG_STATE;
            throw hr;
        }

        hr = GetSourceInfo_( dwID, &pSrc );
        if( FAILED(hr))
        {
            ::DbgMsg(_T("S3DShowWizard::GetRenderEngine: failed in GetSourceInfo_()  (video source was not found)"));
            hr = VFW_E_NOT_FOUND;
            throw hr;
        }

        pSrc->updateCount = 0;
        pSrc->updateMutex.Unlock();
    }
    catch(HRESULT hr1)
    {
		return hr1;
    }

    if(!pSrc->bTextureInited)
    {
        hr = S_FALSE;
    }
    return hr;
}

HRESULT S3DShowWizard::StopPresenting(
        DWORD_PTR dwID,
        BOOL bStop
        )
{
    HRESULT hr = S_OK;
    S3VR_VideoSource *pSrc = NULL;

    try
    {
        if( !m_bInitialized )
        {
            ::DbgMsg(_T("S3DShowWizard::GetRenderEngine: method 'Initialize' was never called"));
            hr = VFW_E_WRONG_STATE;
            throw 1;
        }

        hr = GetSourceInfo_( dwID, &pSrc );
        if( FAILED(hr))
        {
            ::DbgMsg(_T("S3DShowWizard::GetRenderEngine: failed in GetSourceInfo_()  (video source was not found)"));
            hr = VFW_E_NOT_FOUND;
            throw 2;
        }

        pSrc->bStop = bStop;
    }
    catch(int)
    {
    }
    return hr;
}

HRESULT S3DShowWizard::GetVideoSize(
        DWORD_PTR dwID,
        LONG* plWidth,
        LONG* plHeight,
        LONG* pPreferedx,
        LONG* pPreferedy
        )
{
    HRESULT hr = S_OK;
    S3VR_VideoSource *pSrc = NULL;

    if( !plWidth )
    {
        ::DbgMsg(_T("S3DShowWizard::GetVideoSize: received NULL for plWidth"));
        return E_POINTER;
    }
    if( !plHeight )
    {
        ::DbgMsg(_T("S3DShowWizard::GetVideoSize: received NULL for plHeight"));
        return E_POINTER;
    }

    hr = GetSourceInfo_( dwID, &pSrc );
    if( FAILED(hr))
    {
        ::DbgMsg(_T("S3DShowWizard::GetVideoSize: failed in GetSourceInfo_()  (video source was not found)"));
        *plWidth = 0L;
        *plHeight = 0L;
        return VFW_E_NOT_FOUND;
    }

    *plWidth = pSrc->lImageWidth;
    *plHeight = pSrc->lImageHeight;
    *pPreferedx = pSrc->lPreferedx;
    *pPreferedy = pSrc->lPreferedy;
    return S_OK;
}

HRESULT S3DShowWizard::EnableSFRUpload(DWORD_PTR dwID, BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree)
{
    return E_NOTIMPL;
}


/////////////////////// Private class CS3VR_VideoSource ///////////////////////////////////////

/******************************Private*Routine******************************\
* S3VR_VideoSource
*
* constructor
\**************************************************************************/
S3DShowWizard::S3VR_VideoSource::S3VR_VideoSource()
            : dwTag( VIDEO_SOURCE_TAG )
            , dwID( 0L )
            , dwNumBuf( 0L)
            , dwNumBufActuallyAllocated( 0L)
            , lImageWidth( 0L)
            , lImageHeight( 0L)
            , pTexturePriv( NULL )
            , pTempSurfacePriv( NULL )
            , ppSurface( NULL)
            , pGraph( NULL )
            , pVideoAllocator( NULL )
            , pRender (NULL)
            , bTextureInited(FALSE)
            , updateCount(0)
            , bStop(FALSE)
            , dwLast(0)
            , lPreferedx(0)
            , lPreferedy(0)
            , m_UserNV12RT(FALSE)
            , ppSurfaceMem(NULL)
{
}

/******************************Private*Routine******************************\
* ~S3VR_VideoSource
*
* destructor
\**************************************************************************/
S3DShowWizard::S3VR_VideoSource::~S3VR_VideoSource()
{
    dwID = NULL;
    dwTag = NULL;

    if( pTexturePriv   ) { pTexturePriv->Release();     pTexturePriv = NULL; }
    if( pGraph         ) { pGraph->Release();           pGraph = NULL;   }
    if( pVideoAllocator   ) { pVideoAllocator->Release();     pVideoAllocator = NULL;}
    if( pRender           ) { pRender->Release();             pRender = NULL; }

    DeleteSurfaces();
    lImageWidth = lImageHeight = 0L;
    lPreferedx = lPreferedy = 0L;
}

/******************************Private*Routine******************************\
* DisconnectPins
*
* Disconnects pins of VMR
\**************************************************************************/
HRESULT S3DShowWizard::S3VR_VideoSource::DisconnectPins()
{
    HRESULT hr = S_OK;
    if( !pRender )
    {
        return E_POINTER;
    }
    IEnumPins*  pEnum = NULL;
    IPin*       pPin = NULL;

    try
    {
        CHECK_HR(
            hr = pRender->EnumPins( &pEnum ),
            ::DbgMsg(_T("S3VR_VideoSource::DisconnectPins: failed to enumerate pins, hr = 0x%08x"), hr));

        hr = pEnum->Next(1, &pPin, NULL);
        while( S_OK == hr && pPin)
        {
            CHECK_HR(
                hr = pPin->Disconnect(),
                ::DbgMsg(_T("S3VR_VideoSource::DisconnectPins: failed to disconnect pin, hr = 0x%08x"), hr));

            RELEASE(pPin);
            hr = pEnum->Next(1, &pPin, NULL);
        }
    }// try
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }

    RELEASE(pPin);
    RELEASE(pEnum);

    return hr;
}

/******************************Private*Routine******************************\
* DeleteSurfaces
*
* deletes allocated surface buffers
\**************************************************************************/
void S3DShowWizard::S3VR_VideoSource::DeleteSurfaces()
{
    if( ppSurface )
    {
        for( DWORD dwS = 0; dwS<dwNumBuf; dwS++)
        {
            if( ppSurface[dwS] )
            {
                (ppSurface[dwS])->Release();
                ppSurface[dwS] = NULL;
            }
        }
        delete[] ppSurface;
        ppSurface = NULL;
    }

    if( ppSurfaceMem )
    {
        for( DWORD dwS = 0; dwS<dwNumBuf; dwS++)
        {
            if( ppSurfaceMem[dwS] )
            {
                free(ppSurfaceMem[dwS]);
                ppSurfaceMem[dwS] = NULL;
            }
        }
        delete[] ppSurfaceMem;
        ppSurfaceMem = NULL;
    }


    RELEASE(pTempSurfacePriv);
    dwNumBuf = 0L;
    dwNumBufActuallyAllocated = 0L;
}

/******************************Private*Routine******************************\
* AllocateSurfaceBuffer
*
* allocates buffer of dwN surfaces
\**************************************************************************/
HRESULT S3DShowWizard::S3VR_VideoSource::AllocateSurfaceBuffer(
                                                        DWORD dwN )
{
    if( dwN < 1)
    {
        return E_INVALIDARG;
    }

    DeleteSurfaces();
    dwNumBuf = dwN;
    ppSurface = new IDirect3DSurface9*[dwNumBuf];
    ppSurfaceMem = new VOID*[dwNumBuf];

    if( !ppSurface )
    {
        dwNumBuf = 0L;
        return E_OUTOFMEMORY;
    }

    ZeroMemory( ppSurface, dwNumBuf * sizeof(IDirect3DSurface9*));
    ZeroMemory( ppSurfaceMem, dwNumBuf * sizeof(VOID*));
    return S_OK;
}

/******************************Private*Routine******************************\
* SetVideoSize
*
* saves data on the video source image size
\**************************************************************************/
HRESULT S3DShowWizard::S3VR_VideoSource::
                                SetVideoSize(   LONG lImageW,
                                                LONG lImageH )
{
    if( lImageW < 1 ||
        lImageH < 1 )
    {
        ::DbgMsg(_T("S3VR_VideoSource::SetVideoSize: received invalid sizes: image width = %ld, image height = %ld"),
            lImageW, lImageH);
        return E_INVALIDARG;
    }

    lImageWidth = lImageW;
    lImageHeight = lImageH;
    lPreferedx = lImageW;
    lPreferedy = lImageH;
    return S_OK;
}

/////////////////////// Private routine ///////////////////////////////////////

/******************************Private*Routine******************************\
* Clean_
*
* clean all the data, release all interfaces
\**************************************************************************/
void S3DShowWizard::Clean_()
{
    HRESULT hr = S_OK;
    DWORD_PTR dwID = 0L;

    list<S3VR_VideoSource*>::iterator it;

    while( false == m_listVideoSources.empty())
    {
        it = m_listVideoSources.begin();
        if( (S3VR_VideoSource*)(*it) )
        {
            dwID = ((S3VR_VideoSource*)(*it))->dwID;
            hr = Detach( dwID );
            DbgMsg(_T("Clean_: detaching %ld, return code 0x%08x"), dwID, hr);
        }
    }
}

/******************************Private*Routine******************************\
* GetSourceInfo_
*
* returns S3VR_VideoSource associated with dwID
*
* dwID -- sub-graph's cookie assigned in S3DShowWizard::Attach
* source -- [out] reference to S3VR_VideoSource to fill
*
* Return error codes: VFW_E_NOT_FOUND -- source was not found
\**************************************************************************/
HRESULT S3DShowWizard::GetSourceInfo_(DWORD_PTR dwID, S3VR_VideoSource** ppsource)
{
    HRESULT hr = S_OK;
    S3VR_VideoSource *pSrc = NULL;

    if( !dwID )
    {
        return VFW_E_NOT_FOUND;
    }

    pSrc = reinterpret_cast<S3VR_VideoSource*>(dwID);
    if( !pSrc )
    {
        return VFW_E_NOT_FOUND;
    }
    // check that Tag is VIDEO_SOURCE_TAG and we do not have pSrc filled with trash
    if( VIDEO_SOURCE_TAG != pSrc->dwTag )
    {
        return VFW_E_NOT_FOUND;
    }

    // ok, pSrc is a valid S3VR_VideoSource; copy it to source
    *ppsource = pSrc;
    return S_OK;
}

/******************************Public*Routine******************************\
* RenderThreadProc_
*
* spins off rendering thread
*
\**************************************************************************/
HRESULT S3DShowWizard::StartRenderingThread_()
{
    HANDLE hThread = NULL;
    DWORD tid = NULL;

    if( !m_bInitialized || m_RenderThreadStatus != eNotStarted )
    {
        ::DbgMsg(_T("S3DShowWizard::StartRenderingThread_: function called when wizard is not initialized or render thread is already running / closed"));
        return VFW_E_WRONG_STATE;
    }

    m_RenderThreadStatus = eRunning;
    // since we initialized successfully, spin off rendering thread
    hThread = CreateThread( NULL,
                            NULL,
                            RenderThreadProc_,
                            this,
                            NULL,
                            &tid);
    if( INVALID_HANDLE_VALUE == hThread )
    {
        ::DbgMsg(_T("S3DShowWizard::Initialize: failed to create rendering thread"));
        m_RenderThreadStatus = eNotStarted;
        return E_UNEXPECTED;
    }
    CloseHandle(hThread);

    return S_OK;
}

/******************************Public*Routine******************************\
* RenderThreadProc_
*
* fires the end of the rendering thread and waits untils render thread closes
*
\**************************************************************************/
HRESULT S3DShowWizard::StopRenderingThread_()
{
    {
        CAutoLock threadLock(&m_threadMutex);
        if( m_RenderThreadStatus != eRunning )
        {
            return S_FALSE;
        }

        m_RenderThreadStatus = eWaitingToStop;
    }
    while( m_RenderThreadStatus != eFinished )
    {
        Sleep(50);
    }
    return S_OK;
}

/******************************Public*Routine******************************\
* RenderThreadProc_
*
* ThreadProc processing rendering of the Render Engine: calls for Render
*
\**************************************************************************/
DWORD WINAPI S3DShowWizard::RenderThreadProc_( LPVOID lpParameter )
{
    S3DShowWizard* This = NULL;
    S3RenderEngine* pRenderEngine = NULL;

    HRESULT hr = S_OK;

    This = (S3DShowWizard*)lpParameter;

    {
        CAutoLock threadLock(&This->m_threadMutex);
        if( !This )
        {
            ::DbgMsg(_T("CS3VRRenderEngine::RenderThreadProc: parameter is NULL"));
            This->m_RenderThreadStatus = eNotStarted;
            return 0;
        }

        hr = This->GetRenderEngine( &pRenderEngine );
        if( FAILED(hr) || !pRenderEngine )
        {
            ::DbgMsg(_T("CS3VRRenderEngine::RenderThreadProc: cannot find IS3VRRenderEngine"));
            This->m_RenderThreadStatus = eNotStarted;
            return 0;
        }
    }

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    while ( eWaitingToStop != This->m_RenderThreadStatus )
    {
        hr = pRenderEngine->Render();
    } // while true

    This->m_RenderThreadStatus = eFinished;

    return 0;
}