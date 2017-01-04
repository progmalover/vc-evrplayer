#pragma once
#include "S3Utility.h"

class S3LicenseControl
{
public:
	enum LicenseCheck{OKLICENSE = 0, NOLICENSE, LESSTHANPERMISSION};
public:
	S3LicenseControl(void);
	~S3LicenseControl(void);

public:
	bool StartMonitor();
	void StopMonitor();

public:
	BYTE m_bNoLicense;
	
protected:
	void Run();
	static DWORD WINAPI LicenseCheckProc(S3LicenseControl* pThis);

private:
	int           m_maxClientNum;
	bool          m_bRunning;
	HANDLE        m_hThread;
    HANDLE        m_hThreadExitEvent;
	DWORD         m_hMainThreadID;
	S3Utility     m_s3Utility;
};

