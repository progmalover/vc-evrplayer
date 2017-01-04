#pragma once
#include "S3RenderableObject.h"
#include "CommonLib/RenderableObject.h"
#include "S3RenderableObjectguids.h"

class S3PluginObj :
    public S3RenderableObject
{
public:
    S3PluginObj(const std::tstring& name, int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute);
    virtual ~S3PluginObj();

    virtual HRESULT         Start();
    virtual HRESULT         Stop();
    virtual HRESULT         InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice );
    virtual HRESULT         Initalize();

    virtual HRESULT         RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);                           ///device lost handle
    virtual HRESULT         InvalidateDeviceObjects();                        ///device lost handle
    virtual HRESULT         DeleteDeviceObjects();                            ///device lost handle

    virtual HRESULT         PrepareRender();
    virtual HRESULT         EndRender();

    virtual HRESULT         ProcessMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam); ///Mouse message process
    virtual HRESULT         ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual HRESULT         Pause();
    virtual HRESULT         Resume();   

    virtual HRESULT         EnableSFRUpload(BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree);
private:

    //IID                             m_PluginID;
    std::tstring                    m_name;
    std::shared_ptr<IS3RenderableObject>    m_PluginObj;
    std::shared_ptr<IS3ROPlayer>            m_Player;
    std::shared_ptr<IS3ROAdvancedPlayer>    m_AdvancedPlayer;

        //textures
    LPDIRECT3DTEXTURE9              m_pSysTexture[2];
    LPDIRECT3DTEXTURE9              m_pTexture;

    D3DFORMAT                       m_SurfaceFormat;
    int                             m_SurfaceWidth;
    int                             m_SurfaceHeight;

    BOOL                            m_bTransparent;
    BOOL                            m_bDoubleBuffer;
    int                             m_CurrentSurfaceIndex;

    CString                         m_FilePath;

	BOOL							m_bIsCaptureObject;
};

