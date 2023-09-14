#pragma once

#include <afxdialogex.h>

// MySimpleModelessDialog 对话框
// 在CAD中使用非模态对话框, 需要响应WM_ACAD_KEEPFOCUS消息. return TRUE to keep focus
class MySimpleModelessDialog : public CDialogEx
{
	DECLARE_DYNAMIC(MySimpleModelessDialog)

public:
	MySimpleModelessDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~MySimpleModelessDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIMPLE_MODELESS_DIALOG
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	afx_msg void OnClose();
	afx_msg LRESULT onAcadKeepFocus(WPARAM, LPARAM);
	virtual void OnCancel();
	afx_msg void OnUndo();


	DECLARE_MESSAGE_MAP()

public:
	CButton m_undoBtn;

};
