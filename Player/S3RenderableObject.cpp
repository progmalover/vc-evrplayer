#include "stdafx.h"
#include "S3RenderableObject.h"


S3RenderableObject::S3RenderableObject(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute)
{
    m_iWidth = iWidth;
    m_iHeight = iHeight;
    m_ScaleRate = fScaleRate;
    m_Attribute = Attribute;

    m_ObjectType = _T("S3_OBJECT_UNKNOWN");

    m_RenderRect.clear();


}

S3RenderableObject::~S3RenderableObject()
{
    InvalidateDeviceObjects();
    DeleteDeviceObjects();
}



CString S3RenderableObject::GetObjectType()
{
    return m_ObjectType;
}


VOID S3RenderableObject::ClearTexture(LPDIRECT3DTEXTURE9 pTexture, DWORD ClearColor)
{
    LPDIRECT3DSURFACE9 pBackBuffer = NULL;
    LPDIRECT3DSURFACE9 pRT = NULL;
    HRESULT hr;

    if(!m_pd3dDevice || !pTexture)
        return;

    // Store the current render target
    m_pd3dDevice->GetRenderTarget(0, &pBackBuffer);

    CHECK_HR(
            hr = pTexture->GetSurfaceLevel(0, &pRT),
            ::DbgMsg(_T("S3RenderableObject::ClearTexture: failed to get the texture surface, hr = 0x%08x"), hr));

    m_pd3dDevice->SetRenderTarget(0,pRT);

    m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, ClearColor, 0.0f, 0);

    // Restore render target
    m_pd3dDevice->SetRenderTarget(0, pBackBuffer);
    RELEASE(pBackBuffer);
    RELEASE(pRT);
}

HRESULT S3RenderableObject::InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice )
{
    m_pd3dDevice = pd3dDevice;

    return S_OK;
}

HRESULT S3RenderableObject::Initalize()
{
    return S_OK;
}

//HRESULT S3RenderableObject::CheckLoop()
//{
//    return S_FALSE;
//}

HRESULT S3RenderableObject::LoopContent()
{
    return S_OK;
}


HRESULT S3RenderableObject::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    return S_OK;
}


HRESULT S3RenderableObject::InvalidateDeviceObjects()
{
    return S_OK;
}


HRESULT S3RenderableObject::DeleteDeviceObjects()
{
    return S_OK;
}

HRESULT S3RenderableObject::Start()
{
    return S_OK;
}

HRESULT S3RenderableObject::Stop()
{
    return S_OK;
}


HRESULT S3RenderableObject::Pause()
{
    return S_OK;
}

HRESULT S3RenderableObject::Resume()
{
    return S_OK;
}

HRESULT S3RenderableObject::ProcessMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return E_NOTIMPL;
}

HRESULT S3RenderableObject::EnableSFRUpload(BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree)
{
    return E_NOTIMPL;
}

HRESULT S3RenderableObject :: ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return E_NOTIMPL;
}