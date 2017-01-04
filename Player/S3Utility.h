#pragma once
#include "S3Escape.h"
#include <vector>
#include "Utilities/SysCall.h"

class S3Utility
{
public:
	S3Utility(void);
	~S3Utility(void);

public:
	DWORD GetDisplayCount();
    NTSTATUS CallEscape(PS3ESC_REQUEST_PACKET_EX pRequestPacket);

    BOOL GetCaptureDevice(UINT index, CaptureDevice& device);

private:
    BOOL    m_bInitCaptureDeivces;
    std::vector<CaptureDevice> m_CaptureDevices;

};
