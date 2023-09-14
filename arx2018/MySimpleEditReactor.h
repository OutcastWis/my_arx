#pragma once

#include <map>
#include <functional>

#include <aced.h>

// ����ʱ, �Զ�����acedEditor��. ����ʱ, �Զ����
class MySimpleEditReactor : public AcEditorReactor
{
public:
    MySimpleEditReactor();
    ~MySimpleEditReactor();

    // the drawing database pDwg was saved to the file specified by pActualName. 
    void saveComplete(AcDbDatabase* pDwg, const TCHAR* pActualName) override;

    virtual void commandEnded(const TCHAR* cmdStr);
    virtual void commandWillStart(const TCHAR* cmdStr);

public:

    // ��ͬoverride��Ӧ��ͬ�Ĳ�������. first������override�ĺ�����. second�ǲ�������, 
    // const TCHAR*��Ϊ��Ϣ, void* ��ʾ�Զ�������
    std::map < CString, std::function<void(const TCHAR*, void*)>> ops_;
};

