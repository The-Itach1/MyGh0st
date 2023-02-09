#pragma once


// CCreatServerDlg 对话框

class CCreatServerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCreatServerDlg)

public:
	CCreatServerDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CCreatServerDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CREAT_SERVER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_strIp;
	CEdit m_strPort;

	CString nIp;
	CString nPort;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
