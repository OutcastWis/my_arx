#include "pch.h"
#include "MyEditContext.h"

#include <acdocman.h>

#include "resource.h"




MyEntityContext::MyEntityContext()
{
    m_pMenu = new CMenu;
    ASSERT(m_pMenu);
    m_pMenu->LoadMenu(IDR_ENTITY1_MENU);
}

MyEntityContext::~MyEntityContext()
{
    if (m_pMenu)
        delete m_pMenu;
}

// This function should return pointer to the HMENU that is to
// be added to AutoCAD context menu.
void* MyEntityContext::getMenuContext(const AcRxClass* pClass, const AcDbObjectIdArray& ids)
{
    m_tempHMenu = m_pMenu->GetSubMenu(0)->GetSafeHmenu();
    return &m_tempHMenu;
}

// This function is called when user selects a ARX added item
// The cmdIndex is the ID of the menu item as supplied by ARX app.
void MyEntityContext::onCommand(Adesk::UInt32 cmdIndex)
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
void MyEntityContext::OnUpdateMenu()
{
    m_pMenu->EnableMenuItem(ID_ENTITY1_ITEM1, MF_GRAYED);
    m_pMenu->CheckMenuItem(ID_ENTITY1_ITEM2, MF_CHECKED);
}


//////////////////////////////////////////////////////////////////////////////


MyCircleContext::MyCircleContext()
{
    m_pMenu = new CMenu;
    m_pMenu->LoadMenu(IDR_CIRCLE_MENU);
}

MyCircleContext::~MyCircleContext()
{
    if (m_pMenu) {
        delete m_pMenu;
    }
}

// This function should return pointer to the HMENU that is to
// be added to AutoCAD context menu.
void* MyCircleContext::getMenuContext(const AcRxClass* pClass, const AcDbObjectIdArray& ids)
{
    m_tempHMenu = m_pMenu->GetSubMenu(0)->GetSafeHmenu();
    return &m_tempHMenu;
}

// This function is called when user selects a ARX added item
// The cmdIndex is the ID of the menu item as supplied by ARX app.
void MyCircleContext::onCommand(Adesk::UInt32 cmdIndex)
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
void MyCircleContext::OnUpdateMenu()
{
    m_pMenu->EnableMenuItem(ID_CIRCLE_ITEM1, MF_GRAYED);
    m_pMenu->EnableMenuItem(ID_CIRCLE_SUB_ITEM1, MF_GRAYED);
}


//////////////////////////////////////////////////////////////////////////////


MyLineContext::MyLineContext()
{

    m_pMenu = new CMenu;
    m_pMenu->LoadMenu(IDR_LINE_MENU);

}

MyLineContext::~MyLineContext()
{
    if (m_pMenu)
        delete m_pMenu;
}

// Change the state of the menu items here. This function is
// called by AutoCAD just before querying the app for the context menu
void MyLineContext::OnUpdateMenu()
{
    m_pMenu->CheckMenuItem(ID_LINE_ITEM1, MF_CHECKED);
}

// This function should return pointer to the HMENU that is to
// be added to AutoCAD context menu.
void* MyLineContext::getMenuContext(const AcRxClass* pClass, const AcDbObjectIdArray& ids)
{
    m_tempHMenu = m_pMenu->GetSubMenu(0)->GetSafeHmenu();
    return &m_tempHMenu;
}

// This function is called when user selects a ARX added item
// The cmdIndex is the ID of the menu item as supplied by ARX app.
void MyLineContext::onCommand(Adesk::UInt32 cmdIndex)
{

    CString str1, str2;
    m_pMenu->GetMenuString(cmdIndex, str1, MF_BYCOMMAND);
    str2.Format(_T("\nMenu Item selected is %s"), str1);
    acutPrintf(str2);

    // display the command prompt again.
    acedPostCommandPrompt();

}

/////////////////////////////////////////////////////////////////////////