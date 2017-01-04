//------------------------------------------------------------------------------
// File: S3VMRWizard.cpp
//
#include "stdafx.h"
#include "S3VMRWizard.h"
#include "S3RenderEngine.h"
#include "S3RenderMixer.h"
#include <multimon.h>
#ifdef _BUILD_FROM_VS
#include <Windows.h>
#endif
#include "MAMMD3D9.h"
#include "MAMM3DTexture9.h"
#include "S3Signage.h"
using namespace std;


/******************************Public*Routine******************************\
* S3VMRWizard
*
* constructor
\**************************************************************************/
S3VMRWizard::S3VMRWizard()
    :  CUnknown(NAME("S3VMR Wizard"), NULL), S3DShowWizard()

{
}

/******************************Public*Routine******************************\
* ~S3VMRWizard
*
* destructor
\**************************************************************************/
S3VMRWizard::~S3VMRWizard()
{
}

/******************************Public*Routine******************************\
* NonDelegatingQueryInterface
\**************************************************************************/
STDMETHODIMP
S3VMRWizard::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (riid == IID_IVMRSurfaceAllocator9)
    {
        hr =  GetInterface((IVMRSurfaceAllocator9 *)this, ppv);
    }
    else if (riid == IID_IVMRImagePresenter9)
    {
        hr = GetInterface((IVMRImagePresenter9 *)this, ppv );
    }
    else
    {
        hr = CUnknown::NonDelegatingQueryInterface(riid,ppv);
    }
    return hr;
}





HRESULT S3VMRWizard::Attach(
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
        ::DbgMsg(_T("S3VMRWizard::Attach received NULL pointer"));
        return E_POINTER;
    }

    if( !m_bInitialized )
    {
        ::DbgMsg(_T("S3VMRWizard::Attach: method Initialize() was not called!"));
        return VFW_E_WRONG_STATE;
    }

    try
    {
        // check that provided VMR is part of the graph
        hr = pVMR->QueryFilterInfo( &fiVMR );
        CHECK_HR(
            (NULL == fiVMR.pGraph) ? E_FAIL : S_OK,
            ::DbgMsg(_T("S3VMRWizard::Attach: provided VMR was not added to the graph")));

        pFilterGraph = fiVMR.pGraph;
        pFilterGraph->AddRef();

        // check that provided VMR is in renderless mode
        hr = pVMR->QueryInterface(  IID_IVMRFilterConfig9,
                                    (void**)&pFilterConfig );
        CHECK_HR(
            FAILED(hr) ? hr : ( !pFilterConfig ? E_FAIL : S_OK ),
            ::DbgMsg(_T("S3VMRWizard::Attach: failed to QI IVMRFilterConfig9, hr = 0x%08x"), hr));

        CHECK_HR(
            hr = pFilterConfig->GetRenderingMode( &dwVMRMode ),
            ::DbgMsg(_T("S3VMRWizard::Attach: failed to get rendering mode, hr = 0x%08x"), hr));

        CHECK_HR(
            (VMRMode_Renderless != dwVMRMode) ? VFW_E_WRONG_STATE : S_OK,
            ::DbgMsg(_T("S3VMRWizard::Attach: provided VMR9 is not in renderless mode")));

        pVideoSource = new S3VR_VideoSource;

        CHECK_HR(
            !pVideoSource ? E_OUTOFMEMORY : S_OK,
            ::DbgMsg(_T("S3VMRWizard::Attach: failed to allocate S3VR_VideoSource structure")));

        pVideoSource->dwID = (DWORD_PTR)pVideoSource;

        pVideoSource->pRender = pVMR;
        pVideoSource->pRender->AddRef();

        pVideoSource->pGraph = pFilterGraph;
        pVideoSource->pGraph->AddRef();

        // check that provided pVMR exposes IVMRSurfaceAllocatorNotify9 interfaces
        CHECK_HR(
            pVMR->QueryInterface( IID_IVMRSurfaceAllocatorNotify9,
                            (void**)&pVideoSource->pVideoAllocator),
            ::DbgMsg(_T("S3VMRWizard::Attach: cannot QI IVMRSurfaceAllocatorNotify9")));

        CHECK_HR(
            pVideoSource->pGraph->QueryInterface( IID_IMediaControl,
                            (void**)&pMediaControl),
            ::DbgMsg(_T("S3VMRWizard::Attach: cannot QI IMediaControl")));

        CHECK_HR(
            hr = pMediaControl->GetState( 100, &state),
            ::DbgMsg(_T("S3VMRWizard::Attach: failed to get state of IMediaControl, hr = 0x%08x"), hr));

        CHECK_HR(
            ( state != State_Stopped ) ? VFW_E_NOT_STOPPED : S_OK,
            ::DbgMsg(_T("S3VMRWizard::Attach: graph is not stopped, state = %ld"), state));

        // we have to be thread safe only here when we actually mess up with shared data
        CAutoLock Lock(&m_WizardObjectLock);

        // set device
        CHECK_HR(
            hr = m_pRenderEngine->Get3DDevice( &pDevice ),
            ::DbgMsg(_T("S3VMRWizard::Attach: failed to obtain Direct3D device from the render engine, hr = 0x%08x"), hr));

        CHECK_HR(
            hr = pDevice->GetDirect3D( &pd3d9 ),
            ::DbgMsg(_T("S3VMRWizard::Attach: failed to retrieve IDirect3D9")));

        HMONITOR hMonitor = pd3d9->GetAdapterMonitor( D3DADAPTER_DEFAULT );

        CHECK_HR(
            hr = ((IVMRSurfaceAllocatorNotify9*)(pVideoSource->pVideoAllocator))->SetD3DDevice(
                    pDevice,
                    hMonitor),
            ::DbgMsg(_T("S3VMRWizard::Attach: failed in SetD3DDevice() of IVMRSurfaceAllocatorNotify, hr = 0x%08x"), hr));


        // try to advise 'this' custom allocator-presenter to the VMR
        CHECK_HR(
            hr = ((IVMRSurfaceAllocatorNotify9*)pVideoSource->pVideoAllocator)->AdviseSurfaceAllocator(
                                            pVideoSource->dwID,
                                            (IVMRSurfaceAllocator9*)this),
            ::DbgMsg(_T("S3VMRWizard::Attach: failed to advise A/P, hr = 0x%08x"), hr));

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

HRESULT S3VMRWizard::Detach(DWORD_PTR dwID)
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
        ::DbgMsg(_T("S3VMRWizard::Detach: method 'Initialize' was never called"));
        return VFW_E_WRONG_STATE;
    }

    hr = GetSourceInfo_( dwID, &pvideosource );
    if( FAILED(hr) || !pvideosource )
    {
        ::DbgMsg(_T("S3VMRWizard::Detach: Failed in GetSourceInfo_()"));
        return ( FAILED(hr) ? hr : VFW_E_NOT_FOUND );
    }

    if( !m_pRenderEngine )
    {
        ::DbgMsg(_T("S3VMRWizard::Detach: FATAL IS3VRRenderEngine pointer is NULL!"));
        return E_UNEXPECTED;
    }

    if( !pvideosource->pGraph )
    {
        ::DbgMsg(_T("S3VMRWizard::Detach: video source info does not contain pointer to IFilterGraph!"));
        return VFW_E_NOT_FOUND;
    }

    try
    {
        CHECK_HR(
            hr = (pvideosource->pGraph)->QueryInterface(
                                    IID_IMediaControl, (void**)&pMc ),
            ::DbgMsg(_T("S3VMRWizard::Detach: cannot QI IMediaControl of the graph, hr = 0x%08x"), hr));

        CHECK_HR(
            hr = pMc->GetState( 100, &state ),
            ::DbgMsg(_T("S3VMRWizard::Detach: cannot obtain state from IMediaControl, hr = 0x%08x"), hr));

        CHECK_HR(
            ( State_Stopped != state ) ? VFW_E_NOT_STOPPED : S_OK,
            ::DbgMsg(_T("S3VMRWizard::Detach: correspondent graph was not stopped")));

        // advise NULL as A/P to VMR9 (this will return VMR9 to its default A/P)
        CHECK_HR(
            ( !(pvideosource->pVideoAllocator)) ? VFW_E_NOT_FOUND : S_OK,
            ::DbgMsg(_T("S3VMRWizard::Detach: video source info does not contain pointer to IVMRSurfaceAllocatorNotify9")));

        CHECK_HR(
            hr = m_pRenderEngine->GetMixerControl( &pMixerControl ),
            ::DbgMsg(_T("S3VMRWizard::Detach: FATAL, cannot find currently active IS3VRMixerControl!")));

        // we have to be thread safe only here when we actually mess up with shared data
        CAutoLock Lock(&m_WizardObjectLock);

        //don't need to call this, the graph will call it when stop is called
        //CHECK_HR(
        //    hr = StopPresenting( pvideosource->dwID ),
        //    ::DbgMsg("S3VMRWizard::Detach: failed in StopPresenting(), hr = 0x%08x", hr));

        CHECK_HR(
            pvideosource->DisconnectPins(),
            ::DbgMsg(_T("S3VMRWizard::Detach: FATAL, failed to disconnect pins of VMR")));

        CHECK_HR(
            hr = ((IVMRSurfaceAllocatorNotify9*)pvideosource->pVideoAllocator)->AdviseSurfaceAllocator(
                                                        dwID, NULL),
            ::DbgMsg(_T("S3VMRWizard::Detach: failed to unadvise surface allocator, hr = 0x%08x"), hr));

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
                ::DbgMsg(_T("S3VMRWizard::Detach: FATAL, m_listVideoSources contains NULL pointer")));

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
            ::DbgMsg(_T("S3VMRWizard::Detach: FATAL, failed to delete source from the list (source was not found)")));
    }// try
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }

    RELEASE( pMc );

    return hr;
}



HRESULT S3VMRWizard::EndDeviceLoss( IDirect3DDevice9* pDevice )
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
		if( pSource && pSource->pVideoAllocator )
		{
			hr = ((IVMRSurfaceAllocatorNotify9*)pSource->pVideoAllocator)->ChangeD3DDevice( pDevice, hMon );
		}
	}				
	RELEASE( pd3d9 );
	return hr;
}


static int GetFormatBpp(D3DFORMAT Format)
{
    switch(Format)
    {
    case D3DFMT_R8G8B8: return 24;
        break;
    case D3DFMT_A8R8G8B8: return 32;
        break;
    case D3DFMT_X8R8G8B8: return 32;
        break;

    case D3DFMT_R5G6B5: return 16;
        break;
    case D3DFMT_X1R5G5B5: return 16;
        break;
    case D3DFMT_A1R5G5B5: return 16;
        break;
    case D3DFMT_A4R4G4B4: return 16;
        break;
    case D3DFMT_R3G3B2: return 8;
        break; 
    case D3DFMT_A8:     return 8;
        break;
    case D3DFMT_A8R3G3B2: return 16;
        break;
    case D3DFMT_X4R4G4B4: return 16;
        break;
    case D3DFMT_A2B10G10R10: return 32;
        break;
    case D3DFMT_A8B8G8R8: return 32;
        break;
    case D3DFMT_X8B8G8R8: return 32;
        break;
    case D3DFMT_G16R16: return 32;
        break;
    case D3DFMT_A2R10G10B10: return 32;
        break;
    case D3DFMT_A16B16G16R16: return 32;
        break;
    case D3DFMT_A8P8: return 16;
        break;
    case D3DFMT_P8: return 8;
        break;
    case D3DFMT_L8: return 8;
        break;
    case D3DFMT_A8L8: return 16;
        break;
    case D3DFMT_A4L4: return 8;
        break;
    case D3DFMT_V8U8: return 16;
        break;
    case D3DFMT_L6V5U5: return 16;
        break;
    case D3DFMT_X8L8V8U8: return 32;
        break;
    case D3DFMT_Q8W8V8U8: return 32;
        break;
    case D3DFMT_V16U16: return 32;
        break;
    case D3DFMT_A2W10V10U10: return 32;
        break;
    case D3DFMT_UYVY: return 16;
        break;
    case D3DFMT_R8G8_B8G8: return 16;
        break;
    case D3DFMT_YUY2: return 16;
        break;
    case D3DFMT_G8R8_G8B8: return 16;
        break;
    case D3DFMT_DXT1: return 8;
        break;
    case D3DFMT_DXT2: return 8;
        break;
    case D3DFMT_DXT3: return 16;
        break;
    case D3DFMT_DXT4: return 16;
        break;
    case D3DFMT_DXT5: return 16;
        break;
    case D3DFMT_D16_LOCKABLE: return 16;
        break;
    case D3DFMT_D32: return 32;
        break;
    case D3DFMT_D15S1: return 16;
        break;
    case D3DFMT_D24S8: return 32;
        break;
    case D3DFMT_D24X8: return 32;
        break;
    case D3DFMT_D24X4S4: return 32;
        break;
    case D3DFMT_D16: return 16;
        break;
    case D3DFMT_D32F_LOCKABLE: return 32;
        break;
    case D3DFMT_D24FS8: return 32;
        break;
    case D3DFMT_D32_LOCKABLE: return 32;
        break;
    case D3DFMT_S8_LOCKABLE: return 8;
        break;
    case D3DFMT_L16: return 16;
        break;
    case D3DFMT_VERTEXDATA: return 8;
        break;
    case D3DFMT_INDEX16: return 16;
        break;
    case D3DFMT_INDEX32: return 32;
        break;
    case D3DFMT_Q16W16V16U16: return 64;
        break;
    case D3DFMT_MULTI2_ARGB8: return 32;
        break;
    case D3DFMT_R16F: return 16;
        break;
    case D3DFMT_G16R16F: return 32;
        break;
    case D3DFMT_A16B16G16R16F: return 64;
        break;
    case D3DFMT_R32F: return 32;
        break;
    case D3DFMT_G32R32F: return 64;
        break;
    case D3DFMT_A32B32G32R32F: return 128;
        break;
    case D3DFMT_CxV8U8: return 16;
        break;
    case D3DFMT_A1: return 1;
        break;
    //case D3DFMT_A2B10G10R10_XR_BIAS: return 32;
        //break;
    case D3DFMT_BINARYBUFFER: return 8;
        break;
    case '21VN': return 16;
        break;
    }
    return 32;
}

static DWORD CalcMipLevels(UINT Width, UINT Height)
{
    DWORD WBit = 0;
    _BitScanReverse(&WBit, Width);
    DWORD HBit = 0;
    _BitScanReverse(&HBit, Height);

    return max(WBit, HBit) + 1;
}

static BOOL bIsBCFormat(D3DFORMAT Format)
{
    return (Format == D3DFMT_DXT1) || (Format == D3DFMT_DXT2) || (Format == D3DFMT_DXT3) || (Format == D3DFMT_DXT4) || (Format == D3DFMT_DXT5);

}


static DWORD CalcMipSize(D3DFORMAT Format, UINT Width, UINT Height, UINT CurrentLevel, UINT MaxLevel)
{
    DWORD MemorySize = 0;

    if((Width >0 || Height>0) && (CurrentLevel < MaxLevel))
    {
        MemorySize = GetFormatBpp(Format) * Width * Height / 8;

        if(bIsBCFormat(Format))
        {
            MemorySize = GetFormatBpp(Format) * ((Width + 0xf ) & ~0xf) * ((Height + 0xf ) & ~0xf) / 8;
            
        }

        MemorySize = (MemorySize + 0x1000 -1 ) & ~(0x1000 -1);

        MemorySize += CalcMipSize(Format, max(1, Width/2), max(1, Height/2), CurrentLevel + 1, MaxLevel);
    }

    return MemorySize;
}


static DWORD CalcMipSize(D3DFORMAT Format, UINT Width, UINT Height, UINT MaxLevel)
{
    return CalcMipSize(Format, Width, Height, 0, MaxLevel);
}

static DWORD CalcMipOffset(D3DFORMAT Format, UINT Width, UINT Height, UINT MaxLevel)
{
    return CalcMipSize(Format, Width, Height, 0, MaxLevel);
}


STDMETHODIMP S3VMRWizard::AdviseNotify(IVMRSurfaceAllocatorNotify9*  lpIVMRSurfAllocNotify)
{
    return S_OK;
}

STDMETHODIMP S3VMRWizard::InitializeDevice(
        DWORD_PTR  dwUserID,
        VMR9AllocationInfo*  lpAllocInfo,
        DWORD*  lpNumBuffers
        )
{
    HRESULT hr = S_OK;
    D3DFORMAT format;
    D3DSURFACE_DESC ddsd;


    S3VR_VideoSource*       pSrc    = NULL;
    IDirect3DDevice9*       pDevice = NULL;
    IDirect3DSurface9*      pS      = NULL;

    if(lpAllocInfo && lpAllocInfo->Format == '21VY')
    {
        return E_FAIL;
    }

    // first, make sure we got a call from a known VMR
    hr = GetSourceInfo_( dwUserID, &pSrc );
    if( FAILED(hr) || !pSrc )
    {
        ::DbgMsg(_T("S3VMRWizard::InitializeDevice: cannot get info on the calling VMR, dwUserID = 0x%08x, hr = 0x%08x, pSource = 0x%08x"), dwUserID, hr, (void*)pSrc);
        return ( FAILED(hr)? hr : E_FAIL );
    }
    // check we are provided valid parameters
    if( !lpAllocInfo || !lpNumBuffers )
    {
        return E_POINTER;
    }

    if( *lpNumBuffers <1 )
    {
        *lpNumBuffers = 1;
    }
    // check we know about the default IVMRSurfaceAllocatorNotify9
    if(!(pSrc->pVideoAllocator) )
    {
        ::DbgMsg(_T("S3VMRWizard::InitializeDevice: FATAL: video source contains NULL pointer to IVMRSurfaceAllocatorNotify9"));
        return E_FAIL;
    }

    try
    {
        // obtain device from the render engine
        CHECK_HR(
            hr = m_pRenderEngine->Get3DDevice( &pDevice ),
            ::DbgMsg(_T("S3VMRWizard::InitializeDevice: failed to get Direct3D device from the render engine, hr = 0x%08x, pDevice = 0x%08x"), hr, pDevice));

        // we have to be thread safe only here when we actually mess up with shared data
        CAutoLock Lock(&m_WizardObjectLock);

        // just some insanity check
        RELEASE( pSrc->pTexturePriv );
        RELEASE( pSrc->pTempSurfacePriv );

        pSrc->DeleteSurfaces();

        // allocate surface buffer
        CHECK_HR(
            hr = pSrc->AllocateSurfaceBuffer( *lpNumBuffers ),
            ::DbgMsg(_T("S3VMRWizard::InitializeDevice: failed to allocate surface buffer, hr = 0x%08x, dwBuffers = %ld"),
                hr, *lpNumBuffers));

        DWORD TextureWidth = lpAllocInfo->dwWidth;
        DWORD TextureHeight = lpAllocInfo->dwHeight;

        // since we always copy data onto private textures, we create
        // the swap chain as offscreen surface


        pSrc->m_UserNV12RT = FALSE;
        // here we are creating private texture to be used by the render engine
        if (lpAllocInfo->Format > '0000') // surface is YUV.
        {
            IDirect3D9 *pDirect3D9 = NULL;
            pDevice->GetDirect3D(&pDirect3D9);

            hr = pDirect3D9->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, lpAllocInfo->Format);

            if(hr == D3D_OK)
            {
                pSrc->m_UserNV12RT = TRUE;
            }

            pDirect3D9->Release();

        }


        if(m_pRenderEngine->IsMAMMEnabled() || pSrc->m_UserNV12RT)
        {
            DWORD i;
            lpAllocInfo->dwFlags = VMR9AllocFlag_OffscreenSurface;//VMR9AllocFlag_3DRenderTarget | VMR9AllocFlag_TextureSurface;

            TextureWidth = (lpAllocInfo->dwWidth + 127)/128 *128;
            TextureHeight = (lpAllocInfo->dwHeight + 7)/8 *8;


            for(i=0; i<lpAllocInfo->MinBuffers;i++)
            {
                VOID *pMem = NULL;
                VOID *pMemOrg = NULL;

                if(!m_pRenderEngine->IsMAMMEnabled())
                {
                    DWORD MemorySize = GetFormatBpp(lpAllocInfo->Format) * TextureWidth * TextureHeight;
                    pMemOrg = malloc(MemorySize + 0x4000);
                    pMem = (BYTE *)pMemOrg + (0x4000 - (ULONG_PTR)pMemOrg % 0x4000);
                    pSrc->ppSurfaceMem[i] = pMemOrg;

                }

                hr = pDevice->CreateOffscreenPlainSurface(TextureWidth,TextureHeight,lpAllocInfo->Format,D3DPOOL_SYSTEMMEM,&pSrc->ppSurface[i],pMem != NULL ? (HANDLE *)&pMem : NULL);


                if(FAILED(hr))
                {
                    for(;i>0;i--)
                    {
                        RELEASE(pSrc->ppSurface[i-1]);
                    }
                }
            }
            CHECK_HR(
                hr,::DbgMsg(_T("S3VMRWizard::InitializeDevice: failed to create surface hr = 0x%08x, dwBuffers = %ld"), hr, lpAllocInfo->MinBuffers));
            pSrc->dwNumBufActuallyAllocated = lpAllocInfo->MinBuffers;
        }else
        {
            CHECK_HR(
                hr = ((IVMRSurfaceAllocatorNotify9*)pSrc->pVideoAllocator)->AllocateSurfaceHelper(
                                                    lpAllocInfo,
                                                    lpNumBuffers,
                                                    pSrc->ppSurface ),
                ::DbgMsg(_T("S3VMRWizard::InitializeDevice: failed in IVMRSurfaceAllocatorNotify9::AllocateSurfaceHelper, hr = 0x%08x, dwBuffers = %ld"), hr, *lpNumBuffers));
        
            pSrc->dwNumBufActuallyAllocated = *lpNumBuffers;

        }

        if(!pSrc->m_UserNV12RT)
        {
            if(m_pRenderEngine->IsMAMMEnabled())
            {
                hr = pDevice->CreateOffscreenPlainSurface(TextureWidth,TextureHeight,lpAllocInfo->Format,D3DPOOL_DEFAULT,&pSrc->pTempSurfacePriv,NULL);
            }

            format = D3DFMT_X8R8G8B8;// TODO: get current display format
        }else
        {
            format =  lpAllocInfo->Format;
        }



		RELEASE( pSrc->pTexturePriv );

        //using device here, need to mutex with RE, but?
        
        CHECK_HR(
            hr = pDevice->CreateTexture(TextureWidth,                // width
                                        TextureHeight,               // height
                                        1,                    // levels
                                        D3DUSAGE_RENDERTARGET,// usage
                                        format,               // format
                                        D3DPOOL_DEFAULT,      // we are not going to get into surface bits, so we do not need managed
                                        &(pSrc->pTexturePriv),
                                        NULL),
            ::DbgMsg(_T("S3VMRWizard::InitializeDevice: failed to create private texture, hr = 0x%08x"), hr));

        CHECK_HR(
            hr = pSrc->pTexturePriv->GetSurfaceLevel(0, &pS),
            ::DbgMsg(_T("S3VMRWizard::InitializeDevice: failed to get 0-level surface of the private texture, hr = 0x%08x"), hr));

        //CHECK_HR(
        //    hr = pDevice->ColorFill( pS, NULL, D3DCOLOR_XRGB(0x00,0x00,0x00)),
        //    ::DbgMsg("S3VMRWizard::InitializeDevice: failed to fill the surface "\
        //            "of the private texture with solid color, hr = 0x%08x", hr));

        CHECK_HR(
            hr = pSrc->pTexturePriv->GetLevelDesc(0, &ddsd),
            ::DbgMsg(_T("S3VMRWizard::InitializeDevice: failed to obtain surface description of the private texture, hr = 0x%08x"), hr));

        CHECK_HR(
            hr = pSrc->SetVideoSize( lpAllocInfo->dwWidth, lpAllocInfo->dwHeight),
            ::DbgMsg(_T("S3VMRWizard::InitializeDevice: failed to save video size, hr = 0x%08x"), hr));


    }// try
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }

    RELEASE( pS );
    RELEASE( pDevice );

    return hr;
}

STDMETHODIMP S3VMRWizard::TerminateDevice(
        DWORD_PTR  dwID
        )
{
    HRESULT hr = S_OK;

    S3VR_VideoSource *pSrc = NULL;

    hr = GetSourceInfo_( dwID, &pSrc );
    if( FAILED(hr) || !pSrc)
    {
        ::DbgMsg(_T("S3VMRWizard::TerminateDevice: failed in GetSourceInfo_() (wrong dwID), hr = 0x%08x, pSource = 0x%08x"), hr, pSrc);
        return (FAILED(hr)? hr : E_FAIL);
    }

    return hr;
}

STDMETHODIMP S3VMRWizard::GetSurface(
        DWORD_PTR  dwUserID,
        DWORD  SurfaceIndex,
        DWORD  SurfaceFlags,
        IDirect3DSurface9**  lplpSurface
        )
{
    HRESULT hr = S_OK;

    S3VR_VideoSource *pSrc = NULL;

    // check for NULL pointers
    if( !lplpSurface )
    {
        ::DbgMsg(_T("S3VMRWizard::GetSurface: fourth argument is NULL"));
        return E_POINTER;
    }

    // check that dwUserID points to a known VMR
    hr = GetSourceInfo_( dwUserID, &pSrc );
    if( FAILED(hr) || !pSrc )
    {
        ::DbgMsg(_T("S3VMRWizard::GetSurface, failed in GetSourceInfo_() (wrong dwID), hr = 0x%08x, pSource = 0x%08x"), hr, pSrc);
        return ( FAILED(hr) ? hr : E_FAIL );
    }

    // check that requested index does not exceed number of actually allocated buffers
    if( SurfaceIndex >= pSrc->dwNumBufActuallyAllocated  )
    {
        ::DbgMsg(_T("S3VMRWizard::GetSurface: requested surface index %ld falls out of valid range [0, %ld]"), SurfaceIndex, pSrc->dwNumBufActuallyAllocated);
        return E_INVALIDARG;
    }

    // check that requested surface is not null
    if( NULL == pSrc->ppSurface[ SurfaceIndex ] )
    {
        ::DbgMsg(_T("S3VMRWizard::GetSurface: FATAL, requested surface of index %ld is NULL!"),
            SurfaceIndex);
        return E_UNEXPECTED;
    }

    // we have to be thread safe only here when we actually mess up with shared data
    CAutoLock Lock(&m_WizardObjectLock);

    // now we checked everything and can copy
    *lplpSurface = pSrc->ppSurface[ SurfaceIndex ];
    (*lplpSurface)->AddRef();

    return S_OK;
}

STDMETHODIMP S3VMRWizard::StartPresenting(
        DWORD_PTR  dwUserID
        )
{
    return S_OK;
}

STDMETHODIMP S3VMRWizard::StopPresenting(
        DWORD_PTR  dwUserID
        )
{
    return S_OK;
}

STDMETHODIMP S3VMRWizard::PresentImage(
        DWORD_PTR  dwUserID,
        VMR9PresentationInfo*  lpPresInfo
        )
{
    HRESULT hr = S_OK;
    S3VR_VideoSource*  pSrc                = NULL;
    IDirect3DDevice9*       pSampleDevice       = NULL;
    IDirect3DSurface9*      pTexturePrivSurf    = NULL;

    // first, check for NULLs
    if( !lpPresInfo || !(lpPresInfo->lpSurf))
    {
        ::DbgMsg(_T("S3VMRWizard::PresentImage: received NULL pointer"));
        return E_POINTER;
    }

    // check that we know about dwUserID
    hr = GetSourceInfo_( dwUserID, &pSrc );
    if( FAILED(hr) || !pSrc )
    {
        ::DbgMsg(_T("S3VMRWizard::PresentImage: failed in GetSourceInfo_() (invalid dwID), hr = 0x%08x, pSource = 0x%08x"), hr, pSrc);
        return ( FAILED(hr) ? hr : VFW_E_NOT_FOUND);
    }


    if(lpPresInfo->szAspectRatio.cx != 0 && lpPresInfo->szAspectRatio.cy != 0)
    {
        if(lpPresInfo->szAspectRatio.cx == 1440 && lpPresInfo->szAspectRatio.cy == 1080)
        {
            pSrc->lPreferedx = 16;
            pSrc->lPreferedy = 9;
        }
        else
        {
            pSrc->lPreferedx = lpPresInfo->szAspectRatio.cx;
            pSrc->lPreferedy = lpPresInfo->szAspectRatio.cy;
        }
    }

    DWORD minFrameTime = 1000/(m_maxVideoFPS + m_maxVideoFPS/2);
    if(!pSrc->dwLast)
    {
        pSrc->dwLast = timeGetTime();
    }
    else
    {
        DWORD curTime = timeGetTime();
        if((curTime - pSrc->dwLast) < minFrameTime)//This video fps is too high, skip some frames
        {
            return S_OK;
        }
        pSrc->dwLast = curTime;
    }
    
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
#if TRACE_VIDEO_PRESENT
    m_PresentTimeHistory[m_CurPos%1024].BTime = timeGetTime();
    m_PresentTimeHistory[m_CurPos%1024].ID = dwUserID;
#endif
    try
    {
        // now, get the device of the sample passed in (it is not necessarily the same
        // device we created in the render engine
        CHECK_HR(
            hr = lpPresInfo->lpSurf->GetDevice( &pSampleDevice ),
            ::DbgMsg(_T("S3VMRWizard::PresentImage: failed to get the device of the surface passed in, hr = 0x%08x, pSampleDevice = 0x%08x"), hr, pSampleDevice));
        
        if(pSrc->pTexturePriv != NULL)
        {
            if(pSrc->m_UserNV12RT)
            {
                CHECK_HR(
                    hr = pSrc->pTexturePriv->GetSurfaceLevel( 0, &pTexturePrivSurf),
                    ::DbgMsg(_T("S3VMRWizard::PresentImage: failed to get the 0-level surface from the private texture , hr = 0x%08x, pPrivTextureSurf = 0x%08x"), hr, pTexturePrivSurf));

                CHECK_HR(
                    hr = pSampleDevice->UpdateSurface( lpPresInfo->lpSurf,NULL, pTexturePrivSurf, NULL ),
                    ::DbgMsg(_T("S3VMRWizard::PresentImage: failed to UpdateSurface the surface passed in, hr = 0x%08x, pSampleDevice = 0x%08x"), hr, pSampleDevice));
            }else
            {
                if(pSrc->pTempSurfacePriv)
                {

                    CHECK_HR(
                        hr = pSampleDevice->UpdateSurface( lpPresInfo->lpSurf,NULL, pSrc->pTempSurfacePriv, NULL ),
                        ::DbgMsg(_T("S3VMRWizard::PresentImage: failed to UpdateSurface the surface passed in, hr = 0x%08x, pSampleDevice = 0x%08x"), hr, pSampleDevice));
          
                    CHECK_HR(
                        hr = pSrc->pTexturePriv->GetSurfaceLevel( 0, &pTexturePrivSurf),
                        ::DbgMsg(_T("S3VMRWizard::PresentImage: failed to get the 0-level surface from the private texture , hr = 0x%08x, pPrivTextureSurf = 0x%08x"), hr, pTexturePrivSurf));

                    CHECK_HR(
                        hr = pSampleDevice->StretchRect( pSrc->pTempSurfacePriv,
                                                        NULL,
                                                        pTexturePrivSurf,
                                                        NULL,
                                                        D3DTEXF_NONE ),
                        ::DbgMsg(_T("S3VMRWizard::PresentImage: failed to StretchRect() from the video surface to the private texture, hr = 0x%08x"), hr));


                }else
                {

                    CHECK_HR(
                        hr = pSrc->pTexturePriv->GetSurfaceLevel( 0, &pTexturePrivSurf),
                        ::DbgMsg(_T("S3VMRWizard::PresentImage: failed to get the 0-level surface from the private texture , hr = 0x%08x, pPrivTextureSurf = 0x%08x"), hr, pTexturePrivSurf));

                    CHECK_HR(
                        hr = pSampleDevice->StretchRect( lpPresInfo->lpSurf,
                                                        NULL,
                                                        pTexturePrivSurf,
                                                        NULL,
                                                        D3DTEXF_NONE ),
                        ::DbgMsg(_T("S3VMRWizard::PresentImage: failed to StretchRect() from the video surface to the private texture, hr = 0x%08x"), hr));

                }
            }
            pSrc->bTextureInited = TRUE;
        }

    } // try
    catch( HRESULT hr1)
    {
        hr = hr1;
    }
    RELEASE( pTexturePrivSurf );
    RELEASE( pSampleDevice );
#if TRACE_VIDEO_PRESENT
    m_PresentTimeHistory[m_CurPos%1024].ETime = timeGetTime();
    m_CurPos++;
#endif
    pSrc->updateMutex.Unlock();
    m_WizardObjectLock.Unlock();
    return hr;
}

HRESULT S3VMRWizard::EnableSFRUpload(DWORD_PTR dwID, BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree)
{
    S3VR_VideoSource*  pSrc                = NULL;
    HRESULT hr;

    // check that we know about dwUserID
    hr = GetSourceInfo_( dwID, &pSrc );

    if( FAILED(hr) || !pSrc )
    {
        ::DbgMsg(_T("S3VMRWizard::PresentImage: failed in GetSourceInfo_() (invalid dwID), hr = 0x%08x, pSource = 0x%08x"), hr, pSrc);
        return ( FAILED(hr) ? hr : VFW_E_NOT_FOUND);
    }

#ifndef PLAYER_DUMMY
	if(EPlayerType == S3_MAMMPLAYER)
	{
		MAMM3DTexture9 *pTexture9 = NULL;
		hr = pSrc->pTexturePriv->QueryInterface(IID_MAMM3DTexture9, (void **)&pTexture9);

		if(SUCCEEDED(hr))
		{
			D3DSURFACE_DESC SurfaceDesc;
			hr = pSrc->pTexturePriv->GetLevelDesc(0, &SurfaceDesc);

			long DisplayWidth = pDisplayRect->right - pDisplayRect->left;
			long DisplayHeight = pDisplayRect->bottom - pDisplayRect->top;

			RECT DisplayRect = *pDisplayRect;

			if(pSrc->lImageWidth != SurfaceDesc.Width || pSrc->lImageHeight != SurfaceDesc.Height)
			{
				// scale display rect
				DisplayWidth = DisplayWidth * SurfaceDesc.Width/pSrc->lImageWidth;
				DisplayHeight = DisplayHeight * SurfaceDesc.Height/pSrc->lImageHeight;

				DisplayRect.right = DisplayRect.left + DisplayWidth;
				DisplayRect.bottom = DisplayRect.top + DisplayHeight;
			}


			pTexture9->EnableSFRUploadFreeAngle(bEnabled, bSplit, &DisplayRect, RotateDegree);
			pTexture9->Release();
		}
	}
#endif
    return S_OK;
}
