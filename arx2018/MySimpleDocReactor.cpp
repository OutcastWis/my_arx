#include "pch.h"
#include "MySimpleDocReactor.h"

#include <tuple>




MySimpleDocReactor::MySimpleDocReactor() {
    acDocManager->addReactor(this);
}


MySimpleDocReactor::~MySimpleDocReactor() {
    acDocManager->removeReactor(this);
}

void MySimpleDocReactor::documentCreated(AcApDocument* pDocCreating) {
    auto it = ops_.find(_T("documentCreated"));
    if (it != ops_.end()) {
        ops_[_T("documentCreated")](_T(""), (void*)pDocCreating);
    }
}

void MySimpleDocReactor::documentToBeDestroyed(AcApDocument* pDocToDestroy) {
    auto it = ops_.find(_T("documentToBeDestroyed"));
    if (it != ops_.end()) {
        ops_[_T("documentToBeDestroyed")](_T(""), (void*)pDocToDestroy);
    }
}

void MySimpleDocReactor::documentLockModeChanged(AcApDocument* pDoc, AcAp::DocLockMode myPreviousMode, AcAp::DocLockMode myCurrentMode,
    AcAp::DocLockMode currentMode, const TCHAR* pCommandName) {

    auto it = ops_.find(_T("documentLockModeChanged"));
    if (it != ops_.end()) {
        typedef std::tuple< AcApDocument*, AcAp::DocLockMode, AcAp::DocLockMode, AcAp::DocLockMode, const TCHAR* > td_type;
        td_type data{ pDoc ,myPreviousMode,  myCurrentMode , currentMode,pCommandName };
        ops_[_T("documentLockModeChanged")](_T(""), (void*)&data);
    }
}


void MySimpleDocReactor::documentBecameCurrent(AcApDocument* pDoc)
{
    auto it = ops_.find(_T("documentBecameCurrent"));
    if (it != ops_.end()) {
        ops_[_T("documentBecameCurrent")](_T(""), (void*)pDoc);
    }
}



void MySimpleDocReactor::documentToBeDeactivated(AcApDocument* pDeactivatingDoc)
{
    auto it = ops_.find(_T("documentToBeDeactivated"));
    if (it != ops_.end()) {
        ops_[_T("documentToBeDeactivated")](_T(""), (void*)pDeactivatingDoc);
    }
}

void MySimpleDocReactor::documentActivationModified(bool bActivation)
{
    auto it = ops_.find(_T("documentActivationModified"));
    if (it != ops_.end()) {
        ops_[_T("documentActivationModified")](_T(""), (void*)&bActivation);
    }
}

void MySimpleDocReactor::documentActivated(AcApDocument* pActivatingDoc)
{
    auto it = ops_.find(_T("documentActivated"));
    if (it != ops_.end()) {
        ops_[_T("documentActivated")](_T(""), (void*)pActivatingDoc);
    }
}
