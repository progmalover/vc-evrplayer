#ifndef _S3_SINAGE_SETTING_H
#define _S3_SINAGE_SETTING_H

enum S3S_PLAYSETTING{
    S3S_PS_NONE         = 0,
    S3S_PS_REPEAT       = 1,
    S3S_PS_STOP         = 2,
    S3S_PS_STOPBK       = 3,//stop with back ground color
    S3S_PS_NEXT         = 4,//play next content
    S3S_PS_ALIGNDW = 0xFFFFFFFF,
};

typedef std::map<CString, CString> AttribList;

typedef struct S3SIGNAGE_TRANSIION
{
	S3SIGNAGE_TRANSIION()
	{
	
	};
	S3SIGNAGE_TRANSIION(const struct S3SIGNAGE_TRANSIION &_In)
	{
		*this = _In;
	};
	void operator=(const struct S3SIGNAGE_TRANSIION &_In)
	{
		Duration = _In.Duration;
		Name = _In.Name;
		Direction = _In.Direction;
	};

    DWORD       Duration;
	CString     Name;
    CString     Direction;
}S3SIGNAGE_TRANSIION;

typedef struct TRANSFORM_VALUE
{
    DWORD       Time;
    float       XValue;
    float       YValue;
}TRANSFORM_VALUE;

typedef struct TextFileItem
{
    TextFileItem()
    {
        bResourceFile = FALSE;
        ResourceID = -1;
        TextFileScale = 0.0f;
    }
	TextFileItem(const struct TextFileItem &_In)
	{
		*this = _In;
	};
	void operator=(const struct TextFileItem &_In)
	{
		Filename = _In.Filename;
		Content = _In.Content;
		TextFileScale = _In.TextFileScale;
		ResourceID = _In.ResourceID;
		bResourceFile = _In.bResourceFile;
	};
    BOOL        bResourceFile;
    DWORD       ResourceID;
    float       TextFileScale; 
	CString     Filename;
    CString     Content;
}TextFileItem;

typedef vector<TRANSFORM_VALUE> S3SIGNAGE_TRANSFORM;

typedef vector<TextFileItem> S3SIGNAGE_TEXTFILE;

typedef struct S3SIGNAGE_CONTENT
{	
	S3SIGNAGE_CONTENT()
	{

	};
	S3SIGNAGE_CONTENT(const struct S3SIGNAGE_CONTENT &_In)
	{
		*this = _In;
	};

	void operator=(const struct S3SIGNAGE_CONTENT &_In)
	{
		StartTime = _In.StartTime;
		Duration = _In.Duration;
		XPos = _In.XPos;
		YPos = _In.YPos;
		Width = _In.Width;
		Height = _In.Height;
		TypeName = _In.TypeName;
		IDName = _In.IDName;
		Attribute = _In.Attribute;
		BGColor = _In.BGColor;

		TransitionIn = _In.TransitionIn;
		TransitionOut = _In.TransitionOut;

		Rotate = _In.Rotate;
		Scale = _In.Scale;
		Translate = _In.Translate;

		TextFile = _In.TextFile;
	};

    int         StartTime;
    int         Duration;
    int         XPos;
    int         YPos;
    int         Width;
    int         Height;
    CString     TypeName;
    CString     IDName;
    AttribList  Attribute;
    DWORD       BGColor;

    S3SIGNAGE_TRANSIION TransitionIn;
    S3SIGNAGE_TRANSIION TransitionOut;

    S3SIGNAGE_TRANSFORM Rotate;
    S3SIGNAGE_TRANSFORM Scale;
    S3SIGNAGE_TRANSFORM Translate;

    S3SIGNAGE_TEXTFILE  TextFile;

}S3SIGNAGE_CONTENT;






#endif
