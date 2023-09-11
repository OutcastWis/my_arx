#pragma once

#include <aced.h>

class MyEntityContext : public AcEdUIContext
{
public:
    MyEntityContext();
    ~MyEntityContext();
    virtual void* getMenuContext(const AcRxClass* pClass, const AcDbObjectIdArray& ids);
    virtual void  onCommand(Adesk::UInt32 cmdIndex);
    virtual void  OnUpdateMenu();

private:
    CMenu* m_pMenu;
    HMENU m_tempHMenu;
};


class MyCircleContext : public  AcEdUIContext
{
public:
    MyCircleContext();
    ~MyCircleContext();
    virtual void* getMenuContext(const AcRxClass* pClass, const AcDbObjectIdArray& ids);
    virtual void  onCommand(Adesk::UInt32 cmdIndex);
    virtual void  OnUpdateMenu();
private:
    CMenu* m_pMenu;
    HMENU m_tempHMenu;
};


class MyLineContext : public  AcEdUIContext
{
public:
    MyLineContext();
    ~MyLineContext();
    virtual void* getMenuContext(const AcRxClass* pClass, const AcDbObjectIdArray& ids);
    virtual void  onCommand(Adesk::UInt32 cmdIndex);
    virtual void  OnUpdateMenu();
private:
    CMenu* m_pMenu;
    HMENU m_tempHMenu;
};