#include "stdafx.h"
#include "S3Signage.h"
#include "S3ArcObj.h"
#include "AttributeParser.h"



S3ArcObj::S3ArcObj(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute)
    :S3RenderableObject(iWidth, iHeight, fScaleRate, Attribute)
{
    m_pTexture = NULL;

    m_Color = GetColorAttrib(m_Attribute, _T("Color"), 0);

    m_StartAngle = GetFloatAttrib(m_Attribute, _T("StartAngle"), 0);

    m_EndAngle = GetFloatAttrib(m_Attribute, _T("EndAngle"), 360.0f);
}


S3ArcObj::~S3ArcObj(void)
{
    Stop();
    DeleteDeviceObjects();
}


HRESULT S3ArcObj::InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice )
{
    HRESULT hr;
    hr = S3RenderableObject::InitDeviceObjects(pd3dDevice);

    return hr;
}

HRESULT S3ArcObj::Initalize()
{
    HRESULT hr;

    LPDIRECT3DTEXTURE9 pSysTexture = NULL;

        // measure character size
    HDC  hDC       = CreateCompatibleDC( NULL );

    // Prepare to create a bitmap
    DWORD*      pBitmapBits;
    BITMAPINFO bmi;

    ZeroMemory( &bmi.bmiHeader,  sizeof(BITMAPINFOHEADER) );
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       =  (int)m_iWidth;
    bmi.bmiHeader.biHeight      = -(int)m_iHeight;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biBitCount    = 32;
    HBITMAP hbmBitmap = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
        (void**)&pBitmapBits, NULL, 0 );

    SelectObject( hDC, hbmBitmap );

    HBRUSH hBruch = CreateSolidBrush(0);

    RECT FulLRect;

    FulLRect.left = 0;
    FulLRect.top = 0;
    FulLRect.right = m_iWidth;
    FulLRect.bottom = m_iHeight;

    FillRect(hDC, &FulLRect, hBruch);
    DeleteObject( hBruch );

    hBruch = CreateSolidBrush(0xFFFFFF);

    SelectObject(hDC, hBruch);

    if(m_StartAngle == 0 && m_EndAngle == 360)
    {
        Ellipse(hDC, 0,0,  m_iWidth, m_iHeight);
    }
    else
    {
        float X1 = 100* cos(m_StartAngle * 3.1415926f/180);
        float Y1 = -100* sin(m_StartAngle * 3.1415926f/180);
        float X2 = 100* cos(m_EndAngle * 3.1415926f/180);
        float Y2 = -100* sin(m_EndAngle * 3.1415926f/180);

        Pie(hDC, 0,0,  m_iWidth, m_iHeight, (int)floor(X1 + m_iWidth/2 + 0.5f), \
			(int)floor(Y1 + m_iHeight/2 +0.5f), (int)floor(X2 + m_iWidth/2+0.5f), \
			(int)floor(Y2 + m_iHeight/2 +0.5f));
    }

    hr = m_pd3dDevice->CreateTexture(m_iWidth, m_iHeight, 1, D3DUSAGE_DYNAMIC, 
                D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSysTexture, NULL);
    if(FAILED(hr)) return hr;


    hr = m_pd3dDevice->CreateTexture(m_iWidth, m_iHeight, 1, D3DUSAGE_RENDERTARGET, 
            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexture, NULL);
    if(FAILED(hr)) return hr;

    D3DLOCKED_RECT LockInfo;
    hr = pSysTexture->LockRect(0, &LockInfo, NULL, 0);


    if(SUCCEEDED(hr))
    {
    
        BYTE* pDstRow = (BYTE*)LockInfo.pBits;
        DWORD ColorConvertTable[256];

        DWORD FrontA = (m_Color & 0xFF000000) >> 24;
        DWORD FrontR = (m_Color & 0xFF0000) >> 16;
        DWORD FrontG = (m_Color & 0xFF00) >> 8;
        DWORD FrontB = (m_Color & 0xFF);

        DWORD BackA = 0;
        DWORD BackR = 0;
        DWORD BackG = 0;
        DWORD BackB = 0;


        for(int i=0; i< 256; i++)
        {
            DWORD BlendA = (i * FrontA + (255 - i) * BackA + 127) / 255;
            DWORD BlendR = (i * FrontR + (255 - i) * BackR + 127) / 255;
            DWORD BlendG = (i * FrontG + (255 - i) * BackG + 127) / 255;
            DWORD BlendB = (i * FrontB + (255 - i) * BackB + 127) / 255;

            ColorConvertTable[i] = (DWORD)((BlendA<<24) | (BlendR<<16) | (BlendG<<8) | BlendB);
        }

        DWORD* pDst32;

        for(int y=0; y < m_iHeight; y++ )
        {
            pDst32 = (DWORD*)pDstRow;
            for(int x=0; x < m_iWidth; x++ )
            {
                BYTE bAlpha = (BYTE)((pBitmapBits[m_iWidth*y + x] & 0xff));
                *pDst32++ = ColorConvertTable[bAlpha];
            }
            pDstRow += LockInfo.Pitch;
        }
    }

    pSysTexture->UnlockRect(0);

    hr = m_pd3dDevice->UpdateTexture(pSysTexture, m_pTexture);

    pSysTexture->Release();

    DeleteObject( hbmBitmap );
    DeleteDC( hDC );

    return hr;
}

HRESULT S3ArcObj::DeleteDeviceObjects()
{
    SAFE_RELEASE(m_pTexture);
    return S_OK;
}

HRESULT S3ArcObj::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    InitDeviceObjects(pd3dDevice);
    Initalize();
    return S_OK;
}

HRESULT S3ArcObj::InvalidateDeviceObjects()
{
    DeleteDeviceObjects();
    return S_OK;
}


HRESULT S3ArcObj::PrepareRender()
{
    m_RenderRect.clear();

    RenderRect RectObjRect = (RenderRect(RECTF(0,0,(float)m_iWidth, (float)m_iHeight), RECTF(0.0f, 0.0f, 1.0f, 1.0f)));
    RectObjRect.bTransparent = TRUE;
    RectObjRect.pTexture = m_pTexture;

    m_RenderRect.push_back(RectObjRect);
    return S_OK;
}

HRESULT S3ArcObj::EndRender()
{
    return S_OK;
}

