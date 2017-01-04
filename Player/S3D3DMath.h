#pragma once
#include "d3d9.h"
#include "d3d9types.h"          

#define PI 3.1415926535897932384626433832795
#define PI_2 1.5707963267948966192313216916398
#define PI_3 1.0471975511965977461542144610932

//--------------------------
// 4D Vector
//--------------------------
typedef struct S3VECTOR4
{
public:
    S3VECTOR4() {};
    S3VECTOR4( float x, float y, float z, float w );
    ~S3VECTOR4();

    // casting
    operator float* ();
    operator CONST float* () const;

    //// assignment operators
    S3VECTOR4& operator += ( CONST S3VECTOR4& );
    S3VECTOR4& operator -= ( CONST S3VECTOR4& );
    S3VECTOR4& operator *= ( float );
    S3VECTOR4& operator /= ( float );

    //// unary operators
    S3VECTOR4 operator + () const;
    S3VECTOR4 operator - () const;

    //// binary operators
    S3VECTOR4 operator + ( CONST S3VECTOR4& ) const;
    S3VECTOR4 operator - ( CONST S3VECTOR4& ) const;
    S3VECTOR4 operator * ( float ) const;
    S3VECTOR4 operator / ( float ) const;

    friend S3VECTOR4 operator * ( float, CONST S3VECTOR4& );

    BOOL operator == ( CONST S3VECTOR4& ) const;
    BOOL operator != ( CONST S3VECTOR4& ) const;

public:
    union{
        struct{
            float x;
            float y;
            float z;
            float w;
        };
        float m[4];
    };
} S3VECTOR4, *PS3VECTOR4;

void S3Vec4Transform(S3VECTOR4* pDst, S3VECTOR4* pSrc, D3DMATRIX* pMatrix);

void S3SetMatrixIdentity(D3DMATRIX* pDestMatrix);
bool S3IsMatrixIdentity(const D3DMATRIX& DestMatrix);

typedef struct RECTF
{
    float left;
    float top;
    float right;
    float bottom;
    
    float Width();
    float Height();

    RECTF();
    RECTF(float left, float top, float right, float bottom);
    RECTF(RECT Rect);
    void  Clip(RECTF ClipRect);

    BOOL  bIntersect(RECTF RefRect);
    BOOL  bInside(float XPos, float YPos);
} 	RECTF;

typedef struct CoordinateInfo
{
    float       tu;
    float       tv;
}CoordinateInfo;



typedef struct Vertex1T
{
    S3VECTOR4   Pos;
    float       tu;
    float       tv;
}Vertex1T;

typedef struct Vertex2T
{
    S3VECTOR4   Pos;
    float       tu0;
    float       tv0;
    float       tu1;
    float       tv1;


}Vertex2T;

typedef struct Vertex3T
{
    S3VECTOR4   Pos;
    float       tu0;
    float       tv0;

    float       tu1;
    float       tv1;

    float       tu2;
    float       tv2;

}Vertex3T;

typedef struct Vertex4T
{
    S3VECTOR4   Pos;
    CoordinateInfo  T[4];
}Vertex4T;

typedef struct Vertex5T
{
    S3VECTOR4       Pos;
    CoordinateInfo  T[5];
}Vertex5T;


typedef struct Vertex6T
{
    S3VECTOR4       Pos;
    CoordinateInfo  T[6];
}Vertex6T;

typedef struct Vertex7T
{
    S3VECTOR4       Pos;
    CoordinateInfo  T[7];
}Vertex7T;

typedef struct Vertex8T
{
    S3VECTOR4       Pos;
    CoordinateInfo  T[8];
}Vertex8T;


typedef struct RenderRect
{
    RECTF Position;
    CoordinateInfo TextureCoordinate[4];

    LPDIRECT3DTEXTURE9  pTexture;  

    BOOL                bTransparent;

    float Width();
    float Height();

    RenderRect();
    RenderRect(RECTF p, RECTF t);

    void  Clip(RECTF ClipRect);

}RenderRect;


FLOAT GetRotatedScaleOnFitScreen(FLOAT Degree, RECTF* RotateRect, SIZEL *sizlFit);  
FLOAT GetRotatedScaleOnReverseFitScreen(FLOAT Degree, RECTF* RotateRect, SIZEL *sizlFit);   
S3VECTOR4 GetRotatedVector(FLOAT RotationDegree, S3VECTOR4 &v, SIZEL &sizlBitmap);
