#include "pch.h"
#include "docman.h"

#include <AcExtensionModule.h>
#include <acdocman.h>

AC_DECLARE_EXTENSION_MODULE(theArxDll);

namespace wzj {

    namespace detail {
        AcApDocument* selectDocument()
        {
            AcApDocument* documentArray[10];

            AcApDocument* pDoc;
            AcApDocumentIterator* pDocIter;
            int nDocs = 0;;

            pDocIter = acDocManager->newAcApDocumentIterator();

            for (; !pDocIter->done(); pDocIter->step(), nDocs++) {
                pDoc = pDocIter->document();
                documentArray[nDocs] = pDoc;
                acutPrintf(_T("%d.  %s\n"), nDocs + 1, pDoc->fileName());
            }
            delete pDocIter;

            acedInitGet(RSG_NOZERO | RSG_NONEG, NULL);
            int iSelection;
            int inputStatus = acedGetInt(_T("Which document should this command execute in: "), &iSelection);
            if (inputStatus == RTNORM && iSelection <= nDocs) {
                return documentArray[iSelection - 1];
            }
            else {
                return NULL;
            }
        }
        // 打印所有文档
        void listDocuments()
        {
            AcApDocument* pDoc;
            AcApDocumentIterator* pDocIter;

            pDocIter = acDocManager->newAcApDocumentIterator();

            for (; !pDocIter->done(); pDocIter->step()) {
                pDoc = pDocIter->document();
                acutPrintf(_T("  %s\n"), pDoc->fileName());
            }
            delete pDocIter;
        }
        // 监视文档
        void watchDocuments()
        {
            if (getYorN(_T("Start watching Documents"))) {
                gpDocReactor = new AsdkDocReactor();
                gpEditReactor = new AsdkEditorReactor();
                acutPrintf(_T("  Added reactor to the Document Manager.\n"));
            }
            else {
                delete gpDocReactor;
                gpDocReactor = NULL;
                delete gpEditReactor;
                gpEditReactor = NULL;
                acutPrintf(_T("  Removed reactor from the Document Manager.\n"));
            }
        }
    }


    void docman::init_impl() {

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_LISTDOCS"), _T("LOCAL_LISTDOCS"), ACRX_CMD_MODAL, detail::listDocuments);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_WATCHDOCS"), _T("LOCAL_WATCHDOCS"), ACRX_CMD_MODAL, detail::watchDocuments);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_SEND"), _T("LOCAL_SEND"), ACRX_CMD_MODAL | ACRX_CMD_NONEWSTACK,
            detail::send);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_ACTIVATE"), _T("LOCAL_ACTIVATE"), ACRX_CMD_MODAL | ACRX_CMD_NONEWSTACK,
            detail::activate);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_NEWDOC"), _T("LOCAL_NEWDOC"), ACRX_CMD_MODAL,
            detail::newdoc);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_OPENDOC"), _T("LOCAL_OPENDOC"), ACRX_CMD_MODAL | ACRX_CMD_NONEWSTACK,
            detail::opendoc);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_CLOSEDOC"), _T("LOCAL_CLOSEDOC"), ACRX_CMD_MODAL | ACRX_CMD_NONEWSTACK,
            detail::closedoc);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_CMD"), _T("LOCAL_CMD"), ACRX_CMD_MODAL, detail::cmd);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_TOGGLE"), _T("LOCAL_TOGGLE"), ACRX_CMD_MODAL | ACRX_CMD_NONEWSTACK,
            detail::toggleActivate);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_APPCONTEXT"), _T("LOCAL_APPCONTEXT"), ACRX_CMD_MODAL | ACRX_CMD_NONEWSTACK,
            detail::appContext);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_NEWSYNCDOC"), _T("LOCAL_NEWSYNCDOC"), ACRX_CMD_MODAL,
            detail::newSyncDoc);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_OPENSYNCDOC"), _T("LOCAL_OPENSYNCDOC"), ACRX_CMD_MODAL,
            detail::openSyncDoc);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_TESTDLG"), _T("LOCAL_TESTDLG"), ACRX_CMD_MODAL, 
            detail::testdlg, NULL, -1,
            docmanDLL.ModuleResourceInstance());
    }

    void docman::stop_impl() {

    }

}