#pragma once

#include "S3DShowWizard.h"
#include <dxva2api.h>

class S3RenderMixer;
class S3RenderScheduler;

#define TRACE_RENDER_TIMER 0

#if TRACE_RENDER_TIMER
struct RenderTime_Info{
    DWORD BTime;
    DWORD MBTime;
    DWORD METime;
    DWORD ETime;
};
#endif

class S3RenderEngine
{
public:
    S3RenderEngine(void);
    virtual ~S3RenderEngine(void);

    HRESULT Initialize(
        HWND hWnd, 
        DWORD dwFlags,
        S3RenderMixer* pMixerControl,
        S3RenderScheduler*  pScheduler,
        BOOL  bFullScreen, BOOL bEnableMAMM, BOOL bEnableEVR);

    HRESULT Terminate();

    HRESULT Render();

	HRESULT ProcessLostDevice();

    HRESULT SetFrameRate(int nFramesPerSec);

    HRESULT GetFrameRate(int* pnFramesPerSec);

    HRESULT GetMixingPrefs(DWORD* pdwPrefs);

    HRESULT SetWizardOwner(S3DShowWizard* pWizard);

    HRESULT GetWizardOwner(S3DShowWizard** ppWizard);

    HRESULT GetMixerControl(S3RenderMixer** ppMixerControl);

    HRESULT Get3DDevice(IDirect3DDevice9 ** ppDevice);

    HRESULT GetVideoWindow(HWND *phwnd);

    BOOL    IsSupportMAMMRendering();
    BOOL    IsSupportEVR();

    BOOL    IsMAMMEnabled();
    BOOL    IsEVREnabled();

    SIZE    GetDeviceSize();
    HRESULT GetService(REFGUID guidService, REFIID riid, LPVOID *ppvObject);
    HRESULT CheckFormat(D3DFORMAT format);

    HRESULT Pause();
    HRESULT Resume();

//private methods
private:
	HRESULT CreateDevice_();
    HRESULT  ResetDevice_();
    void Clean_();

// private data
private:
    CCritSec                m_REObjectLock; // this object has to be thread-safe
    BOOL                    m_bInitialized; // true if Initialize() was called
    HWND                    m_hwnd;         // handle to the video window

    IDirect3D9*             m_pD3D9;
    IDirect3DDevice9*       m_pDevice;      // Direct3DDevice
    S3RenderMixer*          m_pMixerControl;// S3VRMixerControl
    S3DShowWizard*          m_pOwner;       // S3VRWizard that controls this render engine
    S3RenderScheduler*      m_pScheduler;
    IDirect3DQuery9*        m_pSpeedControlQuery;

    IDirect3DDeviceManager9* m_pDeviceManager;        // Direct3D device manager.
    UINT                    m_DeviceResetToken;     // Reset token for the D3D device manager.

    DWORD                   m_dwCfgFlags;   // configuration flags
    DWORD                   m_dwLast;
	BOOL					m_bChangingDevice;// true if we are in the process of changing devices
    BOOL                    m_bFullScreen;
    BOOL                    m_bMAMMEnabled;
    BOOL                    m_bEVREnabled;
    BOOL                    m_bPaused;

    SIZE                    m_DisplaySize;

    volatile UINT          *m_pVSyncCounter;
    DWORD                   m_dwLastVSync;
    DWORD                   m_setFPS;       // requested FPS, in (frames per sec)
    DWORD                   m_interframe;   // 1000/m_setFPS;
    float                   m_iferror;
    float                   m_accuError;

    UINT                    m_nPresentFailCount;
#if TRACE_RENDER_TIMER
    RenderTime_Info         m_RenderTimeHistory[1024];
    DWORD                   m_CurPos;
#endif
};
