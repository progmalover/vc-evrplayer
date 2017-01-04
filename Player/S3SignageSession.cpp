#include "stdafx.h"

#include "S3RenderEngine.h"
#include "S3RenderMixer.h"
#include "S3RenderScheduler.h"
#include "S3VMRWizard.h"
#include "S3EVRWizard.h"
#include "S3SignagePipe.h"
#include "S3D3DMath.h"
#include "S3MovieObj.h"
#include "S3SoundObj.h"
#include "S3Signage.h"
#include "S3SignageSession.h"
#include "Utilities/SysCall.h"
#include "Utilities/Utils.h"
#include "Utilities/ProcessCall.h"
#include "Localization/Localization.h"

const TCHAR g_achVideoWindowClass[] = TEXT("S3 Signage Window class");

extern UINT WM_DEVICE_LOST;
extern UINT WM_DEVICE_HANG;

S3SignageSession::S3SignageSession(void)
    : m_hwndVideo( NULL )
    , m_pWizard( NULL )
    , m_pRenderEngine( NULL )
    , m_pMixerControl( NULL )
    , m_pRenderScheduler( NULL )
{
}


S3SignageSession::~S3SignageSession(void)
{
    Terminate();

    SAFE_DELETE(m_pPipeControl);
    SAFE_DELETE(m_pWizard);
    SAFE_DELETE(m_pRenderEngine);
    SAFE_DELETE(m_pMixerControl);
    SAFE_DELETE(m_pRenderScheduler);
    SAFE_DELETE(m_pWizard);
}




/******************************Public*Routine******************************\
* Terminate
*
* terminates wizard
\**************************************************************************/
HRESULT S3SignageSession::Terminate()
{
    HRESULT hr = S_OK;

    try
    {
        if(m_pPipeControl)    m_pPipeControl->StopPipe();

        if(m_pRenderScheduler) m_pRenderScheduler->Terminate();

        if(m_pMixerControl)    m_pMixerControl->Terminate();

        // 2. Terminate Wizard, if there is any
        if( m_pWizard )
        {
            hr = m_pWizard->Terminate();
            if( FAILED(hr))
                throw hr;
        }
        // 3. Destroy video window
        if( IsWindow( m_hwndVideo ))
        {
            ::DestroyWindow( m_hwndVideo );
            m_hwndVideo = NULL;
        }

        UnregisterClass( g_achVideoWindowClass, g_hInstance);
    }
    catch( HRESULT hr1)
    {
        hr = hr1;
    }

    if(theApp.m_pMainWnd)
    {
        theApp.m_pMainWnd->Detach();
        delete theApp.m_pMainWnd;
        theApp.m_pMainWnd = NULL;
    }
    return hr;
}

HRESULT S3SignageSession::ProcessWindowsMessages(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(uMsg == WM_CLOSE)
    {
        Terminate();
        PostQuitMessage(0);
    }
    else if(uMsg == WM_KEYDOWN)
    {
		if(m_LaunchSetting.HotKey != 0)
		{	
			std::vector<int> key;
			BOOL bHotKey = TRUE;
			WORD wModifiers = (WORD)(m_LaunchSetting.HotKey>>16 & 0xFFFF);
			WORD wVirtualKeyCode = (WORD)(m_LaunchSetting.HotKey & 0xFFFF);

			if (wModifiers & HOTKEYF_SHIFT)
			{
				key.push_back(VK_SHIFT);
			}

			if (wModifiers & HOTKEYF_CONTROL)
			{
				key.push_back(VK_CONTROL);
			}

			if (wModifiers & HOTKEYF_ALT)
			{
				key.push_back(VK_MENU);
			}

			if (wVirtualKeyCode!=0)
			{
				key.push_back(wVirtualKeyCode);
			}

			vector<int> ::iterator it = key.begin();
			//SHORT keystate = 0, num = 0;
			for (; it != key.end(); it++)
			{
				if((GetKeyState(*it) & 0X8000) == 0)
				{
					bHotKey = FALSE;
					break;
				}
			}

			if (bHotKey)
			{
				if (m_LaunchSetting.ExitPassword == _T(""))
				{
					Terminate();
					if(lParam == 1)
					{
						TimerMessageBox(NULL, _T("Device lost!"), _T("Unexcepted Error"), MB_OK);
					}
					PostQuitMessage(0);
				}
				else
				{
					if(m_LaunchSetting.ExitPassword.Compare(m_InputHistory) == 0)
					{
						Terminate();
						if(lParam == 1)
						{
							TimerMessageBox(NULL, _T("Device lost!"), _T("Unexcepted Error"), MB_OK);
						}
						PostQuitMessage(0); 
					}else
					{
						m_InputHistory = _T("");
					}
				}
			}
		}
        /*
        if(wParam == VK_ESCAPE && m_LaunchSetting.ExitPassword == _T(""))
                {
                    Terminate();
                    if(lParam == 1)
                    {
                        ::MessageBox(NULL, _T("Device lost!"), _T("Unexcepted Error"), MB_OK);
                    }
                    PostQuitMessage(0);
                }
        
                if(wParam == VK_RETURN && m_LaunchSetting.ExitPassword != _T(""))
                {
                    if(m_LaunchSetting.ExitPassword.Compare(m_InputHistory) == 0)
                    {
                        Terminate();
                        if(lParam == 1)
                        {
                            ::MessageBox(NULL, _T("Device lost!"), _T("Unexcepted Error"), MB_OK);
                        }
                        PostQuitMessage(0); 
                    }else
                    {
                        m_InputHistory = _T("");
                    }
                }*/
        
    }else if(uMsg == WM_CHAR)
    {
        TCHAR InputChar = wParam;

        if(InputChar == VK_RETURN )
        {
        }else if(InputChar == VK_BACK)
        {
            m_InputHistory = m_InputHistory.Left(max(0,m_InputHistory.GetLength() - 1));
        }
        else
        {
            m_InputHistory += InputChar;
        }
    }
    else if(uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
    {
        HRESULT hr = S_FALSE;
        if(m_pMixerControl)
        {
            hr = m_pMixerControl->ProcessMouseMessages(uMsg, wParam, lParam);
        }
    }
    else if(uMsg == WM_TIMER)
    {
        if(S3S_MAIN_TIMER == wParam)
        {
            if(m_pRenderScheduler)
            {
                m_pRenderScheduler->UpdateRenderObjects();
            }
            if(m_pPipeControl)
            {
                m_pPipeControl->ProcessPipe();
            }
        }
    }
    else if(uMsg == WM_POWERBROADCAST)
    {         
        HRESULT hr = S_FALSE;
        if(m_pMixerControl)
        {
            hr = m_pMixerControl->ProcessMessage(uMsg, wParam, lParam);
        }
    }
    else if(uMsg == WM_DEVICE_LOST)
    {
        m_pRenderEngine->ProcessLostDevice();
    }
    else if(uMsg == WM_DEVICE_HANG)
    {
        m_pPipeControl->SetDeviceHang(TRUE);
    }

    return S_OK;
}

static HRESULT FindDecoderFilter(BOOL isEvrEnabled)
{
    const IID CLSID_FFDShowDecoder = {0x04fe9017, 0xf873, 0x410e, {0x87, 0x1e, 0xab, 0x91, 0x66, 0x1a, 0x4e, 0xf7}};
    const IID CLSID_DXVADShowDecoder = {0x4bd951fd, 0x84aa, 0x4dcf, {0xb8, 0x08, 0x5f, 0xe7, 0xf7, 0x30, 0x83, 0xcc}};
    const IID CLSID_PDVDDecoder = {0xc16541ff, 0x49ed, 0x4dea, {0x91, 0x26, 0x86, 0x2f, 0x57, 0x72, 0x2e, 0x31}};
    const IID CLSID_VTDecoder = {0x971a86ba, 0xf54d, 0x47d4, {0xa3, 0x77, 0x50, 0x9c, 0x72, 0xbe, 0xe0, 0xf7}};

    HRESULT hr = S_OK;

    if(isEvrEnabled)
    {
        {
            CComPtr<IBaseFilter>    pDecoderFilter;
            //hr = CoCreateInstance(CLSID_VTDecoder, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&(pDecoderFilter.p) );
            hr = CoCreateInstance(CLSID_VTDecoder, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&(pDecoderFilter) );
            if(SUCCEEDED(hr))
            {

            }
        }

        CComPtr<IBaseFilter>    pDecoderFilter;

        //hr = CoCreateInstance(CLSID_DXVADShowDecoder, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(pDecoderFilter.p) );
        hr = CoCreateInstance(CLSID_DXVADShowDecoder, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(pDecoderFilter.p) );

        if(FAILED(hr))
        {
            //hr = CoCreateInstance(CLSID_FFDShowDecoder, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(pDecoderFilter.p) );
            hr = CoCreateInstance(CLSID_FFDShowDecoder, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(pDecoderFilter) );
        }

        if(SUCCEEDED(hr))
        {
            pDecoderFilter.Release();
        }
    }
    //else
    {
        CComPtr<IBaseFilter>    pDecoderFilter;
        //hr = CoCreateInstance(CLSID_FFDShowDecoder, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(pDecoderFilter.p) );
        hr = CoCreateInstance(CLSID_FFDShowDecoder, NULL, CLSCTX_INPROC_SERVER,	IID_IBaseFilter, (void**)&(pDecoderFilter) );

        if(SUCCEEDED(hr))
        {
            pDecoderFilter.Release();

        }
    }

    return hr;
}


/*********************************************************************************\
* Initialize
* loads and initializes custom mixer control and UI Layer
\*********************************************************************************/

//todo init canvas
HRESULT S3SignageSession::Initialize(LAUNCH_SETTINGS *pLaunchSetting)
{
    HRESULT hr = S_OK;

#ifdef _DEBUG
    BOOL    bFullScreen = FALSE;
#else
    BOOL    bFullScreen = TRUE;
#endif

	if(EPlayerType == S3_PLAYERW || EPlayerType == S3_PLAYERLED)
	{
		bFullScreen = FALSE;
	}

    BOOL bEnableMAMM;
    BOOL bEnableEVR;

    try
    {
        if( m_pWizard )
            throw VFW_E_WRONG_STATE;
		

        m_pRenderEngine = new S3RenderEngine();
        CHECK_HR( hr = m_pRenderEngine? S_OK:E_FAIL, DbgMsg(_T("S3SignageSession::Initialize: Failed to create S3RenderEngine, hr = 0x%08x"), hr));

        m_pMixerControl = new S3RenderMixer();
        CHECK_HR( hr = m_pMixerControl? S_OK:E_FAIL, DbgMsg(_T("S3SignageSession::Initialize: Failed to create S3RenderMixer, hr = 0x%08x"), hr));

        m_pRenderScheduler = new S3RenderScheduler(m_pMixerControl);
        CHECK_HR( hr = m_pRenderScheduler? S_OK:E_FAIL, DbgMsg(_T("S3SignageSession::Initialize: Failed to create S3RenderScheduler, hr = 0x%08x"), hr));

        m_pPipeControl = new S3SignagePipe();

        m_LaunchSetting = *pLaunchSetting;
  
        bEnableMAMM = bFullScreen && m_LaunchSetting.bEnableMAMM && m_pRenderEngine->IsSupportMAMMRendering();

        bEnableEVR = m_LaunchSetting.bEnableEVR && m_pRenderEngine->IsSupportEVR() && !bEnableMAMM;
#ifndef PLAYER_DUMMY
        if(FindDecoderFilter(bEnableEVR) != S_OK)
        {
            //SysCall::MessageBoxTimeout(NULL,Translate(_T("S3 Filter not installed,some features will be disabled!")),Translate(_T("Information")),MB_OK, 0, 3000);
            MagicView::CommonUI::XMSGBOXPARAMS xmb;

            xmb.bUseUserDefinedButtonCaptions = TRUE;
            if (MagicView::Utils::CheckVEPD())
                _tcscpy(xmb.UserDefinedButtonCaptions.szReport, Translate(_T("Get S3 DXVA Filter")));
            else
                _tcscpy(xmb.UserDefinedButtonCaptions.szReport, Translate(_T("Get S3 SW Filter")));
                xmb.dwReportUserData = (DWORD)_T("http://www.s3graphics.com/en/drivers/software.aspx");
            xmb.lpReportFunc = GetIt;
            xmb.nTimeoutSeconds = 5;
#define Translate(x) x
            ((CS3SignageApp*)AfxGetApp())->m_msgboxMgr.XMessageBox(NULL,
                Translate(_T("S3 Filter not installed,some features will be disabled!")),
                Translate(_T("Warning")),
                MB_OK|MB_ICONWARNING|MB_DONOTASKAGAIN,
                &xmb);
#undef Translate
        }
#endif
        if(bEnableEVR)
        {
            m_pWizard = new S3EVRWizard();
        }else
        {
            m_pWizard = new S3VMRWizard();
        }

        CHECK_HR( hr = m_pWizard? S_OK: E_FAIL, DbgMsg(_T("S3SignageSession::Initialize: Failed to create S3VMRWizard, hr = 0x%08x"), hr));


        int myScreenx = 0;
        int myScreeny = 0;
        int myWidth, myHeight;
        DWORD wStyle;

	if(EPlayerType == S3_PLAYERW)
	{
		myWidth = m_LaunchSetting.PreviewWidth;
        myHeight = m_LaunchSetting.PreviewHeight;
#ifdef _DEBUG
        wStyle = WS_OVERLAPPEDWINDOW;
#else
        wStyle = WS_OVERLAPPED |WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
#endif
	}
	else if (EPlayerType == S3_PLAYERLED)
	{
        myScreenx = m_LaunchSetting.screenx;
        myScreeny = m_LaunchSetting.screeny;
        myWidth   = m_LaunchSetting.PreviewWidth;
        myHeight  = m_LaunchSetting.PreviewHeight;
        wStyle    = WS_POPUP;
	}
    else
	{
		myWidth = GetSystemMetrics(SM_CXSCREEN);
        myHeight = GetSystemMetrics(SM_CYSCREEN);
        wStyle = WS_POPUP;
	}

        CHECK_HR(
            hr = CreateVideoWindow_(myScreenx, myScreeny, myWidth,myHeight,wStyle),
            DbgMsg(_T("S3SignageSession::Initialize: failed to create video window")));

	if(EPlayerType == S3_PLAYERW)
	{
		SetWindowText(m_hwndVideo, TString(_T("Player Window ")).append(ProcessCall::GetMyVersion()).c_str());
#ifndef _DEBUG
        SetWindowPos(m_hwndVideo, HWND_TOPMOST, 0, 0,
            0, 0,          // Ignores size arguments. 
            SWP_NOSIZE); 
#endif
	}
        // initialize render engine with custom mixer control and UI layer
        CHECK_HR(
            hr = m_pRenderEngine->Initialize( m_hwndVideo, NULL, m_pMixerControl, m_pRenderScheduler, bFullScreen, bEnableMAMM, bEnableEVR),
            DbgMsg(_T("S3SignageSession::Initialize: failed to initialize render engine, hr = 0x%08x"), hr));

        m_pMixerControl->m_DisplaySize = m_pRenderEngine->GetDeviceSize();
        m_pMixerControl->m_RotationDegree = m_LaunchSetting.fRotationDegree;

        // set mixer's clear color
        m_pMixerControl->m_ClearColor = 0x00000000;


        // initialize wizard with customized render engine
        CHECK_HR(
            hr = m_pWizard->Initialize(NULL, m_hwndVideo, m_pRenderEngine),
            DbgMsg(_T("S3SignageSession::Initialize: failed to initialize the wizard, hr = 0x%08x"), hr));

        CHECK_HR(
            hr = m_pRenderEngine->SetFrameRate( pLaunchSetting->nFPS ),
            DbgMsg(_T("S3SignageSession::Initialize: failed to set frame rate, hr = 0x%08x"), hr));

        CHECK_HR(
            hr = m_pRenderScheduler->InitializeScheduler(pLaunchSetting->nFPS, pLaunchSetting->MediaLibraryLocation),
            DbgMsg(_T("S3SignageSession::Initialize: failed to initialize the scheduler, hr = 0x%08x"), hr));


        if(pLaunchSetting->bFitScreen)
        {
            SIZEL ScreenSize = m_pRenderEngine->GetDeviceSize();
            
            BOOL bFillScreen = TRUE;
            if (abs(m_LaunchSetting.RotationDegree) % 90 == 0)
            {
                bFillScreen = FALSE;
            }

            m_pRenderScheduler->SetDisplayConfigure(ScreenSize.cx, ScreenSize.cy, m_LaunchSetting.fRotationDegree, TRUE, bFillScreen);
        }

        if(pLaunchSetting->ProgramPath != _T(""))
        {
            m_pRenderScheduler->LoadPlayList(pLaunchSetting->ProgramPath);
        }
		else
		{
			m_pRenderScheduler->initCanvas();
		}

        m_pPipeControl->Initalize(m_pRenderScheduler, m_pMixerControl);

        m_pPipeControl->StartPipe();
#ifndef PLAYER_DUMMY
        ::ShowWindow( m_hwndVideo, SW_SHOW);
#endif
    }
    catch( HRESULT hr1 )
    {

        if( m_pRenderEngine )
        {
            m_pRenderEngine->Terminate();
        }

        hr = hr1;
    }
    m_pWizard->AddRef();

    return hr;
}



/******************************Public*Routine******************************\
* CreateVideoWindow_
*
* creates video window
\**************************************************************************/
HRESULT S3SignageSession::CreateVideoWindow_(UINT x, UINT y, UINT Width, UINT Height, DWORD dwStyle)
{
    HRESULT hr = S_OK;
    WNDCLASSEX wcex;
    try
    {
        if( IsWindow( m_hwndVideo ))
            throw E_UNEXPECTED;

        wcex.cbSize = sizeof(WNDCLASSEX); 
        wcex.style          = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc    = (WNDPROC)S3SignageSession::VideoWndProc;
        wcex.cbClsExtra     = 0;
        wcex.cbWndExtra     = 0;
        wcex.hInstance      = g_hInstance;
        wcex.hIcon          = NULL;
        wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
        wcex.lpszMenuName   = TEXT("");
        wcex.lpszClassName  = g_achVideoWindowClass;
        wcex.hIconSm        = NULL;

        RegisterClassEx( &wcex);

        m_hwndVideo  = CreateWindow(    g_achVideoWindowClass, 
                                        TEXT("S3 Signage Window"), 
                                        dwStyle,
                                        x, y, 
                                        Width, Height, 
                                        NULL, 
                                        NULL, 
                                        g_hInstance, 
                                        NULL);
        if( !IsWindow( m_hwndVideo ))
            throw E_FAIL;


        ::SetWindowLongPtr( m_hwndVideo, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    }// try
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }

    return hr;
}

/******************************Public*Routine******************************\
* VideoWndProc
*
\**************************************************************************/
LRESULT CALLBACK S3SignageSession::VideoWndProc(
                                        HWND hwnd,      // handle to window
                                        UINT uMsg,      // message identifier
                                        WPARAM wParam,  // first message parameter
                                        LPARAM lParam   // second message parameter
                                        )
{
    S3SignageSession* This = NULL;
    LRESULT lRes = 0;

    This = reinterpret_cast<S3SignageSession*>(::GetWindowLongPtr( hwnd, GWLP_USERDATA));

    if(uMsg == WM_SYSCOMMAND && wParam == SC_SCREENSAVE)
        return 0;

    if(uMsg == WM_ERASEBKGND)
        return 1;

    if(uMsg == WM_POWERBROADCAST && wParam == PBT_APMRESUMEAUTOMATIC)
    {
        mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0); 
        mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0); 
    }

    if(uMsg == WM_ERASEBKGND)
    {
		if(EPlayerType != S3_PLAYERW)
		{
        #ifndef PLAYER_DUMMY
            mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0); 
            mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0); 
        #endif
		}
    }

    lRes = DefWindowProc( hwnd, uMsg, wParam, lParam);
    if(This)
    {
		if(EPlayerType == S3_PLAYERW)
		{
			This->ProcessWindowsMessages(uMsg, wParam, lParam);
		}
		else
		{
			return This->ProcessWindowsMessages(uMsg, wParam, lParam);
		}

    }
    return lRes;
}

S3DShowWizard*  S3SignageSession::GetWizard( )
{
    return m_pWizard;
}

S3RenderEngine* S3SignageSession::GetRenderEngine( )
{
    return m_pRenderEngine;
}

HRESULT S3SignageSession::Start()
{
    if(m_LaunchSetting.ProgramPath != _T(""))
    {
        m_pRenderScheduler->Play();;
    }

    return S_OK;
}