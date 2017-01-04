#include "stdafx.h"
#include "S3Signage.h"
#include "S3RectObj.h"
#include "AttributeParser.h"


S3RectObj::S3RectObj(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute)
    :S3RenderableObject(iWidth, iHeight, fScaleRate, Attribute)
{
    m_pTexture = NULL;

    m_Color = GetColorAttrib(m_Attribute, _T("Color"), 0);
}


S3RectObj::~S3RectObj(void)
{
    Stop();
    DeleteDeviceObjects();
}


HRESULT S3RectObj::InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice )
{
    HRESULT hr;

    S3RenderableObject::InitDeviceObjects(pd3dDevice);

    LPDIRECT3DTEXTURE9 pSysTexture = NULL;

    hr = pd3dDevice->CreateTexture(1, 1, 1, D3DUSAGE_DYNAMIC, 
                D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSysTexture, NULL);
    if(FAILED(hr)) return hr;


    hr = pd3dDevice->CreateTexture(1, 1, 1, D3DUSAGE_RENDERTARGET, 
            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexture, NULL);
    if(FAILED(hr)) return hr;

    D3DLOCKED_RECT LockInfo;
    hr = pSysTexture->LockRect(0, &LockInfo, NULL, 0);

    if(FAILED(hr)) return hr;

    *(DWORD *)LockInfo.pBits = m_Color;

    pSysTexture->UnlockRect(0);

    hr = pd3dDevice->UpdateTexture(pSysTexture, m_pTexture);

    pSysTexture->Release();

    return hr;
}

HRESULT S3RectObj::DeleteDeviceObjects()
{
    SAFE_RELEASE(m_pTexture);
    return S_OK;
}

HRESULT S3RectObj::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    InitDeviceObjects(pd3dDevice);
    Initalize();

    return S_OK;
}

HRESULT S3RectObj::InvalidateDeviceObjects()
{
    DeleteDeviceObjects();
    return S_OK;
}


HRESULT S3RectObj::PrepareRender()
{
    m_RenderRect.clear();

    RenderRect RectObjRect = (RenderRect(RECTF(0,0,(float)m_iWidth, (float)m_iHeight), RECTF(0.0f, 0.0f, 1.0f, 1.0f)));
    RectObjRect.bTransparent = ((m_Color & 0xFF000000) != 0xFF000000);
    RectObjRect.pTexture = m_pTexture;

    m_RenderRect.push_back(RectObjRect);
    return S_OK;
}

HRESULT S3RectObj::EndRender()
{
    return S_OK;
}

