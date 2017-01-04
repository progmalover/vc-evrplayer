#pragma once

#include "S3SignageSetting.h"
#include "S3D3DMath.h"

class S3RenderMixer;
class TiXmlElement;
class S3ObjectContainer;

typedef struct S3SIGNAGE_CONTENT_CONTAINER
{
    S3SIGNAGE_CONTENT  Content;
    S3ObjectContainer *pObject;
    S3ObjectContainer *pObjectForNextLoop;
    BOOL               bRendering;
    BOOL               bHided;
}S3SIGNAGE_CONTENT_CONTAINER;


typedef struct S3SIGNAGE_LAYER
{
    CString     Name;
    int         Duration;
    int         StartTime;
    int         ZOrder;
    int         VisibleCount;
    list<S3SIGNAGE_CONTENT_CONTAINER *> Contents;
}S3SIGNAGE_LAYER;


typedef struct S3SIGNAGE_CANVAS
{
    CString     Name;
    DWORD       BGColor;
    DWORD       Width;
    DWORD       Height;
    list<S3SIGNAGE_LAYER *> Layers;
    S3ObjectContainer *pBackgroundObj;
}S3SIGNAGE_CANVAS;

class S3RenderScheduler
{
public:
    S3RenderScheduler(S3RenderMixer *pMixer);
    ~S3RenderScheduler(void);

    HRESULT InitializeScheduler(int nFPS, CString MediaLibraryPath);

    HRESULT SetDisplayConfigure(int Width, int Height, FLOAT m_RotateDegree, BOOL bFitScreen = TRUE, BOOL bFillScreen = TRUE);
    RECT    GetDisplayRect();
    SIZE    GetCanvasSize();

    HRESULT Add(CString ParentID, CString Desc);
    HRESULT Delete(CString ID);
    HRESULT Hide(CString ID);
    HRESULT Show(CString ID);

    HRESULT LoadPlayList(CString Filename, CString MediaLibraryPath = _T(""));
    HRESULT SetVolume(int Vol);

    HRESULT Play();
    HRESULT Stop();
    HRESULT Pause();
    HRESULT Resume();
    HRESULT Quit();
    HRESULT Terminate();
    HRESULT SaveSnapShot(CString Filename);

    HRESULT UpdateRenderObjects(); // called periodically in message loop, trigger mixer object change

    HRESULT BeginDeviceLoss();
        
    HRESULT EndDeviceLoss(IDirect3DDevice9 *pDevice);
	HRESULT	initCanvas();
protected:
    BOOL                          m_bFitScreen;      
    BOOL                          m_bFillScreen;
    FLOAT                         m_Scale;
    FLOAT                         m_XTrans;
    FLOAT                         m_YTrans;      

    int                           m_Width;
    int                           m_Height;
    FLOAT                         m_RotationDegree;
    int                           m_Volume;


    S3RenderMixer*                m_pMixer;

    S3SIGNAGE_CANVAS              m_Canvas;
    int                           m_nFPS;

    CString                       m_PlaylistPath;
    CString                       m_MediaLibraryPath;
    BOOL                          m_bPaused;

    VOID                          GenerateLayerZOrder();
    VOID                          ResetAllContentZOrder();
    HRESULT                       PreloadObjects();
    HRESULT                       SchedulerObjects();
    HRESULT                       CheckObjectsLoop();
    VOID                          InitAllLayerStartTime(DWORD Time);
    VOID                          CalculateTransform();

    HRESULT                       ParserLayer(S3SIGNAGE_LAYER *pLayer, TiXmlElement *pLayerElement);
    HRESULT                       ParserContent(S3SIGNAGE_CONTENT_CONTAINER *pContent, TiXmlElement *pContentElement);
    HRESULT                       ParserTranstion(S3SIGNAGE_TRANSIION *pTransition, TiXmlElement *pTransformElement);
    HRESULT                       ParserTransform(S3SIGNAGE_TRANSFORM *pTransform, TiXmlElement *pTransformElement);
    HRESULT                       ParserTextFile(TextFileItem *pTextFile, TiXmlElement *pTextFileElement);

    BOOL                          FindID(CString ContentID, S3SIGNAGE_LAYER **ppLayer, S3SIGNAGE_CONTENT_CONTAINER **ppContent);
private:
	UINT m_nNumMixers;
	HMIXER m_hMixer;
	MIXERCAPS m_mxcaps;


	DWORD m_dwMinimum, m_dwMaximum;
	DWORD m_dwVolumeControlID;

	BOOL amdUninitialize();
	BOOL amdInitialize();
	BOOL amdGetMasterVolumeControl();
	BOOL amdGetMasterVolumeValue(DWORD &dwVal) const;
	BOOL amdSetMasterVolumeValue(DWORD dwVal) const;
};

