#pragma once

#include <map>
#include <functional>

#include <aced.h>

// 创建时, 自动加入acedEditor中. 析构时, 自动解除
class MySimpleEditReactor : public AcEditorReactor
{
public:
    MySimpleEditReactor();
    ~MySimpleEditReactor();

    // the drawing database pDwg was saved to the file specified by pActualName. 
    void saveComplete(AcDbDatabase* pDwg, const TCHAR* pActualName) override;

    virtual void commandEnded(const TCHAR* cmdStr);
    virtual void commandWillStart(const TCHAR* cmdStr);


    // Deep Clone Events.
    //
    virtual void beginDeepClone(AcDbDatabase* pTo, AcDbIdMapping& idMap);
    virtual void beginDeepCloneXlation(AcDbIdMapping& idMap, Acad::ErrorStatus* pRetStat);
    virtual void abortDeepClone(AcDbIdMapping& idMap);
    virtual void endDeepClone(AcDbIdMapping& idMap);
public:

    // 不同override对应不同的操作函数. first是上述override的函数名. second是操作函数, 
    // const TCHAR*作为信息, void* 表示自定义数据
    std::map < CString, std::function<void(const TCHAR*, void*)>> ops_;
};

