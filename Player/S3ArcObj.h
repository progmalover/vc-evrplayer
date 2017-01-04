#pragma once
#include "s3renderableobject.h"
class S3ArcObj :
    public S3RenderableObject
{
public:
    S3ArcObj(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute);
    virtual ~S3ArcObj();

    virtual HRESULT         InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice );
    virtual HRESULT         Initalize();  
    virtual HRESULT         RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);                           ///device lost handle
    virtual HRESULT         InvalidateDeviceObjects();                        ///device lost handle
    virtual HRESULT         DeleteDeviceObjects();                            ///device lost handle

    virtual HRESULT         PrepareRender();
    virtual HRESULT         EndRender();

protected:
    LPDIRECT3DTEXTURE9              m_pTexture;
    DWORD                           m_Color;
    float                           m_StartAngle;
    float                           m_EndAngle;
};

