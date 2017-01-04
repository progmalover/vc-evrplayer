#include "stdafx.h"
#include "S3Signage.h"
#include "S3TransitionProvider.h"

#define BLIND_LINE      30
#define SCREW_STEP       8
#define WAVE_STEP       30
#define ROLL_SCALE      0.07f
#define BLOCK_STEP      30

S3TransitionProvider::S3TransitionProvider(void)
{
    m_pTransitionTexture = NULL;
    m_pTransitionSurface = NULL;

    m_pBlockSequence = NULL;
}

S3TransitionProvider::~S3TransitionProvider(void)
{
    SAFE_RELEASE(m_pTransitionSurface);
    SAFE_RELEASE(m_pTransitionTexture);
    SAFE_DELETE_ARRAY(m_pBlockSequence);

}

S3TransitionProvider::S3TransitionProvider(LPDIRECT3DDEVICE9 pd3dDevice,
                                           SIZE TransitionSize)
{
    m_pTransitionTexture = NULL;
    m_pTransitionSurface = NULL;


    m_pBlockSequence = NULL;

    m_RTRect.top = 0;
    m_RTRect.left = 0;
    m_RTRect.right = (float)TransitionSize.cx;
    m_RTRect.bottom = (float)TransitionSize.cy;

    m_pd3dDevice = pd3dDevice;


    if(m_pd3dDevice)
    {
        HRESULT hr;

        
        CHECK_HR(
            hr = m_pd3dDevice->CreateTexture(TransitionSize.cx, TransitionSize.cy, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
            &m_pTransitionTexture, NULL),
            ::DbgMsg(_T("S3TransitionProvider::S3TransitionProvider: failed to create the Transition texture, hr = 0x%08x"), hr));

               
        CHECK_HR(
            hr = m_pTransitionTexture->GetSurfaceLevel(0, &m_pTransitionSurface),
            ::DbgMsg(_T("S3TransitionProvider::S3TransitionProvider: failed to get the Transition texture surface, hr = 0x%08x"), hr));

        m_Graphics.CreateGraphics(m_pTransitionSurface, m_pd3dDevice);
    }


    if(m_RTRect.Width() > m_RTRect.Height())
    {
        m_BlockLength = m_RTRect.Width() / BLOCK_STEP;
        m_BlockHCnt = BLOCK_STEP;
        m_BlockVCnt = (DWORD)(m_RTRect.Height() / m_BlockLength) + 1;
    }
    else
    {
        m_BlockLength = m_RTRect.Height() / BLOCK_STEP;
        m_BlockHCnt = (DWORD)(m_RTRect.Width() / m_BlockLength) + 1;
        m_BlockVCnt = BLOCK_STEP;
    }
    m_BlockToalCnt = m_BlockHCnt * m_BlockVCnt;

    m_pBlockSequence = new DWORD[m_BlockToalCnt];

    for(DWORD i = 0; i < m_BlockToalCnt; i++)
    {
        m_pBlockSequence[i] = i;
    }
    // get the random sequence
    DWORD tempData, tempPos;
    for(DWORD i = 0; i < m_BlockToalCnt; i++)
    {
        tempData = m_pBlockSequence[i];
        tempPos  = rand() % m_BlockToalCnt;
        m_pBlockSequence[i] = m_pBlockSequence[tempPos];
        m_pBlockSequence[tempPos] = tempData;
    }
}

HRESULT S3TransitionProvider::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
    HRESULT hr = S_OK;
    
    m_pd3dDevice = pd3dDevice;


    if(m_pd3dDevice)
    {
        
        CHECK_HR(
            hr = m_pd3dDevice->CreateTexture((int)m_RTRect.Width(), (int)m_RTRect.Height(), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
            &m_pTransitionTexture, NULL),
            ::DbgMsg(_T("S3TransitionProvider::S3TransitionProvider: failed to create the Transition texture, hr = 0x%08x"), hr));

               
        CHECK_HR(
            hr = m_pTransitionTexture->GetSurfaceLevel(0, &m_pTransitionSurface),
            ::DbgMsg(_T("S3TransitionProvider::S3TransitionProvider: failed to get the Transition texture surface, hr = 0x%08x"), hr));

        m_Graphics.CreateGraphics(m_pTransitionSurface, m_pd3dDevice);
    }
    return hr;

}


HRESULT S3TransitionProvider::InvalidateDeviceObjects()
{
    SAFE_RELEASE(m_pTransitionSurface);
    SAFE_RELEASE(m_pTransitionTexture);

    return S_OK;
}



HRESULT S3TransitionProvider::SetTransition(S3SIGNAGE_TRANSITION_SETTING Setting)
{
    m_Setting = Setting;

    // init rand seed
    srand((unsigned int)(time(NULL)));

    if(m_Setting.Effect == S3S_EFFECT_RANDOM)
    {
        //m_Setting.Effect = (S3S_CONTENT_EFFECT)(rand()%(S3S_EFFECT_LAST - 2) + 2);
        S3S_CONTENT_EFFECT AvailableEffect[14] = {S3S_EFFECT_WIPE,
                                                    S3S_EFFECT_EXPAND,
                                                    S3S_EFFECT_SLIDEIN,
                                                    S3S_EFFECT_WHEEL,
                                                    S3S_EFFECT_CLOCK,
                                                    S3S_EFFECT_ROUND,
                                                    S3S_EFFECT_PLUS,
                                                    S3S_EFFECT_BLIND,
                                                    S3S_EFFECT_SCREW,
                                                    S3S_EFFECT_WAVE,
                                                    S3S_EFFECT_TURNOVER,
                                                    S3S_EFFECT_ROLL,
                                                    S3S_EFFECT_FADE,
                                                    S3S_EFFECT_BLOCK,
                                                    };

        m_Setting.Effect = AvailableEffect[rand()%14];
    }

    switch(m_Setting.Effect)
    {
    case S3S_EFFECT_WIPE:
        {
            if(S3S_EFFECT_DIRECTION_RANDOM == m_Setting.EffectDirection || S3S_EFFECT_DIRECTION_NONE == m_Setting.EffectDirection)
            {
                
                S3S_CONTENT_EFFECT_DIRECTION D[10] = {S3S_EFFECT_DIRECTION_UPPER,
                                            S3S_EFFECT_DIRECTION_UPPER_RIGHT,
                                            S3S_EFFECT_DIRECTION_RIGHT,
                                            S3S_EFFECT_DIRECTION_LOWER_RIGHT,
                                            S3S_EFFECT_DIRECTION_LOWER,
                                            S3S_EFFECT_DIRECTION_LOWER_LEFT,
                                            S3S_EFFECT_DIRECTION_LEFT,
                                            S3S_EFFECT_DIRECTION_UPPER_LEFT,
                                            S3S_EFFECT_DIRECTION_LEFT_RIGHT,
                                            S3S_EFFECT_DIRECTION_UP_DOWN};

                m_Setting.EffectDirection = D[rand()%10];
            }
        }
        break;
    case S3S_EFFECT_EXPAND:
        {
            if(S3S_EFFECT_DIRECTION_RANDOM == m_Setting.EffectDirection || S3S_EFFECT_DIRECTION_NONE == m_Setting.EffectDirection)
            {
                
                S3S_CONTENT_EFFECT_DIRECTION D[10] = {S3S_EFFECT_DIRECTION_UPPER,
                                            S3S_EFFECT_DIRECTION_UPPER_RIGHT,
                                            S3S_EFFECT_DIRECTION_RIGHT,
                                            S3S_EFFECT_DIRECTION_LOWER_RIGHT,
                                            S3S_EFFECT_DIRECTION_LOWER,
                                            S3S_EFFECT_DIRECTION_LOWER_LEFT,
                                            S3S_EFFECT_DIRECTION_LEFT,
                                            S3S_EFFECT_DIRECTION_UPPER_LEFT,
                                            S3S_EFFECT_DIRECTION_LEFT_RIGHT,
                                            S3S_EFFECT_DIRECTION_UP_DOWN};

                m_Setting.EffectDirection = D[rand()%10];
            }
        }
        break;
    case S3S_EFFECT_SLIDEIN:
        {
            if(S3S_EFFECT_DIRECTION_RANDOM == m_Setting.EffectDirection || S3S_EFFECT_DIRECTION_NONE == m_Setting.EffectDirection)
            {
                
                S3S_CONTENT_EFFECT_DIRECTION D[8] = {S3S_EFFECT_DIRECTION_UPPER,
                                            S3S_EFFECT_DIRECTION_UPPER_RIGHT,
                                            S3S_EFFECT_DIRECTION_RIGHT,
                                            S3S_EFFECT_DIRECTION_LOWER_RIGHT,
                                            S3S_EFFECT_DIRECTION_LOWER,
                                            S3S_EFFECT_DIRECTION_LOWER_LEFT,
                                            S3S_EFFECT_DIRECTION_LEFT,
                                            S3S_EFFECT_DIRECTION_UPPER_LEFT};

                m_Setting.EffectDirection = D[rand()%8];
            }
        }
        break;
    case S3S_EFFECT_BLIND:
        {
            if(S3S_EFFECT_DIRECTION_RANDOM == m_Setting.EffectDirection || S3S_EFFECT_DIRECTION_NONE == m_Setting.EffectDirection)
            {
                
                S3S_CONTENT_EFFECT_DIRECTION D[4] = {S3S_EFFECT_DIRECTION_UPPER,
                                            S3S_EFFECT_DIRECTION_RIGHT,
                                            S3S_EFFECT_DIRECTION_LOWER,
                                            S3S_EFFECT_DIRECTION_LEFT};

                m_Setting.EffectDirection = D[rand()%4];
            }
        }
        break;
    case S3S_EFFECT_TURNOVER:
        {
            if(S3S_EFFECT_DIRECTION_RANDOM == m_Setting.EffectDirection || S3S_EFFECT_DIRECTION_NONE == m_Setting.EffectDirection)
            {
                
                S3S_CONTENT_EFFECT_DIRECTION D[4] = {S3S_EFFECT_DIRECTION_UPPER,
                                            S3S_EFFECT_DIRECTION_RIGHT,
                                            S3S_EFFECT_DIRECTION_LOWER,
                                            S3S_EFFECT_DIRECTION_LEFT};

                m_Setting.EffectDirection = D[rand()%4];
            }
        }
        break;
    case S3S_EFFECT_ROLL:
        {
            if(S3S_EFFECT_DIRECTION_RANDOM == m_Setting.EffectDirection || S3S_EFFECT_DIRECTION_NONE == m_Setting.EffectDirection)
            {
                
                S3S_CONTENT_EFFECT_DIRECTION D[4] = {S3S_EFFECT_DIRECTION_UPPER,
                                            S3S_EFFECT_DIRECTION_RIGHT,
                                            S3S_EFFECT_DIRECTION_LOWER,
                                            S3S_EFFECT_DIRECTION_LEFT};

                m_Setting.EffectDirection = D[rand()%4];
            }
        }
        break;
    case S3S_EFFECT_WHEEL:
        {
            if(S3S_EFFECT_DIRECTION_RANDOM == m_Setting.EffectDirection || S3S_EFFECT_DIRECTION_NONE == m_Setting.EffectDirection)
            {
                S3S_CONTENT_EFFECT_DIRECTION D[2] = {S3S_EFFECT_DIRECTION_CLOCKWISE, S3S_EFFECT_DIRECTION_COUNTERCLOCKWISE};

                m_Setting.EffectDirection = D[rand()%2];
            }
        }
        break;
    case S3S_EFFECT_CLOCK:
        {
            if(S3S_EFFECT_DIRECTION_RANDOM == m_Setting.EffectDirection || S3S_EFFECT_DIRECTION_NONE == m_Setting.EffectDirection)
            {
                S3S_CONTENT_EFFECT_DIRECTION D[2] = {S3S_EFFECT_DIRECTION_CLOCKWISE, S3S_EFFECT_DIRECTION_COUNTERCLOCKWISE};

                m_Setting.EffectDirection = D[rand()%2];
            }
        }
        break;
    case S3S_EFFECT_WAVE:
        {
            if(S3S_EFFECT_DIRECTION_RANDOM == m_Setting.EffectDirection || S3S_EFFECT_DIRECTION_NONE == m_Setting.EffectDirection)
            {
                S3S_CONTENT_EFFECT_DIRECTION D[4] = {S3S_EFFECT_DIRECTION_UPPER_RIGHT, S3S_EFFECT_DIRECTION_LOWER_RIGHT,
                                                     S3S_EFFECT_DIRECTION_LOWER_LEFT,S3S_EFFECT_DIRECTION_UPPER_LEFT};

                m_Setting.EffectDirection = D[rand()%4];
            }
        }
    case S3S_EFFECT_FADE:
        {
        }
        break;
    case S3S_EFFECT_BLOCK:
        {
        }
        break;
    case S3S_EFFECT_ROUND:
        {
            if(S3S_EFFECT_DIRECTION_RANDOM == m_Setting.EffectDirection || S3S_EFFECT_DIRECTION_NONE == m_Setting.EffectDirection)
            {
                m_Setting.EffectDirection = S3S_EFFECT_DIRECTION_NONE;
            }
        }
        break;
    case S3S_EFFECT_SCREW:
        {
            if(S3S_EFFECT_DIRECTION_RANDOM == m_Setting.EffectDirection || S3S_EFFECT_DIRECTION_NONE == m_Setting.EffectDirection)
            {
                S3S_CONTENT_EFFECT_DIRECTION D[2] = {S3S_EFFECT_DIRECTION_CLOCKWISE,
                                                        S3S_EFFECT_DIRECTION_COUNTERCLOCKWISE};

                m_Setting.EffectDirection = D[rand()%2];
            }
        }
        break;
    case S3S_EFFECT_PLUS:
        {
            if(S3S_EFFECT_DIRECTION_RANDOM == m_Setting.EffectDirection || S3S_EFFECT_DIRECTION_NONE == m_Setting.EffectDirection)
            {

                m_Setting.EffectDirection = S3S_EFFECT_DIRECTION_NONE;
            }
        }
        break;
    default:
        return E_UNEXPECTED;
        break;
    }


    return S_OK;
}

LPDIRECT3DTEXTURE9 S3TransitionProvider::GetTransitionImage()
{
    return m_pTransitionTexture;
}

HRESULT S3TransitionProvider::ProcessTransition(float Schedule,list<RenderRect>  &RenderInfo)
{
    S3S_CONTENT_EFFECT Effect = m_Setting.Effect;

    m_RenderInfo = &RenderInfo;

    Preprocess();

    if(Schedule <=0) return S_OK;

    switch(Effect)
    {
    case S3S_EFFECT_WIPE:
        {
            return ProcessWipeIn(Schedule);
        }
        break;
    case S3S_EFFECT_EXPAND:
        {
            return ProcessExpandIn(Schedule);
        }
        break;
    case S3S_EFFECT_SLIDEIN:
        {
            return ProcessSlideIn(Schedule);
        }
        break;
    case S3S_EFFECT_BLIND:
        {
            return ProcessBlindIn(Schedule);
        }
        break;
    case S3S_EFFECT_TURNOVER:
        {
            return ProcessTurnOverOut(1- Schedule);
        }
        break;
    case S3S_EFFECT_ROLL:
        {
            return ProcessRollOut(1- Schedule);
        }
        break;
    case S3S_EFFECT_WHEEL:
        {
            return ProcessWheelIn(Schedule);
        }
        break;
    case S3S_EFFECT_CLOCK:
        {
            return ProcessClockIn(Schedule);
        }
        break;
    case S3S_EFFECT_WAVE:
        {
            return ProcessWaveIn(Schedule);
        }
        break;
    case S3S_EFFECT_FADE:
        {
            return ProcessFadeIn(Schedule);
        }
        break;
    case S3S_EFFECT_BLOCK:
        {
            return ProcessBlockIn(Schedule);
        }
        break;
    case S3S_EFFECT_ROUND:
        {
            return ProcessRoundIn(Schedule);
        }
        break;
    case S3S_EFFECT_SCREW:
        {
            return ProcessScrewIn(Schedule);
        }
        break;
    case S3S_EFFECT_PLUS:
        {
            return ProcessPlusIn(Schedule);
        }
        break;
    default:
        break;
    }

    return E_FAIL;
}



void S3TransitionProvider::Preprocess()
{
    m_ReplacerDestRect = m_RTRect;
    m_ReplacerSrcRect = m_RTRect;

    S3D2DClipList ClipList;
    ClipList.Clear();
    m_Graphics.SetClip(ClipList);

    m_Graphics.DrawImage(NULL, D3DTADDRESS_WRAP, NULL, RECTF(0,0,0,0));
}

HRESULT S3TransitionProvider::ProcessWipeIn(float Schedule)
{
    HRESULT hr = E_FAIL;

    S3S_CONTENT_EFFECT_DIRECTION Direction = m_Setting.EffectDirection;

    switch(Direction)
    {
    case S3S_EFFECT_DIRECTION_UPPER:
        {
            m_ReplacerDestRect.top = m_RTRect.bottom - Schedule*m_RTRect.Height();

            m_ReplacerSrcRect.top = m_ReplacerSrcRect.bottom - Schedule*m_ReplacerSrcRect.Height();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_UPPER_RIGHT:
        {
            m_ReplacerDestRect.top = m_RTRect.bottom - Schedule*m_RTRect.Height();
            m_ReplacerDestRect.right = m_RTRect.left + Schedule*m_RTRect.Width();

            m_ReplacerSrcRect.top = m_ReplacerSrcRect.bottom - Schedule*m_ReplacerSrcRect.Height();
            m_ReplacerSrcRect.right = m_ReplacerSrcRect.left + Schedule*m_ReplacerSrcRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_RIGHT:
        {
            m_ReplacerDestRect.right = m_RTRect.left + Schedule*m_RTRect.Width();

            m_ReplacerSrcRect.right = m_ReplacerSrcRect.left + Schedule*m_ReplacerSrcRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER_RIGHT:
        {
            m_ReplacerDestRect.bottom = m_RTRect.top + Schedule*m_RTRect.Height();
            m_ReplacerDestRect.right = m_RTRect.left + Schedule*m_RTRect.Width();

            m_ReplacerSrcRect.bottom = m_ReplacerSrcRect.top + Schedule*m_ReplacerSrcRect.Height();
            m_ReplacerSrcRect.right = m_ReplacerSrcRect.left + Schedule*m_ReplacerSrcRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER:
        {
            m_ReplacerDestRect.bottom = m_RTRect.top + Schedule*m_RTRect.Height();

            m_ReplacerSrcRect.bottom = m_ReplacerSrcRect.top + Schedule*m_ReplacerSrcRect.Height();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER_LEFT:
        {
            m_ReplacerDestRect.bottom = m_RTRect.top + Schedule*m_RTRect.Height();
            m_ReplacerDestRect.left = m_RTRect.right - Schedule*m_RTRect.Width();

            m_ReplacerSrcRect.bottom = m_ReplacerSrcRect.top + Schedule*m_ReplacerSrcRect.Height();
            m_ReplacerSrcRect.left = m_ReplacerSrcRect.right - Schedule*m_ReplacerSrcRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LEFT:
        {
            m_ReplacerDestRect.left = m_RTRect.right - Schedule*m_RTRect.Width();

            m_ReplacerSrcRect.left = m_ReplacerSrcRect.right - Schedule*m_ReplacerSrcRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_UPPER_LEFT:
        {
            m_ReplacerDestRect.top = m_RTRect.bottom - Schedule*m_RTRect.Height();
            m_ReplacerDestRect.left = m_RTRect.right - Schedule*m_RTRect.Width();

            m_ReplacerSrcRect.top = m_ReplacerSrcRect.bottom - Schedule*m_ReplacerSrcRect.Height();
            m_ReplacerSrcRect.left = m_ReplacerSrcRect.right - Schedule*m_ReplacerSrcRect.Width();

            hr = S_OK;
        }
        break;

    case S3S_EFFECT_DIRECTION_LEFT_RIGHT:
        {
            m_ReplacerDestRect.left = m_RTRect.right/2 - Schedule*(m_RTRect.right/2 - m_RTRect.left);
            m_ReplacerDestRect.right = m_RTRect.left + m_RTRect.right/2 + Schedule*(m_RTRect.right/2 - m_RTRect.left);

            RECTF tempRect = m_ReplacerSrcRect;

            m_ReplacerSrcRect.left = tempRect.right/2 - Schedule*(tempRect.right/2 - tempRect.left);
            m_ReplacerSrcRect.right = tempRect.left + tempRect.right/2 + Schedule*(tempRect.right/2 - tempRect.left);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_UP_DOWN:
        {
            m_ReplacerDestRect.top = (m_RTRect.bottom/2 - Schedule*(m_RTRect.bottom/2 - m_RTRect.top));
            m_ReplacerDestRect.bottom = (m_RTRect.top + m_RTRect.bottom/2 + Schedule*(m_RTRect.bottom/2 - m_RTRect.top));

            RECTF tempRect = m_ReplacerSrcRect;

            m_ReplacerSrcRect.top = (tempRect.bottom/2 - Schedule*(tempRect.bottom/2 - tempRect.top));
            m_ReplacerSrcRect.bottom = (tempRect.top + tempRect.bottom/2 + Schedule*(tempRect.bottom/2 - tempRect.top));

            hr = S_OK;
        }
        break;

    default:
        break;
    }

    
    if(hr == S_OK)
    {
         RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);
    }

    return hr;
}


HRESULT S3TransitionProvider::ProcessExpandIn(float Schedule)
{
    HRESULT hr = E_FAIL;

    S3S_CONTENT_EFFECT_DIRECTION Direction = m_Setting.EffectDirection;

    switch(Direction)
    {
    case S3S_EFFECT_DIRECTION_UPPER:
        {
            m_ReplacerDestRect.top = m_RTRect.bottom - Schedule*m_RTRect.Height();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_UPPER_RIGHT:
        {
            m_ReplacerDestRect.top = m_RTRect.bottom - Schedule*m_RTRect.Height();
            m_ReplacerDestRect.right = m_RTRect.left + Schedule*m_RTRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_RIGHT:
        {
            m_ReplacerDestRect.right = m_RTRect.left + Schedule*m_RTRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER_RIGHT:
        {
            m_ReplacerDestRect.bottom = m_RTRect.top + Schedule*m_RTRect.Height();
            m_ReplacerDestRect.right = m_RTRect.left + Schedule*m_RTRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER:
        {
            m_ReplacerDestRect.bottom = m_RTRect.top + Schedule*m_RTRect.Height();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER_LEFT:
        {
            m_ReplacerDestRect.bottom = m_RTRect.top + Schedule*m_RTRect.Height();
            m_ReplacerDestRect.left = m_RTRect.right - Schedule*m_RTRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LEFT:
        {
            m_ReplacerDestRect.left = m_RTRect.right - Schedule*m_RTRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_UPPER_LEFT:
        {
            m_ReplacerDestRect.top = m_RTRect.bottom - Schedule*m_RTRect.Height();
            m_ReplacerDestRect.left = m_RTRect.right - Schedule*m_RTRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_CLOCKWISE:
        {
        }
        break;
    case S3S_EFFECT_DIRECTION_LEFT_RIGHT:
        {
            m_ReplacerDestRect.left = m_RTRect.right/2 - Schedule*(m_RTRect.right/2 - m_RTRect.left);
            m_ReplacerDestRect.right = m_RTRect.left + m_RTRect.right/2 + Schedule*(m_RTRect.right/2 - m_RTRect.left);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_UP_DOWN:
        {
            m_ReplacerDestRect.top = (m_RTRect.bottom/2 - Schedule*(m_RTRect.bottom/2 - m_RTRect.top));
            m_ReplacerDestRect.bottom = (m_RTRect.top + m_RTRect.bottom/2 + Schedule*(m_RTRect.bottom/2 - m_RTRect.top));

            hr = S_OK;
        }
        break;

    default:
        break;
    }

    if(hr == S_OK)
    {

        RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);
    }

    return hr;
}



HRESULT S3TransitionProvider::ProcessSlideIn(float Schedule)
{
    HRESULT hr = E_FAIL;

    S3S_CONTENT_EFFECT_DIRECTION Direction = m_Setting.EffectDirection;

    switch(Direction)
    {
    case S3S_EFFECT_DIRECTION_UPPER:
        {
            m_ReplacerDestRect.top = m_RTRect.bottom - Schedule*m_RTRect.Height();

            m_ReplacerSrcRect.bottom = m_ReplacerSrcRect.top + Schedule*m_ReplacerSrcRect.Height();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_UPPER_RIGHT:
        {
            m_ReplacerDestRect.top = m_RTRect.bottom - Schedule*m_RTRect.Height();
            m_ReplacerDestRect.right = m_RTRect.left + Schedule*m_RTRect.Width();

            m_ReplacerSrcRect.bottom = m_ReplacerSrcRect.top + Schedule*m_ReplacerSrcRect.Height();
            m_ReplacerSrcRect.left = m_ReplacerSrcRect.right - Schedule*m_ReplacerSrcRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_RIGHT:
        {
            m_ReplacerDestRect.right = m_RTRect.left + Schedule*m_RTRect.Width();

            m_ReplacerSrcRect.left = m_ReplacerSrcRect.right - Schedule*m_ReplacerSrcRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER_RIGHT:
        {
            m_ReplacerDestRect.bottom = m_RTRect.top + Schedule*m_RTRect.Height();
            m_ReplacerDestRect.right = m_RTRect.left + Schedule*m_RTRect.Width();

            m_ReplacerSrcRect.top = m_ReplacerSrcRect.bottom - Schedule*m_ReplacerSrcRect.Height();
            m_ReplacerSrcRect.left = m_ReplacerSrcRect.right - Schedule*m_ReplacerSrcRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER:
        {
            m_ReplacerDestRect.bottom = m_RTRect.top + Schedule*m_RTRect.Height();

            m_ReplacerSrcRect.top = m_ReplacerSrcRect.bottom - Schedule*m_ReplacerSrcRect.Height();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER_LEFT:
        {
            m_ReplacerDestRect.bottom = m_RTRect.top + Schedule*m_RTRect.Height();
            m_ReplacerDestRect.left = m_RTRect.right - Schedule*m_RTRect.Width();

            m_ReplacerSrcRect.top = m_ReplacerSrcRect.bottom - Schedule*m_ReplacerSrcRect.Height();
            m_ReplacerSrcRect.right = m_ReplacerSrcRect.left + Schedule*m_ReplacerSrcRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LEFT:
        {
            m_ReplacerDestRect.left = m_RTRect.right - Schedule*m_RTRect.Width();

            m_ReplacerSrcRect.right = m_ReplacerSrcRect.left + Schedule*m_ReplacerSrcRect.Width();

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_UPPER_LEFT:
        {
            m_ReplacerDestRect.top = m_RTRect.bottom - Schedule*m_RTRect.Height();
            m_ReplacerDestRect.left = m_RTRect.right - Schedule*m_RTRect.Width();

            m_ReplacerSrcRect.bottom = m_ReplacerSrcRect.top + Schedule*m_ReplacerSrcRect.Height();
            m_ReplacerSrcRect.right = m_ReplacerSrcRect.left + Schedule*m_ReplacerSrcRect.Width();

            hr = S_OK;
        }
        break;

    default:
        break;
    }


    if(hr == S_OK)
    {
        RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);
    }

    return hr;
}





HRESULT S3TransitionProvider::ProcessClockIn(float Schedule)
{
    HRESULT hr = E_FAIL;

    S3S_CONTENT_EFFECT_DIRECTION Direction = m_Setting.EffectDirection;

    switch(Direction)
    {

    case S3S_EFFECT_DIRECTION_COUNTERCLOCKWISE:
        {
            FLOAT StartX, StartY;
            StartX = m_ReplacerDestRect.Width()/2;
            StartY = m_ReplacerDestRect.Height()/2;
            S3D2DClipList ClipList;
            ClipList.Clear();
            ClipList.DrawArc(StartX, StartY, 2*max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) , -PI/2.0, 2*PI*Schedule);

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_CLOCKWISE:
        {
            FLOAT StartX, StartY;
            StartX = m_ReplacerDestRect.Width()/2;
            StartY = m_ReplacerDestRect.Height()/2;
            S3D2DClipList ClipList;
            ClipList.Clear();
            ClipList.DrawArc(StartX, StartY, 2*max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) , -PI/2.0, 2*PI*Schedule, true);

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;

    default:
        break;
    }

    
    if(hr == S_OK)
    {
        RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);
    }

    return hr;
}




HRESULT S3TransitionProvider::ProcessWheelIn(float Schedule)
{
    HRESULT hr = E_FAIL;

    S3S_CONTENT_EFFECT_DIRECTION Direction = m_Setting.EffectDirection;

    switch(Direction)
    {

    case S3S_EFFECT_DIRECTION_COUNTERCLOCKWISE:
        {
            FLOAT StartX, StartY;
            StartX = m_ReplacerDestRect.Width()/2;
            StartY = m_ReplacerDestRect.Height()/2;
            S3D2DClipList ClipList;
            ClipList.Clear();
            ClipList.DrawArc(StartX, StartY, 2*max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) , (float)(-PI/2.0), (float)(PI*Schedule/2.0));
            ClipList.DrawArc(StartX, StartY, 2*max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) , (float)0, (float)(PI*Schedule/2.0));
            ClipList.DrawArc(StartX, StartY, 2*max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) , (float)PI/2.0, (float)(PI*Schedule/2.0));
            ClipList.DrawArc(StartX, StartY, 2*max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) , (float)PI, (float)(PI*Schedule/2.0));

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_CLOCKWISE:
        {
            FLOAT StartX, StartY;
            StartX = m_ReplacerDestRect.Width()/2;
            StartY = m_ReplacerDestRect.Height()/2;
            S3D2DClipList ClipList;
            ClipList.Clear();
            ClipList.DrawArc(StartX, StartY, 2*max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) , (float)-PI/2.0, (float)(PI*Schedule/2.0), true);
            ClipList.DrawArc(StartX, StartY, 2*max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) , (float)0, (float)(PI*Schedule/2.0), true);
            ClipList.DrawArc(StartX, StartY, 2*max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) , (float)PI/2.0, (float)(PI*Schedule/2.0), true);
            ClipList.DrawArc(StartX, StartY, 2*max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) , (float)PI, (float)(PI*Schedule/2.0), true);

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;

    default:
        break;
    }

    
    if(hr == S_OK)
    {
        RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);
    }

    return hr;
}



HRESULT S3TransitionProvider::ProcessRoundIn(float Schedule)
{
    HRESULT hr = E_FAIL;

    S3S_CONTENT_EFFECT_DIRECTION Direction = m_Setting.EffectDirection;

    S3D2DClipList ClipList;



    FLOAT StartX, StartY, Radius;
    StartX = m_ReplacerDestRect.Width()/2;
    StartY = m_ReplacerDestRect.Height()/2;

    Radius = sqrt(StartX*StartX + StartY*StartY);

    ClipList.Clear();
    ClipList.DrawArc(StartX, StartY, Radius*Schedule, 0, (float)(PI*2.0), true, PI/35);
            
    m_Graphics.SetClip(ClipList);

    RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);

    hr = S_OK;


    return hr;
}


HRESULT S3TransitionProvider::ProcessPlusIn(float Schedule)
{
    HRESULT hr = E_FAIL;

    S3S_CONTENT_EFFECT_DIRECTION Direction = m_Setting.EffectDirection;

    S3D2DClipList ClipList;



    RECTF Plus1, Plus2;
    Plus1 = Plus2 = m_ReplacerDestRect;

    Plus1.top    = Plus1.top + (1 - Schedule)*m_ReplacerDestRect.Height()/2;
    Plus1.bottom = Plus1.bottom - (1 - Schedule)*m_ReplacerDestRect.Height()/2;

    Plus2.left   = Plus2.left + (1 - Schedule)*m_ReplacerDestRect.Width()/2;
    Plus2.right  = Plus2.right - (1 - Schedule)*m_ReplacerDestRect.Width()/2;
            
    ClipList.Clear();
    ClipList.DrawRect(Plus1);
    ClipList.DrawRect(Plus2);
            
    m_Graphics.SetClip(ClipList);

    RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);

    hr = S_OK;


    return hr;
}

HRESULT S3TransitionProvider::ProcessBlindIn(float Schedule)
{
    HRESULT hr = E_FAIL;

    S3S_CONTENT_EFFECT_DIRECTION Direction = m_Setting.EffectDirection;

    S3D2DClipList ClipList;


    switch(Direction)
    {
    case S3S_EFFECT_DIRECTION_UPPER:
        {
            float LineHeight =  m_ReplacerDestRect.Height() / BLIND_LINE;
            RECTF Blind = m_ReplacerDestRect;
            
            S3D2DClipList ClipList;
            ClipList.Clear();

            for(int i = 0; i < BLIND_LINE; i++)
            {
                Blind.top = Blind.bottom - Schedule * LineHeight;
                
                ClipList.DrawRect(Blind);

                Blind.bottom = Blind.bottom - LineHeight;
            }

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_RIGHT:
        {
            float LineHeight =  m_ReplacerDestRect.Width() / BLIND_LINE;
            RECTF Blind = m_ReplacerDestRect;
            
            S3D2DClipList ClipList;
            ClipList.Clear();

            for(int i = 0; i < BLIND_LINE; i++)
            {
                Blind.right = Blind.left + Schedule * LineHeight;
                
                ClipList.DrawRect(Blind);

                Blind.left = Blind.left + LineHeight;
            }

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER:
        {
            float LineHeight =  m_ReplacerDestRect.Height() / BLIND_LINE;
            RECTF Blind = m_ReplacerDestRect;
            
            S3D2DClipList ClipList;
            ClipList.Clear();

            for(int i = 0; i < BLIND_LINE; i++)
            {
                Blind.bottom = Blind.top + Schedule * LineHeight;
                
                ClipList.DrawRect(Blind);

                Blind.top = Blind.top + LineHeight;
            }

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LEFT:
        {
            float LineHeight =  m_ReplacerDestRect.Width() / BLIND_LINE;
            RECTF Blind = m_ReplacerDestRect;
            
            S3D2DClipList ClipList;
            ClipList.Clear();

            for(int i = 0; i < BLIND_LINE; i++)
            {
                Blind.left = Blind.right - Schedule * LineHeight;
                
                ClipList.DrawRect(Blind);

                Blind.right = Blind.right - LineHeight;
            }

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;
    default:
        break;
    }

    if(hr == S_OK)
    {
        RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);
    }

    return hr;
}


HRESULT S3TransitionProvider::ProcessScrewIn(float Schedule)
{
    HRESULT hr = E_FAIL;

    S3S_CONTENT_EFFECT_DIRECTION Direction = m_Setting.EffectDirection;

    S3D2DClipList ClipList;

    switch(Direction)
    {
    case S3S_EFFECT_DIRECTION_CLOCKWISE:
        {
            float Sche = Schedule * SCREW_STEP + 1;
            int Moment =  (int)(Sche);
            int Step   =  (int)(Sche * 4) % 4;
            float StepMoment  =  (Sche * 4) - (int)(Sche * 4);
            
            S3D2DClipList ClipList;
            ClipList.Clear();

            RECTF Screw;

            // Left
            Screw = m_ReplacerDestRect;
            Screw.right = m_ReplacerDestRect.Width() / 2;
            Screw.top  = m_ReplacerDestRect.Height() / 2 - Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
            Screw.bottom  = m_ReplacerDestRect.Height() / 2 + Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
            if(Step == 0)
            {
                Screw.left = Screw.right - Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
                
                Screw.right = Screw.left;
                Screw.left = Screw.right - m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.top -= m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.top = Screw.bottom - StepMoment * Screw.Height();

                ClipList.DrawRect(Screw);
            }
            else
            {
                Screw.left = Screw.right - Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);

                Screw.right = Screw.left;
                Screw.left = Screw.right - m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.top -= m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.top = Screw.bottom - Screw.Height();

                ClipList.DrawRect(Screw);
            }

            // top
            Screw = m_ReplacerDestRect;
            Screw.left  = m_ReplacerDestRect.Width() / 2 - Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
            Screw.right  = m_ReplacerDestRect.Width() / 2 + Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
            Screw.bottom = m_ReplacerDestRect.Height() / 2;
            if(Step < 1)
            {
                Screw.top = Screw.bottom - Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
            }
            else if(Step == 1)
            {
                Screw.top = Screw.bottom - Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
                
                Screw.bottom = Screw.top;
                Screw.top = Screw.bottom - m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.right += m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.right = Screw.left + StepMoment * Screw.Width();

                ClipList.DrawRect(Screw);
            }
            else
            {
                Screw.top = Screw.bottom - Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);

                Screw.bottom = Screw.top;
                Screw.top = Screw.bottom - m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.right += m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.right = Screw.left + Screw.Width();

                ClipList.DrawRect(Screw);
            }

            // right
            Screw = m_ReplacerDestRect;
            Screw.left = m_ReplacerDestRect.Width() / 2;
            Screw.top  = m_ReplacerDestRect.Height() / 2 - Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
            Screw.bottom  = m_ReplacerDestRect.Height() / 2 + Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
            if(Step < 2)
            {
                Screw.right = Screw.left + Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
            }
            else if(Step == 2)
            {
                Screw.right = Screw.left + Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
                
                Screw.left = Screw.right;
                Screw.right = Screw.left + m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.bottom += m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.bottom = Screw.top + StepMoment * Screw.Height();

                ClipList.DrawRect(Screw);
            }
            else
            {
                Screw.right = Screw.left + Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);

                Screw.left = Screw.right;
                Screw.right = Screw.left + m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.bottom += m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.bottom = Screw.top + Screw.Height();

                ClipList.DrawRect(Screw);
            }

            // bottom
            Screw = m_ReplacerDestRect;
            Screw.left  = m_ReplacerDestRect.Width() / 2 - Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
            Screw.right  = m_ReplacerDestRect.Width() / 2 + Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
            Screw.top = m_ReplacerDestRect.Height() / 2;
            if(Step < 3)
            {
                Screw.bottom = Screw.top + Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
            }
            else if(Step == 3)
            {
                Screw.bottom = Screw.top + Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
                
                Screw.top = Screw.bottom;
                Screw.bottom = Screw.top + m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.left -= m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.left = Screw.right - StepMoment * Screw.Width();

                ClipList.DrawRect(Screw);
            }
            else
            {
                Screw.bottom = Screw.top + Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);

                Screw.top = Screw.bottom;
                Screw.bottom = Screw.top + m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.left -= m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.left = Screw.right - Screw.Width();
                ClipList.DrawRect(Screw);
            }

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_COUNTERCLOCKWISE:
        {
            float Sche = Schedule * SCREW_STEP + 1;
            int Moment =  (int)(Sche);
            int Step   =  (int)(Sche * 4) % 4;
            float StepMoment  =  (Sche * 4) - (int)(Sche * 4);
            
            S3D2DClipList ClipList;
            ClipList.Clear();

            RECTF Screw;

            // Left
            Screw = m_ReplacerDestRect;
            Screw.right = m_ReplacerDestRect.Width() / 2;
            Screw.top  = m_ReplacerDestRect.Height() / 2 - Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
            Screw.bottom  = m_ReplacerDestRect.Height() / 2 + Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
            if(Step == 0)
            {
                Screw.left = Screw.right - Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
                
                Screw.right = Screw.left;
                Screw.left = Screw.right - m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.bottom += m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.bottom = Screw.top + StepMoment * Screw.Height();

                ClipList.DrawRect(Screw);
            }
            else
            {
                Screw.left = Screw.right - Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);

                Screw.right = Screw.left;
                Screw.left = Screw.right - m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.bottom += m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.bottom = Screw.top + Screw.Height();

                ClipList.DrawRect(Screw);
            }

            // bottom
            Screw = m_ReplacerDestRect;
            Screw.left  = m_ReplacerDestRect.Width() / 2 - Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
            Screw.right  = m_ReplacerDestRect.Width() / 2 + Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
            Screw.top = m_ReplacerDestRect.Height() / 2;
            if(Step < 1)
            {
                Screw.bottom = Screw.top + Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
            }
            else if(Step == 1)
            {
                Screw.bottom = Screw.top + Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
                
                Screw.top = Screw.bottom;
                Screw.bottom = Screw.top + m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.right += m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.right = Screw.left + StepMoment * Screw.Width();

                ClipList.DrawRect(Screw);
            }
            else
            {
                Screw.bottom = Screw.top + Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);

                Screw.top = Screw.bottom;
                Screw.bottom = Screw.top + m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.right += m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.right = Screw.left + Screw.Width();

                ClipList.DrawRect(Screw);
            }

            // right
            Screw = m_ReplacerDestRect;
            Screw.left = m_ReplacerDestRect.Width() / 2;
            Screw.top  = m_ReplacerDestRect.Height() / 2 - Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
            Screw.bottom  = m_ReplacerDestRect.Height() / 2 + Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
            if(Step < 2)
            {
                Screw.right = Screw.left + Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
            }
            else if(Step == 2)
            {
                Screw.right = Screw.left + Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
                
                Screw.left = Screw.right;
                Screw.right = Screw.left + m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.top -= m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.top = Screw.bottom - StepMoment * Screw.Height();

                ClipList.DrawRect(Screw);
            }
            else
            {
                Screw.right = Screw.left + Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);

                Screw.left = Screw.right;
                Screw.right = Screw.left + m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.top -= m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.top = Screw.bottom - Screw.Height();

                ClipList.DrawRect(Screw);
            }

            // top
            Screw = m_ReplacerDestRect;
            Screw.left  = m_ReplacerDestRect.Width() / 2 - Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
            Screw.right  = m_ReplacerDestRect.Width() / 2 + Moment * m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
            Screw.bottom = m_ReplacerDestRect.Height() / 2;
            if(Step < 3)
            {
                Screw.top = Screw.bottom - Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
            }
            else if(Step == 3)
            {
                Screw.top = Screw.bottom - Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);
                
                Screw.bottom = Screw.top;
                Screw.top = Screw.bottom - m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.left -= m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.left = Screw.right - StepMoment * Screw.Width();

                ClipList.DrawRect(Screw);
            }
            else
            {
                Screw.top = Screw.bottom - Moment * m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                ClipList.DrawRect(Screw);

                Screw.bottom = Screw.top;
                Screw.top = Screw.bottom - m_ReplacerDestRect.Height() / (2 * SCREW_STEP);
                Screw.left -= m_ReplacerDestRect.Width() / (2 * SCREW_STEP);
                Screw.left = Screw.right - Screw.Width();

                ClipList.DrawRect(Screw);
            }
            
            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;
    default:
        break;
    }

    if(hr == S_OK)
    {
        RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);
    }

    return hr;
}


HRESULT S3TransitionProvider::ProcessWaveIn(float Schedule)
{
    HRESULT hr = E_FAIL;

    S3S_CONTENT_EFFECT_DIRECTION Direction = m_Setting.EffectDirection;

    S3D2DClipList ClipList;

    switch(Direction)
    {
    case S3S_EFFECT_DIRECTION_UPPER_RIGHT:
        {
            float Step = max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) / WAVE_STEP;
            float Goto = (m_ReplacerDestRect.Height() +  m_ReplacerDestRect.Width()) * Schedule;

            RECTF Wave  = m_ReplacerDestRect;
            Wave.left  -= (m_ReplacerDestRect.Height() +  m_ReplacerDestRect.Width());
            Wave.right -= m_ReplacerDestRect.Width();
            Wave.left  += Goto;
            Wave.right += Goto;
            Wave.left = max(0,Wave.left); 
            
            S3D2DClipList ClipList;
            ClipList.Clear();

            int Lines = (int)(m_ReplacerDestRect.Height() / Step) + 1;
            for(int i = 0; i < Lines; i++)
            {
                Wave.top = Wave.bottom - Step;
                
                ClipList.DrawRect(Wave);

                Wave.bottom = Wave.top;
                //Wave.left  -= Step;
                Wave.right -= Step;
            }

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER_RIGHT:
        {
            float Step = max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) / WAVE_STEP;
            float Goto = (m_ReplacerDestRect.Height() +  m_ReplacerDestRect.Width()) * Schedule;

            RECTF Wave = m_ReplacerDestRect;
            Wave.left  -= (m_ReplacerDestRect.Height() +  m_ReplacerDestRect.Width());
            Wave.right -= m_ReplacerDestRect.Width();
            Wave.left += Goto;
            Wave.right += Goto;
            Wave.left = max(0,Wave.left); 
       
            S3D2DClipList ClipList;
            ClipList.Clear();

            int Lines = (int)(m_ReplacerDestRect.Height() / Step) + 1;
            for(int i = 0; i < Lines; i++)
            {
                Wave.bottom = Wave.top + Step;
                
                ClipList.DrawRect(Wave);

                Wave.top = Wave.bottom;
                //Wave.left  -= Step;
                Wave.right -= Step;
            }

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER_LEFT:
        {
            float Step = max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) / WAVE_STEP;
            float Goto = (m_ReplacerDestRect.Height() +  m_ReplacerDestRect.Width()) * Schedule;

            RECTF Wave = m_ReplacerDestRect;
            Wave.left = Wave.right;
            Wave.right = Wave.left + (m_ReplacerDestRect.Height() +  m_ReplacerDestRect.Width());
            Wave.left -= Goto;
            Wave.right -= Goto;
            Wave.right = min(m_ReplacerDestRect.right, Wave.right);
            
            S3D2DClipList ClipList;
            ClipList.Clear();

            int Lines = (int)(m_ReplacerDestRect.Height() / Step) + 1;
            for(int i = 0; i < Lines; i++)
            {
                Wave.bottom = Wave.top + Step;
                
                ClipList.DrawRect(Wave);

                Wave.top = Wave.bottom;
                Wave.left  += Step;
                //Wave.right += Step;
            }

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_UPPER_LEFT:
        {
            float Step = max(m_ReplacerDestRect.Width(), m_ReplacerDestRect.Height()) / WAVE_STEP;
            float Goto = (m_ReplacerDestRect.Height() +  m_ReplacerDestRect.Width()) * Schedule;

            RECTF Wave = m_ReplacerDestRect;
            Wave.left = Wave.right;
            Wave.right = Wave.left + (m_ReplacerDestRect.Height() +  m_ReplacerDestRect.Width());
            Wave.left -= Goto;
            Wave.right -= Goto;
            Wave.right = min(m_ReplacerDestRect.right, Wave.right);


            S3D2DClipList ClipList;
            ClipList.Clear();

            int Lines = (int)(m_ReplacerDestRect.Height() / Step) + 1;
            for(int i = 0; i < Lines; i++)
            {
                Wave.top = Wave.bottom - Step;
                
                ClipList.DrawRect(Wave);

                Wave.bottom = Wave.top;
                Wave.left  += Step;
                //Wave.right += Step;
            }

            m_Graphics.SetClip(ClipList);

            hr = S_OK;
        }
        break;
    default:
        break;
    }

    if(hr == S_OK)
    {
        RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);
    }

    return hr;
}


HRESULT S3TransitionProvider::ProcessTurnOverOut(float Schedule)
{
    HRESULT hr = E_FAIL;

    S3S_CONTENT_EFFECT_DIRECTION Direction = m_Setting.EffectDirection;

    S3D2DClipList ClipList;


    RECTF TurnOverDestRect = m_ReplacerDestRect;
    RECTF TurnOverSrcRect = m_ReplacerSrcRect;
        
    switch(Direction)
    {
    case S3S_EFFECT_DIRECTION_UPPER:
        {
            S3D2DClipList ClipList;
            RECTF ClipRect;
            float TurnDis = m_RTRect.Height() * Schedule;

            // Draw Replacer
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.bottom = ClipRect.bottom - 2* TurnDis;
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);
            RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);

            //Draw TurnOver Legacy
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.bottom = ClipRect.bottom - TurnDis;
            ClipRect.top  = ClipRect.bottom - TurnDis;
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);

            TurnOverDestRect.top     = TurnOverDestRect.bottom - 2*TurnDis;
            TurnOverDestRect.bottom  = TurnOverDestRect.top + m_RTRect.Height();
            float tempbottom         = TurnOverSrcRect.top;
            TurnOverSrcRect.top      = TurnOverSrcRect.bottom;
            TurnOverSrcRect.bottom   = tempbottom;

            RenderContent(TurnOverSrcRect, TurnOverDestRect, 0xFF);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_RIGHT:
        {
            S3D2DClipList ClipList;
            RECTF ClipRect;
            float TurnDis = m_RTRect.Width() * Schedule;

            // Draw Replacer
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.left = ClipRect.left + 2* TurnDis;
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);
            RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);

            //Draw TurnOver Legacy
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.left = ClipRect.left + TurnDis;
            ClipRect.right  = ClipRect.left + TurnDis;
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);

            TurnOverDestRect.right = TurnOverDestRect.left + 2*TurnDis;
            TurnOverDestRect.left  = TurnOverDestRect.right - m_RTRect.Width();
            float tempRight        = TurnOverSrcRect.left;
            TurnOverSrcRect.left   = TurnOverSrcRect.right;
            TurnOverSrcRect.right  = tempRight;

            RenderContent(TurnOverSrcRect, TurnOverDestRect, 0xFF);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER:
        {
            S3D2DClipList ClipList;
            RECTF ClipRect;
            float TurnDis = m_RTRect.Height() * Schedule;

            // Draw Replacer
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.top = ClipRect.top + 2*TurnDis;
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);
            RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);

            //Draw TurnOver Legacy
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.top = ClipRect.top + TurnDis;
            ClipRect.bottom  = ClipRect.top + TurnDis;
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);

            TurnOverDestRect.bottom = TurnOverDestRect.top + 2*TurnDis;
            TurnOverDestRect.top  = TurnOverDestRect.bottom - m_RTRect.Height();
            float tempbottom         = TurnOverSrcRect.top;
            TurnOverSrcRect.top      = TurnOverSrcRect.bottom;
            TurnOverSrcRect.bottom   = tempbottom;

            RenderContent(TurnOverSrcRect, TurnOverDestRect, 0xFF);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LEFT:
        {
            S3D2DClipList ClipList;
            RECTF ClipRect;
            float TurnDis = m_RTRect.Width() * Schedule;

            // Draw Replacer
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.right = ClipRect.right - 2 * TurnDis;
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);
            RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);

            //Draw TurnOver Legacy
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.right = ClipRect.right - TurnDis;
            ClipRect.left  = ClipRect.right - TurnDis;
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);

            TurnOverDestRect.left  = TurnOverDestRect.right - 2*TurnDis;
            TurnOverDestRect.right = TurnOverDestRect.left + m_RTRect.Width();
            float tempRight        = TurnOverSrcRect.left;
            TurnOverSrcRect.left   = TurnOverSrcRect.right;
            TurnOverSrcRect.right  = tempRight;

            RenderContent(TurnOverSrcRect, TurnOverDestRect, 0xFF);

            hr = S_OK;
        }
        break;
    default:
        break;
    }

    return hr;
}






HRESULT S3TransitionProvider::ProcessRollOut(float Schedule)
{
    HRESULT hr = E_FAIL;

    S3S_CONTENT_EFFECT_DIRECTION Direction = m_Setting.EffectDirection;

    S3D2DClipList ClipList;


    RECTF TurnOverDestRect = m_ReplacerDestRect;
    RECTF TurnOverSrcRect = m_ReplacerSrcRect;
        
    switch(Direction)
    {
    case S3S_EFFECT_DIRECTION_UPPER:
        {
            S3D2DClipList ClipList;
            RECTF ClipRect;
            float TurnDis = m_RTRect.Height() * Schedule;

            // Draw Replacer
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.bottom = ClipRect.bottom - TurnDis;
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);
            RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);

            //Draw TurnOver Legacy
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.bottom = ClipRect.bottom - TurnDis;
            ClipRect.top  = ClipRect.bottom - min(TurnDis, ROLL_SCALE*m_RTRect.Height());
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);

            TurnOverDestRect.top     = TurnOverDestRect.bottom - 2*TurnDis;
            TurnOverDestRect.bottom  = TurnOverDestRect.top + m_RTRect.Height();
            float tempbottom         = TurnOverSrcRect.top;
            TurnOverSrcRect.top      = TurnOverSrcRect.bottom;
            TurnOverSrcRect.bottom   = tempbottom;

            RenderContent(TurnOverSrcRect, TurnOverDestRect, 0xFF);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_RIGHT:
        {
            S3D2DClipList ClipList;
            RECTF ClipRect;
            float TurnDis = m_RTRect.Width() * Schedule;

            // Draw Replacer
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.left = ClipRect.left + TurnDis;
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);
            RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);

            //Draw TurnOver Legacy
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.left = ClipRect.left + TurnDis;
            ClipRect.right  = ClipRect.left + min(TurnDis, ROLL_SCALE*m_RTRect.Width());
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);

            TurnOverDestRect.right = TurnOverDestRect.left + 2*TurnDis;
            TurnOverDestRect.left  = TurnOverDestRect.right - m_RTRect.Width();
            float tempRight        = TurnOverSrcRect.left;
            TurnOverSrcRect.left   = TurnOverSrcRect.right;
            TurnOverSrcRect.right  = tempRight;

            RenderContent(TurnOverSrcRect, TurnOverDestRect, 0xFF);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LOWER:
        {
            S3D2DClipList ClipList;
            RECTF ClipRect;
            float TurnDis = m_RTRect.Height() * Schedule;

            // Draw Replacer
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.top = ClipRect.top + TurnDis;
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);
            RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);

            //Draw TurnOver Legacy
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.top = ClipRect.top + TurnDis;
            ClipRect.bottom  = ClipRect.top + min(TurnDis, ROLL_SCALE*m_RTRect.Height());
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);

            TurnOverDestRect.bottom = TurnOverDestRect.top + 2*TurnDis;
            TurnOverDestRect.top  = TurnOverDestRect.bottom - m_RTRect.Height();
            float tempbottom         = TurnOverSrcRect.top;
            TurnOverSrcRect.top      = TurnOverSrcRect.bottom;
            TurnOverSrcRect.bottom   = tempbottom;

            RenderContent(TurnOverSrcRect, TurnOverDestRect, 0xFF);

            hr = S_OK;
        }
        break;
    case S3S_EFFECT_DIRECTION_LEFT:
        {
            S3D2DClipList ClipList;
            RECTF ClipRect;
            float TurnDis = m_RTRect.Width() * Schedule;

            // Draw Replacer
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.right = ClipRect.right - TurnDis;
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);
            RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, 0xFF);

            //Draw TurnOver Legacy
            ClipList.Clear();
            ClipRect = m_RTRect;
            ClipRect.right = ClipRect.right - TurnDis;
            ClipRect.left  = ClipRect.right - min(TurnDis, ROLL_SCALE*m_RTRect.Width());
            ClipList.DrawRect(ClipRect);
            m_Graphics.SetClip(ClipList);

            TurnOverDestRect.left  = TurnOverDestRect.right - 2*TurnDis;
            TurnOverDestRect.right = TurnOverDestRect.left + m_RTRect.Width();
            float tempRight        = TurnOverSrcRect.left;
            TurnOverSrcRect.left   = TurnOverSrcRect.right;
            TurnOverSrcRect.right  = tempRight;

            RenderContent(TurnOverSrcRect, TurnOverDestRect, 0xFF);

            hr = S_OK;
        }
        break;
    default:
        break;
    }

    return hr;
}


HRESULT S3TransitionProvider::ProcessFadeIn(float Schedule)
{
    HRESULT hr = S_OK;

    RenderContent(m_ReplacerSrcRect, m_ReplacerDestRect, (DWORD)(Schedule * 255));

    return hr;
}



HRESULT S3TransitionProvider::ProcessBlockIn(float Schedule)
{
    // the block is drawn increasemently, this is for performance consideration, but I dont think this is safe.
    HRESULT hr = S_OK;

    DWORD EndBlock = (DWORD)(Schedule * m_BlockToalCnt);
    DWORD H, V;
    RECTF Src;
    float left, top, right, bottom;
    for(DWORD i = 0; i < EndBlock; i++)
    {
        H = m_pBlockSequence[i] / m_BlockHCnt;
        V = m_pBlockSequence[i] % m_BlockHCnt;

        left = (m_ReplacerDestRect.left + V * m_BlockLength);
        top = (m_ReplacerDestRect.top + H * m_BlockLength);
        right = (m_ReplacerDestRect.left + (V + 1) * m_BlockLength);
        bottom = (m_ReplacerDestRect.top + (H + 1) * m_BlockLength);

        Src.left = m_ReplacerSrcRect.left + m_ReplacerSrcRect.Width()*(left - m_ReplacerDestRect.left)/m_ReplacerDestRect.Width();
        Src.right = m_ReplacerSrcRect.left + m_ReplacerSrcRect.Width()*(right - m_ReplacerDestRect.left)/m_ReplacerDestRect.Width();
        Src.top = m_ReplacerSrcRect.top + m_ReplacerSrcRect.Height()*(top - m_ReplacerDestRect.top)/m_ReplacerDestRect.Height();
        Src.bottom = m_ReplacerSrcRect.top + m_ReplacerSrcRect.Height()*(bottom - m_ReplacerDestRect.top)/m_ReplacerDestRect.Height();
        
        RenderContent(Src, RECTF(left, top, right, bottom), 0xFF);
    }

    return hr;
}


void  S3TransitionProvider::RenderContent(RECTF SrcRect, RECTF DestRect, DWORD Alpha,  D3DTEXTUREFILTERTYPE FilterType)
{
    list<RenderRect>::iterator start, end, it;

    start = m_RenderInfo->begin();
    end = m_RenderInfo->end();

    it = start;
    do{
        // render all renderable objects
        RenderRect IntersectRect = (*it);

        if(SrcRect.top > SrcRect.bottom)
        {
            SWAP(SrcRect.top , SrcRect.bottom);
        
            SWAP(IntersectRect.TextureCoordinate[0], IntersectRect.TextureCoordinate[1]);
            SWAP(IntersectRect.TextureCoordinate[2], IntersectRect.TextureCoordinate[3]);
        }

        if(SrcRect.left > SrcRect.right)
        {
            SWAP(SrcRect.left , SrcRect.right);
        
            SWAP(IntersectRect.TextureCoordinate[0], IntersectRect.TextureCoordinate[2]);
            SWAP(IntersectRect.TextureCoordinate[1], IntersectRect.TextureCoordinate[3]);
        }

        IntersectRect.Clip(SrcRect);

        if(IntersectRect.Position.Width() > 0 && IntersectRect.Position.Height()>0)
        {
            RECTF IntersectDest;


            RECTF NewPosAlpha;

            NewPosAlpha.left = (IntersectRect.Position.left - SrcRect.left) /(SrcRect.right - SrcRect.left);
            NewPosAlpha.right = (IntersectRect.Position.right - SrcRect.left) /(SrcRect.right - SrcRect.left);

            NewPosAlpha.top = (IntersectRect.Position.top - SrcRect.top) /(SrcRect.bottom - SrcRect.top);
            NewPosAlpha.bottom = (IntersectRect.Position.bottom - SrcRect.top) /(SrcRect.bottom - SrcRect.top);

            IntersectDest.left = DestRect.left + NewPosAlpha.left * DestRect.Width();
            IntersectDest.right = DestRect.left + NewPosAlpha.right * DestRect.Width();
            IntersectDest.top = DestRect.top + NewPosAlpha.top * DestRect.Height();
            IntersectDest.bottom = DestRect.top + NewPosAlpha.bottom * DestRect.Height();


            m_Graphics.DrawImage(it->pTexture, D3DTADDRESS_WRAP, IntersectRect.TextureCoordinate, IntersectDest, Alpha, FilterType);
        }
        it++;
    }while(it != end);
}
