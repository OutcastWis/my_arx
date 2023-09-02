#pragma once

#include <adui.h>
#include <acui.h>

// MyAcUiDialog 对话框

class MyAcUiDialog : public CAcUiDialog
{
public:
	MyAcUiDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~MyAcUiDialog();

    void DisplayPoint();
    bool ValidatePoint();

    void DisplayAngle();
    bool ValidateAngle();

    void DisplayBlocks(); // 显示所有block
    void DisplayRegApps();


// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

    CAcUiSymbolComboBox m_ctrlRegAppComboBox;
    CAcUiListBox    m_ctrlBlockListBox;
    CAcUiPickButton m_ctrlPickButton;
    CAcUiPickButton m_ctrlAngleButton;
    CAcUiAngleEdit  m_ctrlAngleEdit;
    CAcUiNumericEdit m_ctrlXPtEdit;
    CAcUiNumericEdit m_ctrlYPtEdit;
    CAcUiNumericEdit m_ctrlZPtEdit;

    CString m_strAngle;
    CString m_strXPt;
    CString m_strYPt;
    CString m_strZPt;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持


    afx_msg void OnButtonPoint();
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonAngle();
    afx_msg void OnKillfocusComboRegapps(); // 如果编辑框内有东西, 则注册
    virtual void OnOK();
    afx_msg void OnKillfocusEditAngle();
    afx_msg void OnKillfocusEditXpt();
    afx_msg void OnKillfocusEditYpt();
    afx_msg void OnKillfocusEditZpt();
	DECLARE_MESSAGE_MAP()
};
