#include "stdafx.h"
#include "S3Signage.h"
#include "S3PluginObj.h"
#include "AttributeParser.h"       
#include "MAMMD3D9.h"
#include "MAMM3DTexture9.h"

S3PluginObj::S3PluginObj(/*const IID PluginID*/const std::tstring& name, int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute)
    :S3RenderableObject(iWidth, iHeight, fScaleRate, Attribute)
{
	m_bIsCaptureObject = FALSE;
    //m_PluginID = PluginID;
    m_name = name;

    // TODO: support capture plug-ins in new plug-in Manager
/*
	const CLSID CLSID_S3Capture = {0x540f4d51, 0x5e08, 0x4817, {0x8d, 0x28, 0x97, 0x4a, 0x68, 0xc8, 0x2a, 0x2b}};
	const CLSID CLSID_HikCam = {0x9f265cc, 0x4b74, 0x4fb8, {0x94, 0x1c, 0x2a, 0x6c, 0xa7, 0xa6, 0x73, 0x23}};
	const CLSID CLSID_DahuaCam = {0xee2c6841, 0xd277, 0x41e1, {0x96, 0x1e, 0x3e, 0x1, 0x7a, 0xe5, 0x64, 0x89}};
	if(IsEqualCLSID(m_PluginID, CLSID_S3Capture))
	{
		m_bIsCaptureObject = TRUE;
	}
	if(IsEqualCLSID(m_PluginID, CLSID_HikCam))
	{
		m_bIsCaptureObject = TRUE;
	}
	if(IsEqualCLSID(m_PluginID, CLSID_DahuaCam))
	{
		m_bIsCaptureObject = TRUE;
	}
*/

    m_PluginObj = NULL;
    m_Player = NULL;
    m_AdvancedPlayer = NULL;

    m_SurfaceFormat = D3DFMT_X8R8G8B8;
    m_pTexture = NULL;
    m_pSysTexture[0] = NULL;
    m_pSysTexture[1] = NULL;

	m_bDoubleBuffer = FALSE;

    m_CurrentSurfaceIndex = -1;

    m_FilePath = GetStringAttrib(m_Attribute, _T("FileName"), _T(""));
}


S3PluginObj::~S3PluginObj(void)
{
    Stop();
    DeleteDeviceObjects();
}


HRESULT S3PluginObj::Start()
{
    if(m_Player)
    {
        return m_Player->Run();
    }else if(m_AdvancedPlayer)
    {
        return m_AdvancedPlayer->Run();
    }
    return E_UNEXPECTED;
}

HRESULT S3PluginObj::Stop()
{
    if(m_Player)
    {
        return m_Player->Stop();
    }
    else if(m_AdvancedPlayer)
    {
        return m_AdvancedPlayer->Stop();
    }
    return E_UNEXPECTED;
}

HRESULT S3PluginObj::InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice )
{
    HRESULT hr;
    AttribList::iterator start, end, it;

    // create plugin object
    hr = S3RenderableObject::InitDeviceObjects(pd3dDevice);
    if(FAILED(hr)) goto FAIL_PATH;

/*
    hr = CoCreateInstance(m_PluginID, NULL, CLSCTX_INPROC_SERVER,
        IID_IS3RenderableObject, (void**)&m_PluginObj );
*/
    m_PluginObj = ((CS3SignageApp*)AfxGetApp())->m_pluginMgr.CreateObject(m_name);
    if(!m_PluginObj)
	{
		hr = E_INVALIDARG;
		goto FAIL_PATH;
	}

    m_AdvancedPlayer = std::dynamic_pointer_cast<IS3ROAdvancedPlayer>(m_PluginObj);
    if (!m_AdvancedPlayer)
    {
        m_Player = std::dynamic_pointer_cast<IS3ROPlayer>(m_PluginObj);
        if (!m_Player)
        {
			hr = E_INVALIDARG;
            goto FAIL_PATH;
        }
    }
/*
    hr = m_PluginObj->QueryInterface(IID_IS3ROAdvancedPlayer, (void**)&m_AdvancedPlayer);
    if(FAILED(hr))
	{
		hr = m_PluginObj->QueryInterface(IID_IS3ROPlayer, (void**)&m_Player);
		if(FAILED(hr)) goto FAIL_PATH;
	}
*/
	
    SIZE RectSize, DrawSize;

    RectSize.cx = (int)m_iWidth;
    RectSize.cy = (int)m_iHeight;

    RO_PROPERTY_SETTING PropertySetting;
    // 1 more property to put player data path
    RO_PROPERTY        *pProperties = new RO_PROPERTY[m_Attribute.size() + 1];

    PropertySetting.propertyCount = m_Attribute.size() + 1;
    PropertySetting.pProperties = pProperties;

    int Index = 0;
    // paser all attribute
	if(false == m_Attribute.empty())
	{
		start = m_Attribute.begin();
		end = m_Attribute.end();

		it = start;
		do{
			StringCchCopy(pProperties[Index].propertyName, 256, it->first);
			StringCchCopy(pProperties[Index].propertyValue, 256, it->second);
			it++;
			if(CString(_T("FileName")).CompareNoCase(pProperties[Index].propertyName) == 0)
			{
				PropertySetting.propertyCount--;;
				continue;
			}
			Index ++;
		}while(it != end);
	}

    // Add player data path
    StringCchCopy(pProperties[Index].propertyName, 256, _T("PlayerTempDataPath"));
    StringCchCopy(pProperties[Index].propertyValue, 256, theApp.GetLanchSettings().LogPath);

    // create player
    if(m_AdvancedPlayer)
    {

        hr = m_AdvancedPlayer->CreateAdvancedPlayer(RectSize, m_FilePath, &PropertySetting, &DrawSize,
            &m_bTransparent, &m_bDoubleBuffer);

    }else if(m_Player)
    {

        hr = m_Player->CreatePlayer(RectSize, m_FilePath, &PropertySetting, &DrawSize,
            &m_bTransparent);
    }
    
    delete [] pProperties;
    if(FAILED(hr)) goto FAIL_PATH;
    // create texture for player
    m_SurfaceWidth = DrawSize.cx;
    m_SurfaceHeight = DrawSize.cy;

    //special path for capture, try YUY2 first
    if(m_bIsCaptureObject)
    {
        m_SurfaceFormat = D3DFMT_YUY2;

        hr = pd3dDevice->CreateTexture(m_SurfaceWidth, m_SurfaceHeight, 1, D3DUSAGE_DYNAMIC, 
            m_SurfaceFormat, D3DPOOL_SYSTEMMEM, &m_pSysTexture[0], NULL);
        if(FAILED(hr))//most adapters don't support YUY2 texture
        {
            m_SurfaceFormat = m_bTransparent ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;
            hr = pd3dDevice->CreateTexture(m_SurfaceWidth, m_SurfaceHeight, 1, D3DUSAGE_DYNAMIC, 
                m_SurfaceFormat, D3DPOOL_SYSTEMMEM, &m_pSysTexture[0], NULL);
            if(FAILED(hr)) goto FAIL_PATH;
        }

        if(m_bDoubleBuffer)
        {

            hr = pd3dDevice->CreateTexture(m_SurfaceWidth, m_SurfaceHeight, 1, D3DUSAGE_DYNAMIC, 
                    m_SurfaceFormat, D3DPOOL_SYSTEMMEM, &m_pSysTexture[1], NULL);
            if(FAILED(hr)) goto FAIL_PATH;
        }
    }
    else
    {
        m_SurfaceFormat = m_bTransparent ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;

        hr = pd3dDevice->CreateTexture(m_SurfaceWidth, m_SurfaceHeight, 1, D3DUSAGE_DYNAMIC, 
            m_SurfaceFormat, D3DPOOL_SYSTEMMEM, &m_pSysTexture[0], NULL);
        if(FAILED(hr)) goto FAIL_PATH;

        if(m_bDoubleBuffer)
        {

            hr = pd3dDevice->CreateTexture(m_SurfaceWidth, m_SurfaceHeight, 1, D3DUSAGE_DYNAMIC, 
                    m_SurfaceFormat, D3DPOOL_SYSTEMMEM, &m_pSysTexture[1], NULL);
            if(FAILED(hr)) goto FAIL_PATH;
        }
    }


    hr = pd3dDevice->CreateTexture(m_SurfaceWidth, m_SurfaceHeight, 1, 0, 
            m_SurfaceFormat, D3DPOOL_DEFAULT, &m_pTexture, NULL);
    if(FAILED(hr)) goto FAIL_PATH;

    return hr;

FAIL_PATH:
    DeleteDeviceObjects();
    return hr;
}

HRESULT S3PluginObj::DeleteDeviceObjects()
{
    m_AdvancedPlayer = NULL;
    m_Player = NULL;
    m_PluginObj = NULL;
    SAFE_RELEASE(m_pSysTexture[0]);
    SAFE_RELEASE(m_pSysTexture[1]);
    SAFE_RELEASE(m_pTexture);
    return S_OK;
}

HRESULT S3PluginObj::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT hr;

    hr = S3RenderableObject::InitDeviceObjects(pd3dDevice);
    if(FAILED(hr)) return hr;

    hr = pd3dDevice->CreateTexture(m_SurfaceWidth, m_SurfaceHeight, 1, 0, 
            m_SurfaceFormat, D3DPOOL_DEFAULT, &m_pTexture, NULL);
    if(FAILED(hr)) return hr;

    LPDIRECT3DSURFACE9 pSrcSurface  = NULL;
    LPDIRECT3DSURFACE9 pDestSurface = NULL;

    m_pTexture->GetSurfaceLevel(0, &pDestSurface);


    m_pSysTexture[m_CurrentSurfaceIndex]->GetSurfaceLevel(0, &pSrcSurface);
    hr = m_pd3dDevice->UpdateSurface(pSrcSurface, NULL, 
        pDestSurface, NULL);

    SAFE_RELEASE(pDestSurface);
    SAFE_RELEASE(pSrcSurface);


    return S_OK;
}

HRESULT S3PluginObj::InvalidateDeviceObjects()
{
    SAFE_RELEASE(m_pTexture);
    return S_OK;
}

HRESULT S3PluginObj::Initalize()
{
    HRESULT hr;
    if(m_AdvancedPlayer)
    {
        hr = m_AdvancedPlayer->Initialize(m_bDoubleBuffer ? 2: 1, (HANDLE *)m_pSysTexture);
    }
    else if(m_Player)
    {
        hr = m_Player->Initialize((HANDLE)m_pSysTexture[0]);
    }

    if(FAILED(hr))
    {
        if(m_bIsCaptureObject)
        {
            SAFE_RELEASE(m_pTexture);
            SAFE_RELEASE(m_pSysTexture[0]);
            SAFE_RELEASE(m_pSysTexture[1]);
            m_SurfaceFormat = m_bTransparent ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;

            hr = m_pd3dDevice->CreateTexture(m_SurfaceWidth, m_SurfaceHeight, 1, D3DUSAGE_DYNAMIC, 
                m_SurfaceFormat, D3DPOOL_SYSTEMMEM, &m_pSysTexture[0], NULL);
            if(FAILED(hr))
            {
                return hr;
            }

            if(m_bDoubleBuffer)
            {

                hr = m_pd3dDevice->CreateTexture(m_SurfaceWidth, m_SurfaceHeight, 1, D3DUSAGE_DYNAMIC, 
                        m_SurfaceFormat, D3DPOOL_SYSTEMMEM, &m_pSysTexture[1], NULL);
                if(FAILED(hr))
                {
                    return hr;
                }
            }
            hr = m_pd3dDevice->CreateTexture(m_SurfaceWidth, m_SurfaceHeight, 1, 0, 
                    m_SurfaceFormat, D3DPOOL_DEFAULT, &m_pTexture, NULL);
            if(FAILED(hr))
            {
                return hr;
            }
            if(m_AdvancedPlayer)
            {
                hr = m_AdvancedPlayer->Initialize(m_bDoubleBuffer ? 2: 1, (HANDLE *)m_pSysTexture);
            }
            else if(m_Player)
            {
                hr = m_Player->Initialize((HANDLE)m_pSysTexture[0]);
            }
        }
    }

    return hr;
}
       
HRESULT S3PluginObj::EnableSFRUpload(BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree)
{
#ifdef MAMM_PLAYER
    MAMM3DTexture9 *pTexture9 = NULL;
    HRESULT hr = m_pTexture->QueryInterface(IID_MAMM3DTexture9, (void **)&pTexture9);

    if(SUCCEEDED(hr))
    {
        D3DSURFACE_DESC SurfaceDesc;
        hr = m_pTexture->GetLevelDesc(0, &SurfaceDesc);

        RECT DisplayRect = *pDisplayRect;

        pTexture9->EnableSFRUploadFreeAngle(bEnabled, bSplit, &DisplayRect, RotateDegree);
        pTexture9->Release();
    }
#endif
    return S_OK;
}

HRESULT S3PluginObj::PrepareRender()
{
    if(m_Player || m_AdvancedPlayer)
    {
        HRESULT hr;
        __RO_RENDERPARAM RenderParam;
        memset(&RenderParam, 0, sizeof(RenderParam));

        if(m_Player)
        {
            hr =  m_Player->PrepareRender(&RenderParam);
            if(FAILED(hr)) return hr;
        }else
        {
            hr =  m_AdvancedPlayer->PrepareRender(&RenderParam);
            if(FAILED(hr)) return hr;
        }


        m_CurrentSurfaceIndex = RenderParam.drawObjectIndex;

        // update surface
        if(RenderParam.contentDirtyRect.right != RenderParam.contentDirtyRect.left)
        {
            LPDIRECT3DSURFACE9 pSrcSurface  = NULL;
            LPDIRECT3DSURFACE9 pDestSurface = NULL;

            m_pTexture->GetSurfaceLevel(0, &pDestSurface);

			POINT DestPoint;

			DestPoint.x = RenderParam.contentDirtyRect.left;
			DestPoint.y = RenderParam.contentDirtyRect.top;

            m_pSysTexture[RenderParam.drawObjectIndex]->GetSurfaceLevel(0, &pSrcSurface);
            hr = m_pd3dDevice->UpdateSurface(pSrcSurface, &RenderParam.contentDirtyRect, 
                pDestSurface, &DestPoint);

            SAFE_RELEASE(pDestSurface);
            SAFE_RELEASE(pSrcSurface);
        }

        m_RenderRect.clear();

        RenderRect PluginRect = (RenderRect(RECTF(RenderParam.renderRect.theRect), 
            RECTF(RenderParam.renderRect.TexCoord.tu0, RenderParam.renderRect.TexCoord.tv0
            , RenderParam.renderRect.TexCoord.tu1, RenderParam.renderRect.TexCoord.tv1)));


        PluginRect.bTransparent = m_bTransparent;
        PluginRect.pTexture = m_pTexture;

        m_RenderRect.push_back(PluginRect);
        return S_OK;

    }
    return E_UNEXPECTED;
}

HRESULT S3PluginObj::EndRender()
{
    if(m_Player)
    {
        return m_Player->EndRender();
    }
    else if(m_AdvancedPlayer)
    {
        return m_AdvancedPlayer->EndRender();
    }
    return E_UNEXPECTED;
}

HRESULT S3PluginObj::ProcessMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(m_Player)
    {
        return m_Player->ProcessMessage(uMsg, wParam, lParam);
    }
    else if(m_AdvancedPlayer)
    {
        return m_AdvancedPlayer->ProcessMessage(uMsg, wParam, lParam);
    }
    return E_UNEXPECTED;
}

HRESULT S3PluginObj::ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(m_Player)
    {
        return m_Player->ProcessMessage(uMsg, wParam, lParam);
    }
    else if(m_AdvancedPlayer)
    {
        return m_AdvancedPlayer->ProcessMessage(uMsg, wParam, lParam);
    }
    return S_OK;
}

HRESULT S3PluginObj::Pause()
{
    if(m_Player)
    {
        return m_Player->Pause();
    }
    else if(m_AdvancedPlayer)
    {
        return m_AdvancedPlayer->Pause();
    }
    return E_UNEXPECTED;
}

HRESULT S3PluginObj::Resume()
{
    if(m_Player)
    {
        return m_Player->Run();
    }
    else if(m_AdvancedPlayer)
    {
        return m_AdvancedPlayer->Run();
    }
    return E_UNEXPECTED;
}