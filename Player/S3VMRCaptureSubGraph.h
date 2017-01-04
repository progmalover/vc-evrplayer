#pragma once
class CaptureRenderer;
class S3VMRWizard;

//#define USE_VMR_RENDER

class S3VMRCaptureSubGraph
{
public:
    S3VMRCaptureSubGraph(void);
    virtual ~S3VMRCaptureSubGraph(void);

    // public methods
    HRESULT BuildAndRender( CString DeviceName, int ChannelNo, BOOL bForceXRGB, CString DeviceClsID);
    HRESULT Run();
    HRESULT Stop();
    HRESULT HandleEvent();

    HRESULT DestroyGraph();
    HRESULT DisconnectPins( CComPtr<IBaseFilter> pFilter);
    OAFilterState GetState();

    virtual HRESULT GetTexture(LPDIRECT3DTEXTURE9* ppTexture);
    virtual HRESULT GetVideoSize(LONG* plWidth, LONG* plHeight, LONG* preferedx, LONG* preferedy);
    virtual HRESULT EnableSFRUpload(BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree, RECT Clip);
    virtual HRESULT ReleaseTexture();

private:
    // private data

#ifdef USE_VMR_RENDER
    CComPtr<IBaseFilter>    m_pVMR;     // VMR9
#else
    CaptureRenderer*        m_pCR;
#endif
    CComPtr<IGraphBuilder>  m_pGraph;   // filter graph
    CComPtr<IMediaControl>  m_pMc;      // media control
    CComPtr<ICaptureGraphBuilder2>  m_pCapture;

    CComPtr<IAMStreamConfig> m_pConfig;
    CComPtr<IPin> m_pOutPin;

    CString   m_DeviceName;     
    CString   m_DeviceClsID;
    int       m_ChannelNo;
    BOOL      m_bForceXRGB;
    BOOL      m_bCaptureInited;


    DWORD_PTR m_dwID;   // actual cookie identifying the subgraph; assigned in
                        // IS3VRWizard::Attach
    GUID      m_PreferFormat;
    DWORD     m_LastDetectTime;

    BOOL      m_bDatapath;
    BOOL      m_bTC2000;

    CCritSec  SurfaceMutex;

};
