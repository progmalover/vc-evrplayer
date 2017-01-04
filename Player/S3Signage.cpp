#include "stdafx.h"
#include "S3VMRWizard.h"
#include "S3RenderEngine.h"
#include "S3RenderMixer.h"
#include "S3Signage.h"
#include "S3SignageSession.h"
#include "S3MovieObj.h"
#include "S3SoundObj.h"
#include "Utilities/SysCall.h"
#include "RenderableObject/S3Flash/S3Flash.h"
#include "Localization/Localization.h"
#include "Client/ClientConfigFile.h"
#include "Utilities/CrashRptHelper.h"
#include "Utilities/Utils.h"
#include "Utilities/StringUtility.h"

#include <VMProtectSDK.h>
#include "boost/program_options.hpp"
#include "boost/typeof/typeof.hpp"
using namespace boost::program_options;
using namespace boost::program_options::command_line_style;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
#pragma comment(lib, "libboost_program_options-vc100-mt-gd-1_48")
#else
#pragma comment(lib, "libboost_program_options-vc100-mt-1_48")
#endif
//#define DETECT_ADAPTER
#ifdef PLAYER_DUMMY
#include "ControllerClient/ControllerClientInterface.h"
ConnectionManager m_connManager;
#endif


HINSTANCE g_hInstance = NULL;
BOOL g_bDEMO = FALSE;
S3_PLAYERTYPE EPlayerType;
// CS3SignageApp

BEGIN_MESSAGE_MAP(CS3SignageApp, CWinApp)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


//
//typedef LONG (WINAPI *PFNQUERYDISPLAYCONFIG)(
//__in     UINT32 Flags,
//__inout  UINT32 *pNumPathArrayElements,
//__out    DISPLAYCONFIG_PATH_INFO *pPathInfoArray,
//__inout  UINT32 *pNumModeInfoArrayElements,
//__out    DISPLAYCONFIG_MODE_INFO *pModeInfoArray,
//__out    DISPLAYCONFIG_TOPOLOGY_ID *pCurrentTopologyId
//);
//
//typedef LONG (WINAPI *PFNSETDISPLAYCONFIG)(
//__in      UINT32 numPathArrayElements,
//__in_opt  DISPLAYCONFIG_PATH_INFO *pathArray,
//__in      UINT32 numModeInfoArrayElements,
//__in_opt  DISPLAYCONFIG_MODE_INFO *modeInfoArray,
//__in      UINT32 Flags
//);
//
//
//typedef LONG (WINAPI *PFNGETDISPLAYCONFIGBUFFERSIZES)(
//__in   UINT32 Flags,
//__out  UINT32 *pNumPathArrayElements,
//__out  UINT32 *pNumModeInfoArrayElements
//);

// CS3SignageApp construction

CS3SignageApp::CS3SignageApp()
#ifdef PLAYER_DUMMY
    : m_StartTime(S3Time::Now()),
    m_crashrptHelper(_T("S3 MagicView Player"))
#else
    : m_crashrptHelper(_T("S3 MagicView Player"))
#endif
{
    m_pMainWnd = NULL;
    m_pGetClassObject = NULL;
    m_dwMajorOSVersion = (DWORD)(LOBYTE(LOWORD(GetVersion())));
	m_licenseChkResult = MagicView::Utils::INVALID_LICENSE;
}

CS3SignageApp::~CS3SignageApp()
{

}
// The one and only CS3SignageApp object

CS3SignageApp theApp;


//----------------------------------------------------------------------------
//  VerifyVMR9
//
//  Verifies that VMR9 COM objects exist on the system and that the VMR9
//  can be instantiated.
//
//  Returns: FALSE if the VMR9 can't be created
//----------------------------------------------------------------------------

BOOL CS3SignageApp::VerifyVMR9(void)
{
    HRESULT hr;

    // Verify that the VMR exists on this system
    IBaseFilter* pBF = NULL;
    hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL,
                          CLSCTX_INPROC,
                          IID_IBaseFilter,
                          (LPVOID *)&pBF);
    if(SUCCEEDED(hr))
    {
        pBF->Release();
        return TRUE;
    }
    else
    {
        TimerMessageBox(NULL,
            TEXT("This application requires the Video Mixing Renderer, which is present\r\n")
            TEXT("only on DirectX 9 systems with hardware video acceleration enabled.\r\n\r\n")

            TEXT("The Video Mixing Renderer (VMR9) is not enabled when viewing a \r\n")
            TEXT("remote Windows XP machine through a Remote Desktop session.\r\n")
            TEXT("You can run VMR-enabled applications only on your local machine.\r\n\r\n")

            TEXT("To verify that hardware acceleration is enabled on a Windows XP\r\n")
            TEXT("system, follow these steps:\r\n")
            TEXT("-----------------------------------------------------------------------\r\n")
            TEXT(" - Open 'Display Properties' in the Control Panel\r\n")
            TEXT(" - Click the 'Settings' tab\r\n")
            TEXT(" - Click on the 'Advanced' button at the bottom of the page\r\n")
            TEXT(" - Click on the 'Troubleshooting' tab in the window that appears\r\n")
            TEXT(" - Verify that the 'Hardware Acceleration' slider is at the rightmost position\r\n")

            TEXT("\r\nThis sample will now exit."),

            TEXT("Video Mixing Renderer (VMR9) capabilities are required"), MB_OK);

        return FALSE;
    }
}

static HRESULT FindPPTViewer()
{
	WCHAR ppvpath[MAX_PATH];
	HKEY hKey = (HKEY) NULL;
	DWORD dwSize = MAX_PATH;
	CString PPVPath;
	int nameStart = 0;
	if( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\PowerPointViewer.SlideShow.12\\shell\\Show\\Command"), 0,KEY_QUERY_VALUE, &hKey))
	{
		if( ERROR_SUCCESS == RegQueryValueEx( hKey, NULL, NULL, NULL, (LPBYTE) &ppvpath,&dwSize ) ) 
		{
			PPVPath = ppvpath;
			PPVPath.MakeUpper();
			if(0 > PPVPath.Find(_T("PPTVIEW.EXE")))
			{
				return E_FAIL;
			}
		}
		else
		{
			return E_FAIL;
		}

	}
	else
	{
		return E_FAIL;
	}


	PPVPath.Replace(_T("\\"), _T("\\\\"));
	nameStart = PPVPath.Find(_T("PPTVIEW.EXE"));
	CString m_pptViewerPath = PPVPath.Mid(0, nameStart) + _T("PPTVIEW.EXE");
	return S_OK;
}
static HRESULT FindFlash()
{
	HRESULT hr = S_FALSE;
	// Find renderable obj

    CLSID clsidFlash = { 0xD27CDB6E, 0xAE6D, 0x11CF, { 0x96, 0xB8, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };
    GUID  iFlash = {0xD27CDB6C, 0xAE6D, 0x11CF,{ 0x96, 0xB8, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00} };
	CComPtr<IUnknown> pFlash = NULL;
	hr = CoCreateInstance(clsidFlash, NULL, CLSCTX_INPROC_SERVER,
		iFlash, (void**)&pFlash);
	if(SUCCEEDED(hr))
	{
		pFlash.Release();
		return S_OK;
	}
	else
    {
        return S_FALSE;
    }
}

void GetIt(LPCTSTR lpszMessageText, DWORD dwUserData)
{
    SysCall::BrowseURL((TCHAR*)dwUserData);
}

#pragma optimize("g", off)
// CS3SignageApp initialization
BOOL CS3SignageApp::InitInstance()
{
    VMP_BEGIN("CS3SignageApp::InitInstance");
	if(!ParseCommandLine())
    {
		return FALSE;
	}
   
#ifndef PLAYER_DUMMY
	if(EPlayerType != S3_PLAYERW)
	{
	    VMProtectSerialNumberData sd = {0};
	    m_licenseChkResult = MagicView::Utils::LicenseValidate(sd);
        if (m_licenseChkResult == MagicView::Utils::INVALID_LICENSE)
        {
            DbgMsg(_T("LicenseValidate failed, exiting..."));
            return FALSE;
        }
		
		if(EPlayerType == S3_MAMMPLAYER)
		{
	        if (m_licenseChkResult != (MagicView::Utils::HARDWAREKEY_LICENSE))
	        {
	            TimerMessageBox(NULL, _T("MagicView client in MAMM mode only supports hardware key."), _T("S3Signage.exe"), MB_OK|MB_ICONERROR);
	            DbgMsg(_T("LicenseValidate failed, exiting..."));
	            return FALSE;
	        }
	    }

	    if (m_licenseChkResult == (MagicView::Utils::VMP_LICENSE))
		{
			if( sd.dtExpire.wYear == 0 && sd.dtExpire.bMonth == 0 && sd.dtExpire.bDay == 0)
				g_bDEMO = FALSE;
			else
				g_bDEMO = TRUE;
		}
	}

#endif
    // init rand seed
    time_t now;
    time(&now);
    srand((unsigned int)now);
	/*
	ClientConfigFile myConfigFile = ClientConfigFile::GetConfigFile();
	CString szCurLanguage;
	szCurLanguage = myConfigFile.Get(CLIENT_LANGUAGE).c_str();
	if(szCurLanguage == _T(""))
	{
		szCurLanguage = _T("English");
	}
	std::map<CString,CString>m_LanNameMap = GetLanguageNameMap();
	if(m_LanNameMap.size())
	{
		map<CString ,CString>::iterator it;
		int i = 0;
		for(it = m_LanNameMap.begin();it != m_LanNameMap.end();it++,i++)
		{
			if(szCurLanguage == it->first)
			{
				InitLocalzation(it->second);
				break;
			}
		}
	}
	*/

	// Check The adapter, only run on S3
    IDirect3D9* pD3D9 = NULL;
    while (true)
    {
        pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
        if (!pD3D9)
        {
            int ret = SysCall::MessageBoxTimeout(NULL, _T("Failed to call Direct3DCreate9! Click \"OK\" or wait 5 seconds to retry, or click \"Cancel\" to quit."), _T("Warning"), MB_OKCANCEL, 0, 5000);
            if (ret == IDCANCEL)
            {
                return FALSE;
            }
            continue;
        }
        else
        {
            break;
        }
    }

    D3DADAPTER_IDENTIFIER9 AdapterIdentify;

    pD3D9->GetAdapterIdentifier(0,0,&AdapterIdentify);
#if 0
#ifndef _DEBUG
	if(EPlayerType != S3_PLAYERW)
	{
		if((AdapterIdentify.VendorId != 0x5333) && (AdapterIdentify.VendorId != 0x00001106))
		{

			TimerMessageBox(NULL, _T("The Magic View Player does not support this adapter. Please plug in S3 Graphics Maxwall adaptors."), _T("S3Signage.exe"), MB_OK |MB_ICONEXCLAMATION);

			return E_FAIL;
		}
	}
#endif
#endif
    if(AdapterIdentify.VendorId == 0x1106)
    {
        m_LaunchSettings.bEnableEVR = TRUE;
    }

    RELEASE( pD3D9 );



    // InitCommonControls() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    InitCommonControls();

    CWinApp::InitInstance();

    SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
    // Initialize OLE libraries
	if (!AfxOleInit())
	{
		TimerMessageBox(NULL, _T("OLE initialization failed!"), _T("S3Signage.exe"), MB_OK |MB_ICONEXCLAMATION);
		return FALSE;
	}

	AfxEnableControlContainer();

    g_hInstance = m_hInstance;

    if(FAILED(CoInitialize(NULL)))
    {
        TimerMessageBox(NULL, _T("CoInitialize failed!"), _T("S3Signage.exe"), MB_OK |MB_ICONEXCLAMATION);
		return FALSE;
    }

    if (!m_pluginMgr.Init())
    {
        CoUninitialize();
        return FALSE;
    }

    m_pluginMgr.SetHost(this);

/*
	if(!ParseCommandLine())
	{
		CoUninitialize();
		return FALSE;
	}*/

#ifndef PLAYER_DUMMY
	CString path = GetLanchSettings().LogPath;
	if (path.IsEmpty())
	{
		TCHAR modulePath[MAX_PATH];
		::GetModuleFileName(NULL, modulePath, MAX_PATH);
		path = StringUtility::GetFolder(modulePath).c_str();
	}
    m_msgboxMgr.SetIniPath((path + _T("\\msgbox.ini")).GetString());
#endif

	std::map<CString,CString>m_LanNameMap = GetLanguageNameMap();
	CString szCurLanguage = ChangeEngToLocLanName(m_LaunchSettings.CurLanguage);
	if(m_LanNameMap.size())
	{
		map<CString ,CString>::iterator it;
		int i = 0;
		for(it = m_LanNameMap.begin();it != m_LanNameMap.end();it++,i++)
		{
			//if(m_LaunchSettings.CurLanguage == it->first)
			if(szCurLanguage == it->first)
			{
				InitLocalzation(it->second);
				break;
			}
		}
	}

#ifndef PLAYER_DUMMY
    // Verify that the VMR9 is present on this system
    if(!VerifyVMR9())
    {
        CoUninitialize();
        return FALSE;
    }


	if(FindPPTViewer() != S_OK)
	{
		//SysCall::MessageBoxTimeout(NULL,Translate(_T("PowerPointViewer not installed,some features will be disabled!")),Translate(_T("Information")),MB_OK, 0, 3000);
        MagicView::CommonUI::XMSGBOXPARAMS xmb;

        xmb.bUseUserDefinedButtonCaptions = TRUE;
        _tcscpy(xmb.UserDefinedButtonCaptions.szReport, Translate(_T("Get PPT Viewer")));
        xmb.dwReportUserData = (DWORD)_T("http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=6");
        xmb.lpReportFunc = GetIt;
        xmb.nTimeoutSeconds = 5;
#define Translate(x) x
        m_msgboxMgr.XMessageBox(NULL,
            Translate(_T("PowerPointViewer not installed,some features will be disabled!")),
            Translate(_T("Warning")),
            MB_OK|MB_ICONWARNING|MB_DONOTASKAGAIN,
            &xmb);
#undef Translate
	}

	if(FindFlash() != S_OK)
	{
		//SysCall::MessageBoxTimeout(NULL,Translate(_T("Flash Player not installed,some features will be disabled!")),Translate(_T("Information")),MB_OK, 0, 3000);
        MagicView::CommonUI::XMSGBOXPARAMS xmb;

        xmb.bUseUserDefinedButtonCaptions = TRUE;
        _tcscpy(xmb.UserDefinedButtonCaptions.szReport, Translate(_T("Get Flash Player")));
        xmb.dwReportUserData = (DWORD)_T("http://get.adobe.com/flashplayer/");
        xmb.lpReportFunc = GetIt;
        xmb.nTimeoutSeconds = 5;
#define Translate(x) x
        m_msgboxMgr.XMessageBox(NULL,
            Translate(_T("Flash Player not installed,some features will be disabled!")),
            Translate(_T("Warning")),
            MB_OK|MB_ICONWARNING|MB_DONOTASKAGAIN,
            &xmb);
#undef Translate
	}

#endif

    


#ifdef PLAYER_DUMMY
    ::AllocConsole();
//     freopen("CONIN$", "r+t", stdin);
    freopen("CONOUT$", "w+t", stdout);
    CString dummyLogName = ((CS3SignageApp*)AfxGetApp())->GetLanchSettings().LogPath +
        _T("\\PlayerDummy") + m_LaunchSettings.LogPostfix + _T(".log");
    g_pLogger = new CDualLogger(dummyLogName.GetString());

    LOG(g_pLogger, INFO) << _T("PlayerDummy") << m_LaunchSettings.LogPostfix.GetString() << _T(" Started");
    LOG(g_pLogger, INFO) << _T("Arguments: ") << m_lpCmdLine;

    m_ServerCommunication.Init();
    m_ServerCommunication.StartCommunicate();
#endif


    DbgMsg(_T(""));
    DbgMsg(_T("****************Player Started***********************"));

    m_pSession = new S3SignageSession();
    if(!m_pSession)
    {
        TimerMessageBox(NULL, TEXT("Failed to create a new CS3SignageSession"), _T("S3Signage.exe"), MB_OK);
        CoUninitialize();
        return FALSE;
    }

    if( FAILED(m_pSession->Initialize(&m_LaunchSettings)))
    {
        TimerMessageBox(NULL, TEXT("Error: Unsupported Device!"), _T("S3Signage.exe"), MB_OK);
        PostQuitMessage(-1);
        return TRUE;
    }



    m_pMainWnd =  new CWnd;
    if(!m_pMainWnd)
    {
        TimerMessageBox(NULL, TEXT("Failed to allocate MainWnd"), _T("S3Signage.exe"), MB_OK);
        PostQuitMessage(-1);
        return TRUE;
    }

    m_pMainWnd->Attach(m_pSession->GetWindow());

    m_pSession->Start();

    m_pMainWnd->SetTimer(S3S_MAIN_TIMER, S3S_MAIN_TIMER_DURATION, 0);
#if 0
	SignageShockwaveflash *m_pFlash = new SignageShockwaveflash;
	if(!m_pFlash)
	{
		TimerMessageBox(NULL, Translate(_T("Flash Player not installed,some features will be disabled!")),Translate(_T("information")));
	}
	RECT rect;
	LPRECT rect2;
	m_pMainWnd->GetWindowRect(rect2);
	rect.left = rect2->left;
	rect.right = rect2->right;
	rect.top = rect2->top;
	rect.bottom = rect2->bottom;
	if(!m_pFlash->Create(NULL, WS_CHILD|WS_VISIBLE, rect, m_pMainWnd, 0))
	{
		TimerMessageBox(NULL, Translate(_T("Flash Player not installed,some features will be disabled!")),Translate(_T("information")));
	}
	m_pFlash->DestroyWindow();
#endif

#ifndef _DEBUG
#ifndef PLAYER_DUMMY
	if(EPlayerType != S3_PLAYERW)
	{
	    if (m_licenseChkResult == MagicView::Utils::HARDWAREKEY_LICENSE)
		{
			bool status = m_licenseControl.StartMonitor();
			if (!status)
			{
				CoUninitialize();
				return FALSE;
			}
		}
	}
#endif
#endif

#ifndef _DEBUG   
	if(EPlayerType != S3_PLAYERW
       && EPlayerType != S3_PLAYERLED)
	{
		HWND hWnd;
		hWnd = FindWindow(_T("Shell_TrayWnd"),NULL);
		ShowWindow(hWnd,SW_HIDE);
		ShowCursor(FALSE);
	}
#endif


    return TRUE;
    VMP_END();
}
#pragma optimize("g", on)

BOOL CS3SignageApp::ParseCommandLine()
{
	//std::tstring cmdline4 = _T("Player.exe --playertype=playerw --fitscreen --previewwidth=400 --previewheight=300 --PipeName=\\.\Pipe\S3SignagePlayer33BE55DC-EE60-46F8-8690-43698798E07F --medialibrary=C:\ProgramData\MagicView\S3MagicViewProfessionalEdition\Media --curlanguage=English --logpath=C:\ProgramData\MagicView\S3MagicViewProfessionalEdition");  


	options_description opts;
	opts.add_options()
		("playertype", value<std::string>()->default_value("player"), "Player type")
		("file", value<std::string>(), "Program path")
		("medialibrary", value<std::string>(), "Media library path")
		("password", value<std::string>(), "exit password")
		("pipename", value<std::string>()->default_value("\\\\.\\Pipe\\S3SignagePlayer"), "Pipe Name")
		("serverip", value<std::string>()->default_value("127.0.0.1"), "Server ip")
		("serverport", value<int>()->default_value(2000), "server port")
		("modetype", value<int>()->default_value(1), "client work mode: online or offline")
		("accountname", value<std::string>()->default_value("CharlesClient"), "Account name")
		("accountpassword", value<std::string>()->default_value("CharlesClient"), "Account password")
		("clientname", value<std::string>()->default_value("StarterEditionClient"), "Client name")
		("clientlocation", value<std::string>()->default_value("StarterEditionClient"), "Client location")
		("company", value<std::string>(), "Company name")
		("adminname", value<std::string>()->default_value("StarterEditionClient"), "Admin name")
		("telephone", value<std::string>()->default_value("StarterEditionClient"), "Telephone Number")
		("email", value<std::string>()->default_value("StarterEditionClient"), "Email")
		("hardwareid", value<std::string>()->default_value("00000000"), "hardwareid")
		("updatehour", value<int>()->default_value(1), "Update hour")
		("layoutdescfilename", value<std::string>()->default_value("index.xml"), "the name of layout desci file")
		("curlanguage", value<std::string>()->default_value("english"), "Language")
		("logpostfix", value<std::string>(), "log postfix")
		("hotkey", value<int>()->default_value(0), "hot key exist player")
		("locallibrary", value<std::string>()->default_value("."), "local library path")
		("logpath", value<std::string>(), "log path")
        ("screenx", value<int>()->default_value(0), "window x")
        ("screeny", value<int>()->default_value(0), "window y")    
		("previewwidth", value<int>()->default_value(800), "preview window width")
        ("previewheight", value<int>()->default_value(600), "preview window height")     
        ("rotate", value<int>()->default_value(0), "rotation angle")
		("fitscreen",  "whether fit screen size")
		("fps60", "Fps")
		("rotate90", "rotation 90")
		("rotate180", "rotation 180")
		("rotate270", "rotation 270")
		("EVR","EVR")
		("mamm","mamm")
		("preview","preview")
		;

	variables_map vm;

	//vector<std::tstring> args = split_winmain(cmdline4);
	try
	{
	#ifdef UNICODE
		//BOOST_AUTO(pr, wcommand_line_parser(args).options(opts).style(unix_style | case_insensitive).allow_unregistered().run());
		BOOST_AUTO(pr, wcommand_line_parser(__argc, __wargv).options(opts).style(unix_style | case_insensitive).allow_unregistered().run());
	#else
		BOOST_AUTO(pr, command_line_parser(__argc, __argv).options(opts).style(unix_style | case_insensitive).allow_unregistered().run());
	#endif
		store(pr, vm);
		notify(vm);
	}
	catch(exception &e)
	{
		//TRACE("%s", e.what());
		CString szError = e.what();
		DbgMsg(_T("command line error: %s"), szError);
		return FALSE;
	}

#define GET_STRING_OPTION(s,f) \
            if (vm.count(s)) \
            { \
                m_LaunchSettings.f = vm[s].as<std::string>().c_str(); \
            } \

#define GET_INT_OPTION(s,f) \
            if (vm.count(s)) \
            { \
                m_LaunchSettings.f = vm[s].as<int>(); \
            } \

#define GET_INPUT_OPTION(s,f,v) \
			if (vm.count(s)) \
			{ \
				m_LaunchSettings.f = v; \
			} \

	GET_STRING_OPTION("playertype", Playertype)
	m_LaunchSettings.bEnableMAMM = FALSE;
	if(m_LaunchSettings.Playertype.CompareNoCase(_T("mammplayer")) == 0)
	{
		EPlayerType = S3_MAMMPLAYER;
		m_LaunchSettings.bEnableMAMM = TRUE;
	}
	else if(m_LaunchSettings.Playertype.CompareNoCase(_T("player")) == 0)
	{
		EPlayerType = S3_PLAYER;
	}
	else if(m_LaunchSettings.Playertype.CompareNoCase(_T("playerw")) == 0)
	{
		EPlayerType = S3_PLAYERW;
	}
	else if(m_LaunchSettings.Playertype.CompareNoCase(_T("playerstarter")) == 0)
	{
		EPlayerType = S3_PLAYERSTARTER;
	}
	else if(m_LaunchSettings.Playertype.CompareNoCase(_T("playerdummy")) == 0)
	{
		EPlayerType = S3_PLAYERDUMMY;
	}
    else if (m_LaunchSettings.Playertype.CompareNoCase(_T("playerled")) == 0)
    {
        EPlayerType = S3_PLAYERLED;
    }

	GET_STRING_OPTION("file", ProgramPath)
	GET_STRING_OPTION("medialibrary", MediaLibraryLocation)
	GET_STRING_OPTION("password", ExitPassword)
	GET_STRING_OPTION("pipename", PipeName)
	GET_STRING_OPTION("serverip", ServerIP)
	GET_INT_OPTION("serverport", ServerPort)
	GET_STRING_OPTION("accountname", AccountName)
	GET_STRING_OPTION("accountpassword", AccountPassword)
	GET_STRING_OPTION("clientname", ClientName)
	GET_STRING_OPTION("clientlocation", ClientLocation)
	GET_STRING_OPTION("company", Company)
	GET_STRING_OPTION("accountname", AccountName)
	GET_STRING_OPTION("accountpassword", AccountPassword)
	GET_STRING_OPTION("adminname", AdminName)
	GET_STRING_OPTION("telephone", Telephone)
	GET_STRING_OPTION("email", Email)
	GET_STRING_OPTION("hardwareid", HardwareID)
	GET_INT_OPTION("updatehour", UpdateHour)
	GET_STRING_OPTION("layoutdescfilename", LayoutDescFileName)
	GET_STRING_OPTION("curlanguage", CurLanguage)
	GET_STRING_OPTION("logpostfix", LogPostfix)
	GET_INT_OPTION("hotkey", HotKey)
	GET_STRING_OPTION("locallibrary", LocalLibrary)
	GET_STRING_OPTION("logpath", LogPath)
    GET_INT_OPTION("screenx", screenx)
    GET_INT_OPTION("screeny", screeny)  
	GET_INT_OPTION("previewwidth", PreviewWidth)
    GET_INT_OPTION("previewheight", PreviewHeight)   
    GET_INT_OPTION("rotate", RotationDegree)
	GET_INPUT_OPTION("fitscreen", bFitScreen, TRUE)
    GET_INPUT_OPTION("fps60", nFPS, 60)      
    GET_INPUT_OPTION("rotate90", RotationDegree, 90)
    GET_INPUT_OPTION("rotate180", RotationDegree, 180)
    GET_INPUT_OPTION("rotate270", RotationDegree, 270)
	GET_INPUT_OPTION("EVR", bEnableEVR, TRUE)
	GET_INPUT_OPTION("mamm", bEnableMAMM, TRUE)

#undef GET_INPUT_OPTION
#undef GET_STRING_OPTION
#undef GET_INT_OPTION

    m_LaunchSettings.fRotationDegree = m_LaunchSettings.RotationDegree * PI / 180.0;

    m_LaunchSettings.ExitPassword = _T("");
    if (vm.count("modetype")) 
    { 
        m_LaunchSettings.Mode = (LAUNCH_SETTINGS::ModeType)(vm["modetype"].as<int>()); 
    }           

    return TRUE;
}


S3DShowWizard  *CS3SignageApp::GetWizard()
{
    return m_pSession->GetWizard();
}

S3RenderEngine *CS3SignageApp::GetRenderEngine()
{
    return m_pSession->GetRenderEngine();
}

const LAUNCH_SETTINGS& CS3SignageApp::GetLanchSettings()
{
    return m_LaunchSettings;
}

/*********************************************************************************\
* DbgMsg
* debug tracing function
\*********************************************************************************/

void DbgMsg(TCHAR * szMessage, ... )
{
    TCHAR szFullMessage[4096];
    TCHAR szFormatMessage[4096];
    DWORD dwWritten = 0L;

    // format message
    va_list ap;
    va_start(ap, szMessage);
    _vsntprintf_s( szFormatMessage, 4096, szMessage, ap);
    va_end(ap);
    _tcsncat_s( szFormatMessage, _T("\r\n"), 4096);

    CTime CurrentTime = CTime::GetCurrentTime();
    _tcscpy_s( szFullMessage, CurrentTime.Format(_T("%Y-%m-%d %H:%M:%S ")));
    _tcscat_s( szFullMessage, szFormatMessage );
    OutputDebugString( szFullMessage );

    CString logFileName = ((CS3SignageApp*)AfxGetApp())->GetLanchSettings().LogPath + 
        _T("\\Player") + ((CS3SignageApp*)AfxGetApp())->GetLanchSettings().LogPostfix + _T(".log");
    HANDLE hLog = CreateFile(logFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if( INVALID_HANDLE_VALUE != hLog )
    {
        SetFilePointer(hLog, 0,  NULL, FILE_END);
        WriteFile( hLog, (LPVOID)szFullMessage, (DWORD)_tcslen(szFullMessage) * sizeof(TCHAR), &dwWritten, NULL);
        FlushFileBuffers( hLog);
        CloseHandle(hLog);
    }
}



int CS3SignageApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	int retCode = CWinApp::ExitInstance();

#ifndef _DEBUG
	HWND hWnd;
	hWnd = FindWindow(_T("Shell_TrayWnd"),NULL);
	ShowWindow(hWnd,SW_SHOW);
	ShowCursor(TRUE);
#endif


#ifdef PLAYER_DUMMY
	m_ServerCommunication.StopCommunicate();
#endif


#ifndef _DEBUG
#ifndef PLAYER_DUMMY
	if(EPlayerType != S3_PLAYERW)
	{
		if (m_licenseChkResult == MagicView::Utils::HARDWAREKEY_LICENSE)
		{
			m_licenseControl.StopMonitor();
		}
	}
#endif
#endif

	if(m_pMainWnd)
	{
		m_pMainWnd->Detach();
		delete m_pMainWnd;
		m_pMainWnd = NULL;
	}

	if(m_pSession)
	{
		m_pSession->Terminate();
		delete m_pSession;
		m_pSession = NULL;
	}

    m_pluginMgr.Shutdown();

#ifdef PLAYER_DUMMY
	if (g_pLogger)
	{
		delete g_pLogger;
		g_pLogger = NULL;
	}
#endif

	DbgMsg(_T("Player Exited."));
#ifndef _DEBUG
#ifndef PLAYER_DUMMY
	if(EPlayerType != S3_PLAYERW)
	{
		if (m_licenseControl.m_bNoLicense == S3LicenseControl::NOLICENSE)
		{
		//::MessageBox(NULL, Translate(_T("Please insert key then restart client")), Translate(_T("Information")), MB_OK|MB_SYSTEMMODAL|MB_ICONEXCLAMATION);
		MagicView::CommonUI::XMSGBOXPARAMS xmb;
		xmb.bUseUserDefinedButtonCaptions = TRUE;
#define Translate(x) x
		m_msgboxMgr.XMessageBox(NULL,
			Translate(_T("Please insert key then restart client")),
			Translate(_T("Information")),
			MB_OK|MB_ICONEXCLAMATION|MB_SYSTEMMODAL,
			&xmb);
#undef Translate

			DbgMsg(_T("Please insert key then restart client."));
			Sleep(2000);
		}
		else if (m_licenseControl.m_bNoLicense == S3LicenseControl::LESSTHANPERMISSION)
		{
		//::MessageBox(NULL, Translate(_T("The current monitor number is more than license permission.")), Translate(_T("Information")), MB_OK|MB_SYSTEMMODAL);
		MagicView::CommonUI::XMSGBOXPARAMS xmb;
		xmb.bUseUserDefinedButtonCaptions = TRUE;
#define Translate(x) x
		m_msgboxMgr.XMessageBox(NULL,
			Translate(_T("The current monitor number is more than license permission.")),
			Translate(_T("Information")),
			MB_OK|MB_SYSTEMMODAL,
			&xmb);
#undef Translate
			DbgMsg(_T("The current monitor number is more than license permission."));
			Sleep(2000);
		}
	}
	
#endif
#endif
	return retCode;
}

std::tstring STDMETHODCALLTYPE CS3SignageApp::HostTranslate(const std::tstring& str)
{
    return ::Translate(str.c_str()).GetString();
}


int TimerMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, int TimeoutSeconds)
{
    MagicView::CommonUI::XMSGBOXPARAMS xmb;
    xmb.bUseUserDefinedButtonCaptions = TRUE;
    xmb.nTimeoutSeconds = TimeoutSeconds;
    return ((CS3SignageApp*)AfxGetApp())->m_msgboxMgr.XMessageBox(hWnd, lpText, lpCaption, uType, &xmb);
}