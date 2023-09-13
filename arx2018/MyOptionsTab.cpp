// MyOptionsTab.cpp: 实现文件
//

#include "pch.h"
#include "MyOptionsTab.h"
#include "resource.h"


// MyOptionsTab 对话框

MyOptionsTab::MyOptionsTab(CWnd* pParent /*=nullptr*/)
	: CAcUiTabChildDialog(pParent)
{

}

MyOptionsTab::~MyOptionsTab()
{
}

void MyOptionsTab::DoDataExchange(CDataExchange* pDX)
{
	CAcUiTabChildDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(MyOptionsTab, CAcUiTabChildDialog)
END_MESSAGE_MAP()


// MyOptionsTab 消息处理程序
