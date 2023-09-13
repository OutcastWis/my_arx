#include "pch.h"
#include "MySimpleDocReactor.h"

namespace detail {
    const TCHAR* modeStr(AcAp::DocLockMode mode)
    {
        switch (mode) {
        case AcAp::kNotLocked:
            return _T(/*NOXLATE*/"AcAp::kNotLocked");
        case AcAp::kRead:
            return _T(/*NOXLATE*/"AcAp::kRead");
        case AcAp::kWrite:
            return _T(/*NOXLATE*/"AcAp::kWrite");
        case AcAp::kAutoWrite:
            return _T(/*NOXLATE*/"AcAp::kAutoWrite");
        case AcAp::kProtectedAutoWrite:
            return _T(/*NOXLATE*/"AcAp::kProtectedAutoWrite");
        case AcAp::kXWrite:
            return _T(/*NOXLATE*/"AcAp::kXWrite");
        }
        return _T("ERROR");
    }
}


MySimpleDocReactor::MySimpleDocReactor() {
    acDocManager->addReactor(this);
}


MySimpleDocReactor::~MySimpleDocReactor() {
    acDocManager->removeReactor(this);
}

void MySimpleDocReactor::documentCreated(AcApDocument* pDocCreating) {
    if (pDocCreating)
        acutPrintf(_T("DOCUMENT: Created %s\n"), pDocCreating->fileName());
}

void MySimpleDocReactor::documentToBeDestroyed(AcApDocument* pDocToDestroy) {
    if (!acDocManager)
        return;

    if (acDocManager->documentCount() == 1)
    {
        // Last document destroyed going to zero document state or quitting
        acutPrintf(_T("LAST DOCUMENT: To be destroyed %s\n"), pDocToDestroy->fileName());
    }
    else {
        acutPrintf(_T("DOCUMENT: To be destroyed %s\n"), pDocToDestroy->fileName());
    }
}

void MySimpleDocReactor::documentLockModeChanged(AcApDocument* pDoc, AcAp::DocLockMode myPreviousMode, AcAp::DocLockMode myCurrentMode,
    AcAp::DocLockMode currentMode, const TCHAR* pCommandName) {
    if (!pDoc)
        return;

    acutPrintf(_T("%s %sLOCK %s CHANGED TO %s FOR %s\n"), pDoc->fileName(),
        acDocManager->isApplicationContext() ? _T("APP ") : _T(""),
        detail::modeStr(myPreviousMode),
        detail::modeStr(myCurrentMode),
        pCommandName
    );
}


void MySimpleDocReactor::documentBecameCurrent(AcApDocument* pDoc)
{
    if (pDoc)
        acutPrintf(_T("DOCUMENT: Became current %s\n"), pDoc->fileName());
}

void MySimpleDocReactor::documentToBeActivated(AcApDocument* pActivatingDoc)
{
    if (pActivatingDoc)
        acutPrintf(_T("DOCUMENT: To be Activated %s\n"), pActivatingDoc->fileName());
}

void MySimpleDocReactor::documentToBeDeactivated(AcApDocument* pDeactivatingDoc)
{
    if (pDeactivatingDoc)
        acutPrintf(_T("DOCUMENT: To be DEActivated %s\n"), pDeactivatingDoc->fileName());
}

void MySimpleDocReactor::documentActivationModified(bool bActivation)
{
    if (bActivation)
        acutPrintf(_T("DOCUMENT Activation is ON. \n"));
    else
        acutPrintf(_T("DOCUMENT Activation is OFF. \n"));
}

