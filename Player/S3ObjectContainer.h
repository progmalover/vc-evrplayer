#pragma once
#include <D3D9.h>
#include "S3D3DMath.h"
#include "S3SignageSetting.h"
#include "S3RenderableObject.h"
#include "S3TransitionProvider.h"

class S3TransitionProvider;

class S3ObjectContainer
{
    friend class S3RenderMixer;
public:
    S3ObjectContainer(S3SIGNAGE_CONTENT  &Content, float m_Scale, int nFPS, FLOAT RotateDegree);
    ~S3ObjectContainer(void);

    static bool compare(const S3ObjectContainer* t1,const S3ObjectContainer* t2)
    {
        return t1->m_ZOrder < t2->m_ZOrder;
	}
    
    HRESULT         InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice); ///initalize function

    HRESULT         RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);///device lost handle
    HRESULT         InvalidateDeviceObjects();                        ///device lost handle
    HRESULT         DeleteDeviceObjects();                            ///device lost handle


    HRESULT         Start();                                          ///Mixer start to render this object
    HRESULT         Stop();                                           ///Mixer end render this object
    HRESULT         PrepareRender();
    HRESULT         EndRender();
    CString         GetObjectType();
    HRESULT         ProcessMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam); ///Mouse message process
    HRESULT         ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT         Pause();
    HRESULT         Resume();
    //HRESULT         CheckLoop();
    HRESULT         LoopContent();

    BOOL            IsCreateThreadFinished();
    VOID            WaitCreateThreadFinish();
    VOID            SetZOrder(UINT Zorder);
protected:
    S3SIGNAGE_CONTENT                   m_Content;
    int                                 m_ZOrder;
    float                               m_Scale;

    FLOAT                               m_RotateDegree;

    RECTF                               m_ScreenRect;
    DWORD                               m_StartTime;
    BOOL                                m_bPaused;

    S3RenderableObject                 *m_pRenderableObj;

    LPDIRECT3DDEVICE9                   m_pd3dDevice; 

    int                                 m_ContentWidth;
    int                                 m_ContentHeight;
    float                               m_ContentScale;
    int                                 m_nFPS;

    // used for rendering
    list<RenderRect>                    m_RenderRect;

    S3SIGNAGE_TRANSITION_SETTING        m_InTransition;
    BOOL                                m_bInTransitionInit;
    S3SIGNAGE_TRANSITION_SETTING        m_OutTransition;
    BOOL                                m_bOutTransitionInit;
    S3TransitionProvider               *m_pTransition;

    LPDIRECT3DTEXTURE9                  m_pBGTexture;


    HANDLE                              m_hThread;
    volatile   int                      m_ThreadStatus;//0 no thread, 1 running, 2 require stop
    static DWORD WINAPI                 ObjCreateThread( LPVOID lpParameter );

    HRESULT                             ParserTransition(S3SIGNAGE_TRANSITION_SETTING &TransitionSetting, S3SIGNAGE_TRANSIION &TransitionDesc);

    HRESULT                             InterpolateTransform(S3SIGNAGE_TRANSFORM &Transform, float &XValue, float &YValue);
	VOID								RevertTransitionDirection(S3SIGNAGE_TRANSITION_SETTING &Transition);
    
};

