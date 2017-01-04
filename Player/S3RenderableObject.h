
#ifndef RENDERABLE_OBJECT_H
#define RENDERABLE_OBJECT_H

#include <D3D9.h>
#include "S3D3DMath.h"
#include "S3SignageSetting.h"


class S3RenderableObject
{
    friend class S3ObjectContainer;
public:
    S3RenderableObject(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute);  ///construct function, please just init variable here
    virtual ~S3RenderableObject();

    virtual HRESULT         InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice); ///main thread initalize function
    virtual HRESULT         Initalize();                                      ///sperate thread initalize function

    virtual HRESULT         RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);///device lost handle
    virtual HRESULT         InvalidateDeviceObjects();                        ///device lost handle
    virtual HRESULT         DeleteDeviceObjects();                            

    virtual HRESULT         Start();                                          ///Mixer start to render this object
    virtual HRESULT         Stop();                                           ///Mixer end render this object
    virtual HRESULT         PrepareRender() = 0;
    virtual HRESULT         EndRender() = 0;
    virtual HRESULT         Pause();
    virtual HRESULT         Resume();
    //virtual HRESULT         CheckLoop();
    virtual HRESULT         LoopContent();

    virtual CString         GetObjectType();
    virtual HRESULT         ProcessMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam); ///Mouse message process
    virtual HRESULT         EnableSFRUpload(BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree);
    virtual HRESULT         ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);


protected:
        // these member must be inited when create renderable object
    int                                 m_iWidth;
    int                                 m_iHeight;
    float                               m_ScaleRate;
    CString                             m_ObjectType;
    AttribList                          m_Attribute;
  
    // used for rendering
    LPDIRECT3DDEVICE9                   m_pd3dDevice;   
    
    list<RenderRect>                    m_RenderRect;

    VOID ClearTexture(LPDIRECT3DTEXTURE9 pTexture, DWORD ClearColor);
};



#endif

