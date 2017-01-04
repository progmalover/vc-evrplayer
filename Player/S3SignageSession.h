#pragma once

#include "S3DShowWizard.h"

class S3RenderEngine;
class S3RenderMixer;
class S3RenderScheduler;
class S3SignagePipe;




class S3SignageSession
{
public:
    S3SignageSession(void);
    ~S3SignageSession(void);

    HRESULT         Initialize(LAUNCH_SETTINGS *pLaunchSetting);
    HRESULT         Start();
    HRESULT         Terminate();

    HWND            GetWindow(){ return m_hwndVideo; }
    S3DShowWizard*  GetWizard( );
    S3RenderEngine* GetRenderEngine( );

    // video window processing
    static LRESULT CALLBACK VideoWndProc(
                                        HWND hwnd,      // handle to window
                                        UINT uMsg,      // message identifier
                                        WPARAM wParam,  // first message parameter
                                        LPARAM lParam   // second message parameter
                                        );

    // private methods
protected:
    HRESULT         CreateVideoWindow_(UINT x, UINT y, UINT Width, UINT Height, DWORD dwStyle);
    HRESULT         ProcessWindowsMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);

    // private data 
protected:
    S3DShowWizard*              m_pWizard;
    S3RenderEngine*             m_pRenderEngine;
    S3RenderMixer*              m_pMixerControl;
    S3RenderScheduler*          m_pRenderScheduler;
    S3SignagePipe*              m_pPipeControl;
    HWND                        m_hwndVideo;
    LAUNCH_SETTINGS             m_LaunchSetting;
    CString                     m_InputHistory;

};

