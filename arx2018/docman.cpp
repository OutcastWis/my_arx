#include "pch.h"
#include "docman.h"

#include <thread>
#include <chrono>

#include <AcExtensionModule.h>
#include <acdocman.h>
#include <acedCmdNF.h>
#include <acestext.h>

#include "command.h"


AC_DECLARE_EXTENSION_MODULE(theArxDLL);

namespace wzj {

    static docman* global_one = docman::instance();

    namespace detail {
        void openSyncDocHelper(void* pData)
        {
            AcApDocument* pDoc = acDocManager->curDocument();
            if (acDocManager->isApplicationContext()) {
                acutPrintf(_T("\nSucessfully Switched to App. Context\n"));
                acDocManager->appContextOpenDocument((const TCHAR*)pData);
                acutPrintf(_T("\nOpened a document synchronously:%s.\n"), (const TCHAR*)pData);
            }
            else
                acutPrintf(_T("\nERROR: in Document context : %s\n"), pDoc->fileName());

        }

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
        // 监视文档. 再次调用则取消监视
        void watchDocuments()
        {
            if (getYorN(_T("Start watching Documents"))) {
                global_one->doc_reactor_ = new MySimpleDocReactor();
                global_one->edit_reactor_= new MySimpleEditReactor();
                acutPrintf(_T("  Added reactor to the Document Manager.\n"));
            }
            else {
                global_one->clear();
                acutPrintf(_T("  Removed reactor from the Document Manager.\n"));
            }
        }

        /* sendStringToExecute可以理解为是将LOCAL_LISTDOCS命令发送到pDoc的运行栈上, 等待pDoc激活时运行. 分几种情况:
        *   1. 默认情况下, 即bActivate=true, bWrapUpInactiveDoc=false. 此时pDoc被激活, 并开始运行LOCAL_PAUSE. 见send()
        *   2. bActivate=false, bWrapUpInactiveDoc=true. pDoc不被激活, 但也运行, 等价于后台同步运行. 见send2()
        *   3. bActivate=false, bWrapUpInactiveDoc=false, pDoc不被激活, 也不运行. 下次手动激活pDoc时运行, 等价于延后运行. 见send3()
        */
        // 停顿10s后, 自动切换到pDoc
        void send() {
            AcApDocument* pDoc = selectDocument();

            if (pDoc == NULL) {
                acutPrintf(_T("No document selected.\n"));
                return;
            }
            // 此时将切换到pDoc上, 并在pDoc上运行LOCAL_PAUSE命令
            acDocManager->sendStringToExecute(pDoc, _T("LOCAL_PAUSE\n"));
            // The API inputPending() allows you to check to see if someone else has already
            // made a request via sendStringToExecute() to a target document.
            // You may not care if your command is autonomous and does not depend on the target
            // document being in a quiescent state. If not call both  isQuiescent() and inputPending()
            // 此时inputPending应该返回 _T("LOCAL_LISTDOCS\n")的长度, 为12, 不包含\0
            acutPrintf(_T("\nSent String to Doc: %s Pending Input %d\n"), pDoc->fileName(), acDocManager->inputPending(pDoc));
        }
        // 停顿10s后, 才可以操作. 不自动切换到pDoc
        void send2() {
            AcApDocument* pDoc = selectDocument();

            if (pDoc == NULL) {
                acutPrintf(_T("No document selected.\n"));
                return;
            }

            acDocManager->sendStringToExecute(pDoc, _T("LOCAL_PAUSE\n"), false, true);
            acutPrintf(_T("\nSent String to Doc: %s Pending Input %d\n"), pDoc->fileName(), acDocManager->inputPending(pDoc));
        }
        // 可以直接操作. 手动切换到pDoc时, 停顿10s
        void send3() {
            AcApDocument* pDoc = selectDocument();

            if (pDoc == NULL) {
                acutPrintf(_T("No document selected.\n"));
                return;
            }

            acDocManager->sendStringToExecute(pDoc, _T("LOCAL_PAUSE\n"), false, false);
            acutPrintf(_T("\nSent String to Doc: %s Pending Input %d\n"), pDoc->fileName(), acDocManager->inputPending(pDoc));
        }



        /*
        * 激活文档. 文档的激活会造成上下文切换. 在早期的CAD中, 由于使用了fiber, 这会使得当前上下文中的命令被挂起,
        * 直到该文档被重新激活. 但现在不使用fiber, 所以行为发生了改变: 实际的激活行为在当前命令执行完成后进行. 同时
        * activateDocument调用后, 在实际激活完成前, 鼠标键盘输入被禁
        */
        void activate() {
            AcApDocument* pDoc = selectDocument();

            if (pDoc == NULL) {
                acutPrintf(_T("No document selected.\n"));
                return;
            }

            acDocManager->activateDocument(pDoc);
            // In early releases of AutoCAD, a line of code here will not execute until the document
            // that started this command is activated again.
            ads_printf(_T("Glad you reactivated the document and it's up and running again. "));

            // activateDocument调用后, 输入被禁用
            int i;
            auto es = ads_getint(_T("It's disable, just return RTCAN"), &i);
            assert(es == RTCAN);
        }
        // 功能上等价于使用NEW命令. 但此函数走的是Windows的消息机制
        void newdoc()
        {
            acDocManager->newDocument();
            acutPrintf(_T("New document created.\n"));
        }
        // 功能上等价于使用OPEN命令. 但此函数走的是Windows的消息机制
        void opendoc()
        {
            acDocManager->openDocument();
        }
        // 关闭文档. 只能对当前文档起效
        void closedoc()
        {
            AcApDocument* pDoc = selectDocument();

            if (pDoc == NULL) {
                acutPrintf(_T("No document selected.\n"));
                return;
            }

            auto es = acDocManager->closeDocument(pDoc);
            if (es != Acad::eOk) {
                ads_printf(_T("CLOSE command not work. %s\n"), acadErrorStatusText(es));
                return;
            }
        }
        // 选择可以进行文档的激活. 你可以在不需要进行文档切换的时候, 禁用文档激活功能
        void toggleActivate()
        {
            if (getYorN(_T("Enable Multiple Document Activation"))) {
                if (!(acDocManager->isDocumentActivationEnabled())) {
                    Acad::ErrorStatus st = acDocManager->enableDocumentActivation();
                    if (st == Acad::eOk)
                        acutPrintf(_T(/*NOXLATE*/"==== Acad::eOk ====\n"));
                    else if (st == Acad::eInvalidContext)
                        acutPrintf(_T(/*NOXLATE*/" ==== Acad::eInvalidContext ====\n"));
                    else
                        acutPrintf(_T(" ==== I have no clue what's going on.... ====\n"));
                }
                else {
                    acutPrintf(_T("*******  Activation is already on.*******\n"));
                    return;
                }

                acutPrintf(_T("*******  Activation ON.*******\n"));
            }
            else {
                acDocManager->disableDocumentActivation();
                acutPrintf(_T("*******  Activation OFF.*******\n"));
            }

        }
        // 使用application context打开文档. 和opendoc不同, 它在同步打开文档后, 会将控制权返回调用程序. 可看做后台同步打开文档
        void openSyncDoc()
        {
            static TCHAR pData[] = _T("test1.dwg");

            AcApDocument* pDoc = acDocManager->curDocument();
            if (pDoc) {
                acutPrintf(_T("\nCurrently in Document context : %s, Switching to App.\n"), pDoc->fileName());
                acDocManager->executeInApplicationContext(openSyncDocHelper, (void*)pData);
            }

            ads_printf(_T("You can see me after open sync document"));
        }
        // 停顿10s. 用于send(), send2(), send3()中
        void pause() {
            ads_printf(_T("sleep 10s\n"));
            std::this_thread::sleep_for(
                std::chrono::seconds(10)
            );
            ads_printf(_T("wake up!\n"));
        }
    }


    void docman::init_impl() {

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_LISTDOCS"), _T("LOCAL_LISTDOCS"), ACRX_CMD_MODAL, 
            detail::listDocuments);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_WATCHDOCS"), _T("LOCAL_WATCHDOCS"), ACRX_CMD_MODAL, 
            detail::watchDocuments);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_SEND"), _T("LOCAL_SEND"), ACRX_CMD_MODAL ,
            detail::send);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_SEND2"), _T("LOCAL_SEND2"), ACRX_CMD_MODAL ,
            detail::send2);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_SEND3"), _T("LOCAL_SEND3"), ACRX_CMD_MODAL,
            detail::send3);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_ACTIVATE"), _T("LOCAL_ACTIVATE"), ACRX_CMD_MODAL ,
            detail::activate);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_NEWDOC"), _T("LOCAL_NEWDOC"), ACRX_CMD_MODAL,
            detail::newdoc);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_OPENDOC"), _T("LOCAL_OPENDOC"), ACRX_CMD_MODAL ,
            detail::opendoc);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_CLOSEDOC"), _T("LOCAL_CLOSEDOC"), ACRX_CMD_MODAL ,
            detail::closedoc);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_TOGGLE"), _T("LOCAL_TOGGLE"), ACRX_CMD_MODAL ,
            detail::toggleActivate);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_OPENSYNCDOC"), _T("LOCAL_OPENSYNCDOC"), ACRX_CMD_MODAL,
            detail::openSyncDoc);

        acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_PAUSE"), _T("LOCAL_PAUSE"), ACRX_CMD_MODAL,
            detail::pause);

       /* acedRegCmds->addCommand(_T("MY_COMMAND_DOCMAN"), _T("GLOBAL_TESTDLG"), _T("LOCAL_TESTDLG"), ACRX_CMD_MODAL, 
            detail::testdlg, NULL, -1,
            theArxDLL.ModuleResourceInstance());*/
    }

    void docman::stop_impl() {
        clear();
        acedRegCmds->removeGroup(_T("MY_COMMAND_DOCMAN"));
    }

    void docman::clear() {
        delete doc_reactor_;
        doc_reactor_ = nullptr;

        delete edit_reactor_;
        edit_reactor_ = nullptr;
    }

}