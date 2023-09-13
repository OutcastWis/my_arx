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
    AcApDocument* pDoc = acDocManager->curDocument();
    if (pDoc)
        acutPrintf(_T("DOCUMENT: Save complete %s\n"), pDoc->fileName());
}
