#pragma once
#include "s3renderableobject.h"
#include "S3VMRCaptureSubGraph.h"

class S3VMRWizard;




class S3CaptureObj: public S3RenderableObject
{
public:
    //S3CaptureObj(int iWidth, int iHeight, LPCTSTR DeviceName, int ChannelNo, int Volume);
    S3CaptureObj(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute);
    virtual ~S3CaptureObj();

    virtual HRESULT         Start();
    virtual HRESULT         Stop();
    virtual HRESULT         InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice );
    virtual HRESULT         RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);                           ///device lost handle
    virtual HRESULT         InvalidateDeviceObjects();                        ///device lost handle
    virtual HRESULT         PrepareRender();
    virtual HRESULT         EndRender();
    virtual HRESULT         Pause();
    virtual HRESULT         Resume();
    virtual HRESULT         LoopContent();
    virtual HRESULT         EnableSFRUpload(BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree, CRect ClipInfo);

protected:
    S3VMRCaptureSubGraph   m_SubGraph;
    LONGLONG        m_RestorePositon;


    int             m_MovieWidth;
    int             m_MovieHeight;
    int             m_TextureWidth;
    int             m_TextureHeight;

    LPDIRECT3DTEXTURE9  m_pTexture; 

    CString         m_DeviceName;    
    CString         m_DeviceClsID;
    int             m_Volume;
    int		        m_ChannelNo;
    HRESULT         GetVideoTexture();
    void            ReleaseVideoTexture();

};
