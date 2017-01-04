#include "stdafx.h"
#include "S3SignageSetting.h"
#include "AttributeParser.h"


AttribList::iterator FindAttribNoCase(AttribList &Attrib, CString KeyName)
{
    AttribList::iterator it= Attrib.begin();
    while(it != Attrib.end())
    {
        if(KeyName.CompareNoCase(it->first) == 0) return it;
        it++;
    }
    return it;
}


CString GetStringAttrib(AttribList &Attrib, CString KeyName, CString DefaultValue)
{
    CString ReturnVal;
    AttribList::iterator it= FindAttribNoCase(Attrib, KeyName); 
    if(it == Attrib.end()) 
    {
        ReturnVal = DefaultValue;
    }
    else 
    {
        ReturnVal = it->second;
    }

    return ReturnVal;
}

int GetIntAttrib(AttribList &Attrib, CString KeyName, int DefaultValue)
{
    int ReturnVal;
    AttribList::iterator it= FindAttribNoCase(Attrib, KeyName); 
    if(it == Attrib.end()) 
    {
        ReturnVal = DefaultValue;
    }
    else 
    {
        ReturnVal = _tstoi(it->second);
    }

    return ReturnVal;
}

float GetFloatAttrib(AttribList &Attrib, CString KeyName, float DefaultValue)
{
    float ReturnVal;
    AttribList::iterator it= FindAttribNoCase(Attrib, KeyName); 
    if(it == Attrib.end()) 
    {
        ReturnVal = DefaultValue;
    }
    else 
    {
        ReturnVal = (float)_tstof(it->second);
    }

    return ReturnVal;
}

BOOL GetBoolAttrib(AttribList &Attrib, CString KeyName, BOOL DefaultValue)
{
    BOOL ReturnVal;
    AttribList::iterator it= FindAttribNoCase(Attrib, KeyName); 
    if(it == Attrib.end()) 
    {
        ReturnVal = DefaultValue;
    }
    else 
    {
        CString Value = it->second;

        if(Value.CompareNoCase(_T("TRUE")) == 0)
        {
            ReturnVal = TRUE;
        }else
        {
            ReturnVal = FALSE;
        }
    }

    return ReturnVal;
}


int GetTimeAttrib(AttribList &Attrib, CString KeyName, DWORD DefaultValue)
{
    DWORD ReturnVal;
    AttribList::iterator it= FindAttribNoCase(Attrib, KeyName); 
    if(it == Attrib.end()) 
    {
        ReturnVal = DefaultValue;
    }
    else 
    {
        ReturnVal = ConvertStr2Duration(it->second);
    }

    return ReturnVal;
}

DWORD GetColorAttrib(AttribList &Attrib, CString KeyName, DWORD DefaultValue)
{
    DWORD ReturnVal;
    AttribList::iterator it= FindAttribNoCase(Attrib, KeyName); 
    if(it == Attrib.end()) 
    {
        ReturnVal = DefaultValue;
    }
    else 
    {
        ReturnVal = _tcstoul(it->second, NULL, 16);
    }

    return ReturnVal;
}


int ConvertStr2Duration(CString str)
{
    int Hour = 0, Minute = 0, Second = 0;

    _stscanf_s(str, _T("%d:%d:%d"), &Hour, &Minute, &Second);

    int TotalSecond = (Hour * 60 + Minute) * 60 + Second;

    return TotalSecond;
}