// IniFile.cpp: implementation of the CIniFile class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
//#include "gh0st.h"
#include "IniFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#define MAX_LENGTH 256
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIniFile::CIniFile()
{
	char szAppName[MAX_PATH];
	int  len;

	::GetModuleFileName(NULL, szAppName, sizeof(szAppName));    //�õ������ļ���
	len = strlen(szAppName);
	for(int i=len; i>0; i--)                  //������չ���е�'.'
	{
		if(szAppName[i] == '.')
		{
			szAppName[i+1] = '\0';               //ɾ�����'.'�Ժ���ַ� ��  ".exe"
			break;
		}
	}
	strcat(szAppName, "ini");                     //����  ".ini"
	IniFileName = szAppName;                      //��ֵ���ļ��� �鿴 һ����Ա����  IniFileName
}

CIniFile::~CIniFile()
{
	
}

CString CIniFile::GetString(CString AppName,CString KeyName,CString Default)
{
	TCHAR buf[MAX_LENGTH];
	::GetPrivateProfileString(AppName, KeyName, Default, buf, sizeof(buf), IniFileName);  //���һ�����������ļ�����
	return buf;
}

int CIniFile::GetInt(CString AppName,CString KeyName,int Default)
{
	return ::GetPrivateProfileInt(AppName, KeyName, Default, IniFileName);
}

unsigned long CIniFile::GetDWORD(CString AppName,CString KeyName,unsigned long Default)
{
	TCHAR buf[MAX_LENGTH];
	CString temp;
	temp.Format("%u",Default);
	::GetPrivateProfileString(AppName, KeyName, temp, buf, sizeof(buf), IniFileName);
	return atol(buf);
}

BOOL CIniFile::SetString(CString AppName,CString KeyName,CString Data)
{
	return ::WritePrivateProfileString(AppName, KeyName, Data, IniFileName);
}

BOOL CIniFile::SetInt(CString AppName,CString KeyName,int Data)
{
	CString temp;
	temp.Format("%d", Data);
	return ::WritePrivateProfileString(AppName, KeyName, temp, IniFileName);
}

BOOL CIniFile::SetDouble(CString AppName,CString KeyName,double Data)
{
	CString temp;
	temp.Format("%f",Data);
	return ::WritePrivateProfileString(AppName, KeyName, temp, IniFileName);
}

BOOL CIniFile::SetDWORD(CString AppName,CString KeyName,unsigned long Data)
{
	CString temp;
	temp.Format("%u",Data);
	return ::WritePrivateProfileString(AppName, KeyName, temp, IniFileName);
}