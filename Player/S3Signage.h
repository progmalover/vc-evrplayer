#pragma once

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#pragma warning(push)
#pragma warning(disable:4702)

#include "S3DShowWizard.h"
#include "S3RenderMixer.h"
#include "S3RenderScheduler.h"
#include "S3SignageSetting.h"

#ifndef _DEBUG
#ifndef PLAYER_DUMMY
#include "S3LicenseControl.h"
#endif
#endif

#ifdef PLAYER_DUMMY
#include "ControllerClient/ControllerClientInterface.h"
#include "PlayerDummy/ClientChecker.h"
#include "PlayerDummy/ServerCommunication.h"
extern ConnectionManager m_connManager;
#endif




#include "CommonUI/XMessageBox.h"
#include "CommonLib/PluginMgr.h"
#include "CommonLib/RenderableObject.h"
#include "Utilities/CrashRptHelper.h"


#define S3S_DBGPRINT_SYNC 0

#define S3S_FULLSCREEN_ON_WINDOW_MODE 1
#define S3S_DEMO_ONLY				  0

#define S3S_MAIN_TIMER 1
#define S3S_MAIN_TIMER_DURATION 100

#define S3S_PRELOAD_TIME 5

typedef  HRESULT (STDAPICALLTYPE *pDllGetClassObject)(REFCLSID rClsID, REFIID riid, void **pv);

typedef enum { 
	S3_MAMMPLAYER = 1,                
	S3_PLAYER, 
	S3_PLAYERW, 
	S3_PLAYERSTARTER,
    S3_PLAYERLED,
	S3_PLAYERDUMMY
} S3_PLAYERTYPE;

typedef struct LAUNCH_SETTINGS{
    CString     ProgramPath;
    CString     MediaLibraryLocation;
    BOOL        bFitScreen;
    int         nFPS;
    BOOL        bEnableMAMM;
	BOOL        bEnableEVR;
    int         RotationDegree;    
    FLOAT       fRotationDegree;

    int         screenx;
    int         screeny;
    int         PreviewWidth;
    int         PreviewHeight;


    CString     PipeName;
    CString     LogPostfix;

    CString     ServerIP;
    int         ServerPort;
    
    CString     AccountName;
    CString     AccountPassword;

    enum ModeType { OFFLINE, ONLINE, LOCAL };

    ModeType    Mode;
    CString     LocalLibrary;
    CString     LayoutDescFileName;
    int         UpdateHour;

    CString     ClientName;
    CString     ClientLocation;
    CString     Company;
    CString     AdminName;
    CString     Telephone;
    CString     Email;
    CString     HardwareID;

    CString     ExitPassword;
	CString     CurLanguage;
	int         HotKey;

    CString     LogPath;
	CString     Playertype;
    LAUNCH_SETTINGS()
    {
        bFitScreen = FALSE;
        nFPS = 30;
        bEnableMAMM = FALSE;
		bEnableEVR = FALSE;
        screenx = 0;
        screeny = 0;
        PreviewWidth = 1024;
        PreviewHeight = 768;
        RotationDegree = 0;     
        fRotationDegree = 0.0;

        PipeName = _T("\\\\.\\Pipe\\S3SignagePlayer");

        ServerIP = _T("127.0.0.1");
        ServerPort = 2000;

        AccountName = _T("CharlesClient");
        AccountPassword = _T("CharlesClient");

        Mode = ONLINE;
        LocalLibrary = _T(".");
        MediaLibraryLocation = _T("");
        LayoutDescFileName = _T("index.xml");

        UpdateHour = 1;
        ClientName = _T("StarterEditionClient");
        ClientLocation = _T("StarterEditionClient");
        Company = _T("StarterEditionClient");
        AdminName = _T("StarterEditionClient");
        Telephone = _T("StarterEditionClient");
        Email = _T("StarterEditionClient");

        HardwareID = _T("00000000");
        ExitPassword = _T("");
		CurLanguage = _T("English");
		HotKey = 0;

        LogPath = _T("");
    }
}LAUNCH_SETTINGS;


class S3SignageSession;
// CS3SignageApp:
//

void GetIt(LPCTSTR lpszMessageText, DWORD dwUserData);
/******************************Public*Routine******************************\
* class CS3SignageApp
*
* application class
\**************************************************************************/
class CS3SignageApp : public CWinApp, public IS3ROHost
{
public:
    CS3SignageApp();
    ~CS3SignageApp();
    BOOL VerifyVMR9(void);

// Overrides
    virtual BOOL                InitInstance();
    S3DShowWizard                *GetWizard();
    S3RenderEngine               *GetRenderEngine();
    
    DECLARE_MESSAGE_MAP()
public:
    pDllGetClassObject          m_pGetClassObject;
    const LAUNCH_SETTINGS&      GetLanchSettings();
    MagicView::CommonUI::S3MessageBoxManager m_msgboxMgr;
    MagicView::CommonLib::CPluginManager2 m_pluginMgr;

    virtual std::tstring STDMETHODCALLTYPE HostTranslate(const std::tstring& str);

protected:
    S3SignageSession*           m_pSession;
    LAUNCH_SETTINGS             m_LaunchSettings;
    BOOL                        ParseCommandLine();
    DWORD                       m_dwMajorOSVersion;
	int							m_licenseChkResult;
    CCrashRptHelper             m_crashrptHelper;

    // MessageBoxManager

#ifndef _DEBUG
#ifndef PLAYER_DUMMY
	S3LicenseControl            m_licenseControl;
#endif
#endif

#ifdef PLAYER_DUMMY
public:
    CClientChecker              m_ClientChecker;
    ServerCommunication         m_ServerCommunication;
    S3Time                        m_StartTime;
#endif
	virtual int ExitInstance();
};



extern S3_PLAYERTYPE EPlayerType;
extern CS3SignageApp theApp;
extern HINSTANCE g_hInstance;

extern BOOL g_bDEMO;
#define SAFE_RELEASE(p) {if(p){p->Release(); p=NULL;} }
#define SAFE_DELETE(p) {if(p){delete p; p=NULL;} }
#define SAFE_DELETE_ARRAY(p) {if(p){delete [] p; p=NULL;} }

#define SWAP(A, B) {struct tempStruct { char C[sizeof(A)];} swap_tmp;\
swap_tmp = *( struct tempStruct*) &A;\
*( struct tempStruct*) &A = *( struct tempStruct*) &B;\
*( struct tempStruct*) &B = swap_tmp;}



#pragma comment(lib, "Version.lib ") 

int TimerMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType = MB_OK, int TimeoutSeconds = 5);