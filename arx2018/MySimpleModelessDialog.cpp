// MySimpleModelessDialog.cpp: 实现文件
//

#include "pch.h"
#include "MySimpleModelessDialog.h"
#include "resource.h"

#include <acdocman.h>

#include "modeless_dialog.h"



// MySimpleModelessDialog 对话框

IMPLEMENT_DYNAMIC(MySimpleModelessDialog, CDialogEx)

MySimpleModelessDialog::MySimpleModelessDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SIMPLE_MODELESS_DIALOG, pParent)
{

}

MySimpleModelessDialog::~MySimpleModelessDialog()
{
}

void MySimpleModelessDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_UNDO, m_undoBtn);
}

void MySimpleModelessDialog::OnClose()
{
	wzj::modeless_dialog::instance()->display_dialog_ = false;
	auto db = acdbHostApplicationServices()->workingDatabase();
	if (db)
		wzj::modeless_dialog::instance()->detachDbReactor(db);
	CDialogEx::OnClose();
	wzj::modeless_dialog::instance()->endDlg();
}

LRESULT MySimpleModelessDialog::onAcadKeepFocus(WPARAM, LPARAM)
{
	return TRUE;
}

void MySimpleModelessDialog::OnCancel()
{
	wzj::modeless_dialog::instance()->display_dialog_ = false;
	auto db = acdbHostApplicationServices()->workingDatabase();
	if (db)
		wzj::modeless_dialog::instance()->detachDbReactor(db);

	CDialogEx::OnCancel();
	wzj::modeless_dialog::instance()->endDlg();
}

void MySimpleModelessDialog::OnUndo()
{
	acDocManager->sendStringToExecute(acDocManager->curDocument(), _T("_UNDO 1 "), false, true);
}


BEGIN_MESSAGE_MAP(MySimpleModelessDialog, CDialogEx)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_ACAD_KEEPFOCUS, onAcadKeepFocus)
	ON_BN_CLICKED(IDC_UNDO, OnUndo)
END_MESSAGE_MAP()


// MySimpleModelessDialog 消息处理程序
