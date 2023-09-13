#pragma once

#include <adui.h>
#include <acui.h>
#include <acuiDialog.h>

/**
* MyOptionsTab 对话框
* 这是一个有边框的, child形式的极简dialog. 设计成有边框纯粹是好玩
*/
class MyOptionsTab : public CAcUiTabChildDialog
{

public:
	MyOptionsTab(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~MyOptionsTab();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
