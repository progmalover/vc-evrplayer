#ifndef S3_3D_SOUND_H
#define S3_3D_SOUND_H
#include <tchar.h>
#include <D3D9.h>
#include "S3RenderableObject.h"
#include "S3SignageSetting.h"
#include "S3Signage.h"



class S3SoundObj: public S3RenderableObject
{
public:
    S3SoundObj(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute);
    virtual ~S3SoundObj();

    virtual HRESULT         Start();
    virtual HRESULT         Stop();
    virtual HRESULT         PrepareRender();
    virtual HRESULT         EndRender();
    virtual HRESULT         ProcessMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual HRESULT         Pause();
    virtual HRESULT         Resume();
    virtual HRESULT         LoopContent();

protected:
    CComPtr<IFilterGraph>   m_pGraph;   // filter graph
    CComPtr<IMediaControl>  m_pMc;      // media control
    CComPtr<IMediaSeeking>  m_pMs;      // media seeking
    CComPtr<IBasicAudio>    m_pBa;

    CString                 m_FilePath;
    int                     m_Volume;
    BOOL                    m_bInited;       
};

#endif