#include "stdafx.h"
#include "S3D3DMath.h"

S3VECTOR4::S3VECTOR4(float x1, float y1, float z1, float w1)
{
    x = x1;
    y = y1;
    z = z1;
    w = w1;
}

S3VECTOR4::~S3VECTOR4(void)
{
}

S3VECTOR4 S3VECTOR4::operator + () const
{
    return *this;
}
 
S3VECTOR4 S3VECTOR4::operator - () const
{
    S3VECTOR4 retVal;
    retVal.x = -x;
    retVal.y = -y;
    retVal.z = -z;
    retVal.w = -w;
    return retVal;
}

S3VECTOR4 S3VECTOR4::operator + ( CONST S3VECTOR4& tVector) const
{
    S3VECTOR4 retVal;
    retVal.x = x + tVector.x;
    retVal.y = y + tVector.y;
    retVal.z = z + tVector.z;
    retVal.w = w + tVector.w;
    return retVal;
}

S3VECTOR4 S3VECTOR4::operator - ( CONST S3VECTOR4& tVector) const
{
    S3VECTOR4 retVal;
    retVal.x = x - tVector.x;
    retVal.y = y - tVector.y;
    retVal.z = z - tVector.z;
    retVal.w = w - tVector.w;
    return retVal;
}

S3VECTOR4& S3VECTOR4::operator += ( CONST S3VECTOR4& tVector)
{
    x += tVector.x;
    y += tVector.y;
    z += tVector.z;
    w += tVector.w;
    return *this;
}

S3VECTOR4& S3VECTOR4::operator -= ( CONST S3VECTOR4& tVector)
{
    x -= tVector.x;
    y -= tVector.y;
    z -= tVector.z;
    w -= tVector.w;
    return *this;
}

S3VECTOR4 S3VECTOR4::operator * ( float factor) const
{
    S3VECTOR4 retVal;
    retVal.x = x*factor;
    retVal.y = y*factor;
    retVal.z = z*factor;
    retVal.w = w*factor;
    return retVal;
}

S3VECTOR4 S3VECTOR4::operator / ( float factor) const
{
    S3VECTOR4 retVal;
    retVal.x = x/factor;
    retVal.y = y/factor;
    retVal.z = z/factor;
    retVal.w = w/factor;
    return retVal;
}

S3VECTOR4& S3VECTOR4::operator *= ( float factor)
{
    x*=factor;
    y*=factor;
    z*=factor;
    w*=factor;
    return *this;
}

S3VECTOR4& S3VECTOR4::operator /= ( float factor)
{
    x/=factor;
    y/=factor;
    z/=factor;
    w/=factor;
    return *this;
}

S3VECTOR4::operator float* ()
{
    return m;
}

S3VECTOR4::operator CONST float* () const
{
    return m;
}

BOOL S3VECTOR4::operator == ( CONST S3VECTOR4& dst) const
{
    if(memcmp(m, dst.m, sizeof(S3VECTOR4)))
    {
        return FALSE;
    }
    return TRUE;
}

BOOL S3VECTOR4::operator != ( CONST S3VECTOR4& dst) const
{
    if(memcmp(m, dst.m, sizeof(S3VECTOR4)))
    {
        return TRUE;
    }
    return FALSE;
}

S3VECTOR4 operator * ( float factor, CONST S3VECTOR4& Src)
{
    S3VECTOR4 retVal;
    retVal.x = factor * Src.x;
    retVal.y = factor * Src.y;
    retVal.z = factor * Src.z;
    retVal.y = factor * Src.w;
    return retVal;
}

void S3SetMatrixIdentity(D3DMATRIX* pDestMatrix)
{
    float* pData = &pDestMatrix->m[0][0];
    *pData++ = 1.0f;
    *pData++ = 0.0f;
    *pData++ = 0.0f;
    *pData++ = 0.0f;

    *pData++ = 0.0f;
    *pData++ = 1.0f;
    *pData++ = 0.0f;
    *pData++ = 0.0f;

    *pData++ = 0.0f;
    *pData++ = 0.0f;
    *pData++ = 1.0f;
    *pData++ = 0.0f;

    *pData++ = 0.0f;
    *pData++ = 0.0f;
    *pData++ = 0.0f;
    *pData   = 1.0f;
}

bool S3IsMatrixIdentity(const D3DMATRIX& DestMatrix)
{
    static D3DMATRIX Identity = { 1.0f, 0.0f, 0.0f, 0.0f, \
                                  0.0f, 1.0f, 0.0f, 0.0f, \
                                  0.0f, 0.0f, 1.0f, 0.0f, \
                                  0.0f, 0.0f, 0.0f, 1.0f   };
    const float* pData = &DestMatrix.m[0][0];
    const float* pComp = &Identity.m[0][0];
    if(memcmp(pData, pComp, sizeof(D3DMATRIX)))
    {
        return false;
    }
    return true;
}

void S3Vec4Transform(S3VECTOR4* pDst, S3VECTOR4* pSrc, D3DMATRIX* pMatrix)
{
    pDst->x = pSrc->x * pMatrix->_11 + pSrc->y * pMatrix->_21 + pSrc->z * pMatrix->_31 + pSrc->w * pMatrix->_41;
    pDst->y = pSrc->x * pMatrix->_12 + pSrc->y * pMatrix->_22 + pSrc->z * pMatrix->_32 + pSrc->w * pMatrix->_42;
    pDst->z = pSrc->x * pMatrix->_13 + pSrc->y * pMatrix->_23 + pSrc->z * pMatrix->_33 + pSrc->w * pMatrix->_43;
    pDst->w = pSrc->x * pMatrix->_14 + pSrc->y * pMatrix->_24 + pSrc->z * pMatrix->_34 + pSrc->w * pMatrix->_44;
}

void RotatePoint(POINTF ptCenter, POINTF& ptR, float Degree)
{
    POINTF ptO = ptR;

    ptR.x = (ptO.x - ptCenter.x) * cos(Degree) 
        + (ptO.y - ptCenter.y)*sin(Degree) + ptCenter.x;
    ptR.y = - (ptO.x - ptCenter.x) * sin(Degree) 
        + (ptO.y - ptCenter.y)*cos(Degree) + ptCenter.y;
}
        
FLOAT GetRotatedScaleOnReverseFitScreen(FLOAT Degree, RECTF* RotateRect, SIZEL *sizlFit) 
{
    FLOAT width;
    FLOAT height;

    width  = RotateRect->right - RotateRect->left;
    height = RotateRect->bottom - RotateRect->top;

    POINTF ptCenter;

    ptCenter.x = sizlFit->cx/2;
    ptCenter.y = sizlFit->cy/2; 

    // lb, lt, rb, rt
    POINTF ptLB, ptLT, ptRB, ptRT;
    ptLB.x = RotateRect->left; ptLB.y = RotateRect->bottom;
    ptLT.x = RotateRect->left; ptLT.y = RotateRect->top;
    ptRB.x = RotateRect->right; ptRB.y = RotateRect->bottom;
    ptRT.x = RotateRect->right; ptRT.y = RotateRect->top;

    RotatePoint(ptCenter, ptLB, Degree);
    RotatePoint(ptCenter, ptLT, Degree);
    RotatePoint(ptCenter, ptRB, Degree);
    RotatePoint(ptCenter, ptRT, Degree);

    FLOAT top = min(min(min(ptLT.y, ptLB.y), ptRT.y), ptRB.y);
    FLOAT bottom = max(max(max(ptLT.y, ptLB.y), ptRT.y), ptRB.y);
    FLOAT left = min(min(min(ptLT.x, ptLB.x), ptRT.x), ptRB.x);
    FLOAT right = max(max(max(ptLT.x, ptLB.x), ptRT.x), ptRB.x);

    FLOAT cx = right - left;
    FLOAT cy = bottom - top;   

    FLOAT widthFit;
    FLOAT heightFit;

    widthFit  = sizlFit->cx;
    heightFit = sizlFit->cy;

    FLOAT fScale1 = cx / widthFit;
    FLOAT fScale2 = cy / heightFit;
    if (cy <= heightFit * fScale1)
    {
        return fScale1;
    }

    return fScale2;
}

FLOAT GetRotatedScaleOnFitScreen(FLOAT Degree, RECTF* RotateRect, SIZEL *sizlFit)
{
    FLOAT width;
    FLOAT height;

    width  = RotateRect->right - RotateRect->left;
    height = RotateRect->bottom - RotateRect->top;

    POINTF ptCenter;

    ptCenter.x = sizlFit->cx/2;
    ptCenter.y = sizlFit->cy/2; 

    // lb, lt, rb, rt
    POINTF ptLB, ptLT, ptRB, ptRT;
    ptLB.x = RotateRect->left; ptLB.y = RotateRect->bottom;
    ptLT.x = RotateRect->left; ptLT.y = RotateRect->top;
    ptRB.x = RotateRect->right; ptRB.y = RotateRect->bottom;
    ptRT.x = RotateRect->right; ptRT.y = RotateRect->top;

    RotatePoint(ptCenter, ptLB, Degree);
    RotatePoint(ptCenter, ptLT, Degree);
    RotatePoint(ptCenter, ptRB, Degree);
    RotatePoint(ptCenter, ptRT, Degree);

    FLOAT top = min(min(min(ptLT.y, ptLB.y), ptRT.y), ptRB.y);
    FLOAT bottom = max(max(max(ptLT.y, ptLB.y), ptRT.y), ptRB.y);
    FLOAT left = min(min(min(ptLT.x, ptLB.x), ptRT.x), ptRB.x);
    FLOAT right = max(max(max(ptLT.x, ptLB.x), ptRT.x), ptRB.x);

    FLOAT cx = right - left;
    FLOAT cy = bottom - top;   

    FLOAT widthFit;
    FLOAT heightFit;

    widthFit  = sizlFit->cx;
    heightFit = sizlFit->cy;

    FLOAT fScale1 = widthFit / cx;
    FLOAT fScale2 = heightFit / cy;
    if (cy * fScale1 <= heightFit)
    {
        return fScale1;
    }

    return fScale2;
}

S3VECTOR4 GetRotatedVector(FLOAT RotationDegree, S3VECTOR4 &v, SIZEL &sizlBitmap)
{
    POINTF ptCenter;
    ptCenter.x = sizlBitmap.cx / 2;     
    ptCenter.y = sizlBitmap.cy / 2;

    POINTF ptR;
    ptR.x = v.x;
    ptR.y = v.y;

    S3VECTOR4 RotatedVector = v;
    RotatePoint(ptCenter, ptR, RotationDegree);
    RotatedVector.x = ptR.x;
    RotatedVector.y = ptR.y;
   
    return RotatedVector;
}

float RECTF::Width()
{
    return right - left;
}

float RECTF::Height()
{
    return bottom - top;
}


RECTF::RECTF()
{

}

RECTF::RECTF(FLOAT ileft, FLOAT itop, FLOAT iright, FLOAT ibottom)
:left(ileft), top(itop), right(iright), bottom(ibottom)
{

}

RECTF::RECTF(RECT Rect)
:left((FLOAT)Rect.left), top((FLOAT)Rect.top), right((FLOAT)Rect.right), bottom((FLOAT)Rect.bottom)
{

}

void RECTF::Clip(RECTF ClipRect)
{
    left  = max(left, ClipRect.left);
    right = min(right, ClipRect.right);
    top   = max(top, ClipRect.top);
    bottom= min(bottom, ClipRect.bottom);

    right = max(left, right);
    bottom = max(top, bottom);
}


BOOL  RECTF::bIntersect(RECTF RefRect)
{
    RECTF TextRect;

    TextRect.left  = max(left, RefRect.left);
    TextRect.right = min(right, RefRect.right);
    TextRect.top   = max(top, RefRect.top);
    TextRect.bottom= min(bottom, RefRect.bottom);

    TextRect.right = max(TextRect.left, TextRect.right);
    TextRect.bottom = max(TextRect.top, TextRect.bottom);


    return (TextRect.Width() > 0 && TextRect.Height() > 0);
}

BOOL  RECTF::bInside(float XPos, float YPos)
{
    return (( left <= XPos) && ( right >= XPos) && ( top <= YPos) && ( bottom >= YPos));
}

RenderRect::RenderRect()
{
}

RenderRect::RenderRect(RECTF p, RECTF t)
:Position(p)
{
    TextureCoordinate[0].tu = t.left;   TextureCoordinate[0].tv = t.bottom;
    TextureCoordinate[1].tu = t.left;   TextureCoordinate[1].tv = t.top;
    TextureCoordinate[2].tu = t.right;   TextureCoordinate[2].tv = t.bottom;
    TextureCoordinate[3].tu = t.right;   TextureCoordinate[3].tv = t.top;


}


float RenderRect::Width()
{
    return Position.Width();
}

float RenderRect::Height()
{
    return Position.Height();
}




void  RenderRect::Clip(RECTF ClipRect )
{
    RECTF NewPos;
    NewPos.left  = max(Position.left, ClipRect.left);
    NewPos.right = min(Position.right, ClipRect.right);
    NewPos.top   = max(Position.top, ClipRect.top);
    NewPos.bottom= min(Position.bottom, ClipRect.bottom);

    NewPos.right = max(NewPos.left, NewPos.right);
    NewPos.bottom = max(NewPos.top, NewPos.bottom);


    if((NewPos.left == Position.left) && 
        (NewPos.top == Position.top) &&
        (NewPos.right == Position.right) &&
        (NewPos.bottom == Position.bottom))
    {
        return;
    }

    RECTF NewPosAlpha;

    NewPosAlpha.left = (NewPos.left - Position.left) /(Position.right - Position.left);
    NewPosAlpha.right = (NewPos.right - Position.left) /(Position.right - Position.left);

    NewPosAlpha.top = (NewPos.top - Position.top) /(Position.bottom - Position.top);
    NewPosAlpha.bottom = (NewPos.bottom - Position.top) /(Position.bottom - Position.top);


    CoordinateInfo NewCoordinate[4];

    NewCoordinate[0].tu = (1 - NewPosAlpha.bottom) * TextureCoordinate[1].tu + NewPosAlpha.bottom * TextureCoordinate[0].tu;
    NewCoordinate[0].tv = (1 - NewPosAlpha.bottom) * TextureCoordinate[1].tv + NewPosAlpha.bottom * TextureCoordinate[0].tv;

    NewCoordinate[1].tu = (1 - NewPosAlpha.top) * TextureCoordinate[1].tu + NewPosAlpha.top * TextureCoordinate[0].tu;
    NewCoordinate[1].tv = (1 - NewPosAlpha.top) * TextureCoordinate[1].tv + NewPosAlpha.top * TextureCoordinate[0].tv;

    NewCoordinate[2].tu = (1 - NewPosAlpha.bottom) * TextureCoordinate[3].tu + NewPosAlpha.bottom * TextureCoordinate[2].tu;
    NewCoordinate[2].tv = (1 - NewPosAlpha.bottom) * TextureCoordinate[3].tv + NewPosAlpha.bottom * TextureCoordinate[2].tv;


    NewCoordinate[3].tu = (1 - NewPosAlpha.top) * TextureCoordinate[3].tu + NewPosAlpha.top * TextureCoordinate[2].tu;
    NewCoordinate[3].tv = (1 - NewPosAlpha.top) * TextureCoordinate[3].tv + NewPosAlpha.top * TextureCoordinate[2].tv;




    TextureCoordinate[0].tu = (1 - NewPosAlpha.left) * NewCoordinate[0].tu + NewPosAlpha.left * NewCoordinate[2].tu;
    TextureCoordinate[0].tv = (1 - NewPosAlpha.left) * NewCoordinate[0].tv + NewPosAlpha.left * NewCoordinate[2].tv;


    TextureCoordinate[2].tu = (1 - NewPosAlpha.right) * NewCoordinate[0].tu + NewPosAlpha.right * NewCoordinate[2].tu;
    TextureCoordinate[2].tv = (1 - NewPosAlpha.right) * NewCoordinate[0].tv + NewPosAlpha.right * NewCoordinate[2].tv;


    TextureCoordinate[1].tu = (1 - NewPosAlpha.left) * NewCoordinate[1].tu + NewPosAlpha.left * NewCoordinate[3].tu;
    TextureCoordinate[1].tv = (1 - NewPosAlpha.left) * NewCoordinate[1].tv + NewPosAlpha.left * NewCoordinate[3].tv;


    TextureCoordinate[3].tu = (1 - NewPosAlpha.right) * NewCoordinate[1].tu + NewPosAlpha.right * NewCoordinate[3].tu;
    TextureCoordinate[3].tv = (1 - NewPosAlpha.right) * NewCoordinate[1].tv + NewPosAlpha.right * NewCoordinate[3].tv;


    Position = NewPos;
}