#pragma once

#include <map>
#include <functional>

#include <acdocman.h>

// ����ʱ, �Զ�����acDocManager��. ����ʱ, �Զ����
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

    void  documentToBeDeactivated(AcApDocument* pDeactivatingDoc)override;
    void  documentActivationModified(bool bActivation)override;
    void  documentActivated(AcApDocument* pActivatingDoc)override;

public:

    // ��ͬoverride��Ӧ��ͬ�Ĳ�������. first������override�ĺ�����. second�ǲ�������, 
    // const TCHAR*��Ϊ��Ϣ, void* ��ʾ�Զ�������
    std::map < CString, std::function<void(const TCHAR*, void*)>> ops_;
};

