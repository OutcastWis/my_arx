#include "pch.h"
#include "extend_tabs.h"
#include "resource.h"

#include <aduiTabExtension.h>
#include <AcExtensionModule.h>

AC_DECLARE_EXTENSION_MODULE(theArxDLL);

namespace wzj{

    void extend_tabs::init_impl() {

    }

    void extend_tabs::stop_impl() {
        if (m_tab)
        {
            delete m_tab;
            m_tab = nullptr;
        }
    }

    void extend_tabs::add_tab(void* appId) {
        CAdUiTabExtensionManager* pTabExtMgr = static_cast<CAdUiTabExtensionManager*>(appId);

        // Get name of extensible dialog
        CString tabDlgName = pTabExtMgr->GetDialogName();

        // Add the "ArxDbg" tab to the Options dialog
        if (!tabDlgName.CompareNoCase(_T("OptionsDialog"))) {

            // TBD: It seems that we have to hold on to the dialog between invocations.
            // I would think that we would get a call to endDialog that would allow us
            // to delete the dialog after it closes... but that doesn't appear to be
            // the case.
            if (m_tab == NULL)
                m_tab = new MyOptionsTab;

            if (m_tab) {
                pTabExtMgr->AddTab(theArxDLL.ModuleResourceInstance(), IDD_OPTIONS_TAB, _T("wzj_options_tab"), m_tab);
            }
        }
    }

}