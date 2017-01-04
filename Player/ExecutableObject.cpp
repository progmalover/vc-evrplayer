#include "stdafx.h"
#include "ExecutableObject.h"

int g_LimitExeTexSize = 1920*1080*2;

BOOL CALLBACK FindMyWindow(HWND hWnd, LPARAM wParam)
{
    PFIND_WND_PARAM pFindWindow = (PFIND_WND_PARAM)wParam;
    DWORD processID;
    DWORD threadID = GetWindowThreadProcessId(hWnd, &processID);
    if(processID == pFindWindow->processID)
    {
        if(IsWindowVisible(hWnd))
        {
            *pFindWindow->pHWnd = hWnd;
            return FALSE;
        }
    }
    return TRUE;
}

DWORD ExecutableObject::m_fileID = 0;

ExecutableObject::ExecutableObject(int FrameID, 
        RECT ScreenRect, 
        DWORD ZOrder, 
        BOOL bEnabled, 
        S3SIGNAGE_CONTENTSETTING *pContentSetting)
        :S3RenderableObject(ScreenRect, ZOrder)
{
    m_pTexture = NULL;
    m_pTextureImmediate[0] = NULL;
    m_pTextureImmediate[1] = NULL;
    m_pTempSysTexture = NULL;

    m_AppType = S3_EO_GENERAL;

    m_threadStatus = 0;
    m_appRunning = false;

    m_processStopEvent = NULL;
    m_hBmpFile   = NULL;
    m_pMemBmp    = NULL;

    m_ObjectType = _T("S3_OBJECT_EXE");

    m_Width = 0;
    m_Height = 0;
    m_textureWidth = 0;
    m_textureHeight = 0;

    m_Settings = *pContentSetting;
    CString ErrorTitle;
    CString ErrorMessage;
    ErrorTitle.LoadString(IDS_ERROR_TITLE_EXE);
    
    if(m_Settings.ContentFilePath.IsEmpty())
    {
        ErrorMessage.LoadString(IDS_ERROR_NO_FILE);
        ::MessageBox(NULL, ErrorMessage, ErrorTitle, MB_OK);
    }

#ifdef UNICODE
    swprintf_s(m_bmpFNString, BMP_FILENAME_LENTH, _T("S3E%10d"), m_fileID);
#else
    sprintf_s(m_bmpFNString, BMP_FILENAME_LENTH, _T("S3E%10d"), m_fileID);
#endif

    m_bmpFNString[BMP_FILENAME_LENTH -1] = 0;
    m_fileID++;

    m_inited = false;

    if(m_Settings.TempPosition.bottom > m_Settings.TempPosition.top && m_Settings.TempPosition.right > m_Settings.TempPosition.left)
    {
        m_ShowRect = m_Settings.TempPosition;
    }
    else
    {
        m_ShowRect = ScreenRect;
    }
    m_Width = m_ShowRect.right - m_ShowRect.left;
    m_Height = m_ShowRect.bottom - m_ShowRect.top;

    if(m_Width<=0 || m_Height<=0)
    {
        //error!
    }

    if(m_Width*m_Height*4 > g_LimitExeTexSize)
    {
        float ratio = (float)m_Width/(float)m_Height;
        float tempHeight = sqrt((float)g_LimitExeTexSize / 4.0f / ratio);
        m_textureHeight = (int)floor(tempHeight);
        m_textureWidth =  (int)((float)m_textureHeight * ratio);
    }
    else
    {
        m_textureWidth = m_Width;
        m_textureHeight = m_Height;
    }

    m_ScreenRect.left = (float)m_ShowRect.left;
    m_ScreenRect.right = (float)m_ShowRect.right;
    m_ScreenRect.top = (float)m_ShowRect.top;
    m_ScreenRect.bottom = (float)m_ShowRect.bottom;
}

VOID CALLBACK ExecutableObject::ProcessTerminated(PVOID lpParameter,BOOLEAN TimerOrWaitFired)
{
    ExecutableObject* pExeObj = (ExecutableObject*)lpParameter;
    if(!TimerOrWaitFired)
    {
        pExeObj->Stopped();
    }
}

void ExecutableObject::Stopped()
{
    {
        CAutoLock threadLock(&m_threadMutex);
        if(m_threadStatus == 1)
        {
            m_threadStatus = 2;
        }
    }
    while(m_threadStatus)
    {
        Sleep(10);
    }
    CleanUp();
    if(m_appRunning)
    {
        //terminated by user, not by the scheduler
        //theApp.GetMainWnd()->PostMessage(WM_KEYDOWN, VK_ESCAPE);
        theApp.GetMainWnd()->SetForegroundWindow();
    }
}

HRESULT ExecutableObject::RunAndSetup()
{
    STARTUPINFO si;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory(&m_processInfo, sizeof(m_processInfo));
    si.dwFlags = STARTF_USESHOWWINDOW|STARTF_USEPOSITION|STARTF_USESIZE;
    si.wShowWindow = SW_SHOWNORMAL;
    si.dwX = 0;
    si.dwY = 0;
    si.dwXSize = m_textureWidth;
    si.dwYSize = m_textureHeight;
    TCHAR* pParamStr = NULL;

    //detect app type
    int cmdLen = m_Settings.ContentFilePath.GetLength();
    int nameStart = 0, nameEnd;
    if(m_Settings.ContentFilePath[cmdLen - 1] == '\"')
    {
        nameEnd = cmdLen - 1;
    }
    else
    {
        nameEnd = cmdLen;
    }

    nameStart = m_Settings.ContentFilePath.ReverseFind('\\') + 1;
    CString fileName = m_Settings.ContentFilePath.Mid(nameStart, nameEnd - nameStart);
    if(!fileName.CompareNoCase(_T("PPTVIEW.EXE")))
    {
        m_AppType = S3_EO_PPTVIEWER;
    }

    int strlen = m_Settings.Parameter.GetLength();
    if(strlen> 32767)
    {
        return E_FAIL;
    }
    pParamStr = new TCHAR[32768];
    if(!pParamStr)
    {
        return E_OUTOFMEMORY;
    }
    pParamStr[0] = ' ';
    memcpy(pParamStr + 1, m_Settings.Parameter, strlen*sizeof(TCHAR));
    pParamStr[strlen + 1] = 0;

    if(!CreateProcess(m_Settings.ContentFilePath, pParamStr, 
        NULL, NULL, TRUE, CREATE_UNICODE_ENVIRONMENT|CREATE_SUSPENDED, NULL, NULL, &si, &m_processInfo))
    {
        delete[] pParamStr;
        pParamStr = NULL;
        return E_FAIL;
    }
    delete[] pParamStr;
    pParamStr = NULL;

    m_appRunning = true;
    RegisterWaitForSingleObject(&m_processStopEvent, m_processInfo.hProcess, ProcessTerminated, \
            this, INFINITE, WT_EXECUTEONLYONCE);
    //Mutex
    HANDLE hHookMutex = NULL;
    while(!hHookMutex)
    {
        hHookMutex = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(APP_LAUNCH_CONFIG), S3_EO_MUTEX_NAME);
        if(GetLastError() == ERROR_ALREADY_EXISTS)
        {
            //another thread is using the mutex
            CloseHandle(hHookMutex);
            hHookMutex = NULL;
            Sleep(10);
        }
    }
    PAPP_LAUNCH_CONFIG pLaunchCOnfig = (PAPP_LAUNCH_CONFIG)MapViewOfFile(hHookMutex, FILE_MAP_WRITE, 0, 0, sizeof(APP_LAUNCH_CONFIG));
    pLaunchCOnfig->x = m_ShowRect.left;
    pLaunchCOnfig->y = m_ShowRect.top;
    pLaunchCOnfig->width = m_textureWidth;
    pLaunchCOnfig->height = m_textureHeight;
    pLaunchCOnfig->appType = m_AppType;
    memcpy(pLaunchCOnfig->bitmapFileName, m_bmpFNString, BMP_FILENAME_LENTH*sizeof(WCHAR));
    UnmapViewOfFile(pLaunchCOnfig);

    if(m_AppType == S3_EO_PPTVIEWER)
    {
        m_hBmpFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, m_textureWidth*m_textureHeight*4, m_bmpFNString);
        m_pMemBmp = (BYTE*)MapViewOfFile(m_hBmpFile, FILE_MAP_ALL_ACCESS, 0, 0, m_textureWidth*m_textureHeight*4);
        if(m_pMemBmp)
        {
            memset(m_pMemBmp, 0, m_textureWidth*m_textureHeight*4);
        }
    }
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, S3_EO_EVENT_NAME);
    //Hook it
    LPVOID p=VirtualAllocEx(m_processInfo.hProcess,NULL,theApp.m_HookDllPath.GetLength()*sizeof(WCHAR),MEM_COMMIT,PAGE_READWRITE);
    WriteProcessMemory(m_processInfo.hProcess,p,theApp.m_HookDllPath,theApp.m_HookDllPath.GetLength()*sizeof(WCHAR),NULL);
    FARPROC pfn=GetProcAddress(GetModuleHandle(_T("kernel32.dll")),"LoadLibraryW");
    HANDLE hookedThread = CreateRemoteThread(m_processInfo.hProcess,NULL,0,(LPTHREAD_START_ROUTINE)pfn,p,NULL,0); 

    int waittime = 0;
    while(WaitForSingleObject(hEvent, 0))
    {
        Sleep(100);
        waittime++;
        if(waittime>=20)
        {
            //too long, terminate.
            CloseHandle(hEvent);
            CloseHandle(hHookMutex);
            Terminate();
            return E_FAIL;
        }
    }
    CloseHandle(hEvent);
    CloseHandle(hHookMutex);
    //ResumeThread
    ResumeThread(m_processInfo.hThread);

    return S_OK;
}

void ExecutableObject::CleanUp()
{
	m_Mutex.Lock();
	m_inited = false;
    m_RenderRect.RemoveAll();
	m_Mutex.Unlock();
    if(m_pMemBmp)
    {
        UnmapViewOfFile(m_pMemBmp);
        m_pMemBmp = NULL;
    }
    if(m_hBmpFile)
    {
        CloseHandle(m_hBmpFile);
        m_hBmpFile = NULL;
    }
}

ExecutableObject::~ExecutableObject(void)
{
    DbgMsg("ExecutableObject::~ExecutableObject");
    //terminate the thread first
    if(m_inited)
    {
        Terminate();
    }
    if(m_pTextureImmediate[0])
    {
        m_pTextureImmediate[0]->Release();
        m_pTextureImmediate[0] = NULL;
    }
    if(m_pTextureImmediate[1])
    {
        m_pTextureImmediate[1]->Release();
        m_pTextureImmediate[1] = NULL;
    }
    if(m_pTempSysTexture)
    {
        m_pTempSysTexture->Release();
        m_pTempSysTexture = NULL;
    }
    m_pTexture = NULL;
}

HRESULT ExecutableObject::InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice )
{
    S3RenderableObject::InitDeviceObjects(pd3dDevice);

    HRESULT hr;
    hr = pd3dDevice->CreateTexture(m_textureWidth, m_textureHeight, 1, D3DUSAGE_RENDERTARGET, 
        D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pTextureImmediate[0], NULL);
    if(FAILED(hr) || !m_pTextureImmediate[0])
    {
        return hr;
    }
    hr = pd3dDevice->CreateTexture(m_textureWidth, m_textureHeight, 1, D3DUSAGE_RENDERTARGET, 
         D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pTextureImmediate[1], NULL);
    if(FAILED(hr) || !m_pTextureImmediate[1])
    {
        m_pTextureImmediate[0]->Release();
        m_pTextureImmediate[0] = NULL;
        return hr;
    }

    hr = pd3dDevice->CreateTexture(m_textureWidth, m_textureHeight, 1, D3DUSAGE_DYNAMIC, 
         D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &m_pTempSysTexture, NULL);
    if(FAILED(hr) || !m_pTempSysTexture)
    {
        m_pTextureImmediate[0]->Release();
        m_pTextureImmediate[0] = NULL;
        m_pTextureImmediate[1]->Release();
        m_pTextureImmediate[1] = NULL;
        return hr;
    }

    D3DLOCKED_RECT myRect;
    if(SUCCEEDED(m_pTempSysTexture->LockRect(0, &myRect, NULL, D3DLOCK_DISCARD)))
    {
        int h;
        BYTE* pDst = (BYTE*)myRect.pBits;
        int   pitchDst = myRect.Pitch;
        for(h = 0; h < m_textureHeight; h++)
        {
            memset(pDst, 0, m_textureWidth*4);
            pDst += pitchDst;
        }
        m_pTempSysTexture->UnlockRect(0);
        m_pd3dDevice->UpdateTexture(m_pTempSysTexture, m_pTextureImmediate[0]);
    }

    m_curTex = 0;
    m_pTexture = m_pTextureImmediate[0];

    return S_OK;
}

HRESULT ExecutableObject::Run()
{
	HRESULT hr;
    if(m_inited)
    {
        Terminate();
    }
	hr = RunAndSetup();
    if(FAILED(hr))
    {
        return hr;
    }
    
    m_threadStatus = 1;
    DWORD tid = NULL;
    HANDLE hThread = CreateThread( NULL,
                            NULL,
                            UpdateContentThreadProc_,
                            this,
                            NULL,
                            &tid);
    if( INVALID_HANDLE_VALUE == hThread )
    {
        ::DbgMsg("ExecutableObject::RunAndSetup: failed to create thread");
        m_threadStatus = 0;
        return E_UNEXPECTED;
    }
    CloseHandle(hThread);
	m_Mutex.Lock();
	m_inited = true;
    m_RenderRect.Add(RenderRect(m_ScreenRect, RECTF(0.0f, 0.0f, 1.0f, 1.0f)));
	m_Mutex.Unlock();
    return S_OK;
}

HRESULT ExecutableObject::PrepareRender()
{
	m_Mutex.Lock();
    if(!m_inited)
    {
		m_Mutex.Unlock();
        return E_FAIL;
    }
#if S3S_DBGPRINT_SYNC
    DbgMsg("ExecutableObject::Locked current texture");
#endif
    return S_OK;
}

HRESULT ExecutableObject::EndRender()
{
    if(!m_inited)
    {
        return E_FAIL;
    }
#if S3S_DBGPRINT_SYNC
    DbgMsg("ExecutableObject::Unlock current texture");
#endif
    m_Mutex.Unlock();
    return S_OK;
}

HRESULT ExecutableObject::ProcessMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(!m_inited)
    {
        return E_FAIL;
    }
    if(uMsg < WM_MOUSEFIRST || uMsg > WM_MOUSELAST)
    {
        return E_INVALIDARG;
    }
    //if(uMsg != WM_MOUSEMOVE && uMsg != WM_LBUTTONDOWN && uMsg != WM_LBUTTONUP)
    //{
    //    return S_FALSE;
    //}
    int xPos = GET_X_LPARAM(lParam); 
    int yPos = GET_Y_LPARAM(lParam); 
    if((xPos>=m_ShowRect.left && xPos<m_ShowRect.right) && (yPos>=m_ShowRect.top && yPos<m_ShowRect.bottom))
    {
        xPos -= m_ShowRect.left;
        yPos -= m_ShowRect.top;
        WPARAM lParam2 = (yPos<<16)|(xPos & 0xFFFF);
        //m_pPlayer->SendMessage(uMsg, wParam, lParam2);
    }
    return S_OK;
}

HRESULT ExecutableObject::Terminate()
{
    {
        CAutoLock threadLock(&m_threadMutex);
        if(m_threadStatus == 1)
        {
            m_threadStatus = 2;
        }
    }
    while(m_threadStatus)
    {
        Sleep(10);
    }
    m_appRunning = false;
    UnregisterWait(m_processStopEvent);
    m_processStopEvent = NULL;
    TerminateProcess(m_processInfo.hProcess, 0);
    CloseHandle(m_processInfo.hProcess);
    CloseHandle(m_processInfo.hThread);

    CleanUp();
    return S_OK;
}

HWND ExecutableObject::GetAppWindow()
{
    HWND theWnd = NULL;
    FIND_WND_PARAM findWnd;
    findWnd.processID = m_processInfo.dwProcessId;
    findWnd.threadID = m_processInfo.dwThreadId;
    findWnd.pHWnd = &theWnd;
    EnumWindows(FindMyWindow,(LPARAM)&findWnd);
    return theWnd;
}

void ExecutableObject::UpdateTexture()
{
    //get the window handle
    HWND hWnd = GetAppWindow();
    if(!hWnd || !m_pd3dDevice)
    {
        return;
    }
    //RECT winRect;
    //if(GetWindowRect(hWnd, &winRect))
    //{
    //    if((winRect.right - winRect.left) > m_textureWidth || (winRect.bottom - winRect.top) > m_textureWidth)
    //    {
    //        MoveWindow(hWnd, -m_textureWidth, -m_textureHeight, m_textureWidth, m_textureHeight, FALSE);
    //    }
    //}

    int updatetex;
    if(m_curTex)
    {
        updatetex = 0;
    }
    else
    {
        updatetex = 1;
    }

    BOOL bUpdateSucceeded = FALSE;
    if(m_AppType == S3_EO_PPTVIEWER && m_pMemBmp)
    {
        D3DLOCKED_RECT myRect;
        if(SUCCEEDED(m_pTempSysTexture->LockRect(0, &myRect, NULL, D3DLOCK_DISCARD)))
        {
            int h;
            BYTE* pDst = (BYTE*)myRect.pBits;
            int   pitchDst = myRect.Pitch;
            BYTE* pSrc = m_pMemBmp + (m_textureHeight - 1)*m_textureWidth*4;
            int   pitchSrc = m_textureWidth*4;
            for(h = 0; h < m_textureHeight; h++)
            {
                memcpy(pDst, pSrc, pitchSrc);
                pSrc -= pitchSrc;
                pDst += pitchDst;
            }
            m_pTempSysTexture->UnlockRect(0);
            if(SUCCEEDED(m_pd3dDevice->UpdateTexture(m_pTempSysTexture, m_pTextureImmediate[updatetex])))
            {
                bUpdateSucceeded = TRUE;
            }
        }
    }
    else
    {
        IDirect3DSurface9* ptempSurface = NULL;
        HDC myDC = NULL;
        try{
            if(FAILED(m_pTempSysTexture->GetSurfaceLevel(0, &ptempSurface)))
            {
                throw 1;
            }
            
            if(FAILED(ptempSurface->GetDC(&myDC)))
            {
                throw 2;
            }
            if(!::PrintWindow(hWnd, myDC, 0))
            {
                throw 3;
            }
            if(SUCCEEDED(m_pd3dDevice->UpdateTexture(m_pTempSysTexture, m_pTextureImmediate[updatetex])))
            {
                bUpdateSucceeded = TRUE;
            }
        }
        catch(int)
        {
            if(ptempSurface)
            {
                if(myDC)
                {
                    ptempSurface->ReleaseDC(myDC);
                }
                ptempSurface->Release();
            }
            return;
        }
        ptempSurface->ReleaseDC(myDC);
        ptempSurface->Release();
    }

    if(bUpdateSucceeded)
    {
        CAutoLock switchLock(&m_Mutex);
#if S3S_DBGPRINT_SYNC
        DbgMsg("ExecutableObject::Lock current texture");
#endif
        m_curTex = updatetex;
        m_pTexture = m_pTextureImmediate[m_curTex];
#if S3S_DBGPRINT_SYNC
        DbgMsg("ExecutableObject::Switched texture");
        DbgMsg("ExecutableObject::Unlock current texture");
#endif
    }
}

void ExecutableObject::UpdateTextureThread()
{
    DWORD interframe;
    DWORD maxSleepTime = 10;
    if(m_AppType == S3_EO_PPTVIEWER)
    {
        interframe = 1000/30;//30Hz
    }
    else
    {
        interframe = 1000/30;//30Hz
    }
    while(1)
    {
        if( 0 == m_dwLast )
        {
            m_dwLast = timeGetTime();
            if(m_threadStatus == 2)
            {
                break;
            }
        }
        else
        {
            DWORD delta = timeGetTime() - m_dwLast;
            if( delta < interframe )
            {
                if(m_threadStatus == 2)
                {
                    break;
                }
                Sleep(interframe - delta);
                delta = timeGetTime() - m_dwLast;
            }
            delta = delta/interframe;
            m_dwLast += delta*interframe;
            if(m_threadStatus == 2)
            {
                break;
            }
        }
        UpdateTexture();
    }
    m_threadStatus = 0;
}

/******************************Protected*Routine******************************\
* UpdateContentThreadProc_
*
\**************************************************************************/
DWORD WINAPI ExecutableObject::UpdateContentThreadProc_( LPVOID lpParameter )
{
    ExecutableObject* This = NULL;

    HRESULT hr = S_OK;

    This = (ExecutableObject*)lpParameter;

    {
        CAutoLock threadLock(&This->m_threadMutex);

        if( !This )
        {
            ::DbgMsg("ExecutableObject::UpdateContentThreadProc_: parameter is NULL");
            This->m_threadStatus = 0;
            return 0;
        }
    }
    This->UpdateTextureThread();
    return 0;
}
