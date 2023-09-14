#include "pch.h"
#include "MySimpleEditReactor.h"

#include <acdocman.h>

MySimpleEditReactor::MySimpleEditReactor()
{
	acedEditor->addReactor(this);
}

MySimpleEditReactor::~MySimpleEditReactor() {
	acedEditor->removeReactor(this);
}

void MySimpleEditReactor::saveComplete(AcDbDatabase*, const TCHAR* pActualName)
{
    auto it = ops_.find(_T("saveComplete"));
    if (it != ops_.end()) {
        std::pair< AcDbDatabase*, const TCHAR*> data;
        ops_[_T("saveComplete")](_T(""), &data);
    }
}

void MySimpleEditReactor::commandEnded(const TCHAR* cmdStr)
{
    auto it = ops_.find(_T("commandEnded"));
    if (it != ops_.end()) {
        ops_[_T("commandEnded")](_T(""), (void*)cmdStr);
    }
}

void MySimpleEditReactor::commandWillStart(const TCHAR* cmdStr)
{
    auto it = ops_.find(_T("commandWillStart"));
    if (it != ops_.end()) {
        ops_[_T("commandWillStart")](_T(""), (void*)cmdStr);
    }
}
