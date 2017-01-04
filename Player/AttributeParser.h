
#ifndef _S3_ATTRIBUTE_PARSER_H
#define _S3_ATTRIBUTE_PARSER_H
#include "S3SignageSetting.h"


CString GetStringAttrib(AttribList &Attrib, CString KeyName, CString DefaultValue);
int GetIntAttrib(AttribList &Attrib, CString KeyName, int DefaultValue);
float GetFloatAttrib(AttribList &Attrib, CString KeyName, float DefaultValue);
BOOL GetBoolAttrib(AttribList &Attrib, CString KeyName, BOOL DefaultValue);
int GetTimeAttrib(AttribList &Attrib, CString KeyName, DWORD DefaultValue);
DWORD GetColorAttrib(AttribList &Attrib, CString KeyName, DWORD DefaultValue);

int ConvertStr2Duration(CString str);

#endif