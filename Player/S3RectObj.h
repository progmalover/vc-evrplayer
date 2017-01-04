#pragma once
#include "s3renderableobject.h"
class S3RectObj :
    public S3RenderableObject
{
public:


    S3RectObj(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute);
    virtual ~S3RectObj();

    virtual HRESULT         InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice );

    virtual HRESULT         RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice); ///device lost handle
    virtual HRESULT         InvalidateDeviceObjects();                        ///device lost handle
    virtual HRESULT         DeleteDeviceObjects();                            

    virtual HRESULT         PrepareRender();
    virtual HRESULT         EndRender();

protected:
    LPDIRECT3DTEXTURE9              m_pTexture;
    DWORD                           m_Color;
};

