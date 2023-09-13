#pragma once

#include <acdocman.h>


class MySimpleDocReactor : public AcApDocManagerReactor
{
public:
    MySimpleDocReactor();
    ~MySimpleDocReactor();

    void  documentCreated(AcApDocument* pDocCreating) override;
    void  documentToBeDestroyed(AcApDocument* pDocToDestroy) override;
    void  documentLockModeChanged(AcApDocument* pDoc, AcAp::DocLockMode myPreviousMode, AcAp::DocLockMode myCurrentMode,
        AcAp::DocLockMode currentMode, const TCHAR* pCommandName) override;
    void  documentBecameCurrent(AcApDocument*) override;

    void  documentToBeActivated(AcApDocument* pActivatingDoc)override;
    void  documentToBeDeactivated(AcApDocument* pDeactivatingDoc)override;
    void  documentActivationModified(bool bActivation)override;
};

