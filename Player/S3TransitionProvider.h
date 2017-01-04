#ifndef S3_TRANSITION_PROVIDER_H
#define S3_TRANSITION_PROVIDER_H


#include "S3D2D.h"


enum S3S_CONTENT_EFFECT{
    S3S_EFFECT_NONE     = 0,
    S3S_EFFECT_RANDOM   = 1,
    S3S_EFFECT_WIPE     = 2,
    S3S_EFFECT_EXPAND   = 3,
    S3S_EFFECT_SLIDEIN  = 4,
    S3S_EFFECT_BLIND    = 6,
    S3S_EFFECT_TURNOVER = 8,
    S3S_EFFECT_ROLL     = 9,
    S3S_EFFECT_WHEEL    = 10,
    S3S_EFFECT_CLOCK    = 11,
    S3S_EFFECT_WAVE     = 12,
    S3S_EFFECT_FADE     = 13,
    S3S_EFFECT_BLOCK    = 14,
    S3S_EFFECT_ROUND    = 15,
    S3S_EFFECT_SCREW    = 17,
    S3S_EFFECT_PLUS     = 18,
    S3S_EFFECT_LAST     = 19,
    S3S_EFFECT_ALIGNDW  = 0xFFFFFFFF,
};

enum S3S_CONTENT_EFFECT_DIRECTION{
    S3S_EFFECT_DIRECTION_NONE                   = 0,
    S3S_EFFECT_DIRECTION_UPPER                  = 1,
    S3S_EFFECT_DIRECTION_UPPER_RIGHT            = 2,
    S3S_EFFECT_DIRECTION_RIGHT                  = 3,
    S3S_EFFECT_DIRECTION_LOWER_RIGHT            = 4,
    S3S_EFFECT_DIRECTION_LOWER                  = 5,
    S3S_EFFECT_DIRECTION_LOWER_LEFT             = 6,
    S3S_EFFECT_DIRECTION_LEFT                   = 7,
    S3S_EFFECT_DIRECTION_UPPER_LEFT             = 8,
    S3S_EFFECT_DIRECTION_CLOCKWISE              = 9,
    S3S_EFFECT_DIRECTION_COUNTERCLOCKWISE       = 10,
    S3S_EFFECT_DIRECTION_LEFT_RIGHT             = 11,
    S3S_EFFECT_DIRECTION_RIGHT_LEFT             = 12,
    S3S_EFFECT_DIRECTION_UP_DOWN                = 13,
    S3S_EFFECT_DIRECTION_DOWN_UP                = 14,
    S3S_EFFECT_DIRECTION_RANDOM                 = 255,
    S3S_EFFECT_DIRECTION_ALIGNDW  = 0xFFFFFFFF,
};

typedef struct _S3SIGNAGE_TRANSITION_SETTING{
    S3S_CONTENT_EFFECT  Effect;
    ULONG               EffectDuration;
    S3S_CONTENT_EFFECT_DIRECTION EffectDirection;
}S3SIGNAGE_TRANSITION_SETTING, *PS3SIGNAGE_TRANSITION_SETTING;

class S3TransitionProvider
{
public:

    S3TransitionProvider(void);
    virtual ~S3TransitionProvider(void);

    S3TransitionProvider(LPDIRECT3DDEVICE9 pd3dDevice, SIZE TransitionSize);

    HRESULT SetTransition(S3SIGNAGE_TRANSITION_SETTING Setting);

    LPDIRECT3DTEXTURE9 GetTransitionImage();

    HRESULT ProcessTransition(float Schedule,   list<RenderRect>  &RenderInfo);

    virtual HRESULT         RestoreDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice);///device lost handle
    virtual HRESULT         InvalidateDeviceObjects();                        ///device lost handle


protected:
    //**********************************The transition methods****************************//

    // every process function should call Preprocess() at the beginning to initialize the rect for draw
    void    Preprocess();

    HRESULT ProcessWipeIn(float Schedule);
    HRESULT ProcessExpandIn(float Schedule);
    HRESULT ProcessSlideIn(float Schedule);
    HRESULT ProcessWheelIn(float Schedule);
    HRESULT ProcessClockIn(float Schedule);
    HRESULT ProcessRoundIn(float Schedule);

    HRESULT ProcessPlusIn(float Schedule);
    HRESULT ProcessBlindIn(float Schedule);
    HRESULT ProcessScrewIn(float Schedule);
    HRESULT ProcessWaveIn(float Schedule);
    HRESULT ProcessFadeIn(float Schedule);
    HRESULT ProcessBlockIn(float Schedule);

    HRESULT ProcessTurnOverOut(float Schedule); //??
    HRESULT ProcessRollOut(float Schedule);

    void    RenderContent(RECTF SrcRect, RECTF DestRect, DWORD Alpha = 0xFF,  D3DTEXTUREFILTERTYPE FilterType = D3DTEXF_LINEAR);

    //************************************************************************************//
private:
    LPDIRECT3DDEVICE9               m_pd3dDevice;       // The device that this object blongs to

    S3D2DGraphics                   m_Graphics;

    RECTF                           m_RTRect;

    S3SIGNAGE_TRANSITION_SETTING    m_Setting;
    LPDIRECT3DTEXTURE9              m_pTransitionTexture;
    LPDIRECT3DSURFACE9              m_pTransitionSurface;

    list<RenderRect>               *m_RenderInfo;

    // these rect is used for draw
    RECTF                           m_ReplacerSrcRect;
    RECTF                           m_ReplacerDestRect;

    // this is used to implement the block effect, I save the random NO. of the blocks in the Sequence array.
    // then every frame I will add the block rect to the clip triangle list. 
    // I must design a rule to construct the rect by the NO.
    DWORD                          *m_pBlockSequence;
    DWORD                           m_BlockToalCnt;
    DWORD                           m_BlockHCnt;
    DWORD                           m_BlockVCnt;
    float                           m_BlockLength;
};

#endif