#pragma once
#include "s3renderableobject.h"

enum S3S_TEXT_DIRECTION{
    S3S_TEXT_DIRECTION_NONE                   = 0,
    S3S_TEXT_DIRECTION_LEFT_RIGHT             = 1,
    S3S_TEXT_DIRECTION_RIGHT_LEFT             = 2,
    S3S_TEXT_DIRECTION_UP_DOWN                = 3,
    S3S_TEXT_DIRECTION_DOWN_UP                = 4,
    S3S_TEXT_DIRECTION_RANDOM                 = 255,
    S3S_TEXT_DIRECTION_ALIGNDW  = 0xFFFFFFFF,
};

typedef struct TEXT_INFO
{
    LPDIRECT3DTEXTURE9      pTexture;
    int                     ImageWidth;
    int                     ImageHeight;
    float                   TextFileScale;

    int                     TextureWidth;
    int                     WrappedLineCount;
    CString                 Filename;
}TEXT_INFO;


class S3TextFileObj :
    public S3RenderableObject
{
public:
    S3TextFileObj(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute, S3SIGNAGE_TEXTFILE &TextFile, int nFPS);
    virtual ~S3TextFileObj();

    virtual HRESULT         InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice );
    virtual HRESULT         Initalize();   
    virtual HRESULT         RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);///device lost handle
    virtual HRESULT         InvalidateDeviceObjects();                        ///device lost handle
    virtual HRESULT         DeleteDeviceObjects();                            ///device lost handle

    virtual HRESULT         PrepareRender();
    virtual HRESULT         EndRender();
    virtual HRESULT         Pause();
    virtual HRESULT         Resume();
    virtual HRESULT         Start();  

protected:
    float                   m_Speed;
    S3S_TEXT_DIRECTION      m_TextDirection;
    BOOL                    m_bVerticalLayout;
    S3SIGNAGE_TEXTFILE      m_TextFileInfo;

    vector<TEXT_INFO>       m_TextInfo;
    int                     m_CurrentText;

    DWORD                   m_MessageStartTime;
    int                     m_TextStartXPos;
    int                     m_TextStartYPos;

    float                   m_CurrentXPos;
    float                   m_CurrentYPos;
    int                     m_nFPS;
    BOOL                    m_bPaused;

    HRESULT UploadText(TEXT_INFO &TextInfo, CImage &TextImage);
    HRESULT RenderText(TEXT_INFO &TextInfo, float XPos, float YPos);

    HRESULT SwitchText(DWORD TimeDelta);

    float GetCurTextLength();
    float GetCurTextHeight();
};

