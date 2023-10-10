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

void MySimpleEditReactor::saveComplete(AcDbDatabase* pDwg, const TCHAR* pActualName)
{
    auto it = ops_.find(_T("saveComplete"));
    if (it != ops_.end()) {
        std::pair< AcDbDatabase*, const TCHAR*> data(pDwg, pActualName);
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

void MySimpleEditReactor::beginDeepClone(AcDbDatabase* pTo, AcDbIdMapping& idMap)
{
    auto it = ops_.find(_T("beginDeepClone"));
    if (it != ops_.end()) {
        std::pair<AcDbDatabase*, AcDbIdMapping&>data(pTo, idMap);
        ops_[_T("beginDeepClone")](_T(""), (void*)&data);
    }
}

void MySimpleEditReactor::beginDeepCloneXlation(AcDbIdMapping& idMap, Acad::ErrorStatus* pRetStat)
{
    auto it = ops_.find(_T("beginDeepCloneXlation"));
    if (it != ops_.end()) {
        std::pair<AcDbIdMapping&, Acad::ErrorStatus*>data(idMap, pRetStat);
        ops_[_T("beginDeepCloneXlation")](_T(""), (void*)&data);
    }
}

void MySimpleEditReactor::abortDeepClone(AcDbIdMapping& idMap)
{
    auto it = ops_.find(_T("abortDeepClone"));
    if (it != ops_.end()) {
        ops_[_T("abortDeepClone")](_T(""), (void*)&idMap);
    }
}

void MySimpleEditReactor::endDeepClone(AcDbIdMapping& idMap)
{
    auto it = ops_.find(_T("endDeepClone"));
    if (it != ops_.end()) {
        ops_[_T("endDeepClone")](_T(""), (void*)&idMap);
    }
}
