#include "pch.h"
#include "context_menu.h"


namespace wzj {

    namespace detail {
        void cmd() {
            TCHAR buf[100] = {};
            CString str = _T("Right click to get context menu or hit Escape or Enter to exit. ");
            CString str_with_options = _T("Right click to get context menu or hit Escape or Enter to exit or \n "
                "[Op1/Op2]<Op1>:");
            // wait for user input.
            ads_point pt = {};
            auto r = acedGetPoint(NULL, str, pt);
            while (r != RTCAN) {
                ads_initget(RSG_OTHER, _T("Op1 Op2 _ Dummy1 Dummy2"));
                ads_point pt2 = {};
                if ((r = acedGetPoint(pt, str_with_options, pt2)) == RTKWORD) {
                    TCHAR keyword[16] = {};
                    ads_getinput(keyword);
                    ads_printf(_T("\ninput keyword is %s\n"), keyword);
                }
                ads_point_set(pt2, pt);
            }
        }
    }

    void context_menu::init_impl()
    {
        // Command Mode
        // When an ObjectARX command is running and the user right clicks for a shortcut menu, the menu will contain
        // some standard items read from the CMCOMMAND POP menu in the CUI file
        cc_ = new MyCmdContext;
        acedRegCmds->addCommand(_T("WZJ_COMMAND_CONTEXT_MENU"), _T("GLOBAL_CTX_MENU"), _T("LOCAL_CTX_MENU"), ACRX_CMD_MODAL,
            detail::cmd, cc_);


        // Default Mode
        // When no command is running, there is no pick first selection set, and the user right-clicks for a shortcut menu, 
        // the menu will contain some standard items read from the CMDEFAULT POP menu in the CUI file
        dc_ = new MyDefaultContext;
        if (acedAddDefaultContextMenu(dc_, appId_, _T("my_default_context")) != Adesk::kTrue)
            acutPrintf(_T("\nError registering default menu\n"));;


        // Edit Mode
        // When a pick first selection set exists and the user right clicks for a shortcut menu, the menu will contain 
        // some standard items read from the CMEDIT POP menu in the CUI file.
        ec_ = new MyEntityContext;
        if (acedAddObjectContextMenu(AcDbEntity::desc(), ec_, appId_) == Adesk::kFalse)
            acutPrintf(_T("\nError registering context menu\n"));

        linec_ = new MyLineContext;
        if (acedAddObjectContextMenu(AcDbLine::desc(), linec_, appId_) == Adesk::kFalse)
            acutPrintf(_T("\nError registering context menu\n"));

        circlec_ = new MyCircleContext;
        if (acedAddObjectContextMenu(AcDbCircle::desc(), circlec_, appId_) == Adesk::kFalse)
            acutPrintf(_T("\nError registering context menu\n"));

        appId_ = nullptr;
    }

    void context_menu::stop_impl()
    {
        acedRegCmds->removeGroup(_T("WZJ_COMMAND_CONTEXT_MENU"));
        delete cc_;

        acedRemoveDefaultContextMenu(dc_);
        delete dc_;

        acedRemoveObjectContextMenu(AcDbEntity::desc(), ec_);
        delete ec_;

        acedRemoveObjectContextMenu(AcDbLine::desc(), linec_);
        delete linec_;

        acedRemoveObjectContextMenu(AcDbCircle::desc(), circlec_);
        delete circlec_;
    }
}