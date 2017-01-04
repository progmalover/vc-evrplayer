#include "stdafx.h"
#include "S3SignagePipe.h"
#include "S3RenderScheduler.h"
#include "S3RenderMixer.h"
#include "S3Signage.h"


S3SignagePipe::S3SignagePipe(void)
{
    m_hPipe = INVALID_HANDLE_VALUE;
    m_bStarted = FALSE;
    m_pScheduler = NULL;
    m_pMixer = NULL;
    m_bConnected = FALSE;
    m_bDeviceHang = FALSE;
}


S3SignagePipe::~S3SignagePipe(void)
{
    StopPipe();
}

HRESULT S3SignagePipe::Initalize(S3RenderScheduler *pScheduler, S3RenderMixer *pMixer)
{
    m_pScheduler = pScheduler;
    m_pMixer = pMixer;
    return S_OK;
}

HRESULT S3SignagePipe::StartPipe()
{
    m_bStarted = TRUE;
    m_bConnected = FALSE;
    return S_OK;
}

HRESULT S3SignagePipe::StopPipe()
{
    m_bStarted = TRUE;
    m_bConnected = FALSE;
    Stop();
    return S_OK;
}


HRESULT S3SignagePipe::Start()
{
    CString PipeName = ((CS3SignageApp*)AfxGetApp())->GetLanchSettings().PipeName;
    m_hPipe = CreateNamedPipe(PipeName.GetString(), PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED, 
    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1, 0, 0, 1000, NULL);
    if (m_hPipe == INVALID_HANDLE_VALUE)
    {
        return E_UNEXPECTED;
    }

    memset(&m_AsycStructure, 0, sizeof(m_AsycStructure));

    if(ConnectNamedPipe(m_hPipe, &m_AsycStructure) == S_FALSE)
    {
        Stop();
        return E_UNEXPECTED;
    }
#ifdef PLAYER_DUMMY
    LOG(g_pLogger, INFO) << _T("Open pipe \"") << ((CS3SignageApp*)AfxGetApp())->GetLanchSettings().PipeName.GetString() << _T("\"");
#endif
    
    return S_OK;
}


HRESULT S3SignagePipe::Stop()
{
    if(m_hPipe == INVALID_HANDLE_VALUE) return S_OK;

    CloseHandle(m_hPipe);
    m_hPipe = INVALID_HANDLE_VALUE;

    return S_OK;
}

HRESULT S3SignagePipe::WriteCommand(CString PipeCommand)
{
    if(m_hPipe == INVALID_HANDLE_VALUE) return E_UNEXPECTED;

    if(PipeCommand.GetLength()!= 0)
    {
        DWORD WriteNum; 
        if(WriteFile(m_hPipe, PipeCommand, PipeCommand.GetLength() * sizeof(TCHAR) + sizeof(TCHAR), &WriteNum, NULL) == FALSE)
        {
            Stop();
            return E_UNEXPECTED;
        }
    }
    return S_OK;
}


HRESULT S3SignagePipe::ProcessPipe()
{
    if(!m_bStarted) return S_OK;

    if(m_hPipe == INVALID_HANDLE_VALUE && m_bStarted)
    {
        Start();
    }

    DWORD ReadNumber = 0;
    DWORD ErrorCode = 0;
    if(GetOverlappedResult(m_hPipe, &m_AsycStructure, &ReadNumber, FALSE) == FALSE)
    {
        ErrorCode = GetLastError();
        if(ErrorCode != ERROR_IO_PENDING && ErrorCode != ERROR_IO_INCOMPLETE)
        {
            Stop();
            return E_UNEXPECTED;
        }

        return S_OK;
    }

    m_bConnected = TRUE;

    if(ReadNumber > 0)
    {
        //m_ReadBuffer[ReadNumber / sizeof(TCHAR)] = '\0'; 

        if(FAILED(ProcessCommand(m_ReadBuffer)))
        {
           WriteCommand(_T("0013"));
		}
	}

    if (ReadFile(m_hPipe, m_ReadBuffer, sizeof(m_ReadBuffer), &m_ReadNumber, &m_AsycStructure) == FALSE)
    {
        if(GetLastError() != ERROR_IO_PENDING)
        {
            Stop();
            return E_UNEXPECTED;
        }
    }

    return S_OK;
}

HRESULT S3SignagePipe::ProcessCommand(CString CommandString)
{
    CArray<CString, CString&> CommandStringArray;

    int StringBegin = 0;
    int StringEnd = 0;
    TCHAR SeperateChar = _T(' ');

#ifdef PLAYER_DUMMY
    LOG(g_pLogger, INFO) << _T("Pipe received command: ") << CommandString.GetString();
#endif

    //DbgMsg(_T("Received Pipe Command: %s"), CommandString);


    while(StringEnd < CommandString.GetLength())
    {
        if(CommandString[StringEnd] == SeperateChar && StringBegin != StringEnd)
        {
            CommandStringArray.Add( CommandString.Mid(StringBegin, StringEnd - StringBegin));
            StringBegin = StringEnd;

            if(SeperateChar == _T('\''))
            {
                if(StringEnd + 1 < CommandString.GetLength() && CommandString[StringEnd + 1] != _T(' '))
                {
                    return E_INVALIDARG;
                }
                StringBegin ++;
                StringEnd ++;
            }


            SeperateChar = _T(' ');
        }

        if(CommandString[StringBegin] == SeperateChar)
        {
            StringBegin ++;
        }
        StringEnd ++;

        if(StringEnd < CommandString.GetLength() && CommandString[StringEnd] == _T('\''))
        {
            SeperateChar = _T('\'');
        }
    }

    StringEnd = min(StringEnd, CommandString.GetLength());
    StringBegin = min(StringBegin, CommandString.GetLength());

    if(SeperateChar == _T( ' ') && StringBegin != StringEnd)
    {
         CommandStringArray.Add( CommandString.Mid(StringBegin, StringEnd - StringBegin));
    }

    if(SeperateChar == _T( '\'') && StringBegin != StringEnd)
    {
         return E_INVALIDARG;
    }


    if(CommandStringArray.GetSize() == 0) return E_INVALIDARG;

    else if(CommandStringArray[0].CompareNoCase(_T("Load")) == 0)
    {
        if(CommandStringArray.GetSize() < 2) return E_INVALIDARG;

        HRESULT hr;
        hr = m_pScheduler->Stop();
        if(FAILED(hr)) return hr;

        hr = m_pScheduler->Terminate();
        if(FAILED(hr)) return hr;

        if(CommandStringArray.GetSize() == 3)
        {
            hr = m_pScheduler->LoadPlayList(CommandStringArray[1], CommandStringArray[2]);
        }else
        {
            hr = m_pScheduler->LoadPlayList(CommandStringArray[1]);
        }



        if(FAILED(hr)) return hr;
#ifdef PLAYER_DUMMY
        ((CS3SignageApp*)AfxGetApp())->m_ClientChecker.CheckSchedule(CommandStringArray[1].GetString());
        LOG(g_pLogger, INFO) << _T("Load play list: ") << CommandStringArray[1].GetString();
#else
#endif
        hr = m_pScheduler->Play();

        if(FAILED(hr)) return hr;
		WriteCommand(_T("0010"));
    }
    else if(CommandStringArray[0].CompareNoCase(_T("SnapShot")) == 0)
    {
        int SnapSize = 500;
        if(CommandStringArray.GetSize() < 2) return E_INVALIDARG;
        if(CommandStringArray.GetSize() == 3)
        {
            SnapSize = _tstoi(CommandStringArray[2]);
        }
#ifdef PLAYER_DUMMY
#else
        m_pMixer->SaveSnapShot(CommandStringArray[1], SnapSize, m_pScheduler->GetDisplayRect());
#endif

		WriteCommand(_T("0010"));
    }
    else if(CommandStringArray[0].CompareNoCase(_T("Quit")) == 0)
    {
#ifdef PLAYER_DUMMY
        LOG(g_pLogger, INFO) << _T("Quit");
#endif
        PostQuitMessage(0);
        WriteCommand(_T("0013"));
    }
    else if(CommandStringArray[0].CompareNoCase(_T("Add")) == 0)
    {
        if(CommandStringArray.GetSize() < 3) return E_INVALIDARG;

        HRESULT hr;
        hr = m_pScheduler->Add(CommandStringArray[1], CommandStringArray[2]);
        if(FAILED(hr)) return hr;

		 WriteCommand(_T("0010"));
    }
    else if(CommandStringArray[0].CompareNoCase(_T("Delete")) == 0)
    {
        if(CommandStringArray.GetSize() < 2) return E_INVALIDARG;
        
        HRESULT hr;
        hr = m_pScheduler->Delete(CommandStringArray[1]);
        if(FAILED(hr)) return hr;

		 WriteCommand(_T("0010"));
    }
    else if(CommandStringArray[0].CompareNoCase(_T("SetVolume")) == 0)
    {
        if(CommandStringArray.GetSize() < 2) return E_INVALIDARG;
        
        HRESULT hr;
        hr = m_pScheduler->SetVolume(_tstoi(CommandStringArray[1]));
        if(FAILED(hr)) return hr;

		 WriteCommand(_T("0010"));
    }
    else if(CommandStringArray[0].CompareNoCase(_T("Show")) == 0)
    {
        if(CommandStringArray.GetSize() < 2) return E_INVALIDARG;

        HRESULT hr;
        hr = m_pScheduler->Show(CommandStringArray[1]);
        if(FAILED(hr)) return hr;

		 WriteCommand(_T("0010"));
    }
    else if(CommandStringArray[0].CompareNoCase(_T("Hide")) == 0)
    {
        if(CommandStringArray.GetSize() < 2) return E_INVALIDARG;

        HRESULT hr;
        hr = m_pScheduler->Hide(CommandStringArray[1]);
        if(FAILED(hr)) return hr;

		 WriteCommand(_T("0010"));
    }
    else if(CommandStringArray[0].CompareNoCase(_T("Stop")) == 0)
    {
        HRESULT hr;
        hr = m_pScheduler->Terminate();
        if(FAILED(hr)) return hr;

		m_pScheduler->initCanvas();
		if(FAILED(hr)) return hr;
		 WriteCommand(_T("0010"));
    }
    else if(CommandStringArray[0].CompareNoCase(_T("QueryCanvas")) == 0)
    {
        SIZE CanvasSize;
        CanvasSize = m_pScheduler->GetCanvasSize();
        CString CanvasSizeCommand;
        CanvasSizeCommand.Format(_T("0020 %d %d"), CanvasSize.cx, CanvasSize.cy);
        WriteCommand(CanvasSizeCommand);

        return S_OK;
    }
	else if(CommandStringArray[0].CompareNoCase(_T("IsLive")) == 0)
	{
		CString StatusString;

        if(m_bDeviceHang)
        {
            StatusString = _T("0017");
        }else
        {
		    StatusString.Format(_T("0015 %d fps"), m_pMixer->GetFPS());
        }

		WriteCommand(StatusString);

		return S_OK;
	}
	else if(CommandStringArray[0].CompareNoCase(_T("IsNotEmpty")) == 0)
	{
		if(m_pMixer->GetObjectCount() < 2)
		{
			WriteCommand(_T("0016"));
		}
		else
		{
			 WriteCommand(_T("0010"));
		}

		return S_OK;
	}
    else
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT S3SignagePipe::SetDeviceHang(BOOL bDeviceHang)
{
    m_bDeviceHang = bDeviceHang;
    return S_OK;
}