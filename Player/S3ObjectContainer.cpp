#include "stdafx.h"
#include "S3RenderableObject.h"
#include "S3ObjectContainer.h"
#include "S3MovieObj.h"
#include "S3CaptureObj.h"
#include "S3SoundObj.h"
#include "S3PluginObj.h"
#include "S3RectObj.h"
#include "S3ArcObj.h"
#include "S3TextFileObj.h"
#include "S3TransitionProvider.h"
#include "Utilities/CrashRptHelper.h"     
#include "AttributeParser.h"
#include "S3Utility.h"
#include "Utilities/SysCall.h"
#include "Utilities/StringUtility.h"
#include "Utilities/Singleton.h"

S3ObjectContainer::S3ObjectContainer(S3SIGNAGE_CONTENT  &Content, float Scale,  int nFPS, FLOAT RotateDegree)
{
    m_hThread = NULL;
    m_ZOrder = 0;
    m_Content = Content;
    m_Scale = Scale;
    m_RotateDegree = RotateDegree;

    m_pRenderableObj = NULL;
    m_pTransition = NULL;

    m_bInTransitionInit = FALSE;
    m_bOutTransitionInit = FALSE;

    m_ScreenRect.left = (float)m_Content.XPos;
    m_ScreenRect.right = (float)(m_Content.XPos +  m_Content.Width );
    m_ScreenRect.top = (float)m_Content.YPos;
    m_ScreenRect.bottom = (float)(m_Content.YPos +  m_Content.Height );
    

        // scale rect to control the render size
    float AspectRatio = m_ScreenRect.Width()/ m_ScreenRect.Height();;

    float MaxSize = 4096;

    if(m_Content.TypeName.CompareNoCase(_T("S3FlashPlayer")) == 0)
    {
        MaxSize = 2048;
    }

    if(AspectRatio >= 1)
    {
        m_ContentWidth = min((int)(m_ScreenRect.Width() + 0.5f), MaxSize);
        m_ContentHeight = (int)(m_ContentWidth /AspectRatio + 0.5f);
    }else
    {
        m_ContentHeight = min((int)(m_ScreenRect.Height() + 0.5f), MaxSize);
        m_ContentWidth = (int)(m_ContentHeight * AspectRatio + 0.5f);
    }

    m_ContentScale = m_ScreenRect.Width()/m_ContentWidth;

    m_nFPS = nFPS;
    m_ThreadStatus = 2;
    if(m_Content.TypeName.CompareNoCase(_T("Movie")) == 0)
    {
        m_pRenderableObj = new S3MovieObj(m_ContentWidth, m_ContentHeight, Scale/m_ContentScale, m_Content.Attribute);
    }else if(m_Content.TypeName.CompareNoCase(_T("Capture")) == 0)
    {
        UINT CaptureNo = StringUtility::stoi(GetStringAttrib(m_Content.Attribute, _T("CaptureNo"), _T("")).GetString());

        CaptureDevice device;
        if (Singleton<S3Utility>::Instance()->GetCaptureDevice(CaptureNo, device))
        {  
            m_pRenderableObj = new S3CaptureObj(m_ContentWidth, m_ContentHeight, Scale/m_ContentScale, m_Content.Attribute);
        }

    }else if(m_Content.TypeName.CompareNoCase( _T("Audio")) == 0)
    {
        m_pRenderableObj = new S3SoundObj(m_ContentWidth, m_ContentHeight, Scale/m_ContentScale, m_Content.Attribute);
    }else if(m_Content.TypeName.CompareNoCase( _T("Rectangle")) == 0)
    {
        m_pRenderableObj = new S3RectObj(m_ContentWidth, m_ContentHeight, Scale/m_ContentScale, m_Content.Attribute);
    }
    else if(m_Content.TypeName.CompareNoCase( _T("Ellipse")) == 0)
    {
        m_pRenderableObj = new S3ArcObj(m_ContentWidth, m_ContentHeight, Scale/m_ContentScale, m_Content.Attribute);
    }
    else if(m_Content.TypeName.CompareNoCase( _T("Arc")) == 0)
    {
        m_pRenderableObj = new S3ArcObj(m_ContentWidth, m_ContentHeight, Scale/m_ContentScale, m_Content.Attribute);
    }
    else if(m_Content.TypeName.CompareNoCase( _T("ScrollTextFile")) == 0)
    {
        m_pRenderableObj = new S3TextFileObj(m_ContentWidth, m_ContentHeight, Scale/m_ContentScale, m_Content.Attribute, m_Content.TextFile, m_nFPS);
    }
    else
    {
#if 0
        BOOL bFindObject = FALSE;
        int ReturnVal = ERROR_SUCCESS;

        HKEY hRegisteredPluginKey = NULL;
        ReturnVal = RegOpenKey(HKEY_LOCAL_MACHINE, _T("Software\\S3Graphics\\RenderableObjects\\"), &hRegisteredPluginKey);
    
        int Index = 0;

        while(ReturnVal == ERROR_SUCCESS)
        {
            TCHAR PluginName[256];
            DWORD StringLength = 256;

            ReturnVal = RegEnumKeyEx(hRegisteredPluginKey, Index, PluginName, &StringLength, NULL, NULL, NULL, NULL);
            
            if(m_Content.TypeName.CompareNoCase(PluginName) == 0)
            {
                HKEY PluginKey = NULL;
                ReturnVal = RegOpenKey(hRegisteredPluginKey, PluginName, &PluginKey);
                if(ReturnVal == ERROR_SUCCESS)
                {
                    TCHAR PluginGUID[256];
                    DWORD GUIDLength = 256;   
                    TCHAR ValueName[256];
                    DWORD ValueSize = 256;;

                    ReturnVal = RegEnumValue(PluginKey, 0, ValueName, &ValueSize, NULL, NULL,
                        (LPBYTE)PluginGUID, &GUIDLength);

                    CString ID_Plugin = PluginGUID;
                    ID_Plugin.TrimLeft(_T("CLSID\\"));

                    IID CLSID_Plugin;
                    CLSIDFromString((LPOLESTR)(LPCTSTR)ID_Plugin, &CLSID_Plugin);
					
                    m_pRenderableObj = new S3PluginObj(CLSID_Plugin, m_ContentWidth, m_ContentHeight, Scale/m_ContentScale, m_Content.Attribute);
                }
                RegCloseKey(PluginKey);
                bFindObject = TRUE;
                break;
            }
            Index ++;
        }
        RegCloseKey(hRegisteredPluginKey);

        if(!bFindObject)
        {
            DbgMsg(_T("Error: Can not Create Object %s"), m_Content.TypeName);
        }
#endif
/*
        IID CLSID_Plugin;
        if (!((CS3SignageApp*)AfxGetApp())->m_pluginMgr.FindIid(m_Content.TypeName.GetString(), CLSID_Plugin))
        {
            DbgMsg(_T("Error: Can not Create Object %s"), m_Content.TypeName);
        }
        else
*/
        {
            m_pRenderableObj = new S3PluginObj(m_Content.TypeName.GetString(), m_ContentWidth, m_ContentHeight, Scale/m_ContentScale, m_Content.Attribute);
        }
    }

    ParserTransition(m_InTransition, m_Content.TransitionIn);
    ParserTransition(m_OutTransition, m_Content.TransitionOut);


    //invert direction for transitionout

	if(m_InTransition.Effect == S3S_EFFECT_ROLL)
	{
		RevertTransitionDirection(m_InTransition);
    }

	if (m_OutTransition.Effect != S3S_EFFECT_TURNOVER && m_OutTransition.Effect != S3S_EFFECT_ROLL)
	{
		RevertTransitionDirection(m_OutTransition);
	}
    m_StartTime = 0;

    m_pBGTexture = NULL;
}

VOID S3ObjectContainer::RevertTransitionDirection(S3SIGNAGE_TRANSITION_SETTING &Transition)
{
	switch(Transition.EffectDirection)
	{
	case S3S_EFFECT_DIRECTION_UPPER:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_LOWER;
		break;
	case S3S_EFFECT_DIRECTION_UPPER_RIGHT:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_LOWER_LEFT;
		break;
	case S3S_EFFECT_DIRECTION_RIGHT:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_LEFT;
		break;
	case S3S_EFFECT_DIRECTION_LOWER_RIGHT:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_UPPER_LEFT;
		break;
	case S3S_EFFECT_DIRECTION_LOWER:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_UPPER;
		break;
	case S3S_EFFECT_DIRECTION_LOWER_LEFT:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_UPPER_RIGHT;
		break;
	case S3S_EFFECT_DIRECTION_LEFT:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_RIGHT;
		break;
	case S3S_EFFECT_DIRECTION_UPPER_LEFT:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_LOWER_RIGHT;
		break;
	case S3S_EFFECT_DIRECTION_CLOCKWISE:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_COUNTERCLOCKWISE;
		break;
	case S3S_EFFECT_DIRECTION_COUNTERCLOCKWISE:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_CLOCKWISE;
		break;
	case S3S_EFFECT_DIRECTION_LEFT_RIGHT:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_RIGHT_LEFT;
		break;
	case S3S_EFFECT_DIRECTION_RIGHT_LEFT:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_LEFT_RIGHT;
		break;
	case S3S_EFFECT_DIRECTION_UP_DOWN:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_DOWN_UP;
		break;
	case S3S_EFFECT_DIRECTION_DOWN_UP:
		Transition.EffectDirection = S3S_EFFECT_DIRECTION_UP_DOWN;
		break;

	}
}

S3ObjectContainer::~S3ObjectContainer(void)
{
    WaitCreateThreadFinish();

    SAFE_RELEASE(m_pBGTexture);
    SAFE_DELETE(m_pRenderableObj);
    SAFE_DELETE(m_pTransition);

    if(m_hThread)    CloseHandle(m_hThread);
}

HRESULT S3ObjectContainer::ParserTransition(S3SIGNAGE_TRANSITION_SETTING &TransitionSetting, S3SIGNAGE_TRANSIION &TransitionDesc)
{
    TransitionSetting.Effect = S3S_EFFECT_NONE;
    TransitionSetting.EffectDuration = TransitionDesc.Duration;

    if(TransitionDesc.Name.CompareNoCase(_T("Random")) == 0)   TransitionSetting.Effect = S3S_EFFECT_RANDOM;
    if(TransitionDesc.Name.CompareNoCase(_T("Wipe")) == 0)   TransitionSetting.Effect = S3S_EFFECT_WIPE;
    if(TransitionDesc.Name.CompareNoCase(_T("Expand")) == 0)   TransitionSetting.Effect = S3S_EFFECT_EXPAND;
    if(TransitionDesc.Name.CompareNoCase(_T("Slide")) == 0)   TransitionSetting.Effect = S3S_EFFECT_SLIDEIN;
    if(TransitionDesc.Name.CompareNoCase(_T("Blind")) == 0)   TransitionSetting.Effect = S3S_EFFECT_BLIND;
    if(TransitionDesc.Name.CompareNoCase(_T("TurnOver")) == 0)   TransitionSetting.Effect = S3S_EFFECT_TURNOVER;
    if(TransitionDesc.Name.CompareNoCase(_T("Roll")) == 0)   TransitionSetting.Effect = S3S_EFFECT_ROLL;
    if(TransitionDesc.Name.CompareNoCase(_T("Wheel")) == 0)   TransitionSetting.Effect = S3S_EFFECT_WHEEL;
    if(TransitionDesc.Name.CompareNoCase(_T("Clock")) == 0)   TransitionSetting.Effect = S3S_EFFECT_CLOCK;
    if(TransitionDesc.Name.CompareNoCase(_T("Wave")) == 0)   TransitionSetting.Effect = S3S_EFFECT_WAVE;
    if(TransitionDesc.Name.CompareNoCase(_T("Fade")) == 0)   TransitionSetting.Effect = S3S_EFFECT_FADE;
    if(TransitionDesc.Name.CompareNoCase(_T("Block")) == 0)   TransitionSetting.Effect = S3S_EFFECT_BLOCK;
    if(TransitionDesc.Name.CompareNoCase(_T("Round")) == 0)   TransitionSetting.Effect = S3S_EFFECT_ROUND;
    if(TransitionDesc.Name.CompareNoCase(_T("Screw")) == 0)   TransitionSetting.Effect = S3S_EFFECT_SCREW;
    if(TransitionDesc.Name.CompareNoCase(_T("Plus")) == 0)   TransitionSetting.Effect = S3S_EFFECT_PLUS;


    TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_NONE;

    if(TransitionDesc.Direction.CompareNoCase(_T("Upper")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_UPPER;
    if(TransitionDesc.Direction.CompareNoCase(_T("UpperRight")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_UPPER_RIGHT;
    if(TransitionDesc.Direction.CompareNoCase(_T("Right")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_RIGHT;
    if(TransitionDesc.Direction.CompareNoCase(_T("LowerRight")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_LOWER_RIGHT;
    if(TransitionDesc.Direction.CompareNoCase(_T("Lower")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_LOWER;
    if(TransitionDesc.Direction.CompareNoCase(_T("LowerLeft")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_LOWER_LEFT;
    if(TransitionDesc.Direction.CompareNoCase(_T("Left")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_LEFT;
    if(TransitionDesc.Direction.CompareNoCase(_T("UpperLeft")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_UPPER_LEFT;
    if(TransitionDesc.Direction.CompareNoCase(_T("CW")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_CLOCKWISE;
    if(TransitionDesc.Direction.CompareNoCase(_T("CCW")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_COUNTERCLOCKWISE;
    if(TransitionDesc.Direction.CompareNoCase(_T("LeftRight")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_LEFT_RIGHT;
    if(TransitionDesc.Direction.CompareNoCase(_T("RightLeft")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_RIGHT_LEFT;
    if(TransitionDesc.Direction.CompareNoCase(_T("UpDown")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_UP_DOWN;
    if(TransitionDesc.Direction.CompareNoCase(_T("DownUp")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_DOWN_UP;
    if(TransitionDesc.Direction.CompareNoCase(_T("Random")) == 0) TransitionSetting.EffectDirection = S3S_EFFECT_DIRECTION_RANDOM;



    return S_OK;
}

CString S3ObjectContainer::GetObjectType()
{
    
    if(! m_pRenderableObj) return _T("");
    return m_pRenderableObj->GetObjectType();
}

//HRESULT S3ObjectContainer::CheckLoop()
//{
//    if(! m_pRenderableObj) return E_UNEXPECTED;
//
//    if(m_ThreadStatus == 2)
//    {
//        DbgMsg(_T("Error: Incorrect status to render Object %s"), m_Content.TypeName);
//        return E_UNEXPECTED;
//    }
//
//    if(!IsCreateThreadFinished())
//    {
//        DbgMsg(_T("Error: Incorrect status to render Object %s"), m_Content.TypeName);
//        return E_UNEXPECTED;
//    }
//
//    return m_pRenderableObj->CheckLoop();
//}

HRESULT S3ObjectContainer::LoopContent()
{
    if(! m_pRenderableObj) return E_UNEXPECTED;

    if(m_ThreadStatus == 2)
    {
        DbgMsg(_T("Error: Incorrect status to render Object %s"), m_Content.TypeName);
        return E_UNEXPECTED;
    }

    if(!IsCreateThreadFinished())
    {
        DbgMsg(_T("Error: Incorrect status to render Object %s"), m_Content.TypeName);
        return E_UNEXPECTED;
    }

    return m_pRenderableObj->LoopContent();
}


HRESULT S3ObjectContainer::InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice )
{
    HRESULT hr;
    m_pd3dDevice = pd3dDevice;

    if(! m_pRenderableObj) return E_UNEXPECTED;

    hr = m_pRenderableObj->InitDeviceObjects(pd3dDevice);
    if(FAILED(hr))
    {
        DbgMsg(_T("Error: Can not Initalize Object %s"), m_Content.TypeName);
        return hr;
    }

    if(m_Content.Translate.size() == 0 && m_Content.Scale.size() == 0)
    {
        RECT SFRRect;
        SFRRect.left = m_Content.XPos;
        SFRRect.right = m_Content.XPos +  m_Content.Width;
        SFRRect.top = m_Content.YPos;
        SFRRect.bottom = m_Content.YPos +  m_Content.Height;

        
        m_pRenderableObj->EnableSFRUpload(TRUE, m_InTransition.Effect == S3S_EFFECT_NONE &&
            m_OutTransition.Effect == S3S_EFFECT_NONE, &SFRRect, m_RotateDegree);
    }

    if(m_Content.TransitionIn.Name != _T("") || m_Content.TransitionOut.Name != _T(""))
    {
        SIZE TransitionSize;
        TransitionSize.cx = (int)m_ContentWidth;
        TransitionSize.cy = (int)m_ContentHeight;
        m_pTransition = new S3TransitionProvider(pd3dDevice, TransitionSize);
    }

    // create  background texture
    LPDIRECT3DTEXTURE9 pSysTexture = NULL;

    hr = pd3dDevice->CreateTexture(1, 1, 1, D3DUSAGE_DYNAMIC, 
                D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSysTexture, NULL);
    if(FAILED(hr)) return hr;


    hr = pd3dDevice->CreateTexture(1, 1, 1, D3DUSAGE_RENDERTARGET, 
            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pBGTexture, NULL);
    if(FAILED(hr)) return hr;

    D3DLOCKED_RECT LockInfo;
    hr = pSysTexture->LockRect(0, &LockInfo, NULL, 0);

    if(FAILED(hr)) return hr;

    *(DWORD *)LockInfo.pBits = m_Content.BGColor;

    pSysTexture->UnlockRect(0);

    hr = pd3dDevice->UpdateTexture(pSysTexture, m_pBGTexture);

    pSysTexture->Release();



    m_ThreadStatus = 1;
    DWORD  tid = NULL;
    m_hThread = CreateThread( NULL,
                            NULL,
                            ObjCreateThread,
                            this,
                            NULL,
                            &tid);
    if( INVALID_HANDLE_VALUE == m_hThread )
    {
        ::DbgMsg(_T("S3ObjectContainer::failed to create initalize thread"));
        m_ThreadStatus = 0;
        return E_UNEXPECTED;
    }

    return S_OK;
}

HRESULT S3ObjectContainer::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    if(! m_pRenderableObj) return E_UNEXPECTED;

    if(m_ThreadStatus == 2) return E_UNEXPECTED;

    WaitCreateThreadFinish();

    HRESULT hr;
    hr = m_pRenderableObj->RestoreDeviceObjects(pd3dDevice);

    if(FAILED(hr)) return hr;

    if(m_pTransition) hr = m_pTransition->RestoreDeviceObjects(pd3dDevice);

    if(FAILED(hr)) return hr;

    // create  background texture
    LPDIRECT3DTEXTURE9 pSysTexture = NULL;

    hr = pd3dDevice->CreateTexture(1, 1, 1, D3DUSAGE_DYNAMIC, 
                D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSysTexture, NULL);
    if(FAILED(hr)) return hr;


    hr = pd3dDevice->CreateTexture(1, 1, 1, D3DUSAGE_RENDERTARGET, 
            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pBGTexture, NULL);
    if(FAILED(hr)) return hr;

    D3DLOCKED_RECT LockInfo;
    hr = pSysTexture->LockRect(0, &LockInfo, NULL, 0);

    if(FAILED(hr)) return hr;

    *(DWORD *)LockInfo.pBits = m_Content.BGColor;

    pSysTexture->UnlockRect(0);

    hr = pd3dDevice->UpdateTexture(pSysTexture, m_pBGTexture);

    pSysTexture->Release();

    return hr;
}


HRESULT S3ObjectContainer::InvalidateDeviceObjects()
{
    if(! m_pRenderableObj) return E_UNEXPECTED;

    if(m_ThreadStatus == 2) return E_UNEXPECTED;

    WaitCreateThreadFinish();

    SAFE_RELEASE(m_pBGTexture);


    if(m_pTransition) m_pTransition->InvalidateDeviceObjects();

    return m_pRenderableObj->InvalidateDeviceObjects();
}


HRESULT S3ObjectContainer::DeleteDeviceObjects()
{
    if(! m_pRenderableObj) return E_UNEXPECTED;

    if(m_ThreadStatus == 2) return E_UNEXPECTED;

    WaitCreateThreadFinish();

    SAFE_RELEASE(m_pBGTexture);

    return m_pRenderableObj->DeleteDeviceObjects();
}

HRESULT S3ObjectContainer::Start()
{
    HRESULT hr;

    if(! m_pRenderableObj)
    {
        hr = E_UNEXPECTED;
        goto FAILED;
    }

    if(m_ThreadStatus == 2)
    {
        hr = E_UNEXPECTED;
        goto FAILED;
    }

    if(!IsCreateThreadFinished())
    {
        hr = E_UNEXPECTED;
        goto FAILED;
    }

    hr =  m_pRenderableObj->Start();

    if(FAILED(hr))
    {
        DbgMsg(_T("Error: Can not Start Object %s"), m_Content.TypeName);
        goto FAILED;
    }
    m_StartTime = timeGetTime();

    return hr;
FAILED:
    //DbgMsg(_T("Warning: Incorrect status to Start Object %s"), m_Content.TypeName);

    return hr;
}

HRESULT S3ObjectContainer::Stop()
{
    if(! m_pRenderableObj) return E_UNEXPECTED;

    if(m_ThreadStatus == 2)
    {
        DbgMsg(_T("Error: Incorrect status to Stop Object %s"), m_Content.TypeName);
        return E_UNEXPECTED;
    }
    if(!IsCreateThreadFinished())
    {
        DbgMsg(_T("Error: Incorrect status to Stop Object %s"), m_Content.TypeName);
        return E_UNEXPECTED;
    }

    HRESULT hr;
    hr = m_pRenderableObj->Stop();

    if(FAILED(hr))
    {
        DbgMsg(_T("Error: Can not Stop Object %s"), m_Content.TypeName);
        return hr;
    }

    return hr;
}


HRESULT S3ObjectContainer::PrepareRender()
{
    HRESULT hr;
    m_RenderRect.clear();

    if(! m_pRenderableObj) return E_UNEXPECTED;

    if(m_ThreadStatus == 2)
    {
        DbgMsg(_T("Error: Incorrect status to render Object %s"), m_Content.TypeName);
        return E_UNEXPECTED;
    }

    if(!IsCreateThreadFinished())
    {
        DbgMsg(_T("Error: Incorrect status to render Object %s"), m_Content.TypeName);
        return E_UNEXPECTED;
    }

    hr =  m_pRenderableObj->PrepareRender();

    if(hr != S_OK) return hr;


    m_RenderRect = m_pRenderableObj->m_RenderRect;

    if(m_RenderRect.size() == 1 && 
        (m_Content.TypeName.CompareNoCase(_T("Movie")) == 0 ||
        m_Content.TypeName.CompareNoCase(_T("S3ImageViewer")) == 0))
    {
        RECTF OrignalRect = RECTF(0,0, (float)m_ContentWidth, (float)m_ContentHeight);
        RECTF ClipRect = m_RenderRect.begin()->Position;
        // process background

        if(ClipRect.top != OrignalRect.top)
        {
            RECTF NewRect;
            NewRect.left = ClipRect.left;
            NewRect.right = ClipRect.right;
            NewRect.top = OrignalRect.top;
            NewRect.bottom = ClipRect.top;

            RenderRect BackgroundRender = RenderRect(NewRect, RECTF(0,0,1.0f,1.0f));
            BackgroundRender.bTransparent = ((m_Content.BGColor & 0xFF000000) != 0xFF000000);
            BackgroundRender.pTexture = m_pBGTexture;
            m_RenderRect.push_back(BackgroundRender);
        }

        if(ClipRect.bottom != OrignalRect.bottom)
        {
            RECTF NewRect;
            NewRect.left = ClipRect.left;
            NewRect.right = ClipRect.right;
            NewRect.top = ClipRect.bottom;
            NewRect.bottom = OrignalRect.bottom;

            RenderRect BackgroundRender = RenderRect(NewRect, RECTF(0,0,1.0f,1.0f));
            BackgroundRender.bTransparent = ((m_Content.BGColor & 0xFF000000) != 0xFF000000);
            BackgroundRender.pTexture = m_pBGTexture;
            m_RenderRect.push_back(BackgroundRender);
        }


        if(ClipRect.left != OrignalRect.left)
        {
            RECTF NewRect;
            NewRect.left = OrignalRect.left;
            NewRect.right = ClipRect.left;
            NewRect.top = OrignalRect.top;
            NewRect.bottom = OrignalRect.bottom;

            RenderRect BackgroundRender = RenderRect(NewRect, RECTF(0,0,1.0f,1.0f));
            BackgroundRender.bTransparent = ((m_Content.BGColor & 0xFF000000) != 0xFF000000);
            BackgroundRender.pTexture = m_pBGTexture;
            m_RenderRect.push_back(BackgroundRender);
        }

        if(ClipRect.right != OrignalRect.right)
        {
            RECTF NewRect;
            NewRect.left = ClipRect.right;
            NewRect.right = OrignalRect.right;
            NewRect.top = OrignalRect.top;
            NewRect.bottom = OrignalRect.bottom;

            RenderRect BackgroundRender = RenderRect(NewRect, RECTF(0,0,1.0f,1.0f));
            BackgroundRender.bTransparent = ((m_Content.BGColor & 0xFF000000) != 0xFF000000);
            BackgroundRender.pTexture = m_pBGTexture;
            m_RenderRect.push_back(BackgroundRender);
        }
    }

    // process in transition
    int CurrentTime = timeGetTime();

    if(m_InTransition.Effect != S3S_EFFECT_NONE && (CurrentTime - m_StartTime <  m_InTransition.EffectDuration * 1000))
    {
        if(!m_bInTransitionInit)
        {
            m_bInTransitionInit = TRUE;
            m_pTransition->SetTransition(m_InTransition);
        }

        float CurrentTransitionTime = ((float)CurrentTime - (float)m_StartTime)/
            ((float)m_InTransition.EffectDuration * 1000);

        CurrentTransitionTime = max(CurrentTransitionTime, 0);
        CurrentTransitionTime = min(CurrentTransitionTime, 1.0f);

        m_pTransition->ProcessTransition( CurrentTransitionTime, m_RenderRect);

        m_RenderRect.clear();

        // build transition result 
        RenderRect TransitionRect = (RenderRect(RECTF(0,0,(float)m_ContentWidth, (float)m_ContentHeight), RECTF(0.0f, 0.0f, 1.0f, 1.0f)));

        TransitionRect.bTransparent = TRUE;
        TransitionRect.pTexture = m_pTransition->GetTransitionImage();

        m_RenderRect.push_back(TransitionRect);
    // process out transition
    }else if (m_OutTransition.Effect != S3S_EFFECT_NONE && (CurrentTime - m_StartTime >(m_Content.Duration - m_OutTransition.EffectDuration) * 1000))
    {
        if(!m_bOutTransitionInit)
        {
            m_bOutTransitionInit = TRUE;
            m_pTransition->SetTransition(m_OutTransition);
        }

        float CurrentTransitionTime = ((float)CurrentTime - (float)m_StartTime - (m_Content.Duration - m_OutTransition.EffectDuration) * 1000)/
            ((float)m_OutTransition.EffectDuration * 1000);

        CurrentTransitionTime = max(CurrentTransitionTime, 0);
        CurrentTransitionTime = min(CurrentTransitionTime, 1.0f);

        m_pTransition->ProcessTransition( 1 - CurrentTransitionTime, m_RenderRect);

        m_RenderRect.clear();

        // build transition result 
        RenderRect TransitionRect = (RenderRect(RECTF(0,0,(float)m_ContentWidth, (float)m_ContentHeight), RECTF(0.0f, 0.0f, 1.0f, 1.0f)));

        TransitionRect.bTransparent = TRUE;
        TransitionRect.pTexture = m_pTransition->GetTransitionImage();

        m_RenderRect.push_back(TransitionRect);
    }

    // transform render info
    list<RenderRect>::iterator start, end, it;

    start = m_RenderRect.begin();
    end = m_RenderRect.end();

    it = start;
    do{

        if(m_ContentScale != 1.0f)
        {
            it->Position.left *= m_ContentScale;
            it->Position.right *= m_ContentScale;
            it->Position.top *= m_ContentScale;
            it->Position.bottom *= m_ContentScale;
        }

        // transform object's origon to it's geometry center
        float XOrigion = m_ScreenRect.Width() /2;
        float YOrigion = m_ScreenRect.Height() /2;

        it->Position.left -= XOrigion;
        it->Position.right -= XOrigion;
        it->Position.top -= YOrigion;
        it->Position.bottom -= YOrigion;


        float XValue, YValue;

        if(SUCCEEDED(InterpolateTransform(m_Content.Scale, XValue, YValue)))
        {
            it->Position.left *= XValue;
            it->Position.right *= XValue;
            it->Position.top *= YValue;
            it->Position.bottom *= YValue;
        }

        if(SUCCEEDED(InterpolateTransform(m_Content.Translate, XValue, YValue)))
        {
            it->Position.left += XValue;
            it->Position.right += XValue;
            it->Position.top += YValue;
            it->Position.bottom += YValue;
        }


        it->Position.left += XOrigion;
        it->Position.right += XOrigion;
        it->Position.top += YOrigion;
        it->Position.bottom += YOrigion;


        // put object to correct position;
        it->Position.left += m_ScreenRect.left;
        it->Position.right += m_ScreenRect.left;
        it->Position.top += m_ScreenRect.top;
        it->Position.bottom += m_ScreenRect.top;

        it++;
    }while(it != end);


    return S_OK;
}

HRESULT S3ObjectContainer::EndRender()
{
    if(! m_pRenderableObj) return E_UNEXPECTED;

    if(m_ThreadStatus == 2) return E_UNEXPECTED;

    if(!IsCreateThreadFinished()) return E_UNEXPECTED;

    return m_pRenderableObj->EndRender();
}


DWORD WINAPI S3ObjectContainer::ObjCreateThread( LPVOID lpParameter )
{
    CCrashRptThreadHelper helper;
    S3ObjectContainer* This = NULL;

    This = (S3ObjectContainer*)lpParameter;

    if(FAILED(CoInitialize(NULL)))
    {
        ::DbgMsg(_T("S3ObjectContainer::CreateThread: CoInitialize Failed"));
        This->m_ThreadStatus = 0;
        return 0;
    }
    
    This->m_pRenderableObj->Initalize();
    
    This->m_ThreadStatus = 0;

    CoUninitialize();

    return 0;
}

BOOL  S3ObjectContainer::IsCreateThreadFinished()
{
    return m_ThreadStatus != 1;
}

VOID  S3ObjectContainer::WaitCreateThreadFinish()
{
    while(!IsCreateThreadFinished())
    {
        Sleep(2);
    };
}

HRESULT S3ObjectContainer::ProcessMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(! m_pRenderableObj) return E_UNEXPECTED;

    if(m_ThreadStatus == 2) return E_UNEXPECTED;

    if(!IsCreateThreadFinished()) return E_UNEXPECTED;

    int MouseX = lParam & 0xFFFF;
    int MouseY = (lParam & 0xFFFF0000) >> 16;

    if(m_ScreenRect.bInside((float)MouseX, (float)MouseY))
    {
        MouseX -= (int)m_ScreenRect.left;
        MouseY -= (int)m_ScreenRect.top;
        return m_pRenderableObj->ProcessMouseMessages(uMsg, wParam, (MouseX | (MouseY << 16)));
    }

    return S_FALSE;
}

HRESULT S3ObjectContainer::ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    if(! m_pRenderableObj) return E_UNEXPECTED;

    if(m_ThreadStatus == 2) return E_UNEXPECTED;

    if(!IsCreateThreadFinished()) return E_UNEXPECTED;

    return m_pRenderableObj->ProcessMessage(uMsg, wParam, lParam);
}

VOID S3ObjectContainer::SetZOrder(UINT Zorder)
{
    m_ZOrder = Zorder;
}


HRESULT S3ObjectContainer::InterpolateTransform(S3SIGNAGE_TRANSFORM &Transform, float &XValue, float &YValue)
{
    if(Transform.size() == 0) return E_UNEXPECTED;

    DWORD CurrentTime = timeGetTime() - m_StartTime;

    DWORD i;
    for( i=0; i<Transform.size(); i++ )
    {
        if(Transform[i].Time * 1000 >= CurrentTime)
        {
            break;
        }
    }

    if(i == Transform.size() )
    {
        XValue = Transform[i - 1].XValue;
        YValue = Transform[i - 1].YValue;
        return S_OK;
    }

    if(i == 0 )
    {
        XValue = Transform[0].XValue;
        YValue = Transform[0].YValue;
        return S_OK;
    }

    float Rate = ((float)CurrentTime - (float)Transform[i - 1].Time * 1000)/((float)Transform[i].Time * 1000 - (float)Transform[i - 1].Time * 1000);

    XValue = Transform[i].XValue * Rate + (1 - Rate) * Transform[i - 1].XValue;
    YValue = Transform[i].YValue * Rate + (1 - Rate) * Transform[i - 1].YValue;

    return S_OK;
}



HRESULT S3ObjectContainer::Pause()
{
    if(! m_pRenderableObj) return E_UNEXPECTED;

    if(m_ThreadStatus == 2) return E_UNEXPECTED;

    if(!IsCreateThreadFinished()) return E_UNEXPECTED;

    if(!m_bPaused)
    {
        DWORD CurrentTime = timeGetTime();
        m_StartTime = CurrentTime - m_StartTime;
        m_bPaused = TRUE;
    }

    return m_pRenderableObj->Pause();
}

HRESULT S3ObjectContainer::Resume()
{
    if(! m_pRenderableObj) return E_UNEXPECTED;

    if(m_ThreadStatus == 2) return E_UNEXPECTED;

    if(!IsCreateThreadFinished()) return E_UNEXPECTED;

    if(m_bPaused)
    {
        DWORD CurrentTime = timeGetTime();
        m_StartTime = CurrentTime - m_StartTime;
        m_bPaused = FALSE;
    }

    return m_pRenderableObj->Resume();
}