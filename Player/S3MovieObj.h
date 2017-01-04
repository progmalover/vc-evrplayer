#ifndef S3_3D_MOVIE_H
#define S3_3D_MOVIE_H
#include <tchar.h>
#include <D3D9.h>
#include "S3RenderableObject.h"
#include "S3SignageSetting.h"
#include "S3DShowSubgraph.h"


class S3MovieObj: public S3RenderableObject
{
public:
    S3MovieObj(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute);
    virtual ~S3MovieObj();

    virtual HRESULT         Start();
    virtual HRESULT         Stop();
    virtual HRESULT         InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice );
    virtual HRESULT         RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);                           ///device lost handle
    virtual HRESULT         InvalidateDeviceObjects();                        ///device lost handle
    virtual HRESULT         PrepareRender();
    virtual HRESULT         EndRender();
    virtual HRESULT         Pause();
    virtual HRESULT         Resume();
    //virtual HRESULT         CheckLoop();
    virtual HRESULT         LoopContent();

    virtual HRESULT         EnableSFRUpload(BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree);

protected:
    S3DShowWizard*   m_pWizard;
    S3DShowSubGraph* m_pSubGraph;

    int             m_MovieWidth;
    int             m_MovieHeight;
    int             m_TextureWidth;
    int             m_TextureHeight;
    LONGLONG        m_RestorePositon;

    CString         m_FilePath;
    int             m_Volume;
    DWORD           m_BGColor;
    BOOL            m_bKeepAspect;
    LPDIRECT3DTEXTURE9  m_pTexture; 

    HRESULT         GetVideoTexture();
    void            ReleaseVideoTexture();

    BOOL            m_bSFREnable;
    BOOL            m_bSplitEnable;
    RECT            m_DisplayRect;
    FLOAT           m_RotateDegree;

};


#endif