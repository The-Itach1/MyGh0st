// CProcessDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyRemote.h"
#include "CProcessDlg.h"
#include "afxdialogex.h"
#include "..\common\macros.h"

// CProcessDlg 对话框

IMPLEMENT_DYNAMIC(CProcessDlg, CDialog)

CProcessDlg::CProcessDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext* pContext)
	: CDialog(IDD_PROCESS, pParent)
{
	m_iocpServer = pIOCPServer;        //就是一个赋值没什么特别的我们到oninitdialog
	m_pContext = pContext;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_PROCESS));
	//这里判断是窗口管理还是进程管理因为进程管理的数据头是TOKEN_PSLIST
	//窗口管理的数据头TOKEN_WSLIST  我们可以用这两个数据头来区分
	char* lpBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(0));
	m_bHow = lpBuffer[0];
}

CProcessDlg::~CProcessDlg()
{
}

void CProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROCESS, m_list_process);
}


BEGIN_MESSAGE_MAP(CProcessDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESS, &CProcessDlg::OnNMRClickListProcess)
	ON_COMMAND(IDM_PROCESS_KILL, &CProcessDlg::OnProcessKill)
	ON_COMMAND(IDM_PROCESS_REFRESH, &CProcessDlg::OnProcessRefresh)
END_MESSAGE_MAP()


// CProcessDlg 消息处理程序


void CProcessDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	CDialog::OnClose();
}


void CProcessDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	AdjustList();
	// TODO: 在此处添加消息处理程序代码
}


BOOL CProcessDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen); //得到连接的ip 
	str.Format("\\\\%s - 系统管理", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);//设置对话框标题

	if (m_bHow == TOKEN_PSLIST)      //进程管理初始化列表
	{
		m_list_process.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);  //初始化进程的列表
		m_list_process.InsertColumn(0, "映像名称", LVCFMT_LEFT, 100);
		m_list_process.InsertColumn(1, "PID", LVCFMT_LEFT, 50);
		m_list_process.InsertColumn(2, "程序路径", LVCFMT_LEFT, 400);
		ShowProcessList();   //由于第一个发送来的消息后面紧跟着进程的数据所以把数据显示到列表当中
	}
	AdjustList();       //各个列表的大小
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CProcessDlg::GetProcessList(void)
{
	BYTE bToken = COMMAND_PSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}


void CProcessDlg::ShowProcessList(void)
{
	char* lpBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	char* strExeFile;
	char* strProcessName;
	DWORD	dwOffset = 0;
	CString str;
	m_list_process.DeleteAllItems();
	//遍历发送来的每一个字符别忘了他的数据结构啊 Id+进程名+0+完整名+0
	int i;
	for (i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);        //这里得到进程ID
		strExeFile = lpBuffer + dwOffset + sizeof(DWORD);      //进程名就是ID之后的啦
		strProcessName = strExeFile + lstrlen(strExeFile) + 1;  //完整名就是进程名之后的啦
		//他的数据结构的构建很巧妙

		m_list_process.InsertItem(i, strExeFile);       //将得到的数据加入到列表当中
		str.Format("%5u", *lpPID);
		m_list_process.SetItemText(i, 1, str);
		m_list_process.SetItemText(i, 2, strProcessName);
		// ItemData 为进程ID
		m_list_process.SetItemData(i, *lpPID);

		dwOffset += sizeof(DWORD) + lstrlen(strExeFile) + lstrlen(strProcessName) + 2;   //跳过这个数据结构 进入下一个循环
	}

	str.Format("程序路径 / %d", i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_list_process.SetColumn(2, &lvc); //在列表中显示有多少个进程
}

void CProcessDlg::AdjustList()
{
	if (m_list_process.m_hWnd == NULL)
	{
		return;
	}
	RECT	rectClient;
	RECT	rectList;
	GetClientRect(&rectClient);
	rectList.left = 0;
	rectList.top = 0;
	rectList.right = rectClient.right;
	rectList.bottom = rectClient.bottom;

	m_list_process.MoveWindow(&rectList);
}

void CProcessDlg::OnReceiveComplete(void)
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_PSLIST:
		ShowProcessList();
		break;
	default:
		// 传输发生异常数据
		break;
	}
}

void CProcessDlg::OnNMRClickListProcess(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu	Menu;
	if (m_bHow == TOKEN_PSLIST)      //进程管理初始化列表
	{
		Menu.LoadMenu(IDR_MENU_PROCESS);
	}
	CMenu* pMenu = Menu.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);

	pMenu->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);

	*pResult = 0;
}


void CProcessDlg::OnProcessKill()
{
	// TODO: 在此添加命令处理程序代码
	CListCtrl* pListCtrl = NULL;
	if (m_list_process.IsWindowVisible())
		pListCtrl = &m_list_process;
	else
		return;

	// TODO: Add your command handler code here
	//非配缓冲区
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + (pListCtrl->GetSelectedCount() * 4));
	//加入结束进程的数据头
	lpBuffer[0] = COMMAND_KILLPROCESS;
	//显示警告信息
	char* lpTips = "警告: 终止进程会导致不希望发生的结果，\n"
		"包括数据丢失和系统不稳定。在被终止前，\n"
		"进程将没有机会保存其状态和数据。";
	CString str;
	if (pListCtrl->GetSelectedCount() > 1)
	{
		str.Format("%s确实\n想终止这%d项进程吗?", lpTips, pListCtrl->GetSelectedCount());
	}
	else
	{
		str.Format("%s确实\n想终止该项进程吗?", lpTips);
	}
	if (::MessageBox(m_hWnd, str, "进程结束警告", MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;

	DWORD	dwOffset = 1;
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	//得到要结束哪个进程
	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = pListCtrl->GetNextSelectedItem(pos);
		DWORD dwProcessID = pListCtrl->GetItemData(nItem);
		memcpy(lpBuffer + dwOffset, &dwProcessID, sizeof(DWORD));
		dwOffset += sizeof(DWORD);
	}
	//发送数据到服务端到服务端查找COMMAND_KILLPROCESS这个数据头
	m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}


void CProcessDlg::OnProcessRefresh()
{
	// TODO: 在此添加命令处理程序代码
	if (m_list_process.IsWindowVisible())
		GetProcessList();
}
