#include "stdafx.h"
#include "S3Signage.h"
#include "S3LicenseControl.h"
#include "Utilities/HaspManager.h"
#include "Utilities/CrashRptHelper.h"

S3LicenseControl::S3LicenseControl(void)
	: m_maxClientNum(-1)
	, m_bRunning(false)
    , m_hThreadExitEvent(NULL)
	, m_bNoLicense(S3LicenseControl::OKLICENSE)
{
}


S3LicenseControl::~S3LicenseControl(void)
{
}

bool S3LicenseControl::StartMonitor()
{
	if (!m_bRunning)
	{
		m_hMainThreadID = ::GetCurrentThreadId();
        m_hThreadExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (NULL == m_hThreadExitEvent)
		{
			DWORD dwError = GetLastError();
			SetLastError(dwError);
			return false;
		}
		m_hThread = AtlCreateThread(LicenseCheckProc, this);
		if (NULL == m_hThread)
		{
			DWORD dwError = GetLastError();
			SetLastError(dwError);
			return false;
		}
	}

	return true;
}

void S3LicenseControl::StopMonitor()
{
	m_bRunning = false;

	DWORD timeOut = 60*1000;
	if (NULL != m_hThread)
	{
        SetEvent(m_hThreadExitEvent);

		if (WAIT_TIMEOUT == WaitForSingleObject(m_hThread, timeOut))
		{
			TerminateThread(m_hThread, 1);
		}

		CloseHandle(m_hThread);
		m_hThread = NULL;

        if (m_hThreadExitEvent)
        {
            CloseHandle(m_hThreadExitEvent);
            m_hThreadExitEvent = NULL;
        }
	}
}

void S3LicenseControl::Run()
{
	m_bRunning = true;
	while(m_bRunning)
	{
		if (!HaspManager::IsHardKeyConnected())
		{
			DbgMsg(_T("No license key found! Exit..."));
			m_bNoLicense = S3LicenseControl::NOLICENSE;
			PostMessage(theApp.GetMainWnd()->m_hWnd, WM_CLOSE, 0,0);
			break;
			
		}

		if(EPlayerType == S3_MAMMPLAYER)
		{
			if (HaspManager::GetClientsLimit(HaspManager::MAMMPLAYER) < m_s3Utility.GetDisplayCount())
			{
				DbgMsg(_T("The max monitor number supported is %d, Current is %d! Exit...."), HaspManager::GetClientsLimit(HaspManager::MAMMPLAYER), m_s3Utility.GetDisplayCount());
				m_bNoLicense = S3LicenseControl::LESSTHANPERMISSION;
				PostMessage(theApp.GetMainWnd()->m_hWnd, WM_CLOSE, 0,0);
				break;
			}
		}
		else
		{
			if (4 < m_s3Utility.GetDisplayCount())
			{
				if (HaspManager::GetClientsLimit(HaspManager::MAMMPLAYER) < m_s3Utility.GetDisplayCount())
				{
					DbgMsg(_T("The max monitor number supported is %d, Current is %d! Exit...."), HaspManager::GetClientsLimit(HaspManager::MAMMPLAYER), m_s3Utility.GetDisplayCount());
					PostMessage(theApp.GetMainWnd()->m_hWnd, WM_CLOSE, 0,0);
					break;
				}
			}
		}
        WaitForSingleObject(m_hThreadExitEvent, 10*1000);
	}
}

DWORD WINAPI S3LicenseControl::LicenseCheckProc(S3LicenseControl* pThis)
{
    CCrashRptThreadHelper helper;
	if (NULL != pThis)
	{
		pThis->Run();
	}

	return 0;
}
