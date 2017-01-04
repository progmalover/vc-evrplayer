#pragma once

struct __declspec(uuid("{7A013080-900F-4c19-918B-5560C2819EFB}")) CLSID_S3CaptureRenderer;

class S3RenderEngine;

class CaptureRenderer;
class CaptureAllocator;

class CaptureMediaSample: public CMediaSample
{
public:

    CaptureMediaSample(
        TCHAR *pName,
        CaptureAllocator *pAllocator,
        HRESULT *phr,
        LPBYTE pBuffer = NULL,
        LONG length = 0,
        CaptureRenderer *pRender = NULL);
    virtual ~CaptureMediaSample();

    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP GetVideoSurface(LPDIRECT3DSURFACE9 *ppSurface);
    STDMETHODIMP SyncVideoSurface();

private:

    CaptureRenderer *m_pCaptureRender;
    LPDIRECT3DSURFACE9   m_pVideoSurface;

};



class CaptureAllocator: public CBaseAllocator
{
public:
    CaptureAllocator(TCHAR *, LPUNKNOWN, HRESULT *, CaptureRenderer *);


protected:
    // override to free the memory when decommit completes
    // - we actually do nothing, and save the memory until deletion.
    void Free(void);

    // called from the destructor (and from Alloc if changing size/count) to
    // actually free up the memory
    void ReallyFree(void);

    // overriden to allocate the memory when commit called
    HRESULT Alloc(void);

public:

    STDMETHODIMP SetProperties(
		    ALLOCATOR_PROPERTIES* pRequest,
		    ALLOCATOR_PROPERTIES* pActual);

    STDMETHODIMP GetBuffer(IMediaSample **ppBuffer,
                           REFERENCE_TIME * pStartTime,
                           REFERENCE_TIME * pEndTime,
                           DWORD dwFlags);

    ~CaptureAllocator();

    CaptureRenderer *m_pCaptureRender;

};

class CaptureInputPin: public CRendererInputPin
{
public:
    CaptureInputPin(CaptureRenderer *pRenderer,
                      HRESULT *phr,
                      LPCWSTR Name);
    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps);
    STDMETHODIMP GetAllocator(IMemAllocator ** ppAllocator);

    CaptureRenderer *m_pCaptureRender;
};





class CaptureRenderer :
    public CBaseVideoRenderer
{
public:

	friend class S3VMRCaptureSubGraph;
    friend class CaptureInputPin;
    friend class CaptureAllocator;
    friend class CaptureMediaSample;

    CaptureRenderer(HRESULT *phr);
    virtual ~CaptureRenderer(void);

public:
    HRESULT CheckMediaType(const CMediaType *pmt );     // Format acceptable?
    HRESULT SetMediaType(const CMediaType *pmt );       // Video format notification
    HRESULT DoRenderSample(IMediaSample *pMediaSample); // New video sample
    HRESULT Init(LPDIRECT3DDEVICE9 pDevice, S3RenderEngine *pRenderEngine);
    HRESULT ForceMediaType(GUID MediaType);
    HRESULT ForceNumaNode(int Node);
    HRESULT SetEncrpytEnable();

    STDMETHODIMP Pause();

    CBasePin *GetPin(int n);

    virtual HRESULT GetTexture(LPDIRECT3DTEXTURE9* ppTexture);
    virtual HRESULT GetVideoSize(LONG* plWidth, LONG* plHeight, LONG* preferedx, LONG* preferedy);
    virtual HRESULT EnableSFRUpload(BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree, CRect ClipInfo);

private:
    LONG            m_InternalWidth;
    LONG            m_InternalHeight;
    LONG            m_lVidWidth;   // Video width
    LONG            m_lVidHeight;  // Video Height
    LONG            m_lVidPitch;   // Video Pitch
    GUID            m_videoSubtype;
    D3DFORMAT       m_SurfaceFormat;


    LPDIRECT3DSURFACE9   m_pVideoSurface;
    LPDIRECT3DSURFACE9   m_pSurfaceTemp;

    LPDIRECT3DTEXTURE9   m_pTexture;

    LPDIRECT3DDEVICE9    m_pDevice;
    S3RenderEngine      *m_pRenderEngine;

    BOOL                 m_bEnableSFRUpload;
    BOOL                 m_bEnableSplitUpload;
    RECT                 m_SFRRect;
    RECT                 m_ClipInfo;

	FLOAT                m_RotateDegree;

	GUID                 m_ForceMediaType;
	BOOL                 m_bForceMediaType;
    DWORD                m_lastFrameTime;

    int                  m_PreferedNumaNode;

    BOOL                 m_CaptureToPCIe;
    BOOL                 m_CaptureToVideo;
    BOOL                 m_bHDCPEnable;

};

#define USE_NV12 0
#define D3DFMT_NV12                 MAKEFOURCC('N', 'V', '1', '2')
