// CSettingDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyRemote.h"
#include "CSettingDlg.h"
#include "afxdialogex.h"
#include "IniFile.h"


// CSettingDlg 对话框

IMPLEMENT_DYNAMIC(CSettingDlg, CDialogEx)

CSettingDlg::CSettingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SETTING, pParent)
	, m_nListernProt(_T(""))
	, m_nMax_Connect(_T(""))
	, m_nListernIp(_T(""))
{
	
}

CSettingDlg::~CSettingDlg()
{
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nListernProt);
	DDX_Text(pDX, IDC_EDIT_MAX, m_nMax_Connect);
	DDX_Text(pDX, IDC_EDIT_IP, m_nListernIp);
}


BEGIN_MESSAGE_MAP(CSettingDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSettingDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSettingDlg 消息处理程序


void CSettingDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	((CMyRemoteApp*)AfxGetApp())->m_IniFile.SetString("Settings", "ListemIp", m_nListernIp);
	((CMyRemoteApp*)AfxGetApp())->m_IniFile.SetString("Settings", "ListenPort", m_nListernProt);      //向ini文件中写入值
	((CMyRemoteApp*)AfxGetApp())->m_IniFile.SetString("Settings", "MaxConnection", m_nMax_Connect);
	MessageBox("设置成功，重启本程序后生效！");
	CDialogEx::OnOK();
}
