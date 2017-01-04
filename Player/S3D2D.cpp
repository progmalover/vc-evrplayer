#include "stdafx.h"
#include "S3D2D.h"


void S3D2DClipLine::BuildClipLine(POINTF v1, POINTF v2)
{
    a = v1.y - v2.y;
    b = -(v1.x - v2.x);
    c = v1.x*v2.y - v2.x* v1.y;

}

void S3D2DClipList::Clear()
{
    m_ClipList.RemoveAll();
}

void S3D2DClipList::DrawRect(RECTF RectClip)
{
    S3D2DClipTriangle T1;
    S3D2DClipTriangle T2;

    T1.V[0].x = RectClip.left;     T1.V[0].y = RectClip.bottom; 
    T1.V[1].x = RectClip.left;     T1.V[1].y = RectClip.top; 
    T1.V[2].x = RectClip.right;    T1.V[2].y = RectClip.bottom; 


    T2.V[0].x = RectClip.right;    T2.V[0].y = RectClip.bottom; 
    T2.V[1].x = RectClip.left;     T2.V[1].y = RectClip.top; 
    T2.V[2].x = RectClip.right;    T2.V[2].y = RectClip.top; 

    m_ClipList.Add(T1);
    m_ClipList.Add(T2);
}


void S3D2DClipList::DrawArc(float StartX, float StartY, float Radius, float StartAngle, float SweepAngle,
                            bool bCounterDirection, float Step)
{
    if(!bCounterDirection)
    {
        float LastArc = StartAngle;
        float EndArc = StartAngle - SweepAngle;

        while(LastArc > EndArc)
        {
            FLOAT CurrentArc = LastArc - Step;
            CurrentArc = max(CurrentArc, EndArc);

            S3D2DClipTriangle T1;

            T1.V[0].x = StartX;
            T1.V[0].y = StartY;

            T1.V[1].x = (StartX + Radius * cos(CurrentArc));
            T1.V[1].y = (StartY + Radius * sin(CurrentArc));
         
            T1.V[2].x = (StartX + Radius * cos(LastArc));
            T1.V[2].y = (StartY + Radius * sin(LastArc));

            m_ClipList.Add(T1);

            LastArc = CurrentArc;
        }
    }
    else
    {
        float LastArc = StartAngle;
        float EndArc = StartAngle + SweepAngle;

        while(LastArc < EndArc)
        {
            FLOAT CurrentArc = LastArc + Step;
            CurrentArc = min(CurrentArc, EndArc);

            S3D2DClipTriangle T1;

            T1.V[0].x = StartX;
            T1.V[0].y = StartY;       

            T1.V[1].x = (StartX + Radius * cos(LastArc));
            T1.V[1].y = (StartY + Radius * sin(LastArc));

            T1.V[2].x = (StartX + Radius * cos(CurrentArc));
            T1.V[2].y = (StartY + Radius * sin(CurrentArc));

            m_ClipList.Add(T1);

            LastArc = CurrentArc;
        }
    }

}


S3D2DGraphics::S3D2DGraphics(void)
{
    m_ClipList.RemoveAll();
    S3SetMatrixIdentity(&m_SrcMatrix);
    m_bSrcIdentity = true;
    S3SetMatrixIdentity(&m_DestMatrix);
    m_bDstIdentity = true;
}


void S3D2DGraphics::CreateGraphics(LPDIRECT3DSURFACE9 pRenderTarget, LPDIRECT3DDEVICE9 pDevice)
{
    m_pRenderTarget = pRenderTarget;
    m_pDevice = pDevice;
}

void S3D2DGraphics::SetClip(S3D2DClipList &ClipList)
{
    m_ClipList.RemoveAll();

    for(int i=0; i<ClipList.m_ClipList.GetCount(); i++)
    {
        m_ClipList.Add(ClipList.m_ClipList[i]);
    }
}

void S3D2DGraphics::SetSrcTransform(D3DMATRIX &matrix)
{
    m_SrcMatrix = matrix;
    if(S3IsMatrixIdentity(m_SrcMatrix))
    {
        m_bSrcIdentity = true;
    }
    else
    {
        m_bSrcIdentity = false;
    }
}

void S3D2DGraphics::SetDestTransform(D3DMATRIX &matrix)
{
    m_DestMatrix = matrix;
    if(S3IsMatrixIdentity(m_DestMatrix))
    {
        m_bDstIdentity = true;
    }
    else
    {
        m_bDstIdentity = false;
    }
}

void S3D2DGraphics::DrawImage(LPDIRECT3DTEXTURE9 pSrc, D3DTEXTUREADDRESS Addressmode, CoordinateInfo *pSrcCoordinate, RECTF DestRect, 
        DWORD Alpha,  D3DTEXTUREFILTERTYPE FilterType)
{
        S3D2DRenderTriangleList RenderTriangleList;

    // Store the current render target
    LPDIRECT3DSURFACE9 pBackBuffer = 0;
    m_pDevice->GetRenderTarget(0, &pBackBuffer);

    m_pDevice->SetRenderTarget(0,m_pRenderTarget);

    if(!pSrc)
    {
        m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0.0f, 0);
        // Restore render target
        m_pDevice->SetRenderTarget(0, pBackBuffer);
        RELEASE(pBackBuffer);

        return;
    }


	D3DSURFACE_DESC SrcDesc;
	memset(&SrcDesc, 0, sizeof(SrcDesc));

    pSrc->GetLevelDesc(0, &SrcDesc);

    // Prepare transform
    S3VECTOR4 DestPosition[4];
    S3VECTOR4 SrcPosition[4];

    DestPosition[0] = S3VECTOR4(DestRect.left, DestRect.bottom, 0.9f, 1.0f);
    DestPosition[1] = S3VECTOR4(DestRect.left, DestRect.top, 0.9f, 1.0f);
    DestPosition[2] = S3VECTOR4(DestRect.right, DestRect.bottom, 0.9f, 1.0f);
    DestPosition[3] = S3VECTOR4(DestRect.right, DestRect.top, 0.9f, 1.0f);

    SrcPosition[0] = S3VECTOR4(pSrcCoordinate[0].tu * SrcDesc.Width, pSrcCoordinate[0].tv * SrcDesc.Height, 0.9f, 1.0f);
    SrcPosition[1] = S3VECTOR4(pSrcCoordinate[1].tu * SrcDesc.Width, pSrcCoordinate[1].tv * SrcDesc.Height, 0.9f, 1.0f);
    SrcPosition[2] = S3VECTOR4(pSrcCoordinate[2].tu * SrcDesc.Width, pSrcCoordinate[2].tv * SrcDesc.Height, 0.9f, 1.0f);
    SrcPosition[3] = S3VECTOR4(pSrcCoordinate[3].tu * SrcDesc.Width, pSrcCoordinate[3].tv * SrcDesc.Height, 0.9f, 1.0f);


    // transform dest
    if(!m_bDstIdentity)
    {
        S3Vec4Transform(&DestPosition[0], &DestPosition[0], &m_DestMatrix);
        S3Vec4Transform(&DestPosition[1], &DestPosition[1], &m_DestMatrix);
        S3Vec4Transform(&DestPosition[2], &DestPosition[2], &m_DestMatrix);
        S3Vec4Transform(&DestPosition[3], &DestPosition[3], &m_DestMatrix);
    }


    //// transform src
    if(!m_bSrcIdentity)
    {
        S3Vec4Transform(&SrcPosition[0], &SrcPosition[0], &m_SrcMatrix);
        S3Vec4Transform(&SrcPosition[1], &SrcPosition[1], &m_SrcMatrix);
        S3Vec4Transform(&SrcPosition[2], &SrcPosition[2], &m_SrcMatrix);
        S3Vec4Transform(&SrcPosition[3], &SrcPosition[3], &m_SrcMatrix);
    }


    // build Render Triangle
    S3D2DRenderTriangle T1, T2;
    T1.V[0].Pos = DestPosition[0];
    T1.V[1].Pos = DestPosition[1];
    T1.V[2].Pos = DestPosition[2];

    T1.V[0].tu = SrcPosition[0].x/SrcDesc.Width;   T1.V[0].tv =  SrcPosition[0].y/SrcDesc.Height;
    T1.V[1].tu = SrcPosition[1].x/SrcDesc.Width;   T1.V[1].tv =  SrcPosition[1].y/SrcDesc.Height;
    T1.V[2].tu = SrcPosition[2].x/SrcDesc.Width;   T1.V[2].tv =  SrcPosition[2].y/SrcDesc.Height;



    T2.V[0].Pos = DestPosition[1];
    T2.V[1].Pos = DestPosition[3];
    T2.V[2].Pos = DestPosition[2];

    T2.V[0].tu = SrcPosition[1].x/SrcDesc.Width;   T2.V[0].tv =  SrcPosition[1].y/SrcDesc.Height;
    T2.V[1].tu = SrcPosition[3].x/SrcDesc.Width;   T2.V[1].tv =  SrcPosition[3].y/SrcDesc.Height;
    T2.V[2].tu = SrcPosition[2].x/SrcDesc.Width;   T2.V[2].tv =  SrcPosition[2].y/SrcDesc.Height;

    // clip triangle with clip list

    if(m_ClipList.GetCount())
    {
        for(int i=0; i<m_ClipList.GetCount(); i++)
        {
            ClipTriangleByTriangle(T1, m_ClipList[i], RenderTriangleList);
            ClipTriangleByTriangle(T2, m_ClipList[i], RenderTriangleList);
        }
    }else
    {
        RenderTriangleList.Add(T1);
        RenderTriangleList.Add(T2);
    }

    // render triangle list
    HRESULT hr;

    hr = m_pDevice->SetTexture(0, pSrc);
    hr = m_pDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX1 );

    hr = m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    hr = m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

    hr = m_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    hr = m_pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU ,Addressmode);
    hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV ,Addressmode);
    
    hr = m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, FilterType);
    hr = m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, FilterType);
     
    if(Alpha < 0xFF)
    {
        hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

        hr = m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);
        hr = m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVBLENDFACTOR);

        // The following is the blending factor, use values from 0 to 255
        // A value of 0 will make image transparent and a value of 255
        // will make it opaque.
        hr = m_pDevice->SetRenderState(D3DRS_BLENDFACTOR, Alpha << 24 | Alpha << 16 | Alpha << 8 | Alpha);

    }else
    {
        hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    }

    for(int i=0; i<RenderTriangleList.GetCount(); i++)
    {
        RenderTriangleList[i].V[0].Pos.x -= 0.5f;
        RenderTriangleList[i].V[0].Pos.y -= 0.5f;
        RenderTriangleList[i].V[1].Pos.x -= 0.5f;
        RenderTriangleList[i].V[1].Pos.y -= 0.5f;
        RenderTriangleList[i].V[2].Pos.x -= 0.5f;
        RenderTriangleList[i].V[2].Pos.y -= 0.5f;
    }

    hr = m_pDevice->DrawPrimitiveUP(  D3DPT_TRIANGLELIST,
        RenderTriangleList.GetCount(),
        (LPVOID)(RenderTriangleList.GetData()),
        sizeof(Vertex1T));


    hr = m_pDevice->SetTextureStageState(0, D3DTSS_CONSTANT, 0xFFFFFFFF);
    hr = m_pDevice->SetTexture(0, NULL);


    // Restore render target
    m_pDevice->SetRenderTarget(0, pBackBuffer);
    RELEASE(pBackBuffer);


}

void S3D2DGraphics::DrawImage(LPDIRECT3DTEXTURE9 pSrc, D3DTEXTUREADDRESS Addressmode, RECTF SrcRect, RECTF DestRect, 
                              DWORD Alpha, D3DTEXTUREFILTERTYPE FilterType)
{
    S3D2DRenderTriangleList RenderTriangleList;

    // Store the current render target
    LPDIRECT3DSURFACE9 pBackBuffer = 0;
    m_pDevice->GetRenderTarget(0, &pBackBuffer);

    m_pDevice->SetRenderTarget(0,m_pRenderTarget);

    if(!pSrc)
    {
        m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0.0f, 0);
        // Restore render target
        m_pDevice->SetRenderTarget(0, pBackBuffer);
        RELEASE(pBackBuffer);

        return;
    }


    D3DSURFACE_DESC SrcDesc;
	memset(&SrcDesc, 0, sizeof(SrcDesc));

    pSrc->GetLevelDesc(0, &SrcDesc);

    // Prepare transform
    S3VECTOR4 DestPosition[4];
    S3VECTOR4 SrcPosition[4];

    DestPosition[0] = S3VECTOR4(DestRect.left, DestRect.bottom, 0.9f, 1.0f);
    DestPosition[1] = S3VECTOR4(DestRect.left, DestRect.top, 0.9f, 1.0f);
    DestPosition[2] = S3VECTOR4(DestRect.right, DestRect.bottom, 0.9f, 1.0f);
    DestPosition[3] = S3VECTOR4(DestRect.right, DestRect.top, 0.9f, 1.0f);

    SrcPosition[0] = S3VECTOR4(SrcRect.left, SrcRect.bottom, 0.9f, 1.0f);
    SrcPosition[1] = S3VECTOR4(SrcRect.left, SrcRect.top, 0.9f, 1.0f);
    SrcPosition[2] = S3VECTOR4(SrcRect.right, SrcRect.bottom, 0.9f, 1.0f);
    SrcPosition[3] = S3VECTOR4(SrcRect.right, SrcRect.top, 0.9f, 1.0f);


    // transform dest
    if(!m_bDstIdentity)
    {
        S3Vec4Transform(&DestPosition[0], &DestPosition[0], &m_DestMatrix);
        S3Vec4Transform(&DestPosition[1], &DestPosition[1], &m_DestMatrix);
        S3Vec4Transform(&DestPosition[2], &DestPosition[2], &m_DestMatrix);
        S3Vec4Transform(&DestPosition[3], &DestPosition[3], &m_DestMatrix);
    }


    //// transform src
    if(!m_bSrcIdentity)
    {
        S3Vec4Transform(&SrcPosition[0], &SrcPosition[0], &m_SrcMatrix);
        S3Vec4Transform(&SrcPosition[1], &SrcPosition[1], &m_SrcMatrix);
        S3Vec4Transform(&SrcPosition[2], &SrcPosition[2], &m_SrcMatrix);
        S3Vec4Transform(&SrcPosition[3], &SrcPosition[3], &m_SrcMatrix);
    }


    // build Render Triangle
    S3D2DRenderTriangle T1, T2;
    T1.V[0].Pos = DestPosition[0];
    T1.V[1].Pos = DestPosition[1];
    T1.V[2].Pos = DestPosition[2];

    T1.V[0].tu = SrcPosition[0].x/SrcDesc.Width;   T1.V[0].tv =  SrcPosition[0].y/SrcDesc.Height;
    T1.V[1].tu = SrcPosition[1].x/SrcDesc.Width;   T1.V[1].tv =  SrcPosition[1].y/SrcDesc.Height;
    T1.V[2].tu = SrcPosition[2].x/SrcDesc.Width;   T1.V[2].tv =  SrcPosition[2].y/SrcDesc.Height;



    T2.V[0].Pos = DestPosition[1];
    T2.V[1].Pos = DestPosition[3];
    T2.V[2].Pos = DestPosition[2];

    T2.V[0].tu = SrcPosition[1].x/SrcDesc.Width;   T2.V[0].tv =  SrcPosition[1].y/SrcDesc.Height;
    T2.V[1].tu = SrcPosition[3].x/SrcDesc.Width;   T2.V[1].tv =  SrcPosition[3].y/SrcDesc.Height;
    T2.V[2].tu = SrcPosition[2].x/SrcDesc.Width;   T2.V[2].tv =  SrcPosition[2].y/SrcDesc.Height;

    // clip triangle with clip list

    if(m_ClipList.GetCount())
    {
        for(int i=0; i<m_ClipList.GetCount(); i++)
        {
            ClipTriangleByTriangle(T1, m_ClipList[i], RenderTriangleList);
            ClipTriangleByTriangle(T2, m_ClipList[i], RenderTriangleList);
        }
    }else
    {
        RenderTriangleList.Add(T1);
        RenderTriangleList.Add(T2);
    }

    // render triangle list
    HRESULT hr;

    hr = m_pDevice->SetTexture(0, pSrc);
    hr = m_pDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX1 );

    hr = m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    hr = m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CONSTANT);

    hr = m_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    hr = m_pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU ,Addressmode);
    hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV ,Addressmode);
    
    hr = m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, FilterType);
    hr = m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, FilterType);
     
    if(Alpha < 0xFF)
    {
        hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

        hr = m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);
        hr = m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVBLENDFACTOR);

        // The following is the blending factor, use values from 0 to 255
        // A value of 0 will make image transparent and a value of 255
        // will make it opaque.
        hr = m_pDevice->SetRenderState(D3DRS_BLENDFACTOR, Alpha << 24 | Alpha << 16 | Alpha << 8 | Alpha);

    }else
    {
        hr = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    }

    for(int i=0; i<RenderTriangleList.GetCount(); i++)
    {
        RenderTriangleList[i].V[0].Pos.x -= 0.5f;
        RenderTriangleList[i].V[0].Pos.y -= 0.5f;
        RenderTriangleList[i].V[1].Pos.x -= 0.5f;
        RenderTriangleList[i].V[1].Pos.y -= 0.5f;
        RenderTriangleList[i].V[2].Pos.x -= 0.5f;
        RenderTriangleList[i].V[2].Pos.y -= 0.5f;
    }

    hr = m_pDevice->DrawPrimitiveUP(  D3DPT_TRIANGLELIST,
        RenderTriangleList.GetCount(),
        (LPVOID)(RenderTriangleList.GetData()),
        sizeof(Vertex1T));


    hr = m_pDevice->SetTextureStageState(0, D3DTSS_CONSTANT, 0xFFFFFFFF);
    hr = m_pDevice->SetTexture(0, NULL);


    // Restore render target
    m_pDevice->SetRenderTarget(0, pBackBuffer);
    RELEASE(pBackBuffer);
}

void S3D2DGraphics::TriTexcoordInterpolate(const S3D2DRenderTriangle& Triangle, Vertex1T& ResultVertex)
{
    double S012 = GetTriArea(Triangle.V[0], Triangle.V[1], Triangle.V[2]);
    double S123 = GetTriArea(Triangle.V[1], Triangle.V[2], ResultVertex);
    double S023 = GetTriArea(Triangle.V[0], Triangle.V[2], ResultVertex);
    double S013 = GetTriArea(Triangle.V[0], Triangle.V[1], ResultVertex);

    ResultVertex.tu = (float)((S123 * Triangle.V[0].tu + S023 * Triangle.V[1].tu + S013 * Triangle.V[2].tu)/S012);
    ResultVertex.tv = (float)((S123 * Triangle.V[0].tv + S023 * Triangle.V[1].tv + S013 * Triangle.V[2].tv)/S012);
}

void S3D2DGraphics::ClipTriangleByTriangle(S3D2DRenderTriangle &SrcTriangle, S3D2DClipTriangle &ClipTriangle, S3D2DRenderTriangleList &ResultList)
{    
    bool bClipTriangleInSrcTriangle = true;

    // if the Clip Triangle is inside of Src Triangle, just need put the Clip Triangle into the resultlist
    S3D2DClipLine SrcLine[3];
    POINTF V1, V2, V3;
    V1.x = SrcTriangle.V[0].Pos.x;
    V1.y = SrcTriangle.V[0].Pos.y;
    V2.x = SrcTriangle.V[1].Pos.x;
    V2.y = SrcTriangle.V[1].Pos.y;
    V3.x = SrcTriangle.V[2].Pos.x;
    V3.y = SrcTriangle.V[2].Pos.y;
    SrcLine[0].BuildClipLine(V1, V2);
    SrcLine[1].BuildClipLine(V2, V3);
    SrcLine[2].BuildClipLine(V3, V1);

    for(int iLine = 0; iLine<3; iLine++)
    {
        float DisResult[3];
        DisResult[0] = SrcLine[iLine].a * ClipTriangle.V[0].x + SrcLine[iLine].b * ClipTriangle.V[0].y + SrcLine[iLine].c;
        DisResult[1] = SrcLine[iLine].a * ClipTriangle.V[1].x + SrcLine[iLine].b * ClipTriangle.V[1].y + SrcLine[iLine].c;
        DisResult[2] = SrcLine[iLine].a * ClipTriangle.V[2].x + SrcLine[iLine].b * ClipTriangle.V[2].y + SrcLine[iLine].c;

        if(!((DisResult[0] >= 0) && (DisResult[1] >= 0) && (DisResult[2] >= 0)))
        {
            bClipTriangleInSrcTriangle = false;
            break;
        }
    }

    if(bClipTriangleInSrcTriangle)
    {
        S3D2DRenderTriangleList ClipResult;

        S3D2DRenderTriangle NewTriangle;

        NewTriangle.V[0].Pos = S3VECTOR4(ClipTriangle.V[0].x, ClipTriangle.V[0].y, 0.9f, 1.0f);
        NewTriangle.V[1].Pos = S3VECTOR4(ClipTriangle.V[1].x, ClipTriangle.V[1].y, 0.9f, 1.0f);
        NewTriangle.V[2].Pos = S3VECTOR4(ClipTriangle.V[2].x, ClipTriangle.V[2].y, 0.9f, 1.0f);

        TriTexcoordInterpolate(SrcTriangle, NewTriangle.V[0]);
        TriTexcoordInterpolate(SrcTriangle, NewTriangle.V[1]);
        TriTexcoordInterpolate(SrcTriangle, NewTriangle.V[2]);

        ClipResult.Add(NewTriangle);
        ResultList.Append(ClipResult);
        return;
    }

    // if the Clip Triangle is not inside of Src Triangle, we need to clip the Src Triangle into some peices
    S3D2DRenderTriangleList ClipResult;
    S3D2DRenderTriangleList TempResult;
    S3D2DClipLine ClipLine[3];

    ClipLine[0].BuildClipLine(ClipTriangle.V[0], ClipTriangle.V[1]);
    ClipLine[1].BuildClipLine(ClipTriangle.V[1], ClipTriangle.V[2]);
    ClipLine[2].BuildClipLine(ClipTriangle.V[2], ClipTriangle.V[0]);

    ClipResult.Add(SrcTriangle);

    for(int iLine = 0; iLine<3; iLine++)
    {
        TempResult.RemoveAll();
        for(int i=0; i<ClipResult.GetCount(); i++ )
        {
            S3D2DRenderTriangleList LineClipResult;

            ClipTriangleByLine(ClipResult[i], ClipLine[iLine], LineClipResult);
            TempResult.Append(LineClipResult);
        }

        ClipResult.RemoveAll();
        ClipResult.Append(TempResult);
    }

    ResultList.Append(ClipResult);
}

void S3D2DGraphics::ClipTriangleByLine(S3D2DRenderTriangle &SrcTriangle, S3D2DClipLine &ClipLine, S3D2DRenderTriangleList &ResultList)
{

    double DisResult[3];
    DisResult[0] = (double)ClipLine.a * SrcTriangle.V[0].Pos.x + (double)ClipLine.b * SrcTriangle.V[0].Pos.y + (double)ClipLine.c;
    DisResult[1] = (double)ClipLine.a * SrcTriangle.V[1].Pos.x + (double)ClipLine.b * SrcTriangle.V[1].Pos.y + (double)ClipLine.c;
    DisResult[2] = (double)ClipLine.a * SrcTriangle.V[2].Pos.x + (double)ClipLine.b * SrcTriangle.V[2].Pos.y + (double)ClipLine.c;

    if((DisResult[0] >= 0) && (DisResult[1] >= 0) && (DisResult[2] >= 0))
    {
        ResultList.Add(SrcTriangle);
        return;
    }

    if((DisResult[0] <= 0) && (DisResult[1] <= 0) && (DisResult[2] <= 0))
    {
        return;
    }else
    {
        Vertex1T VertexOut[4];
        DWORD Count = 0;
        DWORD LoopIndex = 0;

        for(int i=0; i< 3; i++)
        {
            if(DisResult[i] < 0)
            {
                LoopIndex = i;
            }
        }


        for(DWORD i = LoopIndex; i< (LoopIndex + 3); i++)
        {
            if(DisResult[i%3] > 0)
            {
                VertexOut[Count++] = SrcTriangle.V[i%3];
            }
        }

        for(DWORD i = LoopIndex; i< (LoopIndex + 3); i++)
        {
            if((DisResult[i%3] * DisResult[ (i + 1)%3] < 0) ||
                (DisResult[i%3] > 0 && DisResult[ (i + 1)%3] == 0) ||
                (DisResult[i%3] == 0 && DisResult[ (i + 1)%3] > 0))
            {
                Vertex1T V1 = SrcTriangle.V[i%3];
                Vertex1T V2 = SrcTriangle.V[(i + 1)%3];
                Vertex1T VOut;

                double dis1 = abs(DisResult[i%3]);
                double dis2 = abs(DisResult[(i + 1)%3]);


                VOut.Pos = (V1.Pos * dis2 + V2.Pos * dis1)/(dis1 + dis2);
                VOut.tu = (V1.tu * dis2 + V2.tu * dis1)/(dis1 + dis2);
                VOut.tv = (V1.tv * dis2 + V2.tv * dis1)/(dis1 + dis2);


                VertexOut[Count++] = VOut;
            }
        }


        S3D2DRenderTriangle NewTriangle;
        NewTriangle.V[0] = VertexOut[0];
        NewTriangle.V[1] = VertexOut[1];
        NewTriangle.V[2] = VertexOut[2];
        ResultList.Add(NewTriangle);

        if(Count == 4)
        {
            NewTriangle.V[0] = VertexOut[1];
            NewTriangle.V[1] = VertexOut[2];
            NewTriangle.V[2] = VertexOut[3];
            ResultList.Add(NewTriangle);
        }

    }
}
