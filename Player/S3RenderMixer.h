#pragma once

#include "S3ObjectContainer.h"
#include "S3SignageSetting.h"

class S3RenderEngine;

#define S3S_TRACE_PERF    1

#define S3S_CAPTURE_STATE_INIT                 0
#define S3S_CAPTURE_STATE_REQUIRE_SAVE         1
#define S3S_CAPTURE_STATE_SAVE_FINISH          2






typedef struct LayerInfo
{
    CoordinateInfo          TextureCoordinate[4];
    int                     ZOrder;
    BOOL                    bTransparent;
    BOOL                    bUseBlendFactor;
    DWORD                   BlendFactor;
    LPDIRECT3DTEXTURE9      pTexture;   // The d3d texture for this object;
}LayerInfo;


typedef struct RenderRectInfo
{
    RECTF                   m_ScreenRect;
    vector<LayerInfo>       m_LayerInfo;
    BOOL                    m_bFavorHorizontalSplit;
    static bool compare(const RenderRectInfo* t1,const RenderRectInfo* t2);
    RenderRectInfo*         CloneWithClip(RECTF &ClipRect);

}RenderRectInfo;




class S3RenderMixer
{
public:
    S3RenderMixer();
    virtual ~S3RenderMixer();

    HRESULT Render(IDirect3DDevice9 *pDevice, void* lpParam);

    HRESULT SetRenderEngineOwner(S3RenderEngine* pRenderEngine);

    HRESULT GetRenderEngineOwner(S3RenderEngine** ppRenderEngine);

    HRESULT Initialize(IDirect3DDevice9 *pDevice);

    STDMETHOD(BeginDeviceLoss)(void);
        
    STDMETHOD(EndDeviceLoss)(IDirect3DDevice9 *pDevice);
    
    HRESULT RestoreDeviceObjects( IDirect3DDevice9 *pDevice );

    void    AddRenderableObject(S3ObjectContainer *pObj);
    void    RemoveRenderableObject(S3ObjectContainer *pObj);
    void    ReplaceRenderableObject(S3ObjectContainer *pOldObj, S3ObjectContainer *pNewObj);

    
    void    Terminate();

    void    SaveSnapShot(CString Filename, int Width, RECT CaptureRect);

    HRESULT ProcessMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);

    HRESULT ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void    Lock();
    void    Unlock();

    DWORD   GetObjectCount();
    int     GetFPS();

    FLOAT                         m_RotationDegree;
    SIZEL                         m_DisplaySize;
    DWORD                         m_ClearColor;
    RECTF                         m_ScreenClip;

private:
    CCritSec                      m_MixerObjectLock;       // this object has to be thread-safe
    list<S3ObjectContainer*>      m_listObjects;
    S3RenderEngine*               m_pOwner;
    BOOL                          m_bInitialized;

    CString                       m_CaptureFile;
    INT                           m_CaptureWidth;
    volatile DWORD                m_CaptureStatus;
    RECT                          m_CaptureRect;

#if S3S_TRACE_PERF
    UINT    m_TimeHistory[256];
    UINT    m_HisType[256];
    UINT    m_curItem;
#endif


private:
    
    void SplitRenderRectInfo();
    void RenderSplitedRect(IDirect3DDevice9 *pDevice);
    void AddRenderRectInfoByObject(S3ObjectContainer *pObj);
    void AddRenderRectInfo(RenderRectInfo *pNewRect);
    void RectSubstract(RenderRectInfo * pNewRect, RECTF &ClipRect, BOOL bHorizonntalSplit);
    void Clean_();

    list<RenderRectInfo*>     m_RenderProcessingList;
    list<RenderRectInfo*>     m_RenderList;

    int m_CalcFPS;
    int m_FramesRendered;
    int m_CalcStartTime;
    int m_ClearCount;
};

