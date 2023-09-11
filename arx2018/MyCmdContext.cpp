#include "pch.h"
#include "MyCmdContext.h"

#include "resource.h"

MyCmdContext::MyCmdContext()
{
    m_pMenu = new CMenu();
    m_pMenu->LoadMenu(IDR_CMD_MENU);

}

MyCmdContext::~MyCmdContext()
{
    if (m_pMenu)
        delete m_pMenu;
}

// Change the state of the menu items here. This function is
// called by AutoCAD just before querying the app for the context menu
void MyCmdContext::OnUpdateMenu()
{
    m_pMenu->CheckMenuItem(ID_CMD_SUB_ITEM1, MF_CHECKED);
    m_pMenu->EnableMenuItem(ID_CMD_ITEM1, MF_GRAYED);
}

// This function should return pointer to the HMENU that is to
// be added to AutoCAD context menu.
void* MyCmdContext::getMenuContext(const AcRxClass*, const AcDbObjectIdArray&)
{
    m_tempHMenu = m_pMenu->GetSubMenu(0)->GetSafeHmenu();
    return &m_tempHMenu;
}

// This function is called when user selects a ARX added item
// The cmdIndex is the ID of the menu item as supplied by ARX app.
void MyCmdContext::onCommand(Adesk::UInt32 cmdIndex)
{
    CString str1, str2;
    m_pMenu->GetMenuString(cmdIndex, str1, MF_BYCOMMAND);
    str2.Format(_T("\nMenu item selected is %s"), str1);
    acutPrintf(str2);

    // Again put up the command prompt.
    CString str(_T("Right click to get context menu or hit Escape or Enter to exit. "));
    str = _T('\n') + str;
    acutPrintf(str);
}
