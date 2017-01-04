
#ifndef _S3_D2D_H
#define _S3_D2D_H

#include <D3D9.h>
#include "S3D3DMath.h"
#include "S3RenderableObject.h"


class S3D2DClipTriangle
{
public:
    POINTF V[3];
};

class S3D2DClipLine
{
public:
    FLOAT a;
    FLOAT b;
    FLOAT c;
    void BuildClipLine(POINTF v1, POINTF v2);
};

class S3D2DRenderTriangle
{
public:
    Vertex1T V[3];
};


class S3D2DClipList
{
public:

    void DrawArc(float StartX, float StartY, float Radius, float StartAngle, float SweepAngle, bool bCounterDirection = false, float Step = PI/3.0);
    void DrawRect(RECTF RectClip);
    void Clear();



    CArray<S3D2DClipTriangle, S3D2DClipTriangle&> m_ClipList;
};

class S3D2DGraphics
{
public:
    S3D2DGraphics(void);


    void CreateGraphics(LPDIRECT3DSURFACE9 pRenderTarget, LPDIRECT3DDEVICE9 pDevice);
    void SetClip(S3D2DClipList &ClipList);
    void SetSrcTransform(D3DMATRIX &matrix);
    void SetDestTransform(D3DMATRIX &matrix);
    void DrawImage(LPDIRECT3DTEXTURE9 pSrc, D3DTEXTUREADDRESS Addressmode, RECTF SrcRect, RECTF DestRect, 
        DWORD Alpha = 0xFF,  D3DTEXTUREFILTERTYPE FilterType = D3DTEXF_LINEAR);

    void DrawImage(LPDIRECT3DTEXTURE9 pSrc, D3DTEXTUREADDRESS Addressmode, CoordinateInfo *pSrcCoordinate, RECTF DestRect, 
        DWORD Alpha = 0xFF,  D3DTEXTUREFILTERTYPE FilterType = D3DTEXF_LINEAR);
    void Destroy();

protected:
    typedef CArray<S3D2DRenderTriangle, S3D2DRenderTriangle&>  S3D2DRenderTriangleList;
    void ClipTriangleByTriangle(S3D2DRenderTriangle &SrcTriangle, S3D2DClipTriangle &ClipTriangle, S3D2DRenderTriangleList &ResultList);
    void ClipTriangleByLine(S3D2DRenderTriangle &SrcTriangle, S3D2DClipLine &ClipLine, S3D2DRenderTriangleList &ResultList);


    inline double GetTriArea(const Vertex1T& V1, const Vertex1T& V2, const Vertex1T& V3)
                            {
                                return abs(((V1.Pos.x * V2.Pos.y - V1.Pos.y * V2.Pos.x)
                                    + (V2.Pos.x * V3.Pos.y - V2.Pos.y * V3.Pos.x)
                                    + (V3.Pos.x * V1.Pos.y - V3.Pos.y * V1.Pos.x)) / 2);
                            }

    void TriTexcoordInterpolate(const S3D2DRenderTriangle& Triangle, Vertex1T& ResultVertex);

    float Dis();

    CArray<S3D2DClipTriangle, S3D2DClipTriangle&> m_ClipList;
    D3DMATRIX m_SrcMatrix;
    bool      m_bSrcIdentity;
    D3DMATRIX m_DestMatrix;
    bool      m_bDstIdentity;

    LPDIRECT3DSURFACE9 m_pRenderTarget;
    LPDIRECT3DDEVICE9  m_pDevice;
};


#endif