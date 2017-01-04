#include "stdafx.h"
#include "S3RenderEngine.h"
#include "S3RenderMixer.h"
#include "S3DShowWizard.h"
#include <mferror.h>
#include "S3Escape.h"
#include "MAMMD3D9.h"
#include "S3RenderScheduler.h"
#include "EVRPresenterInclude.h"
#include "S3Signage.h"

#define ESCAPE_INTERNAL_FUNCTION                0x00000002
#define ESCAPE_INTERNAL_GET_VSYNC               0x06
#define ESCAPE_INTERNAL_TURN_ON_SFR             0x07
static const int kMaxPresentFailed = 100;

#define S3_POINTER_64 
typedef struct  
{
    union
    {
        LARGE_INTEGER       Value;
        VOID *S3_POINTER_64 Pointer;
    };
}S3_LARGE_INTEGER;


typedef struct _UMD_ESCAPE_PACKET
{
    IN  DWORD       dwFunction;
    IN  DWORD       dwOpCode;
    IN union {
        struct {
            DWORD       dwInput1;
            DWORD       dwInput2;
        }Input;
        DWORD       dwInput;
        VOID* S3_POINTER_64 hInput;
    };
    OUT S3_LARGE_INTEGER qwOutput[2];
    OUT DWORD reserved;        // no use, but because we distinguish different USM_PACKET by its size
    // So every PACKET should have particular size, this DWORD use for this purpose
    // No this sizeof(UMD_ESCAPE_PACKET) = sizeof(UMD_ESCAPE_PACKET_NEW), if have
    // time combine all different PACKET to one.
} UMD_ESCAPE_PACKET, *PUMD_ESCAPE_PACKET;


BOOL GetVSyncPointerWin7(UINT **ppVsync)
{

    HDC hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);



    HINSTANCE    hInst = NULL;

    hInst = LoadLibrary( TEXT( "gdi32.dll" ));
    if ( hInst == NULL )
    {
        DeleteDC(hDC);
        return( FALSE );
    }


    PFND3DKMT_OPENADAPTERFROMHDC pfnKTOpenAdapterFromHdc = NULL;
    PFND3DKMT_CLOSEADAPTER pfnKTCloseAdapter = NULL;
    PFND3DKMT_ESCAPE pfnKTEscape;

    pfnKTOpenAdapterFromHdc = (PFND3DKMT_OPENADAPTERFROMHDC)
        GetProcAddress((HMODULE)hInst, "D3DKMTOpenAdapterFromHdc" );
    pfnKTCloseAdapter = (PFND3DKMT_CLOSEADAPTER)
        GetProcAddress((HMODULE)hInst, "D3DKMTCloseAdapter" );
    pfnKTEscape = (PFND3DKMT_ESCAPE)
        GetProcAddress((HMODULE)hInst, "D3DKMTEscape" );



    if ( pfnKTOpenAdapterFromHdc == NULL
        || pfnKTCloseAdapter == NULL
        || pfnKTEscape == NULL)
    {
        DeleteDC(hDC);
        return( FALSE );
    }    

    D3DKMT_OPENADAPTERFROMHDC    OpenAdapterData;
    D3DKMT_ESCAPE                LHescape;
    D3DKMT_CLOSEADAPTER          CloseAdapterData;

    HRESULT                      hRes;



    // Very important, must zero this struct

    //   or the app could crash.

    ZeroMemory( &OpenAdapterData, sizeof( OpenAdapterData ));
    OpenAdapterData.hDc = hDC;
    hRes = (*pfnKTOpenAdapterFromHdc)( &OpenAdapterData );

    if(SUCCEEDED(hRes))
    {
        UMD_ESCAPE_PACKET EscapeData;


        ZeroMemory( &LHescape, sizeof( LHescape ));
        LHescape.hAdapter  = OpenAdapterData.hAdapter;
        LHescape.Type                  = D3DKMT_ESCAPE_DRIVERPRIVATE;
        LHescape.pPrivateDriverData    = (VOID*) &EscapeData;
        LHescape.PrivateDriverDataSize = sizeof( EscapeData);

        memset(&EscapeData, 0, sizeof(EscapeData));

        EscapeData.dwFunction = ESCAPE_INTERNAL_FUNCTION;
        EscapeData.dwOpCode = ESCAPE_INTERNAL_GET_VSYNC;


        hRes = (*pfnKTEscape)( &LHescape );
        if(FAILED(hRes))
        {
            DeleteDC(hDC);
            *ppVsync = NULL;
            return FALSE;
        }else
        {
            *ppVsync = (UINT *)EscapeData.qwOutput->Value.LowPart;
        }

        CloseAdapterData.hAdapter = OpenAdapterData.hAdapter;

        (*pfnKTCloseAdapter)( &CloseAdapterData );

    }


    DeleteDC(hDC);

    return TRUE;
}


BOOL TurnOnSFRWin7()
{

    HDC hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);



    HINSTANCE    hInst = NULL;

    hInst = LoadLibrary( TEXT( "gdi32.dll" ));
    if ( hInst == NULL )
    {
        return( FALSE );
    }


    PFND3DKMT_OPENADAPTERFROMHDC pfnKTOpenAdapterFromHdc = NULL;
    PFND3DKMT_CLOSEADAPTER pfnKTCloseAdapter = NULL;
    PFND3DKMT_ESCAPE pfnKTEscape;

    pfnKTOpenAdapterFromHdc = (PFND3DKMT_OPENADAPTERFROMHDC)
        GetProcAddress((HMODULE)hInst, "D3DKMTOpenAdapterFromHdc" );
    pfnKTCloseAdapter = (PFND3DKMT_CLOSEADAPTER)
        GetProcAddress((HMODULE)hInst, "D3DKMTCloseAdapter" );
    pfnKTEscape = (PFND3DKMT_ESCAPE)
        GetProcAddress((HMODULE)hInst, "D3DKMTEscape" );



    if ( pfnKTOpenAdapterFromHdc == NULL
        || pfnKTCloseAdapter == NULL
        || pfnKTEscape == NULL)
    {
        return( FALSE );
    }    

    D3DKMT_OPENADAPTERFROMHDC    OpenAdapterData;
    D3DKMT_ESCAPE                LHescape;
    D3DKMT_CLOSEADAPTER          CloseAdapterData;

    HRESULT                      hRes;



    // Very important, must zero this struct

    //   or the app could crash.

    ZeroMemory( &OpenAdapterData, sizeof( OpenAdapterData ));
    OpenAdapterData.hDc = hDC;
    hRes = (*pfnKTOpenAdapterFromHdc)( &OpenAdapterData );

    if(SUCCEEDED(hRes))
    {
        UMD_ESCAPE_PACKET EscapeData;


        ZeroMemory( &LHescape, sizeof( LHescape ));
        LHescape.hAdapter  = OpenAdapterData.hAdapter;
        LHescape.Type                  = D3DKMT_ESCAPE_DRIVERPRIVATE;
        LHescape.pPrivateDriverData    = (VOID*) &EscapeData;
        LHescape.PrivateDriverDataSize = sizeof( EscapeData);

        memset(&EscapeData, 0, sizeof(EscapeData));

        EscapeData.dwFunction = ESCAPE_INTERNAL_FUNCTION;
        EscapeData.dwOpCode = ESCAPE_INTERNAL_TURN_ON_SFR;


        hRes = (*pfnKTEscape)( &LHescape );
        if(FAILED(hRes))
        {
            DeleteDC(hDC);
            return FALSE;
        }

        CloseAdapterData.hAdapter = OpenAdapterData.hAdapter;

        (*pfnKTCloseAdapter)( &CloseAdapterData );

    }


    DeleteDC(hDC);

    return TRUE;
}




#define S3_VM_GET_VSYNC             0x30ce
#define S3_VM_TRIGGER_FLUSH         0x30cf


BOOL GetVSyncPointerXP(UINT **ppVsync)
{

    HRESULT hr;
    HDC hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);



    // get Vsync Counter;
    hr = ExtEscape(hDC, S3_VM_GET_VSYNC, 0, NULL, sizeof(DWORD*), (LPSTR)(void *)ppVsync);

    if(FAILED(hr))
    {
        *ppVsync = NULL;
        return FALSE;
    }


    DeleteDC(hDC);

    return TRUE;
}

UINT WM_DEVICE_LOST = ::RegisterWindowMessage(_T("PROCESS_DEVICE_LOST"));
UINT WM_DEVICE_HANG = ::RegisterWindowMessage(_T("PROCESS_DEVICE_HANG"));

PTR_DXVA2CreateDirect3DDeviceManager9	g_pfDXVA2CreateDirect3DDeviceManager9;
PTR_MFCreateDXSurfaceBuffer				g_pfMFCreateDXSurfaceBuffer;
PTR_MFCreateVideoSampleFromSurface		g_pfMFCreateVideoSampleFromSurface;
PTR_MFCreateVideoMediaType				g_pfMFCreateVideoMediaType;
PTR_MFGetService				        g_pfMFGetService;
PTR_MFFrameRateToAverageTimePerFrame    g_pfMFFrameRateToAverageTimePerFrame;
PTR_MFCreateMediaType                   g_pfMFCreateMediaType;

S3RenderEngine::S3RenderEngine()
    : m_hwnd( NULL )
    , m_pDevice( NULL)
    , m_pMixerControl( NULL )
    , m_pOwner( NULL )
    , m_bInitialized( FALSE )
    , m_setFPS( 30 )
    , m_interframe(33)
    , m_iferror(1000.0f/30 - 33)
    , m_accuError(0)
    , m_dwLast(0)
    , m_bChangingDevice( FALSE )
    , m_pVSyncCounter(NULL)
    , m_dwLastVSync(0)
    , m_bMAMMEnabled(FALSE)
    , m_bEVREnabled(FALSE)
    , m_pSpeedControlQuery(NULL)
    , m_pDeviceManager(NULL)
    , m_bPaused(FALSE)
    , m_nPresentFailCount(0)
{

	// Load EVR specifics DLLs
	HMODULE hLib = LoadLibrary (L"dxva2.dll");
	g_pfDXVA2CreateDirect3DDeviceManager9	= hLib ? (PTR_DXVA2CreateDirect3DDeviceManager9) GetProcAddress (hLib, "DXVA2CreateDirect3DDeviceManager9") : NULL;
        	
	// Load EVR functions
	hLib = LoadLibrary (L"evr.dll");
	g_pfMFCreateDXSurfaceBuffer			= hLib ? (PTR_MFCreateDXSurfaceBuffer)			GetProcAddress (hLib, "MFCreateDXSurfaceBuffer") : NULL;
	g_pfMFCreateVideoSampleFromSurface	= hLib ? (PTR_MFCreateVideoSampleFromSurface)	GetProcAddress (hLib, "MFCreateVideoSampleFromSurface") : NULL;
	g_pfMFCreateVideoMediaType			= hLib ? (PTR_MFCreateVideoMediaType)			GetProcAddress (hLib, "MFCreateVideoMediaType") : NULL;


	hLib = LoadLibrary (L"mf.dll");
	g_pfMFGetService			        = hLib ? (PTR_MFGetService)			            GetProcAddress (hLib, "MFGetService") : NULL;

	hLib = LoadLibrary (L"Mfplat.dll");
	g_pfMFFrameRateToAverageTimePerFrame= hLib ? (PTR_MFFrameRateToAverageTimePerFrame)	GetProcAddress (hLib, "MFFrameRateToAverageTimePerFrame") : NULL;
	g_pfMFCreateMediaType			    = hLib ? (PTR_MFCreateMediaType)			    GetProcAddress (hLib, "MFCreateMediaType") : NULL;


    // make sure timer is at least 1 ms accurate
    timeBeginPeriod(1);
#if TRACE_RENDER_TIMER
    memset(m_RenderTimeHistory, 0, sizeof(RenderTime_Info)*1024);
    m_CurPos = 0;
#endif
}

/******************************Public*Routine******************************\
* S3RenderEngine
*
* destructor
\**************************************************************************/
S3RenderEngine::~S3RenderEngine()
{
    HRESULT hr = S_OK;
    CAutoLock Lock(&m_REObjectLock);

    // here we have to disconnect child IS3VRMixerControl
    if( m_pMixerControl )
    {
        hr = m_pMixerControl->SetRenderEngineOwner(NULL);
        if( FAILED(hr))
        {
            ::DbgMsg(_T("~S3RenderEngine: failed to disconnect child mixer control, he = 0x%08x"), hr);
        }
    }
    
    Clean_();
    // call off the timer
    timeEndPeriod(1);
#if TRACE_RENDER_TIMER
    FILE* pMyFile = NULL;
    char  CurrentPath[MAX_PATH+1];
    char  pDstStr[78];

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
    strcpy_s(CurrentPath + pathEnd, MAX_PATH - pathEnd, "RenderTime.txt");

    fopen_s(&pMyFile, CurrentPath, "w+");
    if(pMyFile)
    {
        DWORD i;
        for(i=0;i<1024;i++)
        {
            sprintf_s(pDstStr, 78, "%4d Begin:%8d Begin Present:%8d End Present:%8d End:%8d\n", 
                i,
                m_RenderTimeHistory[m_CurPos%1024].BTime, 
                m_RenderTimeHistory[m_CurPos%1024].MBTime,
                m_RenderTimeHistory[m_CurPos%1024].METime,
                m_RenderTimeHistory[m_CurPos%1024].ETime);
            m_CurPos++;
            fwrite(pDstStr, 77, 1, pMyFile);
        }
        fclose(pMyFile);
    }
#endif
}

HRESULT S3RenderEngine::Initialize(
    HWND hWnd, 
    DWORD dwFlags,
    S3RenderMixer* pMixerControl,
    S3RenderScheduler*  pScheduler,
    BOOL bFullScreen,
    BOOL bEnableMAMM,
    BOOL bEnableEVR
    )
{
#if 0
    m_hwnd = hWnd;
    m_bInitialized = TRUE;
    return S_OK;
#else // PLAYER_DUMMY
    HRESULT hr = S_OK;

    if( FALSE == IsWindow( hWnd ))
    {
        ::DbgMsg(_T("S3RenderEngine::Initialize: received invalid window handle"));
        return E_INVALIDARG;
    }

    if( m_bInitialized )
    {
        ::DbgMsg(_T("S3RenderEngine::Initialize: method was already called"));
        return VFW_E_WRONG_STATE;
    }
    if(!pMixerControl)
    {
        ::DbgMsg(_T("S3RenderEngine::Initialize: received NULL pMixerControl"));
        return E_INVALIDARG;
    }

    if(m_pMixerControl)
    {
        ::DbgMsg(_T("S3RenderEngine::Initialize: m_pMixerControl is not NULL!"));
        return VFW_E_WRONG_STATE;
    }

    m_bFullScreen = bFullScreen;

    CAutoLock Lock(&m_REObjectLock);

    m_pMixerControl = pMixerControl;

    m_pScheduler = pScheduler;

    m_bMAMMEnabled = bEnableMAMM && IsSupportMAMMRendering();

    m_bEVREnabled = bEnableEVR;

    try
    {
        m_hwnd = hWnd;

        // TODO: check flags
        m_dwCfgFlags = dwFlags;

        CHECK_HR(
            m_pMixerControl->SetRenderEngineOwner( this ),
            ::DbgMsg(_T("S3RenderEngine::Initialize: failed to advise 'this' owner to the mixer control")));

		hr = CreateDevice_();

		CHECK_HR(
            hr = m_pMixerControl->Initialize( m_pDevice ),
            ::DbgMsg(_T("S3RenderEngine::Initialize: failed to initialize mixer control, hr = 0x%08x"), hr));

        m_bInitialized = TRUE;
    }// try
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }

    return hr;
#endif // PLAYER_DUMMY
}

/******************************Public*Routine******************************\
* Terminate
* 
\**************************************************************************/
HRESULT S3RenderEngine::Terminate( void )
{
#ifdef PLAYER_DUMMY
    return S_OK;
#else // PLAYER_DUMMY
    HRESULT hr = S_OK;
    if( m_pMixerControl )
    {
        hr = m_pMixerControl->SetRenderEngineOwner(NULL);
        if( FAILED(hr))
        {
            ::DbgMsg(_T("S3RenderEngine::Terminate: failed to unadvise RenderEngineOwner for mixer control, hr = 0x%08x"), hr);
            return hr;
        }
    }

    return S_OK;
#endif // PLAYER_DUMMY
}

HRESULT S3RenderEngine::Render()
{
#ifdef PLAYER_DUMMY
    Sleep(10);
    return S_OK;
#else // PLAYER_DUMMY
    HRESULT hr = S_OK;
    HRESULT hrMC = S_OK;

    // detect Os Version
    DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(GetVersion())));

    if( 0 == m_dwLast )
    {
        m_dwLast = timeGetTime();

        if(dwMajorVersion > 5) //Vista and Win7
        {
            GetVSyncPointerWin7((UINT **)&m_pVSyncCounter);

        }else
        {
            GetVSyncPointerXP((UINT **)&m_pVSyncCounter);
        }

        if(m_pVSyncCounter != NULL)
        {
            m_dwLastVSync = *m_pVSyncCounter;
        }
    }
    else
    {
        if(m_pVSyncCounter) // S3 VideoWall Path
        {
            DWORD TimeOutCount = 0;
            
            if(m_setFPS > 40)
            {
                while( (*m_pVSyncCounter - m_dwLastVSync < 1)  && (TimeOutCount < 20))
                {
                    Sleep(1);
                    TimeOutCount ++;
                }
            }
            else
            {
                while( (*m_pVSyncCounter - m_dwLastVSync < 2)  && (TimeOutCount < 20))
                {
                    Sleep(2);
                    TimeOutCount ++;
                }
            }


            m_dwLastVSync = *m_pVSyncCounter;
        }
        else
        {
            DWORD interframe = m_interframe;
            if(m_accuError > 0.5f)
            {
                interframe++;
                m_accuError -= 1.0f;
            }
            DWORD delta = timeGetTime() - m_dwLast;
            if( delta < interframe )
            {
                Sleep(interframe - delta);
                m_dwLast += interframe;
                m_accuError += m_iferror;
            }
            else
            {
                DWORD times = delta/interframe;
                m_dwLast += times*interframe;
                m_accuError += times*m_iferror;
            }
        }
    }

    if( FALSE == m_bInitialized )
    {
        ::DbgMsg(_T("S3RenderEngine::Render: object is not initialized"));
        return VFW_E_WRONG_STATE;
    }


    CAutoLock Lock(&m_REObjectLock);

    if(m_bPaused) return S_OK;

	if( m_bChangingDevice )
	{
        ::DbgMsg(_T("S3RenderEngine::Render: device lost, skip rendering"));
        ::PostMessage(m_hwnd, WM_DEVICE_LOST, 0, 0);
		return hr;
	}
 
#if TRACE_RENDER_TIMER
    m_RenderTimeHistory[m_CurPos%1024].BTime = timeGetTime();
#endif

    if( !m_pMixerControl )
    {
        ::DbgMsg(_T("S3RenderEngine::Render: FATAL, cannot find IS3VRMixerControl"));
        return E_UNEXPECTED;
    }

    try
    {
        //hr = m_pMixerControl->Compose( NULL );
        //if( FAILED(hr))
        //{
        //    ::DbgMsg("S3RenderEngine::Render: failed in IS3VRMixerControl::Compose, hr = 0x%08x", hr);
        //    return hr;
        //}
		if( !m_pDevice )
			throw S_OK;

        hr = m_pSpeedControlQuery->Issue(D3DISSUE_END);
        

        CHECK_HR(
            hr = m_pDevice->BeginScene(),
            ::DbgMsg(_T("S3RenderEngine::Render: failed in BeginScene(), hr = 0x%08x"),hr));

        // first, render all the video source by means of mixer control
        hrMC = m_pMixerControl->Render( m_pDevice, NULL);
       
        CHECK_HR(
            hr = m_pDevice->EndScene(),
            ::DbgMsg(_T("S3RenderEngine::Render: failed in EndScene(), hr = 0x%08x"),hr));

#if TRACE_RENDER_TIMER
        m_RenderTimeHistory[m_CurPos%1024].MBTime = timeGetTime();
#endif
        CHECK_HR(
            hr = m_pDevice->Present(NULL,NULL,NULL,NULL),
            ::DbgMsg(_T("S3RenderEngine::Render: failed in Present(), hr = 0x%08x"),hr));

        if (hr != S_OK && ++m_nPresentFailCount > kMaxPresentFailed)
        {
            PostThreadMessage(((CS3SignageApp*)AfxGetApp())->m_nThreadID, WM_QUIT, 100, 0);
        }
        
        if (hr == S_OK)
        {
            m_nPresentFailCount = 0;
        }

#if TRACE_RENDER_TIMER
        m_RenderTimeHistory[m_CurPos%1024].METime = timeGetTime();
#endif
        DWORD EventQueryData;
        DWORD waitTimes = 0;

        hr = m_pSpeedControlQuery->GetData(&EventQueryData, sizeof(DWORD), D3DGETDATA_FLUSH);

        if(dwMajorVersion > 5)
        {
            while (hr != S_OK) 
            {
                Sleep(5);
                hr = m_pSpeedControlQuery->GetData(&EventQueryData, sizeof(DWORD), D3DGETDATA_FLUSH);
                waitTimes++;
                if(waitTimes>=7)
                {
                    hr = S_OK;
                    break;
                }
            };
        }

        if( FAILED( hrMC ))
        {
            ::DbgMsg(_T("S3RenderEngine::Render: failed in IS3VRMixerControl::Render, error code 0x%08x"), hrMC);
            hr = hrMC;
        }

    } // try
    catch( HRESULT hr1 )
    {
		if( (hr1 == D3DERR_DEVICELOST || 
			 hr1 == D3DERR_DRIVERINTERNALERROR) && m_pOwner )
		{
			m_bChangingDevice = TRUE;
            ::PostMessage(m_hwnd, WM_DEVICE_LOST, 0, 0);
		}else if((hr1 == D3DERR_DEVICEHUNG) && m_pOwner)
        {
            ::PostMessage(m_hwnd, WM_DEVICE_HANG, 0, 0);
        }
		else
		{
			hr = hr1;
            if (++m_nPresentFailCount > kMaxPresentFailed)
            {
                PostQuitMessage(100);
            }
		}
    }
#if TRACE_RENDER_TIMER
    m_RenderTimeHistory[m_CurPos%1024].ETime = timeGetTime();
    m_CurPos++;
#endif
    return hr;
#endif // PLAYER_DUMMY
}

/******************************Public*Routine******************************\
* ProcessLostDevice
*
* responds to the device loss event
\**************************************************************************/
HRESULT S3RenderEngine::ProcessLostDevice()
{
#ifdef PLAYER_DUMMY
    return S_OK;
#else // PLAYER_DUMMY
	HRESULT hr = S_OK;
	// tell all the components that we are lost. so that they would release all the 
	// allocated video resources
#if 1

    CAutoLock Lock(&m_REObjectLock);

	if( m_pScheduler )
	{
		hr = m_pScheduler->BeginDeviceLoss();
	}

    hr = ResetDevice_();
	if( SUCCEEDED(hr) && m_pDevice )
	{
		if( m_pScheduler )
		{
			hr = m_pScheduler->EndDeviceLoss( m_pDevice );
		}

        MSG Messages;

        while(PeekMessage(&Messages, m_hwnd, WM_DEVICE_LOST, WM_DEVICE_LOST + 1, PM_REMOVE));
		m_bChangingDevice = FALSE;
	}
#else
    PostMessage(m_hwnd, WM_KEYDOWN, VK_ESCAPE, 1);
#endif
    return hr;
#endif // PLAYER_DUMMY
}

HRESULT S3RenderEngine::SetFrameRate(int nFramesPerSec)
{
    HRESULT hr = S_OK;
    if( FALSE == m_bInitialized )
    {
        ::DbgMsg(_T("S3RenderEngine::SetFrameRate: object is not initialized"));
        return VFW_E_WRONG_STATE;
    }
    if( nFramesPerSec<1 ||
        nFramesPerSec >120 )
    {
        ::DbgMsg(_T("S3RenderEngine::SetFrameRate: desired rate must be between 1 and 120"));
        return E_INVALIDARG;
    }

    CAutoLock Lock(&m_REObjectLock);
    m_setFPS = nFramesPerSec;
    m_interframe = 1000/m_setFPS;
    m_iferror = (1000.0f/m_setFPS) - m_interframe;
    m_accuError = 0.0f;

    return hr;
}

HRESULT S3RenderEngine::GetFrameRate(int* pnFramesPerSec)
{
    if( FALSE == m_bInitialized )
    {
        ::DbgMsg(_T("S3RenderEngine::GetFrameRate: object is not initialized"));
        return VFW_E_WRONG_STATE;
    }
    if( !pnFramesPerSec)
    {
        ::DbgMsg(_T("S3RenderEngine::GetFrameRate: received NULL pointer"));
        return E_POINTER;
    }

    *pnFramesPerSec = m_setFPS;
    return S_OK;
}

HRESULT S3RenderEngine::GetMixingPrefs(DWORD* pdwPrefs)
{
#ifdef PLAYER_DUMMY
    return S_OK;
#else // PLAYER_DUMMY
    if( FALSE == m_bInitialized )
    {
        ::DbgMsg(_T("S3RenderEngine::GetMixingPrefs: object is not initialized"));
        return VFW_E_WRONG_STATE;
    }
    if( !pdwPrefs )
    {
        ::DbgMsg(_T("S3RenderEngine::GetMixingPrefs: received NULL pointer"));
        return E_POINTER;
    }

    *pdwPrefs = m_dwCfgFlags;
    return S_OK;
#endif // PLAYER_DUMMY
}

HRESULT S3RenderEngine::SetWizardOwner(S3DShowWizard* pWizard)
{
    HRESULT hr = S_OK;
    if( FALSE == m_bInitialized )
    {
        ::DbgMsg(_T("S3RenderEngine::SetWizardOwner: object is not initialized"));
        return VFW_E_WRONG_STATE;
    }
    if(!pWizard)
    {
        ::DbgMsg(_T("S3RenderEngine::SetWizardOwner: pWizard can not be NULL!"));
        return E_INVALIDARG;
    }
    if(m_pOwner)
    {
        ::DbgMsg(_T("S3RenderEngine::SetWizardOwner: Owner already set!"));
        return VFW_E_WRONG_STATE;
    }
    CAutoLock Lock(&m_REObjectLock);

    m_pOwner = pWizard;

    return hr;
}

HRESULT S3RenderEngine::GetWizardOwner(S3DShowWizard** ppWizard)
{
    if( FALSE == m_bInitialized )
    {
        ::DbgMsg(_T("S3RenderEngine::GetWizardOwner: object is not initialized"));
        return VFW_E_WRONG_STATE;
    }

    if( !ppWizard )
    {
        ::DbgMsg(_T("S3RenderEngine::GetWizardOwner: received NULL pointer"));
        return E_POINTER;
    }

    if( m_pOwner )
    {
        *ppWizard = m_pOwner;
    }
    else
    {
        *ppWizard = NULL;
    }
    return S_OK;
}

HRESULT S3RenderEngine::GetMixerControl(S3RenderMixer** ppMixerControl)
{
    if( FALSE == m_bInitialized )
    {
        ::DbgMsg(_T("S3RenderEngine::GetMixerControl: object is not initialized"));
        return VFW_E_WRONG_STATE;
    }
    if( !ppMixerControl )
    {
        ::DbgMsg(_T("S3RenderEngine::GetMixerControl: received NULL pointer"));
        return E_POINTER;
    }
    if( m_pMixerControl )
    {
        *ppMixerControl = m_pMixerControl;
    }
    else
    {
        *ppMixerControl = NULL;
    }

    return S_OK;
}

HRESULT S3RenderEngine::Get3DDevice(IDirect3DDevice9 ** ppDevice)
{
#ifdef PLAYER_DUMMY
    *ppDevice = NULL;
    return S_OK;
#else // PLAYER_DUMMY
    if( FALSE == m_bInitialized )
    {
        ::DbgMsg(_T("S3RenderEngine::Get3DDevice: object is not initialized"));
        return VFW_E_WRONG_STATE;
    }
    if( !ppDevice )
    {
        ::DbgMsg(_T("S3RenderEngine::Get3DDevice: received NULL pointer"));
        return E_POINTER;
    }
    if( m_pDevice )
    {
        *ppDevice = m_pDevice;
        (*ppDevice)->AddRef();
    }
    else
    {
        *ppDevice = NULL;
    }

    return S_OK;
#endif // PLAYER_DUMMY
}

HRESULT S3RenderEngine::GetVideoWindow(HWND *phwnd)
{
    if( FALSE == m_bInitialized )
    {
        ::DbgMsg(_T("S3RenderEngine::GetVideoWindow: object is not initialized"));
        return VFW_E_WRONG_STATE;
    }
    if( !phwnd )
    {
        ::DbgMsg(_T("S3RenderEngine::GetVideoWindow: received NULL pointer"));
        return E_POINTER;
    }

    *phwnd = m_hwnd;
    return S_OK;
}

typedef HRESULT ( WINAPI* LPDIRECT3DCREATE9EX )(UINT , IDirect3D9Ex **);

/////////////////////// Private routine ///////////////////////////////////////

/******************************Private*Routine******************************\
* CreateDevice_
*
* creates IDirect3DDevice9
\**************************************************************************/
HRESULT S3RenderEngine::CreateDevice_()
{
#ifdef PLAYER_DUMMY
    RECT WindowRect;
    GetClientRect(m_hwnd, &WindowRect);
    m_DisplaySize.cx = WindowRect.right - WindowRect.left;
    m_DisplaySize.cy = WindowRect.bottom - WindowRect.top;

    return S_OK;
#else /* PLAYER_DUMMY */
	HRESULT hr = S_OK;

    D3DPRESENT_PARAMETERS pp;
    IDirect3D9Ex *pD3DEx = NULL;

	try
	{
		if(EPlayerType == S3_MAMMPLAYER && m_bMAMMEnabled)
		{
			m_pD3D9 = (IDirect3D9 *)MAMM3DCreate9(D3D_SDK_VERSION);
		}
		else
		{
			LPDIRECT3DCREATE9EX   pDynamicDirect3DCreate9Ex = NULL;
        
            // This may fail if Direct3D 9 isn't installed
            HMODULE hModD3D9 = LoadLibrary( L"d3d9.dll" );
            if( hModD3D9 != NULL )
            {
                pDynamicDirect3DCreate9Ex = ( LPDIRECT3DCREATE9EX )GetProcAddress( hModD3D9, "Direct3DCreate9Ex" );
            }

            if(pDynamicDirect3DCreate9Ex)
            {
                pDynamicDirect3DCreate9Ex(D3D_SDK_VERSION, &pD3DEx);
                m_pD3D9 = pD3DEx;
            }
            else
            {
	        // create Direct3D
                m_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
            }

            if(g_pfDXVA2CreateDirect3DDeviceManager9 && m_bEVREnabled)
            {
                g_pfDXVA2CreateDirect3DDeviceManager9(&m_DeviceResetToken, &m_pDeviceManager);
            }
		}

		CHECK_HR( 
            m_pD3D9 ? S_OK : E_FAIL,
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed to create Direct3D9 object")));

        D3DCAPS9 hwCaps;

        CHECK_HR(
            m_pD3D9->GetDeviceCaps( 0, D3DDEVTYPE_HAL, &hwCaps ),
            ::DbgMsg(_T("S3RenderEngine::Initialize: No compatible HW found")));


        // create device
        DEVMODE   curMode;
        if(!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &curMode ))
        {
            hr = E_FAIL;
        }
        CHECK_HR(
            hr,
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed to get current mode")));
        D3DFORMAT fmt;
        
        switch(curMode.dmBitsPerPel)
        {
            case 32:
                fmt = D3DFMT_X8R8G8B8;
                hr = S_OK;
                break;
            case 16:
                fmt = D3DFMT_R5G6B5;
                hr = S_OK;
                break;
            default:
                hr = E_FAIL;
                break;
        }
        CHECK_HR(
            hr,
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: unsupported display mode")));


        // detect Os Version

        DWORD dwVersion = 0; 
        DWORD dwMajorVersion = 0;
        dwVersion = GetVersion();
        dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));

        if(dwMajorVersion > 5) //Vista and Win7
        {
            TurnOnSFRWin7();
        }


        ZeroMemory( &pp, sizeof(D3DPRESENT_PARAMETERS));

        if(m_bFullScreen)
        {

            pp.BackBufferWidth = GetSystemMetrics(SM_CXSCREEN);
            pp.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
            pp.Windowed = FALSE;
            pp.hDeviceWindow = m_hwnd;
            pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            pp.BackBufferCount = 1;
            pp.BackBufferFormat = fmt;
            pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
            pp.FullScreen_RefreshRateInHz = curMode.dmDisplayFrequency;

            m_DisplaySize.cx = GetSystemMetrics(SM_CXSCREEN);
            m_DisplaySize.cy = GetSystemMetrics(SM_CYSCREEN);


            if(m_bMAMMEnabled)
            {
                pp.BackBufferWidth = 0;
                pp.BackBufferHeight = 0;
            }
        }
        else
        {
            pp.Windowed = TRUE;
            pp.hDeviceWindow = m_hwnd;
            pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            pp.BackBufferCount = 1;
            pp.BackBufferFormat = fmt;
            pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

            RECT WindowRect;
            GetClientRect(m_hwnd, &WindowRect);
            m_DisplaySize.cx = WindowRect.right - WindowRect.left;
            m_DisplaySize.cy = WindowRect.bottom - WindowRect.top;
        }

 
        if(pD3DEx)
        {
            if(m_bFullScreen)
            {
                D3DDISPLAYMODEEX DisplayMode;
                DisplayMode.Size = sizeof(D3DDISPLAYMODEEX);
                DisplayMode.Format = pp.BackBufferFormat;
                DisplayMode.RefreshRate = pp.FullScreen_RefreshRateInHz;
                DisplayMode.Width = pp.BackBufferWidth;
                DisplayMode.Height = pp.BackBufferHeight;
                DisplayMode.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;

                pD3DEx->CreateDeviceEx(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,
                                        m_hwnd,
                                        D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
                                        &pp,
                                        &DisplayMode,
                                        (IDirect3DDevice9Ex**)&m_pDevice);
            }else
            {
                pD3DEx->CreateDeviceEx(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,
                        m_hwnd,
                        D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
                        &pp,
                        NULL,
                        (IDirect3DDevice9Ex**)&m_pDevice);
            }

            if(m_pDeviceManager)
            {
                m_pDeviceManager->ResetDevice(m_pDevice, m_DeviceResetToken);
            }


        }else
        {
            hr = m_pD3D9->CreateDevice(D3DADAPTER_DEFAULT ,
                                    D3DDEVTYPE_HAL,
                                    m_hwnd,
                                    D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
                                    &pp,
                                    &m_pDevice);
        }




        hr = m_pDevice ? S_OK : hr;
        CHECK_HR(
            hr,
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed to create device, error code 0x%08x"), hr));


        if(m_bMAMMEnabled)
        {
            m_DisplaySize.cx = pp.BackBufferWidth;
            m_DisplaySize.cy = pp.BackBufferHeight;
        }

        hr = m_pDevice->CreateQuery(D3DQUERYTYPE_EVENT, &m_pSpeedControlQuery);


        CHECK_HR(
            hr,
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed to create Query, error code 0x%08x"), hr));


        //maximum ambient light
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_AMBIENT,RGB(255,255,255)),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetRenderState(D3DRS_AMBIENT...)")));

        //lighting disabled
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_LIGHTING,FALSE),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetRenderState(D3DRS_LIGHTING...)")));

        //don't cull backside
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetRenderState(D3DRS_CULLMODE...)")));

        //DISABLE depth buffering
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetRenderState(D3DRS_ZENABLE...)")));

        // enable dithering
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetRenderState(D3DRS_DITHERENABLE...)")));

        // disable stensil
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetRenderState(D3DRS_STENCILENABLE...)")));

        // manage blending
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetRenderState(D3DRS_ALPHABLENDENABLE...)")));

        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetRenderState(D3DRS_SRCBLEND...)")));

        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetRenderState(D3DRS_DESTBLEND...)")));

        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetRenderState(D3DRS_ALPHATESTENABLE...)")));

        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_ALPHAREF, 0x10),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetRenderState(D3DRS_ALPHAREF...)")));

        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetRenderState(D3DRS_ALPHAFUNC...)")));

        // set up sampler

        CHECK_HR(
            m_pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetSamplerState(D3DSAMP_ADDRESSU...)")));

        CHECK_HR(
            m_pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetSamplerState(D3DSAMP_ADDRESSV...)")));

        CHECK_HR(
            m_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetSamplerState(D3DSAMP_MAGFILTER...)")));

        CHECK_HR(
            m_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR),
            ::DbgMsg(_T("S3RenderEngine::CreateDevice_: failed in SetSamplerState(D3DSAMP_MINFILTER...)")));

        // turn on S3 Signage Driver Path
	    IDirect3DTexture9 *pSignageAppTexture = NULL;
        if(m_pDevice->CreateTexture(1,1,1, 0, (D3DFORMAT)MAKEFOURCC('S', '3', 'S', 'A'), D3DPOOL_DEFAULT, &pSignageAppTexture, NULL) == S_OK)
        {
            pSignageAppTexture->Release();
        }
    }// try
	catch( HRESULT hr1 )
	{
		hr = hr1;
	}

	return hr;
#endif /* PLAYER_DUMMY */
}

HRESULT  S3RenderEngine::ResetDevice_()
{
	HRESULT hr = S_OK;

    D3DPRESENT_PARAMETERS pp;
	try
	{
        if(m_pSpeedControlQuery) {m_pSpeedControlQuery->Release(); m_pSpeedControlQuery = NULL;}

        // create device
        DEVMODE   curMode;
        if(!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &curMode ))
        {
            hr = E_FAIL;
        }
        CHECK_HR(
            hr,
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed to get current mode")));
        D3DFORMAT fmt;
        
        switch(curMode.dmBitsPerPel)
        {
            case 32:
                fmt = D3DFMT_X8R8G8B8;
                hr = S_OK;
                break;
            case 16:
                fmt = D3DFMT_R5G6B5;
                hr = S_OK;
                break;
            default:
                hr = E_FAIL;
                break;
        }
        CHECK_HR(
            hr,
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: unsupported display mode")));


        ZeroMemory( &pp, sizeof(D3DPRESENT_PARAMETERS));

        if(m_bFullScreen)
        {

            pp.BackBufferWidth = GetSystemMetrics(SM_CXSCREEN);
            pp.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
            pp.Windowed = FALSE;
            pp.hDeviceWindow = m_hwnd;
            pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            pp.BackBufferCount = 1;
            pp.BackBufferFormat = fmt;
            pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
            pp.FullScreen_RefreshRateInHz = curMode.dmDisplayFrequency;

            m_DisplaySize.cx = GetSystemMetrics(SM_CXSCREEN);
            m_DisplaySize.cy = GetSystemMetrics(SM_CYSCREEN);


            if(m_bMAMMEnabled)
            {
                pp.BackBufferWidth = 0;
                pp.BackBufferHeight = 0;
            }
        }
        else
        {
            pp.Windowed = TRUE;
            pp.hDeviceWindow = m_hwnd;
            pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            pp.BackBufferCount = 1;
            pp.BackBufferFormat = fmt;
            pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

            RECT WindowRect;
            GetClientRect(m_hwnd, &WindowRect);
            m_DisplaySize.cx = WindowRect.right - WindowRect.left;
            m_DisplaySize.cy = WindowRect.bottom - WindowRect.top;
        }

        hr = m_pDevice->Reset(&pp);
        CHECK_HR(
            hr,
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed to reset device, error code 0x%08x"), hr));

        hr = m_pDevice->CreateQuery(D3DQUERYTYPE_EVENT, &m_pSpeedControlQuery);


        CHECK_HR(
            hr,
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed to create Query, error code 0x%08x"), hr));


        //maximum ambient light
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_AMBIENT,RGB(255,255,255)),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetRenderState(D3DRS_AMBIENT...)")));

        //lighting disabled
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_LIGHTING,FALSE),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetRenderState(D3DRS_LIGHTING...)")));

        //don't cull backside
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetRenderState(D3DRS_CULLMODE...)")));

        //DISABLE depth buffering
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetRenderState(D3DRS_ZENABLE...)")));

        // enable dithering
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetRenderState(D3DRS_DITHERENABLE...)")));

        // disable stensil
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetRenderState(D3DRS_STENCILENABLE...)")));

        // manage blending
        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetRenderState(D3DRS_ALPHABLENDENABLE...)")));

        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetRenderState(D3DRS_SRCBLEND...)")));

        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetRenderState(D3DRS_DESTBLEND...)")));

        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetRenderState(D3DRS_ALPHATESTENABLE...)")));

        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_ALPHAREF, 0x10),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetRenderState(D3DRS_ALPHAREF...)")));

        CHECK_HR(
            m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetRenderState(D3DRS_ALPHAFUNC...)")));

        // set up sampler

        CHECK_HR(
            m_pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetSamplerState(D3DSAMP_ADDRESSU...)")));

        CHECK_HR(
            m_pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetSamplerState(D3DSAMP_ADDRESSV...)")));

        CHECK_HR(
            m_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetSamplerState(D3DSAMP_MAGFILTER...)")));

        CHECK_HR(
            m_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR),
            ::DbgMsg(_T("S3RenderEngine::ResetDevice_: failed in SetSamplerState(D3DSAMP_MINFILTER...)")));

        // turn on S3 Signage Driver Path
	    IDirect3DTexture9 *pSignageAppTexture = NULL;
        if(m_pDevice->CreateTexture(1,1,1, 0, (D3DFORMAT)MAKEFOURCC('S', '3', 'S', 'A'), D3DPOOL_DEFAULT, &pSignageAppTexture, NULL) == S_OK)
        {
            pSignageAppTexture->Release();
        }
    }// try
	catch( HRESULT hr1 )
	{
		hr = hr1;
	}
    return hr;
}


/******************************Private*Routine******************************\
* Clean_
*
* clean all the data, release all interfaces
\**************************************************************************/
void S3RenderEngine::Clean_()
{
#ifdef PLAYER_DUMMY
#else // PLAYER_DUMMY
    if(m_pSpeedControlQuery) {m_pSpeedControlQuery->Release(); m_pSpeedControlQuery = NULL;}
    if( m_pOwner ) { /*m_pOwner->Release();*/ m_pOwner = NULL; }
    if( m_pMixerControl ) { /*m_pMixerControl->Release();*/ m_pMixerControl = NULL; }
    if( m_pDevice ) { m_pDevice->Release(); m_pDevice = NULL; }
    if( m_pD3D9 )   { m_pD3D9->Release(); m_pD3D9 = NULL; }
    if( m_pDeviceManager) {m_pDeviceManager->Release(); m_pDeviceManager = NULL;};

#endif // PLAYER_DUMMY
}

BOOL S3RenderEngine::IsSupportMAMMRendering()
{
	if(EPlayerType == S3_PLAYERW
       || EPlayerType == S3_PLAYERLED)
	{
		return FALSE;
	}
	else
	{
		DWORD dwVersion = 0; 
		DWORD dwMajorVersion = 0;
		dwVersion = GetVersion();
		dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));

		if(dwMajorVersion > 5) //Vista and Win7
		{
			return TRUE;

		}
		return FALSE;
	}
}

BOOL S3RenderEngine::IsSupportEVR()
{
    if(g_pfDXVA2CreateDirect3DDeviceManager9)
        return TRUE;
    return FALSE;
}


BOOL S3RenderEngine::IsMAMMEnabled()
{
    return m_bMAMMEnabled;
}


BOOL S3RenderEngine::IsEVREnabled()
{
    return m_bEVREnabled;
}


SIZE S3RenderEngine::GetDeviceSize()
{
    return m_DisplaySize;
}

HRESULT S3RenderEngine::GetService(REFGUID guidService, REFIID riid, void** ppv)
{
    assert(ppv != NULL);

    HRESULT hr = S_OK;

    if (riid == __uuidof(IDirect3DDeviceManager9))
    {
        if (m_pDeviceManager == NULL)
        {
            hr = MF_E_UNSUPPORTED_SERVICE;
        }
        else
        {
            *ppv = m_pDeviceManager;
            m_pDeviceManager->AddRef();
        }
    }
    else
    {
        hr = MF_E_UNSUPPORTED_SERVICE;
    }

    return hr;
}


//-----------------------------------------------------------------------------
// CheckFormat
//
// Queries whether the D3DPresentEngine can use a specified Direct3D format.
//-----------------------------------------------------------------------------

HRESULT S3RenderEngine::CheckFormat(D3DFORMAT format)
{
    HRESULT hr = S_OK;

    UINT uAdapter = D3DADAPTER_DEFAULT;
    D3DDEVTYPE type = D3DDEVTYPE_HAL;

    D3DDISPLAYMODE mode;
    D3DDEVICE_CREATION_PARAMETERS params;

    if (m_pDevice)
    {
        hr = m_pDevice->GetCreationParameters(&params);
        if (FAILED(hr))
        {
            goto done;
        }

        uAdapter = params.AdapterOrdinal;
        type = params.DeviceType;

    }

    hr = m_pD3D9->GetAdapterDisplayMode(uAdapter, &mode);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_pD3D9->CheckDeviceFormat(uAdapter, type,
    mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, format);

done:
    return hr;
}


HRESULT S3RenderEngine::Pause()
{
    CAutoLock Lock(&m_REObjectLock);
    m_bPaused = TRUE;
    return S_OK;
}

HRESULT S3RenderEngine::Resume()
{
    CAutoLock Lock(&m_REObjectLock);
    m_bPaused = FALSE;
    return S_OK;
}