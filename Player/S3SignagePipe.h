#pragma once

class S3RenderScheduler;
class S3RenderMixer;

class S3SignagePipe
{
public:
    S3SignagePipe(void);
    ~S3SignagePipe(void);
    HRESULT Initalize(S3RenderScheduler *pScheduler, S3RenderMixer *pMixer);
    HRESULT ProcessPipe();
    HRESULT StartPipe();
    HRESULT StopPipe();
    HRESULT SetDeviceHang(BOOL bDeviceHang);
protected:
    HANDLE              m_hPipe;
    OVERLAPPED          m_AsycStructure;
    TCHAR               m_ReadBuffer[1024];
    DWORD               m_ReadNumber;
    BOOL                m_bStarted;
    S3RenderScheduler  *m_pScheduler;
    S3RenderMixer      *m_pMixer;
    DWORD               m_LastUpateTime;
    BOOL                m_bConnected;
    BOOL                m_bDeviceHang;


    HRESULT ProcessCommand(CString CommandString);
    HRESULT WriteCommand(CString PipeCommand);
    HRESULT Start();
    HRESULT Stop();
};

