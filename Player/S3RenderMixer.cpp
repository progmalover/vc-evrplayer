#include "stdafx.h"
#include "S3RenderEngine.h"
#include "S3RenderMixer.h"
#include "S3Signage.h"


bool RenderRectInfo::compare(const RenderRectInfo* t1,const RenderRectInfo* t2)
{
    if(t1->m_ScreenRect.left < t2->m_ScreenRect.left)
    {
        return TRUE;
    }else if(t1->m_ScreenRect.left > t2->m_ScreenRect.left)
    {
        return FALSE;
    }else if(t1->m_ScreenRect.top < t2->m_ScreenRect.top)
    {
        return TRUE;
    }


    return FALSE;
}


RenderRectInfo* RenderRectInfo::CloneWithClip(RECTF &ClipRect)
{
    RenderRectInfo *pNewRect = new RenderRectInfo;

    // clone object from original render rect info
    *pNewRect = *this;
    pNewRect->m_ScreenRect = ClipRect;

    // calc new texture coordinate for clipped object;
    RECTF OriginalPos = this->m_ScreenRect;

    RECTF NewPosAlpha;

    NewPosAlpha.left = (ClipRect.left - OriginalPos.left) /(OriginalPos.right - OriginalPos.left);
    NewPosAlpha.right = (ClipRect.right - OriginalPos.left) /(OriginalPos.right - OriginalPos.left);

    NewPosAlpha.top = (ClipRect.top - OriginalPos.top) /(OriginalPos.bottom - OriginalPos.top);
    NewPosAlpha.bottom = (ClipRect.bottom - OriginalPos.top) /(OriginalPos.bottom - OriginalPos.top);

    for(DWORD i=0; i<pNewRect->m_LayerInfo.size(); i++ )
    {
        CoordinateInfo NewCoordinate[4];
        CoordinateInfo TextureCoordinate[4];

        memcpy(TextureCoordinate, pNewRect->m_LayerInfo[i].TextureCoordinate, sizeof(CoordinateInfo) * 4);

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

        memcpy(pNewRect->m_LayerInfo[i].TextureCoordinate, TextureCoordinate, sizeof(CoordinateInfo) * 4);

    }

    return pNewRect;
}


/******************************Public*Routine******************************\
* CGameMixer
*
* constructor
\**************************************************************************/
S3RenderMixer::S3RenderMixer()
    : m_bInitialized( FALSE )
    , m_pOwner( NULL )
    , m_CaptureStatus(0)
{
#if S3S_TRACE_PERF
    memset(m_TimeHistory, 0, sizeof(UINT)*256);
    memset(m_HisType, 0, sizeof(UINT)*256);
    m_curItem = 0;
#endif
    
    m_CalcFPS = 0;
    m_FramesRendered = 0;
    m_CalcStartTime = -1;
    m_RotationDegree = 0.0;
    m_ClearCount = 0;
}

/******************************Public*Routine******************************\
* ~S3RenderMixer
*
* destructor
\**************************************************************************/
S3RenderMixer::~S3RenderMixer()
{
    Terminate();
}

int S3RenderMixer::GetFPS()
{
    return m_CalcFPS;
}

/******************************Public*Routine******************************\
* Render
\**************************************************************************/
HRESULT S3RenderMixer::Render(
                   IDirect3DDevice9* pDevice, 
                   void *lpParam )
{
    HRESULT hr = S_OK;
    
    S3DShowWizard* pWizard;
    CComPtr<IDirect3DTexture9> pTexture;

    if( !pDevice )
    {
        return E_POINTER;
    }

    if(m_CalcStartTime == -1)
    {
        m_CalcStartTime = timeGetTime();
    }

    if(m_CalcStartTime == -1 || m_FramesRendered >= 100)
    {
        int CurrentTime = timeGetTime();
        m_CalcFPS = m_FramesRendered * 1000 / (CurrentTime - m_CalcStartTime);

        m_FramesRendered = 0;
        m_CalcStartTime = CurrentTime;
    }

    m_FramesRendered ++;
    //Process changes
#if S3S_TRACE_PERF
    //LARGE_INTEGER tBegin,tEnd;
    //QueryPerformanceCounter(&tBegin);
#endif
    CAutoLock Lock(&m_MixerObjectLock);
#if S3S_TRACE_PERF
    //QueryPerformanceCounter(&tEnd);
    //UINT delta = tEnd.LowPart - tBegin.LowPart;
    //if(delta > 0x500)
    //{
    //    delta = 0;
    //}
#endif
#if S3S_DBGPRINT_SYNC
    DbgMsg("S3RenderMixer::Render Lock Render Objects");
#endif


    if(m_listObjects.empty() || m_ClearCount < 3)
    {
        pDevice->Clear( 0, NULL, D3DCLEAR_TARGET, m_ClearColor, 1.0f,0);
        m_ClearCount ++;
    }
    try
    {
        // error if not initialized
        CHECK_HR(
            m_bInitialized ? S_OK : VFW_E_WRONG_STATE,
            DbgMsg(_T("CGameMixer::Render: mixer was not initialized!")));

        CHECK_HR(
            m_pOwner ? S_OK : E_UNEXPECTED,
            DbgMsg(_T("CGameMixer::Render: mixer is initialized, but has no Render engine owner!")));

        CHECK_HR(
            hr = m_pOwner->GetWizardOwner( &(pWizard) ),
            DbgMsg(_T("CGameMixer::Render: mixer has owner, but has no parent wizard!")));

        // restore device objects for movies
        CHECK_HR(
            hr = RestoreDeviceObjects( pDevice ),
            DbgMsg(_T("CGameMixer::Render: failed in RestoreDeviceObjects, hr = 0x%08x"), hr));
        
        //Calculate layouts.

        //Draw layouts.

        //
        list<S3ObjectContainer*>::iterator start, end, it;


        // render all the movies
        start = m_listObjects.begin();
        end = m_listObjects.end();


        if( false == m_listObjects.empty() )
        {
            it = start;
            do{
#ifndef PLAYER_DUMMY
                // render all renderable objects
                (S3ObjectContainer *)(*it)->PrepareRender();
#endif
                it++;
            }while(it != end);

            m_RenderProcessingList.clear();

            it = start;
            do{
                AddRenderRectInfoByObject((S3ObjectContainer *)(*it));
                it++;
            }while(it != end);

            SplitRenderRectInfo();
#ifndef PLAYER_DUMMY
            RenderSplitedRect(pDevice);
#endif


            it = start;
            do{
#ifndef PLAYER_DUMMY
                (S3ObjectContainer *)(*it)->EndRender();
#endif
                it++;
            }while(it != end);
        }

        if(m_CaptureStatus == S3S_CAPTURE_STATE_REQUIRE_SAVE)
        {
            LPDIRECT3DSURFACE9 pRenderTarget = NULL;
            LPDIRECT3DSURFACE9 pStretchedRT = NULL;
            CImage             SavedImage;
            HRESULT            hr;
            D3DSURFACE_DESC    RTDesc;
            DWORD              ImageWidth;
            DWORD              ImageHeight;


            pDevice->GetRenderTarget(0, &pRenderTarget);
            pRenderTarget->GetDesc(&RTDesc);

            if((m_CaptureRect.right -  m_CaptureRect.left) > (m_CaptureRect.bottom -  m_CaptureRect.top))
            {
                ImageWidth = m_CaptureWidth;
                ImageHeight = m_CaptureWidth *  
                    (m_CaptureRect.bottom -  m_CaptureRect.top)/
                    (m_CaptureRect.right -  m_CaptureRect.left);  
            }else
            {
                ImageHeight = m_CaptureWidth;
                ImageWidth = m_CaptureWidth *  
                    (m_CaptureRect.right -  m_CaptureRect.left)/
                    (m_CaptureRect.bottom -  m_CaptureRect.top);
            }



            hr = pDevice->CreateRenderTarget(ImageWidth, ImageHeight, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE,0, TRUE, &pStretchedRT, NULL);

            hr = pDevice->StretchRect(pRenderTarget, &m_CaptureRect, pStretchedRT, NULL, D3DTEXF_LINEAR);

            SavedImage.Create(ImageWidth, ImageHeight, 32, 0);

            HDC SavedImageDC = SavedImage.GetDC();
            HDC StretchedRTDC = NULL;

            hr = pStretchedRT->GetDC(&StretchedRTDC);

            BitBlt(SavedImageDC, 0,0, ImageWidth, ImageHeight, StretchedRTDC, 0, 0, SRCCOPY);

            hr = pStretchedRT->ReleaseDC(StretchedRTDC);
            pStretchedRT->Release();

            SavedImage.ReleaseDC();

            SavedImage.Save(m_CaptureFile);

            m_CaptureStatus = S3S_CAPTURE_STATE_SAVE_FINISH;

            pRenderTarget->Release();
        }


    }
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }

#if S3S_DBGPRINT_SYNC
    DbgMsg("S3RenderMixer::Render Unlock  Render Objects");
#endif
    return hr;
}

/******************************Public*Routine******************************\
* SetRenderEngineOwner
\**************************************************************************/
HRESULT S3RenderMixer::SetRenderEngineOwner(
        S3RenderEngine* pRenderEngine)
{
    HRESULT hr = S_OK;
    CAutoLock Lock(&m_MixerObjectLock);
#if S3S_DBGPRINT_SYNC
    DbgMsg("S3RenderMixer::SetRenderEngineOwner Lock");
#endif

    m_pOwner = pRenderEngine;

#if S3S_DBGPRINT_SYNC
    DbgMsg("S3RenderMixer::SetRenderEngineOwner Unlock");
#endif
    return hr;
}

/******************************Public*Routine******************************\
* GetRenderEngineOwner
\**************************************************************************/
HRESULT S3RenderMixer::GetRenderEngineOwner(
        S3RenderEngine** ppRenderEngine)
{
    if( !ppRenderEngine )
    {
        return E_POINTER;
    }
    if( !m_pOwner )
    {
        return VFW_E_NOT_FOUND;
    }

    *ppRenderEngine = m_pOwner;
    return S_OK;
}

/******************************Public*Routine*****************************\
* Initialize
\**************************************************************************/
HRESULT S3RenderMixer::Initialize(
                       IDirect3DDevice9 *pDevice)
{
    HRESULT hr = S_OK;
    TCHAR achMessage[MAX_PATH];

    CAutoLock Lock(&m_MixerObjectLock);
#if S3S_DBGPRINT_SYNC
    DbgMsg("S3RenderMixer::Initialize Lock  Render Objects");
#endif

    achMessage[0] = TEXT('\0');

    if( m_bInitialized )
    {
        return VFW_E_WRONG_STATE;
    }
#ifndef PLAYER_DUMMY
    if( !pDevice )
    {
        return E_POINTER;
    }
#endif
    try
    {
        list<S3ObjectContainer*>::iterator start, end, it;
        S3ObjectContainer *pObj = NULL;

        start = m_listObjects.begin();
        end = m_listObjects.end();

        for( it=start; it!=end; it++)
        {
            pObj = (S3ObjectContainer*)(*it);
            if( pObj )
            {
                pObj->InitDeviceObjects(pDevice);
            }
        }

        m_bInitialized = TRUE;
    }
    catch( HRESULT hr1 )
    {
        hr = hr1;
    }
#if S3S_DBGPRINT_SYNC
    DbgMsg("S3RenderMixer::Initialize Unlock  Render Objects");
#endif

    return hr;
}

HRESULT S3RenderMixer::BeginDeviceLoss(void)
{
    list<S3ObjectContainer*>::iterator start, end, it;
    S3ObjectContainer *pObj = NULL;

    start = m_listObjects.begin();
    end = m_listObjects.end();

    HRESULT hr;

    for( it=start; it!=end; it++)
    {
        hr = (((S3ObjectContainer*)(*it))->InvalidateDeviceObjects());
        if(FAILED(hr)) return hr;
    }
    return hr;
}

HRESULT S3RenderMixer::EndDeviceLoss(IDirect3DDevice9 *pDevice)
{
   list<S3ObjectContainer*>::iterator start, end, it;
    S3ObjectContainer *pObj = NULL;

    start = m_listObjects.begin();
    end = m_listObjects.end();

    HRESULT hr;

    for( it=start; it!=end; it++)
    {
        hr = (((S3ObjectContainer*)(*it))->RestoreDeviceObjects(pDevice));
        if(FAILED(hr)) return hr;
    }
    return hr;
}
/******************************Public*Routine******************************\
* RestoreDeviceObjects
\**************************************************************************/
HRESULT S3RenderMixer::RestoreDeviceObjects( IDirect3DDevice9 *pDevice )
{
    HRESULT hr = S_OK;

    if( !pDevice )
    {
        return E_POINTER;
    }

    try
    {
        CHECK_HR(
            hr = pDevice->SetRenderState( D3DRS_ALPHATESTENABLE,      FALSE ),
            DbgMsg(_T("CGameMixer::RestoreDeviceObjects: failed to set render state D3DRS_AMBIENT/0xFFFFFFFF, hr = 0x%08x"),hr));

        CHECK_HR(
            hr = pDevice->SetRenderState( D3DRS_AMBIENT,      0x00FFFFFF ),
            DbgMsg(_T("CGameMixer::RestoreDeviceObjects: failed to set render state D3DRS_AMBIENT/0xFFFFFFFF, hr = 0x%08x"),hr));
        CHECK_HR(
            hr = pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE),
            DbgMsg(_T("CGameMixer::RestoreDeviceObjects: failed to set render state D3DRS_CULLMODE/D3DCULL_NONE, hr = 0x%08x"),hr));
        CHECK_HR(
            hr = pDevice->SetRenderState(D3DRS_ZENABLE, FALSE),
            DbgMsg(_T("CGameMixer::RestoreDeviceObjects: failed to set render state D3DRS_ZENABLE/FALSE, hr = 0x%08x"),hr));

        CHECK_HR(
            hr = pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU ,D3DTADDRESS_CLAMP),
            DbgMsg(_T("CGameMixer::RestoreDeviceObjects: failed to set sampler state D3DSAMP_ADDRESSU/D3DTADDRESS_CLAMP, hr = 0x%08x"),hr));
        CHECK_HR(
            hr = pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV ,D3DTADDRESS_CLAMP),
            DbgMsg(_T("CGameMixer::RestoreDeviceObjects: failed to set sampler state D3DSAMP_ADDRESSV/D3DTADDRESS_CLAMP, hr = 0x%08x"),hr));

        m_ClearCount = 0;
    }
    catch(HRESULT hr1)
    {
        hr = hr1;
    }
    return hr;
}



///////////////////////////// PRIVATE DOMAIN ///////////////////////////////////

/******************************Private*Routine*****************************\
* Clean_
\**************************************************************************/
void S3RenderMixer::Clean_()
{
    //At this point, scheduler thread should have been stopped.
    //The rendering thread should be still running.
    //So only the m_listObjects need to be locked

    // clean the list
    list<S3ObjectContainer*>::iterator start, end, it;
    m_MixerObjectLock.Lock();
#if S3S_DBGPRINT_SYNC
    DbgMsg("S3RenderMixer::~S3RenderMixer Lock Render Objects");
#endif 
    m_listObjects.clear();
    m_MixerObjectLock.Unlock();
#if S3S_DBGPRINT_SYNC
    DbgMsg("S3RenderMixer::~S3RenderMixer Unlock Render Objects");
#endif
}

void S3RenderMixer::Terminate()
{
    Clean_();
}

void S3RenderMixer::AddRenderableObject(S3ObjectContainer *pObj)
{
    CAutoLock Lock(&m_MixerObjectLock);
#if S3S_DBGPRINT_SYNC
    DbgMsg("S3RenderMixer::AddRenderableObject Lock  Render Objects");
#endif

    m_listObjects.push_back( pObj );

    m_listObjects.sort( S3ObjectContainer::compare);

#if S3S_DBGPRINT_SYNC
    DbgMsg("S3RenderMixer::AddRenderableObject Unlock  Render Objects");
#endif
}


void S3RenderMixer::RemoveRenderableObject(S3ObjectContainer *pObj)
{
    m_MixerObjectLock.Lock();
#if S3S_DBGPRINT_SYNC
    DbgMsg("S3RenderMixer::RemoveRenderableObject Lock  Render Objects");
#endif

    m_listObjects.remove( pObj );

    m_MixerObjectLock.Unlock();

#if S3S_DBGPRINT_SYNC
    DbgMsg("S3RenderMixer::RemoveRenderableObject Unlock  Render Objects");
#endif
}


void S3RenderMixer::ReplaceRenderableObject(S3ObjectContainer *pOldObj, S3ObjectContainer *pNewObj)
{
    m_MixerObjectLock.Lock();


    m_listObjects.remove( pOldObj );
    m_listObjects.push_back( pNewObj );

    m_MixerObjectLock.Unlock();
}

HRESULT S3RenderMixer::ProcessMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(uMsg < WM_MOUSEFIRST || uMsg > WM_MOUSELAST)
    {
        return E_INVALIDARG;
    }
    list<S3ObjectContainer*>::iterator start, end, it;
    S3ObjectContainer *pObj = NULL;

    start = m_listObjects.begin();
    end = m_listObjects.end();

    // process mouse message as Z order
    for( it=start; it!=end; it++)
    {
        if((S3ObjectContainer*)(*it)->ProcessMouseMessages(uMsg, wParam, lParam) == S_OK)
        {
            return S_OK;
        }
    }

    return S_FALSE;
}

HRESULT S3RenderMixer::ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    list<S3ObjectContainer*>::iterator start, end, it;
    S3ObjectContainer *pObj = NULL;

    start = m_listObjects.begin();
    end = m_listObjects.end();

    // process mouse message as Z order
    for( it=start; it!=end; it++)
    {
        if((S3ObjectContainer*)(*it)->ProcessMessage(uMsg, wParam, lParam) == S_OK)
        {
            return S_OK;
        }
    }

    return E_FAIL;
}

void S3RenderMixer::SplitRenderRectInfo()
{
    m_RenderList.clear();


    while(m_RenderProcessingList.begin() != m_RenderProcessingList.end())
    {
        list<RenderRectInfo*>::iterator it;

        it = m_RenderProcessingList.begin();
        // always pick up the first obj as the active obj
        RenderRectInfo *pActiveObj = (RenderRectInfo * )(*it);
        it++;

        BOOL bRectProcessed = FALSE;


        while(it != m_RenderProcessingList.end())
        {
            RenderRectInfo *pTestObj = (RenderRectInfo*)(*it);

            if(pActiveObj->m_ScreenRect.bIntersect(pTestObj->m_ScreenRect))
            {
                
                // split these two object

                // we only detect the first zorder in layer info as 
                // layerinfo should be sorted by z order
                RenderRectInfo *pFrontObj;
                RenderRectInfo *pBackObj;

                if(pActiveObj->m_LayerInfo[0].ZOrder > pTestObj->m_LayerInfo[0].ZOrder)
                {
                    pFrontObj = pActiveObj;        pBackObj  = pTestObj;
                }else
                {
                    pFrontObj = pTestObj;          pBackObj  = pActiveObj;
                }

                // calc overlapped rect;
                RECTF RectOverlapped = pFrontObj->m_ScreenRect;

                RectOverlapped.Clip(pBackObj->m_ScreenRect);

                // back subtract front
                m_RenderProcessingList.remove(pBackObj);

                RectSubstract(pBackObj, RectOverlapped, pFrontObj->m_bFavorHorizontalSplit);


                // if front obj is transparent, we also need split front
                if(pFrontObj->m_LayerInfo[0].bTransparent)
                {
                    m_RenderProcessingList.remove(pFrontObj);

                    RectSubstract(pFrontObj, RectOverlapped, pFrontObj->m_bFavorHorizontalSplit);

                    RenderRectInfo *pOverlappedFront = pFrontObj->CloneWithClip(RectOverlapped);
                    RenderRectInfo *pOverlappedBack = pBackObj->CloneWithClip(RectOverlapped);

                    // merge overlapped front and back
                    for(DWORD i=0; i<pOverlappedBack->m_LayerInfo.size(); i++)
                    {
                        vector<LayerInfo>::iterator itLayerInfo;
                        vector<LayerInfo>::iterator InsertPosition = pOverlappedFront->m_LayerInfo.end();

                        itLayerInfo = pOverlappedFront->m_LayerInfo.begin();

                        // find the position to insert
                        while(itLayerInfo != pOverlappedFront->m_LayerInfo.end())
                        {
                            if(pOverlappedBack->m_LayerInfo[i].ZOrder < itLayerInfo->ZOrder)
                            {
                                InsertPosition  = itLayerInfo;
                            }else
                            {
                                break;
                            }

                            if(!itLayerInfo->bTransparent)
                            {
                                InsertPosition = pOverlappedFront->m_LayerInfo.end();
                                break;
                            }

                            itLayerInfo ++;
                        }

                        if(InsertPosition != pOverlappedFront->m_LayerInfo.end())
                        {
                            // insert back layer info into front, we will delete back later
                            pOverlappedFront->m_LayerInfo.insert(++InsertPosition, pOverlappedBack->m_LayerInfo[i]);

                            if(!pOverlappedBack->m_LayerInfo[i].bTransparent)
                            {
                                //if the current insert layer in not transparent, we should remove all layers behind it
                                vector<LayerInfo>::iterator StartRemoveLayer = pOverlappedFront->m_LayerInfo.begin();

                                // find the position to insert
                                while(StartRemoveLayer != pOverlappedFront->m_LayerInfo.end())
                                {
                                    if(pOverlappedBack->m_LayerInfo[i].ZOrder > StartRemoveLayer->ZOrder)
                                    {
                                        break;
                                    }
                                    StartRemoveLayer ++;
                                }
                                pOverlappedFront->m_LayerInfo.erase( StartRemoveLayer, pOverlappedFront->m_LayerInfo.end());
                                break;
                            }
                        }else
                        {
                            break;
                        }
                    }

                    AddRenderRectInfo(pOverlappedFront);

                    delete pOverlappedBack;
                    delete pFrontObj;
                }
                delete pBackObj;

                // restart new split test
                bRectProcessed = TRUE; 
                break;
            }else if(pActiveObj->m_ScreenRect.right <= pTestObj->m_ScreenRect.left)
            {
                // as the list is sorted, the left object will not intersect with active object any more
                // we as remove this object from processing list and add it to render list
                m_RenderList.push_back(pActiveObj);
                m_RenderProcessingList.remove(pActiveObj);
                // restart new split test
                bRectProcessed = TRUE; 
                break;
            }
            it++;
        };

        if(!bRectProcessed)
        {
            m_RenderList.push_back(pActiveObj);
            m_RenderProcessingList.remove(pActiveObj);
        }
    }
}


void S3RenderMixer::AddRenderRectInfoByObject(S3ObjectContainer *pObj)
{
    list<RenderRect>::iterator start, end, it;

    start = pObj->m_RenderRect.begin();
    end = pObj->m_RenderRect.end();

    it = start;
    
    while(it != end)
    {
        RenderRect *pRenderRect = &(RenderRect)(*it);

        pRenderRect->Clip(m_ScreenClip);

        RenderRectInfo *pNewRect = new RenderRectInfo;
        pNewRect->m_ScreenRect = pRenderRect->Position;
        pNewRect->m_bFavorHorizontalSplit = FALSE;

        LayerInfo NewLayer;
        NewLayer.ZOrder = pObj->m_ZOrder;
        NewLayer.bTransparent = pRenderRect->bTransparent;
        NewLayer.bUseBlendFactor = FALSE;
        NewLayer.BlendFactor = 0;
        NewLayer.pTexture = pRenderRect->pTexture;
        memcpy(&NewLayer.TextureCoordinate, &pRenderRect->TextureCoordinate, sizeof(CoordinateInfo) * 4);

        pNewRect->m_LayerInfo.push_back(NewLayer);

        AddRenderRectInfo(pNewRect);

        it++;
    };


}


void S3RenderMixer::AddRenderRectInfo(RenderRectInfo * pNewRect)
{
    list<RenderRectInfo*>::iterator start, end, it;
    RenderRectInfo *pObj = NULL;

    start = m_RenderProcessingList.begin();
    end = m_RenderProcessingList.end();

    it = start;
    
    while(it != end)
    {
        if(RenderRectInfo::compare(pNewRect, (RenderRectInfo*)(*it)))
            break;

        it++;
    };

    m_RenderProcessingList.insert(it, pNewRect );
}


void S3RenderMixer::RectSubstract(RenderRectInfo * pRenderRect, RECTF &ClipRect, BOOL bHorizonntalSplit)
{
    RECTF OrignalRect = pRenderRect->m_ScreenRect;

    if(bHorizonntalSplit)
    {
        if(ClipRect.top != OrignalRect.top)
        {
            RECTF NewRect;
            NewRect.left = OrignalRect.left;
            NewRect.right = OrignalRect.right;
            NewRect.top = OrignalRect.top;
            NewRect.bottom = ClipRect.top;

            AddRenderRectInfo(pRenderRect->CloneWithClip(NewRect));
        }

        if(ClipRect.bottom != OrignalRect.bottom)
        {
            RECTF NewRect;
            NewRect.left = OrignalRect.left;
            NewRect.right = OrignalRect.right;
            NewRect.top = ClipRect.bottom;
            NewRect.bottom = OrignalRect.bottom;

            AddRenderRectInfo(pRenderRect->CloneWithClip(NewRect));
        }


        if(ClipRect.left != OrignalRect.left)
        {
            RECTF NewRect;
            NewRect.left = OrignalRect.left;
            NewRect.right = ClipRect.left;
            NewRect.top = ClipRect.top;
            NewRect.bottom = ClipRect.bottom;

            AddRenderRectInfo(pRenderRect->CloneWithClip(NewRect));
        }

        if(ClipRect.right != OrignalRect.right)
        {
            RECTF NewRect;
            NewRect.left = ClipRect.right;
            NewRect.right = OrignalRect.right;
            NewRect.top = ClipRect.top;
            NewRect.bottom = ClipRect.bottom;

            AddRenderRectInfo(pRenderRect->CloneWithClip(NewRect));
        }


    }else
    {
        if(ClipRect.top != OrignalRect.top)
        {
            RECTF NewRect;
            NewRect.left = ClipRect.left;
            NewRect.right = ClipRect.right;
            NewRect.top = OrignalRect.top;
            NewRect.bottom = ClipRect.top;

            AddRenderRectInfo(pRenderRect->CloneWithClip(NewRect));
        }

        if(ClipRect.bottom != OrignalRect.bottom)
        {
            RECTF NewRect;
            NewRect.left = ClipRect.left;
            NewRect.right = ClipRect.right;
            NewRect.top = ClipRect.bottom;
            NewRect.bottom = OrignalRect.bottom;

            AddRenderRectInfo(pRenderRect->CloneWithClip(NewRect));
        }


        if(ClipRect.left != OrignalRect.left)
        {
            RECTF NewRect;
            NewRect.left = OrignalRect.left;
            NewRect.right = ClipRect.left;
            NewRect.top = OrignalRect.top;
            NewRect.bottom = OrignalRect.bottom;

            m_RenderList.push_back(pRenderRect->CloneWithClip(NewRect));
        }

        if(ClipRect.right != OrignalRect.right)
        {
            RECTF NewRect;
            NewRect.left = ClipRect.right;
            NewRect.right = OrignalRect.right;
            NewRect.top = OrignalRect.top;
            NewRect.bottom = OrignalRect.bottom;

            AddRenderRectInfo(pRenderRect->CloneWithClip(NewRect));
        }
    }


}


void S3RenderMixer::RenderSplitedRect(IDirect3DDevice9 *pDevice)
{
    list<RenderRectInfo*>::iterator start, end, it;
    RenderRectInfo *pObj = NULL;
    DWORD i;

    start = m_RenderList.begin();
    end = m_RenderList.end();

    if(start == end) return;

    it = start;
    do{
#if 0
        pObj = (RenderRectInfo*)(*it);
        LayerInfo RenderLayerInfo = pObj->m_LayerInfo[0];

        HRESULT hr;

        hr = pDevice->SetTexture(0, RenderLayerInfo.pTexture);
        hr = pDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX1 );

        hr = pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        hr = pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CONSTANT);
        hr = pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        hr = pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

        hr = pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        hr = pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

        hr = pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU ,D3DTADDRESS_WRAP);
        hr = pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV ,D3DTADDRESS_WRAP);

        if(RenderLayerInfo.bTransparent)
        {
            hr = pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            if(RenderLayerInfo.bUseBlendFactor)
            {
                hr = pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);
                hr = pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVBLENDFACTOR);

                // The following is the blending factor, use values from 0 to 255
                // A value of 0 will make image transparent and a value of 255
                // will make it opaque.
                hr = pDevice->SetRenderState(D3DRS_BLENDFACTOR, RenderLayerInfo.BlendFactor);

            }
            else
            {
                hr = pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
                hr = pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            }

        }else
        {
            hr = pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        }

        Vertex1T RenderVertex[7];

        FLOAT z = 0.9f;
        FLOAT rhw = 1.0f;


        RenderVertex[0].Pos = S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw);
        RenderVertex[1].Pos = S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw);
        RenderVertex[2].Pos = S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw);
        RenderVertex[3].Pos = S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw);


        RenderVertex[4].Pos = S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw);
        RenderVertex[5].Pos = S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw);
        RenderVertex[6].Pos = S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw);


        RenderVertex[0].tu = RenderLayerInfo.TextureCoordinate[0].tu;   RenderVertex[0].tv = RenderLayerInfo.TextureCoordinate[0].tv;
        RenderVertex[1].tu = RenderLayerInfo.TextureCoordinate[1].tu;   RenderVertex[1].tv = RenderLayerInfo.TextureCoordinate[1].tv;
        RenderVertex[2].tu = RenderLayerInfo.TextureCoordinate[2].tu;   RenderVertex[2].tv = RenderLayerInfo.TextureCoordinate[2].tv;
        RenderVertex[3].tu = RenderLayerInfo.TextureCoordinate[3].tu;   RenderVertex[3].tv = RenderLayerInfo.TextureCoordinate[3].tv;

        hr = pDevice->DrawPrimitiveUP(  D3DPT_LINESTRIP,
            6,
            (LPVOID)(RenderVertex),
            sizeof(Vertex1T));  
        


        pDevice->SetTextureStageState(0, D3DTSS_CONSTANT, 0xFFFFFFFF);
        hr = pDevice->SetTexture(0, NULL);

#else

        pObj = (RenderRectInfo*)(*it);

        HRESULT hr;
        hr = pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

        DWORD TextureStage = 0;
        for(i=0; i< pObj->m_LayerInfo.size(); i++)
        {
            if(pObj->m_LayerInfo[i].bTransparent && pObj->m_LayerInfo[i].bUseBlendFactor)
            {
                TextureStage++;
            }
            TextureStage++;
        }
        DWORD MaxTextureStage = TextureStage;

        vector<LayerInfo>::iterator itLayer;

        for(vector<LayerInfo>::iterator itLayer = pObj->m_LayerInfo.begin(); itLayer != pObj->m_LayerInfo.end(); itLayer++)
        {
            if((*itLayer).bTransparent && (*itLayer).bUseBlendFactor)
            {
                TextureStage--;

                hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_COLORARG2, D3DTA_CURRENT);
                hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

                hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);


                hr = pDevice->SetSamplerState(TextureStage, D3DSAMP_ADDRESSU ,D3DTADDRESS_CLAMP);
                hr = pDevice->SetSamplerState(TextureStage, D3DSAMP_ADDRESSV ,D3DTADDRESS_CLAMP);

                hr = pDevice->SetTexture(TextureStage, (*itLayer).pTexture);

                LayerInfo NewInfo = (*itLayer);
                itLayer = pObj->m_LayerInfo.insert(itLayer, NewInfo);
                itLayer ++;
            }

            TextureStage--;
           
            hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_COLORARG2, D3DTA_CURRENT);
            hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

            if((*itLayer).bTransparent)
            {
                if((*itLayer).bUseBlendFactor)
                {
                    hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
                    
                    // The following is the blending factor, use values from 0 to 255 for every channel
                    // A value of 0 will make image transparent and a value of 255
                    // will make it opaque.
                    hr = pDevice->SetRenderState(D3DRS_TEXTUREFACTOR, (*itLayer).BlendFactor);
                    hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                    hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                    hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
                }
                else
                {
                    hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
                }

            }else
            {
                hr = pDevice->SetTextureStageState(TextureStage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            }


            hr = pDevice->SetSamplerState(TextureStage, D3DSAMP_ADDRESSU ,D3DTADDRESS_CLAMP);
            hr = pDevice->SetSamplerState(TextureStage, D3DSAMP_ADDRESSV ,D3DTADDRESS_CLAMP);
            
            hr = pDevice->SetSamplerState(TextureStage, D3DSAMP_MAGFILTER , D3DTEXF_LINEAR);
            hr = pDevice->SetSamplerState(TextureStage, D3DSAMP_MINFILTER , D3DTEXF_LINEAR);

            hr = pDevice->SetTexture(TextureStage, (*itLayer).pTexture);

            
        }
        
        // disable unused stage
        hr = pDevice->SetTextureStageState(MaxTextureStage, D3DTSS_COLOROP, D3DTOP_DISABLE);
        hr = pDevice->SetTextureStageState(MaxTextureStage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);


        if(MaxTextureStage == 1)
        {
            hr = pDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX1 );


            Vertex1T RenderVertex[4];

            FLOAT z = 0.9f;
            FLOAT rhw = 1.0f;


            RenderVertex[0].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[1].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[2].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[3].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);

            LayerInfo RenderLayerInfo = pObj->m_LayerInfo[0];

            RenderVertex[0].tu = RenderLayerInfo.TextureCoordinate[0].tu;   RenderVertex[0].tv = RenderLayerInfo.TextureCoordinate[0].tv;
            RenderVertex[1].tu = RenderLayerInfo.TextureCoordinate[1].tu;   RenderVertex[1].tv = RenderLayerInfo.TextureCoordinate[1].tv;
            RenderVertex[2].tu = RenderLayerInfo.TextureCoordinate[2].tu;   RenderVertex[2].tv = RenderLayerInfo.TextureCoordinate[2].tv;
            RenderVertex[3].tu = RenderLayerInfo.TextureCoordinate[3].tu;   RenderVertex[3].tv = RenderLayerInfo.TextureCoordinate[3].tv;



            hr = pDevice->DrawPrimitiveUP(  D3DPT_TRIANGLESTRIP,
                2,
                (LPVOID)(RenderVertex),
                sizeof(Vertex1T)); 
        }

       
        if(MaxTextureStage == 2)
        {
            hr = pDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX2 );


            Vertex2T RenderVertex[4];

            FLOAT z = 0.9f;
            FLOAT rhw = 1.0f;


            RenderVertex[0].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[1].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[2].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[3].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);

            LayerInfo RenderLayerInfo0 = pObj->m_LayerInfo[1];
            LayerInfo RenderLayerInfo1 = pObj->m_LayerInfo[0];

            RenderVertex[0].tu0 = RenderLayerInfo0.TextureCoordinate[0].tu;   RenderVertex[0].tv0 = RenderLayerInfo0.TextureCoordinate[0].tv;
            RenderVertex[1].tu0 = RenderLayerInfo0.TextureCoordinate[1].tu;   RenderVertex[1].tv0 = RenderLayerInfo0.TextureCoordinate[1].tv;
            RenderVertex[2].tu0 = RenderLayerInfo0.TextureCoordinate[2].tu;   RenderVertex[2].tv0 = RenderLayerInfo0.TextureCoordinate[2].tv;
            RenderVertex[3].tu0 = RenderLayerInfo0.TextureCoordinate[3].tu;   RenderVertex[3].tv0 = RenderLayerInfo0.TextureCoordinate[3].tv;

            RenderVertex[0].tu1 = RenderLayerInfo1.TextureCoordinate[0].tu;   RenderVertex[0].tv1 = RenderLayerInfo1.TextureCoordinate[0].tv;
            RenderVertex[1].tu1 = RenderLayerInfo1.TextureCoordinate[1].tu;   RenderVertex[1].tv1 = RenderLayerInfo1.TextureCoordinate[1].tv;
            RenderVertex[2].tu1 = RenderLayerInfo1.TextureCoordinate[2].tu;   RenderVertex[2].tv1 = RenderLayerInfo1.TextureCoordinate[2].tv;
            RenderVertex[3].tu1 = RenderLayerInfo1.TextureCoordinate[3].tu;   RenderVertex[3].tv1 = RenderLayerInfo1.TextureCoordinate[3].tv;


            hr = pDevice->DrawPrimitiveUP(  D3DPT_TRIANGLESTRIP,
                2,
                (LPVOID)(RenderVertex),
                sizeof(Vertex2T)); 
        }



        if(MaxTextureStage == 3)
        {
            hr = pDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX3 );


            Vertex3T RenderVertex[4];

            FLOAT z = 0.9f;
            FLOAT rhw = 1.0f;


            RenderVertex[0].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[1].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[2].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[3].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);

            LayerInfo RenderLayerInfo0 = pObj->m_LayerInfo[2];
            LayerInfo RenderLayerInfo1 = pObj->m_LayerInfo[1];
            LayerInfo RenderLayerInfo2 = pObj->m_LayerInfo[0];

            RenderVertex[0].tu0 = RenderLayerInfo0.TextureCoordinate[0].tu;   RenderVertex[0].tv0 = RenderLayerInfo0.TextureCoordinate[0].tv;
            RenderVertex[1].tu0 = RenderLayerInfo0.TextureCoordinate[1].tu;   RenderVertex[1].tv0 = RenderLayerInfo0.TextureCoordinate[1].tv;
            RenderVertex[2].tu0 = RenderLayerInfo0.TextureCoordinate[2].tu;   RenderVertex[2].tv0 = RenderLayerInfo0.TextureCoordinate[2].tv;
            RenderVertex[3].tu0 = RenderLayerInfo0.TextureCoordinate[3].tu;   RenderVertex[3].tv0 = RenderLayerInfo0.TextureCoordinate[3].tv;

            RenderVertex[0].tu1 = RenderLayerInfo1.TextureCoordinate[0].tu;   RenderVertex[0].tv1 = RenderLayerInfo1.TextureCoordinate[0].tv;
            RenderVertex[1].tu1 = RenderLayerInfo1.TextureCoordinate[1].tu;   RenderVertex[1].tv1 = RenderLayerInfo1.TextureCoordinate[1].tv;
            RenderVertex[2].tu1 = RenderLayerInfo1.TextureCoordinate[2].tu;   RenderVertex[2].tv1 = RenderLayerInfo1.TextureCoordinate[2].tv;
            RenderVertex[3].tu1 = RenderLayerInfo1.TextureCoordinate[3].tu;   RenderVertex[3].tv1 = RenderLayerInfo1.TextureCoordinate[3].tv;


            RenderVertex[0].tu2 = RenderLayerInfo2.TextureCoordinate[0].tu;   RenderVertex[0].tv2 = RenderLayerInfo2.TextureCoordinate[0].tv;
            RenderVertex[1].tu2 = RenderLayerInfo2.TextureCoordinate[1].tu;   RenderVertex[1].tv2 = RenderLayerInfo2.TextureCoordinate[1].tv;
            RenderVertex[2].tu2 = RenderLayerInfo2.TextureCoordinate[2].tu;   RenderVertex[2].tv2 = RenderLayerInfo2.TextureCoordinate[2].tv;
            RenderVertex[3].tu2 = RenderLayerInfo2.TextureCoordinate[3].tu;   RenderVertex[3].tv2 = RenderLayerInfo2.TextureCoordinate[3].tv;


            hr = pDevice->DrawPrimitiveUP(  D3DPT_TRIANGLESTRIP,
                2,
                (LPVOID)(RenderVertex),
                sizeof(Vertex3T)); 
        }

        if(MaxTextureStage == 4)
        {
            hr = pDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX4 );


            Vertex4T RenderVertex[4];

            FLOAT z = 0.9f;
            FLOAT rhw = 1.0f;


            RenderVertex[0].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[1].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[2].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[3].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);

            for(i = 0; i < MaxTextureStage; i++)
            {
                LayerInfo RenderLayerInfo = pObj->m_LayerInfo[MaxTextureStage - 1 - i];
                RenderVertex[0].T[i].tu = RenderLayerInfo.TextureCoordinate[0].tu;   RenderVertex[0].T[i].tv = RenderLayerInfo.TextureCoordinate[0].tv;
                RenderVertex[1].T[i].tu = RenderLayerInfo.TextureCoordinate[1].tu;   RenderVertex[1].T[i].tv = RenderLayerInfo.TextureCoordinate[1].tv;
                RenderVertex[2].T[i].tu = RenderLayerInfo.TextureCoordinate[2].tu;   RenderVertex[2].T[i].tv = RenderLayerInfo.TextureCoordinate[2].tv;
                RenderVertex[3].T[i].tu = RenderLayerInfo.TextureCoordinate[3].tu;   RenderVertex[3].T[i].tv = RenderLayerInfo.TextureCoordinate[3].tv;
            }

            hr = pDevice->DrawPrimitiveUP(  D3DPT_TRIANGLESTRIP,
                2,
                (LPVOID)(RenderVertex),
                sizeof(Vertex4T)); 
        }

        if(MaxTextureStage == 5)
        {
            hr = pDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX5 );


            Vertex5T RenderVertex[4];

            FLOAT z = 0.9f;
            FLOAT rhw = 1.0f;


            RenderVertex[0].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[1].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[2].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[3].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);

            for(i = 0; i < MaxTextureStage; i++)
            {
                LayerInfo RenderLayerInfo = pObj->m_LayerInfo[MaxTextureStage - 1 - i];
                RenderVertex[0].T[i].tu = RenderLayerInfo.TextureCoordinate[0].tu;   RenderVertex[0].T[i].tv = RenderLayerInfo.TextureCoordinate[0].tv;
                RenderVertex[1].T[i].tu = RenderLayerInfo.TextureCoordinate[1].tu;   RenderVertex[1].T[i].tv = RenderLayerInfo.TextureCoordinate[1].tv;
                RenderVertex[2].T[i].tu = RenderLayerInfo.TextureCoordinate[2].tu;   RenderVertex[2].T[i].tv = RenderLayerInfo.TextureCoordinate[2].tv;
                RenderVertex[3].T[i].tu = RenderLayerInfo.TextureCoordinate[3].tu;   RenderVertex[3].T[i].tv = RenderLayerInfo.TextureCoordinate[3].tv;
            }

            hr = pDevice->DrawPrimitiveUP(  D3DPT_TRIANGLESTRIP,
                2,
                (LPVOID)(RenderVertex),
                sizeof(Vertex5T)); 
        }

        if(MaxTextureStage == 6)
        {
            hr = pDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX6 );


            Vertex6T RenderVertex[4];

            FLOAT z = 0.9f;
            FLOAT rhw = 1.0f;


            RenderVertex[0].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[1].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[2].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[3].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);

            for(i = 0; i < MaxTextureStage; i++)
            {
                LayerInfo RenderLayerInfo = pObj->m_LayerInfo[MaxTextureStage - 1 - i];
                RenderVertex[0].T[i].tu = RenderLayerInfo.TextureCoordinate[0].tu;   RenderVertex[0].T[i].tv = RenderLayerInfo.TextureCoordinate[0].tv;
                RenderVertex[1].T[i].tu = RenderLayerInfo.TextureCoordinate[1].tu;   RenderVertex[1].T[i].tv = RenderLayerInfo.TextureCoordinate[1].tv;
                RenderVertex[2].T[i].tu = RenderLayerInfo.TextureCoordinate[2].tu;   RenderVertex[2].T[i].tv = RenderLayerInfo.TextureCoordinate[2].tv;
                RenderVertex[3].T[i].tu = RenderLayerInfo.TextureCoordinate[3].tu;   RenderVertex[3].T[i].tv = RenderLayerInfo.TextureCoordinate[3].tv;
            }

            hr = pDevice->DrawPrimitiveUP(  D3DPT_TRIANGLESTRIP,
                2,
                (LPVOID)(RenderVertex),
                sizeof(Vertex6T)); 
        }

        if(MaxTextureStage == 7)
        {
            hr = pDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX7 );


            Vertex7T RenderVertex[4];

            FLOAT z = 0.9f;
            FLOAT rhw = 1.0f;


            RenderVertex[0].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[1].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[2].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[3].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);

            for(i = 0; i < MaxTextureStage; i++)
            {
                LayerInfo RenderLayerInfo = pObj->m_LayerInfo[MaxTextureStage - 1 - i];
                RenderVertex[0].T[i].tu = RenderLayerInfo.TextureCoordinate[0].tu;   RenderVertex[0].T[i].tv = RenderLayerInfo.TextureCoordinate[0].tv;
                RenderVertex[1].T[i].tu = RenderLayerInfo.TextureCoordinate[1].tu;   RenderVertex[1].T[i].tv = RenderLayerInfo.TextureCoordinate[1].tv;
                RenderVertex[2].T[i].tu = RenderLayerInfo.TextureCoordinate[2].tu;   RenderVertex[2].T[i].tv = RenderLayerInfo.TextureCoordinate[2].tv;
                RenderVertex[3].T[i].tu = RenderLayerInfo.TextureCoordinate[3].tu;   RenderVertex[3].T[i].tv = RenderLayerInfo.TextureCoordinate[3].tv;
            }

            hr = pDevice->DrawPrimitiveUP(  D3DPT_TRIANGLESTRIP,
                2,
                (LPVOID)(RenderVertex),
                sizeof(Vertex7T)); 
        }

        if(MaxTextureStage == 8)
        {
            hr = pDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX8 );


            Vertex8T RenderVertex[4];

            FLOAT z = 0.9f;
            FLOAT rhw = 1.0f;


            RenderVertex[0].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[1].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.left - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[2].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.bottom - 0.5f, z, rhw), m_DisplaySize);
            RenderVertex[3].Pos = GetRotatedVector(m_RotationDegree, S3VECTOR4(pObj->m_ScreenRect.right - 0.5f, pObj->m_ScreenRect.top - 0.5f, z, rhw), m_DisplaySize);

            for(i = 0; i < MaxTextureStage; i++)
            {
                LayerInfo RenderLayerInfo = pObj->m_LayerInfo[MaxTextureStage - 1 - i];
                RenderVertex[0].T[i].tu = RenderLayerInfo.TextureCoordinate[0].tu;   RenderVertex[0].T[i].tv = RenderLayerInfo.TextureCoordinate[0].tv;
                RenderVertex[1].T[i].tu = RenderLayerInfo.TextureCoordinate[1].tu;   RenderVertex[1].T[i].tv = RenderLayerInfo.TextureCoordinate[1].tv;
                RenderVertex[2].T[i].tu = RenderLayerInfo.TextureCoordinate[2].tu;   RenderVertex[2].T[i].tv = RenderLayerInfo.TextureCoordinate[2].tv;
                RenderVertex[3].T[i].tu = RenderLayerInfo.TextureCoordinate[3].tu;   RenderVertex[3].T[i].tv = RenderLayerInfo.TextureCoordinate[3].tv;
            }

            hr = pDevice->DrawPrimitiveUP(  D3DPT_TRIANGLESTRIP,
                2,
                (LPVOID)(RenderVertex),
                sizeof(Vertex8T)); 
        }

        // clear texture stage
        for(DWORD i=0; i< MaxTextureStage; i++)
        {
            DWORD TextureStage = MaxTextureStage - i - 1;

            hr = pDevice->SetTexture(TextureStage, NULL);
        }

#endif

        delete pObj;

        it++;
    }while(it != end);

    m_RenderList.clear();
}


void S3RenderMixer::SaveSnapShot(CString Filename, int Width, RECT CaptureRect)
{
    m_CaptureFile = Filename;
    m_CaptureWidth = Width;
    m_CaptureStatus = S3S_CAPTURE_STATE_REQUIRE_SAVE;
    m_CaptureRect = CaptureRect;

    while(m_CaptureStatus != S3S_CAPTURE_STATE_SAVE_FINISH)
    {
        Sleep(33);
    }

}


void S3RenderMixer::Lock()
{
    m_MixerObjectLock.Lock();
}

void S3RenderMixer::Unlock()
{
    m_MixerObjectLock.Unlock();
}

DWORD S3RenderMixer::GetObjectCount()
{
    return m_listObjects.size();
}