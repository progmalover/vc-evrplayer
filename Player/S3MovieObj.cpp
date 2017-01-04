#include "stdafx.h"
#include "S3Signage.h"
#include "S3MovieObj.h"
#include "AttributeParser.h"
#include "S3VMRSubGraph.h"
#include "S3EVRSubGraph.h"
#include "S3RenderEngine.h"

#define S3MO_TRACE_PERF 1

S3MovieObj::S3MovieObj(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute)
:S3RenderableObject(iWidth, iHeight, fScaleRate, Attribute)
{

    m_ObjectType = _T("S3_OBJECT_MOVIE");

    m_MovieWidth = 0;
    m_MovieHeight = 0;

    m_TextureWidth = 0;
    m_TextureHeight = 0;

    m_bSFREnable = FALSE;
    m_bSplitEnable = FALSE;

    m_pWizard = NULL;

    m_FilePath = GetStringAttrib(m_Attribute, _T("FileName"), _T(""));
    m_Volume = GetIntAttrib(m_Attribute, _T("Volume"), 100);
    m_bKeepAspect = GetBoolAttrib(m_Attribute, _T("KeepAspect"), FALSE);

    if(theApp.GetRenderEngine()->IsEVREnabled())
    {
        m_pSubGraph = new S3EVRSubGraph();
    }else
    {
        m_pSubGraph = new S3VMRSubgraph();
    }
}

S3MovieObj::~S3MovieObj()
{
    m_pSubGraph->Stop();
    if (m_pWizard)
        m_pWizard->Detach( m_pSubGraph->GetID());

    m_pSubGraph->DestroyGraph();
    delete m_pSubGraph;
}

HRESULT S3MovieObj::InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice )
{
    S3RenderableObject::InitDeviceObjects(pd3dDevice);

    m_pWizard = theApp.GetWizard();

    USES_CONVERSION; 
 
    WCHAR  wszWideString[MAX_PATH]; 

    wcscpy_s(wszWideString, MAX_PATH, CT2CW(m_FilePath));

    m_pSubGraph->SetRepeatMode(S3S_PS_REPEAT);

    return m_pSubGraph->BuildAndRender( wszWideString, m_pWizard, theApp.GetMainWnd()->m_hWnd);

}

HRESULT S3MovieObj::PrepareRender()
{
    HRESULT hr;

    hr = GetVideoTexture();

    if(hr != S_OK) return hr;

    if(!m_pWizard)
    {
        return S_FALSE;
    }
    LONG videoWidth, videoHeight;
    LONG preferedx, preferedy;
    if(FAILED(m_pWizard->GetVideoSize(m_pSubGraph->GetID(), &videoWidth, &videoHeight, &preferedx, &preferedy)))
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

        m_MovieWidth = videoWidth;
        m_MovieHeight = videoHeight;
        if(!m_bKeepAspect)
        {
            MovieRect = (RenderRect(RECTF(0,0,(float)m_iWidth, (float)m_iHeight), RECTF(0.0f, 0.0f, (float)m_MovieWidth/(float)m_TextureWidth, (float)m_MovieHeight/(float)m_TextureHeight)));

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

            RECT SRFDisplayRect = m_DisplayRect;
            SRFDisplayRect.left += XTrans;
            SRFDisplayRect.right -= XTrans;
            SRFDisplayRect.top += YTrans;
            SRFDisplayRect.bottom -= YTrans;

            m_pWizard->EnableSFRUpload(m_pSubGraph->GetID(), m_bSFREnable, m_bSplitEnable, &SRFDisplayRect, m_RotateDegree);

        }
        MovieRect.pTexture = m_pTexture;
        MovieRect.bTransparent = FALSE;

        m_RenderRect.clear();
        m_RenderRect.push_back(MovieRect);
    }
    return S_OK;
}

HRESULT S3MovieObj::EndRender()
{
    ReleaseVideoTexture();
    return S_OK;
}


HRESULT  S3MovieObj::GetVideoTexture()
{
#if S3MO_TRACE_PERF
    LARGE_INTEGER tBegin,tEnd;
    QueryPerformanceCounter(&tBegin);
#endif
    HRESULT hr = S_FALSE;
    if(m_pWizard)
    {
        hr = m_pWizard->GetTexture(m_pSubGraph->GetID(), &m_pTexture);
    }

    if(m_pTexture == NULL) return E_UNEXPECTED;

    if(m_pTexture && m_TextureWidth == 0)
    {
        D3DSURFACE_DESC TextureDesc;
        m_pTexture->GetLevelDesc(0, &TextureDesc);

        m_TextureWidth = TextureDesc.Width;
        m_TextureHeight = TextureDesc.Height;
    }

#if S3MO_TRACE_PERF
    QueryPerformanceCounter(&tEnd);
    UINT delta = tEnd.LowPart - tBegin.LowPart;
    if(delta > 0x500)
    {
        delta = 0;
    }
#endif

    return hr;
}


HRESULT  S3MovieObj::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    m_MovieWidth = 0;
    m_MovieHeight = 0;
    InitDeviceObjects(pd3dDevice);
    m_pSubGraph->Seek(m_RestorePositon);
    m_pSubGraph->Run(m_Volume);
    return S_OK;
}

HRESULT S3MovieObj::EnableSFRUpload(BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree)
{
    m_bSFREnable = bEnabled;
    m_bSplitEnable = bSplit;
    m_DisplayRect = *pDisplayRect;
    m_RotateDegree = RotateDegree;

    //if(!m_bKeepAspect)
    {
        return m_pWizard->EnableSFRUpload(m_pSubGraph->GetID(), bEnabled, bSplit, pDisplayRect, RotateDegree);
    }
    return S_OK;
}



HRESULT  S3MovieObj::InvalidateDeviceObjects()
{
    m_pSubGraph->GetPosition(&m_RestorePositon);

    m_pSubGraph->Stop();
    if (m_pWizard)
        m_pWizard->Detach( m_pSubGraph->GetID());

    m_pSubGraph->DestroyGraph();
    return S_OK;
}

void S3MovieObj::ReleaseVideoTexture()
{
    if(m_pWizard)
    {
        m_pWizard->ReleaseTexture(m_pSubGraph->GetID());
    }
}

HRESULT S3MovieObj::Start()
{
    m_pSubGraph->Seek(0);
    m_pSubGraph->Run(m_Volume);
    return S_OK;
}

HRESULT S3MovieObj::Stop()
{
    if(m_pWizard)
    {
        m_pWizard->StopPresenting(m_pSubGraph->GetID(), TRUE);
    }
    
    m_pSubGraph->Stop();

    return S_OK;
}

HRESULT S3MovieObj::Pause()
{
    m_pSubGraph->Pause();
    return S_OK;
}

HRESULT S3MovieObj::Resume()
{
    m_pSubGraph->Resume();
    return S_OK;
}

HRESULT S3MovieObj::LoopContent()
{
	m_pSubGraph->AutoLoop();
    return S_OK;
}