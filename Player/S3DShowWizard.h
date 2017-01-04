#pragma once
class S3RenderMixer;
class S3RenderEngine;

#define VIDEO_SOURCE_TAG 0x533301

interface S3DShowWizard : public IUnknown
{
public:
    S3DShowWizard(void);
    virtual ~S3DShowWizard(void);

        /*** IUnknown methods ***/
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS)  PURE;

    // S3DShowWizard implementation
    virtual HRESULT Initialize(
        DWORD dwFlags,
        HWND hWnd,
        S3RenderEngine *pRenderEngine);

    // terminate wizard
    virtual HRESULT Terminate();


    virtual HRESULT Attach(IBaseFilter* pRenderFilter, DWORD_PTR* pdwID);

    virtual HRESULT Detach(DWORD_PTR dwID);

	virtual HRESULT BeginDeviceLoss();

	virtual HRESULT EndDeviceLoss(IDirect3DDevice9* pDevice);

    virtual HRESULT VerifyID(DWORD_PTR dwID);

    virtual HRESULT GetGraph(DWORD_PTR dwID, IFilterGraph** ppGraph);

    virtual HRESULT GetRenderEngine(S3RenderEngine** pRenderEngine);

    virtual HRESULT GetMixerControl(S3RenderMixer** ppMixerControl);

    virtual HRESULT GetTexture(DWORD_PTR dwID,LPDIRECT3DTEXTURE9* ppTexture);

    virtual HRESULT ReleaseTexture(DWORD_PTR dwID);

    virtual HRESULT StopPresenting(DWORD_PTR dwID, BOOL bStop);

    virtual HRESULT GetVideoSize(DWORD_PTR dwID, LONG* plWidth, LONG* plHeight, LONG* preferedx, LONG* preferedy);

    virtual HRESULT EnableSFRUpload(DWORD_PTR dwID, BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree);

    // Private classes
protected:
    class S3VR_VideoSource
    {
    public:
        S3VR_VideoSource();
        ~S3VR_VideoSource();

        // methods 
        void DeleteSurfaces();
        HRESULT DisconnectPins();
        HRESULT AllocateSurfaceBuffer( DWORD dwN );
        HRESULT SetVideoSize(   LONG lImageW, 
                                LONG lImageH );


        // data

        // we use this tag to verify that (S3VR_VideoSource*)(void*)pVideoSource
        // is really S3VR_VideoSource
        DWORD_PTR dwTag;
        DWORD_PTR dwID; 
        DWORD dwNumBuf;
        DWORD dwNumBufActuallyAllocated;

        LONG lImageWidth;
        LONG lImageHeight;
        LONG lPreferedx;
        LONG lPreferedy;

        BOOL               m_UserNV12RT;
        LPDIRECT3DTEXTURE9 pTexturePriv;
        LPDIRECT3DSURFACE9 pTempSurfacePriv;

        IDirect3DSurface9 **ppSurface;
        VOID              **ppSurfaceMem;
        IUnknown           *pVideoAllocator;
        VOID               *pVideoPresenter;
        IFilterGraph *pGraph;
        IBaseFilter *pRender;

        BOOL bTextureInited;
        volatile LONG updateCount;
        volatile BOOL bStop;
        DWORD dwLast;
        CCritSec  updateMutex;
    };

    typedef enum RenderThreadStatus
    {
        eNotStarted = 0x0,
        eRunning,
        eWaitingToStop,
        eFinished,
    } RenderThreadStatus;

    // Private methods
protected:
    HRESULT StartRenderingThread_();
    HRESULT StopRenderingThread_();
    void    Clean_();
    HRESULT GetSourceInfo_(DWORD_PTR dwID, S3VR_VideoSource** ppsource );

    static DWORD WINAPI RenderThreadProc_( LPVOID lpParameter );

    // Private data
protected:
    HWND                            m_hwnd;             // video window
    CCritSec                        m_WizardObjectLock;       // this object has to be thread-safe
    BOOL                            m_bInitialized;     // true if Initialize() was called and succeeded
    volatile RenderThreadStatus     m_RenderThreadStatus; // 0: not started, 1: running, 2: requested to stop, 3: stopped
    CCritSec                        m_threadMutex;
    DWORD                           m_dwConfigFlags;    // configuration flags
    DWORD                           m_maxVideoFPS;

    S3RenderEngine*                 m_pRenderEngine;    // render engine
 
    list<S3VR_VideoSource*>         m_listVideoSources;

};
