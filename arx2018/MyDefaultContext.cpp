#include "pch.h"
#include "MyDefaultContext.h"

#include "resource.h"


MyDefaultContext::MyDefaultContext()
{
    m_pMenu = new CMenu;
    m_pMenu->LoadMenu(IDR_DEFAULT_MENU);

}

MyDefaultContext::~MyDefaultContext()
{
    if (m_pMenu)
        delete m_pMenu;
}

// This function should return pointer to the HMENU that is to
// be added to AutoCAD context menu.
void* MyDefaultContext::getMenuContext(const AcRxClass*, const AcDbObjectIdArray&)
{
    m_tempHMenu = m_pMenu->GetSubMenu(0)->GetSafeHmenu();
    return &m_tempHMenu;
}

// This function is called when user selects a ARX added item
// The cmdIndex is the ID of the menu item as supplied by ARX app.
void MyDefaultContext::onCommand(Adesk::UInt32 cmdIndex)
{

    CString str1, str2;
    m_pMenu->GetMenuString(cmdIndex, str1, MF_BYCOMMAND);
    str2.Format(_T("\nMenu Item selected is %s"), str1);
    acutPrintf(str2);

    // display the command prompt again.
    acedPostCommandPrompt();

}

// Change the state of the menu items here. This function is
// called by AutoCAD just before querying the app for the context menu
void MyDefaultContext::OnUpdateMenu()
{
    m_pMenu->EnableMenuItem(ID_DEFAULT_ITEM1, MF_ENABLED);
    m_pMenu->EnableMenuItem(ID_DEFAULT_SUB_ITEM1, MF_GRAYED);
}
