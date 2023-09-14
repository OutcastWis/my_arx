#pragma once

#include <map>
#include <functional>

#include <acdocman.h>

// 创建时, 自动加入acDocManager中. 析构时, 自动解除
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

    // 不同override对应不同的操作函数. first是上述override的函数名. second是操作函数, 
    // const TCHAR*作为信息, void* 表示自定义数据
    std::map < CString, std::function<void(const TCHAR*, void*)>> ops_;
};

