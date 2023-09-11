#pragma once

#include <aced.h>

class MyCmdContext : public  AcEdUIContext
{
public:
    MyCmdContext();
    ~MyCmdContext();
    virtual void* getMenuContext(const AcRxClass* pClass, const AcDbObjectIdArray& ids);
    virtual void  onCommand(Adesk::UInt32 cmdIndex);
    virtual void  OnUpdateMenu();
private:
    CMenu* m_pMenu;
    HMENU m_tempHMenu;
};

