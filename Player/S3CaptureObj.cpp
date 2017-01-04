#include "stdafx.h"
#include "S3CaptureObj.h"
#include "S3VMRWizard.h"
#include "AttributeParser.h"
#include "S3Utility.h"
#include "Utilities/SysCall.h"
#include "Utilities/StringUtility.h"
#include "Utilities/Singleton.h"

#define DETECT_CAPTURE_RESOLUTION_CHANGE 1

S3CaptureObj::S3CaptureObj(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute)
:S3RenderableObject(iWidth, iHeight, fScaleRate, Attribute)
{

    m_ObjectType = _T("S3_OBJECT_MOVIE");

    UINT CaptureNo = StringUtility::stoi(GetStringAttrib(Attribute, _T("CaptureNo"), _T("")).GetString());

    CaptureDevice device;
    if (Singleton<S3Utility>::Instance()->GetCaptureDevice(CaptureNo, device))
    {
        m_DeviceName = SysCall::GetCaptureDevicePath(device.device_url);
    }

    m_Volume = GetIntAttrib(m_Attribute, _T("Volume"), 100);
    m_ChannelNo = GetIntAttrib(m_Attribute, _T("ChannelNo"), 0);    
    m_DeviceClsID = GetStringAttrib(m_Attribute, _T("ClsID"), _T(""));


    m_MovieWidth    = 0;
    m_MovieHeight   = 0;
    m_TextureWidth  = 0;
    m_TextureHeight = 0;
}

S3CaptureObj::~S3CaptureObj()
{
    m_SubGraph.Stop();

    m_SubGraph.DestroyGraph();
}

HRESULT S3CaptureObj::InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice )
{
    S3RenderableObject::InitDeviceObjects(pd3dDevice);


    return m_SubGraph.BuildAndRender( m_DeviceName, m_ChannelNo, FALSE, m_DeviceClsID);

}

HRESULT S3CaptureObj::PrepareRender()
{
    HRESULT hr;

    hr = GetVideoTexture();

    return hr;
}

HRESULT S3CaptureObj::EndRender()
{
    ReleaseVideoTexture();
    return S_OK;
}


HRESULT  S3CaptureObj::GetVideoTexture()
{

    HRESULT hr = S_FALSE;

    m_RenderRect.clear();


    hr = m_SubGraph.GetTexture(&m_pTexture);

    if(FAILED(hr)) return S_FALSE;

    if(m_pTexture == NULL) return E_UNEXPECTED;

    if(m_pTexture)
    {
        D3DSURFACE_DESC TextureDesc;
        m_pTexture->GetLevelDesc(0, &TextureDesc);

        m_TextureWidth = TextureDesc.Width;
        m_TextureHeight = TextureDesc.Height;
    }

    LONG videoWidth, videoHeight;
    LONG preferedx, preferedy;
    if(FAILED(m_SubGraph.GetVideoSize(&videoWidth, &videoHeight, &preferedx, &preferedy)))
    {
        return S_FALSE;
    }

    if(videoHeight == 1088)
    {
        videoHeight = 1080;
    }

    //if(videoWidth != m_MovieWidth || videoHeight != m_MovieHeight)
    {
        RenderRect MovieRect;

        // Address mode 1 pixel issue
        m_MovieWidth = videoWidth > 0 ? videoWidth - 1 : videoWidth;
        m_MovieHeight = videoHeight > 0 ? videoHeight - 1 : videoHeight;
        if(1)
        {
            MovieRect = (RenderRect(RECTF(0,0,(float)m_iWidth, (float)m_iHeight), 
                RECTF(0.0f, 0.0f, 
                (float)m_MovieWidth/(float)m_TextureWidth, 
                (float)m_MovieHeight/(float)m_TextureHeight)));

        }else
        {
            float origAR = (float)m_MovieWidth/(float)m_MovieHeight;
            float preferedAR = (float)preferedx/(float)preferedy;
            long  AdjustedWidth = m_MovieWidth;
            long  AdjustedHeight = m_MovieHeight;
            if(origAR > preferedAR)
            {
                AdjustedHeight = (long)floor((origAR/preferedAR)*AdjustedHeight + 0.5f);
            }
            else
            {
                AdjustedWidth = (long)floor((preferedAR/origAR)*AdjustedWidth + 0.5f);
            }

            float ScaleRate;
            if((FLOAT)AdjustedWidth / (FLOAT)AdjustedHeight > (FLOAT)m_iWidth / (FLOAT)m_iHeight)
            {
                ScaleRate = (FLOAT)m_iWidth/ (FLOAT)AdjustedWidth;
            }else
            {
                ScaleRate = (FLOAT)m_iHeight/ (FLOAT)AdjustedHeight;
            }

            float XTrans = (m_iWidth - ScaleRate * AdjustedWidth) / 2;
            float YTrans = (m_iHeight - ScaleRate * AdjustedHeight) / 2;



            MovieRect = (RenderRect(RECTF(0,0,(float)m_iWidth, (float)m_iHeight), RECTF(0.0f, 0.0f, (float)m_MovieWidth/(float)m_TextureWidth, (float)m_MovieHeight/(float)m_TextureHeight)));
            MovieRect.Position.left += XTrans;
            MovieRect.Position.right -= XTrans;
            MovieRect.Position.top += YTrans;
            MovieRect.Position.bottom -= YTrans;
        }
        MovieRect.pTexture = m_pTexture;
        MovieRect.bTransparent = FALSE;

        m_RenderRect.clear();
        m_RenderRect.push_back(MovieRect);
    }

    return hr;
}


HRESULT  S3CaptureObj::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    InitDeviceObjects(pd3dDevice);
    m_SubGraph.Run();
    return S_OK;
}

HRESULT  S3CaptureObj::InvalidateDeviceObjects()
{
    m_SubGraph.Stop();

    m_SubGraph.DestroyGraph();
    return S_OK;
}

void S3CaptureObj::ReleaseVideoTexture()
{
    m_SubGraph.ReleaseTexture();
}

HRESULT S3CaptureObj::Start()
{
    m_SubGraph.Run();
    return S_OK;
}

HRESULT S3CaptureObj::Stop()
{
    
    m_SubGraph.Stop();

    return S_OK;
}

HRESULT S3CaptureObj::Pause()
{
    return S_OK;
}

HRESULT S3CaptureObj::Resume()
{
    return S_OK;
}

HRESULT S3CaptureObj::LoopContent()
{
#ifdef DETECT_CAPTURE_RESOLUTION_CHANGE
    m_SubGraph.HandleEvent();
#endif
    return S_OK;
}

HRESULT S3CaptureObj::EnableSFRUpload(BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree,CRect ClipInfo)
{
    return m_SubGraph.EnableSFRUpload(bEnabled, bSplit, pDisplayRect, RotateDegree, ClipInfo);
}
