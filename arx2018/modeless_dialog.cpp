#include "pch.h"
#include "modeless_dialog.h"

#include <cinttypes>
#include <tuple>
#include <utility>

#include <AcExtensionModule.h>
#include <acdocman.h>

#include "resource.h"
#include "command.h"

AC_DECLARE_EXTENSION_MODULE(theArxDLL);



namespace wzj {

    static modeless_dialog* global_one = modeless_dialog::instance();



    namespace detail {
        // 开启edit, document reactor. 并为当前文档添加db reactor, 显示界面
        void display() {
            global_one->display_dialog_ = true;

            global_one->attachDbReactor(acdbHostApplicationServices()->workingDatabase());
            global_one->attach();
            global_one->startDlg();
        }
        // 取消当前文档的reactor, 并隐藏界面
        void conceal() {
            global_one->detachDbReactor(curDoc()->database());
            if (global_one->dlg_)
                global_one->dlg_->ShowWindow(SW_HIDE);
        }
        // 取消所有文档的reactor, 删除界面. 但edit, doc的reactor依旧存在
        void conceal_all() {
            global_one->display_dialog_ = false;
            global_one->endDlg();

            for (auto& it : global_one->data_) {
                it.first->database()->removeReactor(it.second.dbReactor);
                delete it.second.dbReactor;
                it.second.dbReactor = nullptr;
            }
        }
    }

    void modeless_dialog::init_impl() {
        display_dialog_ = false;
        acedRegCmds->addCommand(_T("WZJ_COMMAND_MODELESS"), _T("GLOBAL_DISPLAY"), _T("LOCAL_DISPLAY"), ACRX_CMD_MODAL, 
            &detail::display, NULL, -1, theArxDLL.ModuleResourceInstance());

        acedRegCmds->addCommand(_T("WZJ_COMMAND_MODELESS"), _T("GLOBAL_CONCEAL"), _T("LOCAL_CONCEAL"), ACRX_CMD_MODAL,
            &detail::conceal);

        acedRegCmds->addCommand(_T("WZJ_COMMAND_MODELESS"), _T("GLOBAL_CONCEAL_ALL"), _T("LOCAL_CONCEAL_ALL"), ACRX_CMD_MODAL,
            &detail::conceal_all);
    }

    void modeless_dialog::stop_impl() {
        display_dialog_ = false;
        detach();

        for (auto& it : data_) {
            it.first->database()->removeReactor(it.second.dbReactor);
            delete it.second.dbReactor;
            it.second.dbReactor = nullptr;
        }
        data_.clear();
        endDlg();

        acedRegCmds->removeGroup(_T("WZJ_COMMAND_MODELESS"));
    }

    void modeless_dialog::attach() {
        if (edit_reactor_)
            return;

        edit_reactor_ = new MySimpleEditReactor;
        edit_reactor_->ops_[_T("commandWillStart")] = [&](const TCHAR*, void* data) {
            if (!_tcscmp(_T("UNDO"), (const TCHAR*)data))
                data_[curDoc()].undoCmd = true;
        };
        edit_reactor_->ops_[_T("commandEnded")] = [&](const TCHAR*, void* data) {
            auto& dd = data_[curDoc()];
            if (dlg_) // 因为关闭conceal和conceal_all都不会删除edit_reactor, 所以这里要判断
                dlg_->m_undoBtn.EnableWindow(dd.undoBtnAvailable);
            dd.undoBtnAvailable = false;
            dd.undoCmd = false; //undo is finished
        };

        doc_reactor_ = new MySimpleDocReactor;
        doc_reactor_->ops_[_T("documentActivated")] = [&](const TCHAR*, void* data) {
            auto it = data_.find(curDoc());
            if (it == data_.end() || it->second.dbReactor == nullptr || display_dialog_ == false)
                dlg_->ShowWindow(SW_HIDE);
            else if (dlg_)
                dlg_->ShowWindow(SW_SHOW);
        };
        doc_reactor_->ops_[_T("documentToBeDestroyed")] = [&](const TCHAR*, void* data) {
            auto pDoc = (AcApDocument*)data;
            detachDbReactor(pDoc->database());
            data_.erase(pDoc);
            objDisplay(pDoc->fileName(), _T("DOC DESTORY"), NULL);
        };
    }

    void modeless_dialog::detach() { 
        delete edit_reactor_;
        edit_reactor_ = nullptr;

        delete doc_reactor_;
        doc_reactor_ = nullptr;
    }

    void modeless_dialog::attachDbReactor(AcDbDatabase* pDb) {
        if (display_dialog_) {
            auto db = acdbHostApplicationServices()->workingDatabase();
            assert(pDb == db);
            AcDbDatabase* pCurDb = curDoc()->database();
            assert(pCurDb == pDb);
            acDocManager->lockDocument(curDoc(), AcAp::kWrite);

            auto& dd = data_[curDoc()];
            if (dd.dbReactor == nullptr) {
                dd.dbReactor = new MySimpleDbReactor;
                dd.dbReactor->ops_[_T("objectModified")] = [&](const TCHAR*, void* data) {
                    std::pair< const AcDbDatabase*, const AcDbObject* >* pd = (std::pair< const AcDbDatabase*, const AcDbObject* >*)(data);
                    objDisplay(acDocManager->document(pd->first)->fileName(), _T("modified"), pd->second);
                };
                dd.dbReactor->ops_[_T("objectErased")] = [&](const TCHAR*, void* data) {
                    typedef std::tuple<const AcDbDatabase*, const AcDbObject*, Adesk::Boolean> td_type;
                    td_type* td = (td_type*)(data);
                    TCHAR buf[40] = {};
                    _stprintf_s(buf, _T("%serased"), (std::get<2>(*td) ? _T("") : _T("not ")));
                    objDisplay(acDocManager->document(std::get<0>(*td))->fileName(), buf, std::get<1>(*td));
                };
                db->addReactor(dd.dbReactor);
                acutPrintf(_T("\nAttached MySimpleDbReactor to the current database.\n"));
            }
            else {
                acutPrintf(_T("\It has already MySimpleDbReactor attached.\n"));
            }

            acDocManager->unlockDocument(curDoc());
            acedPostCommandPrompt();
        }
    }

    void modeless_dialog::detachDbReactor(AcDbDatabase* pDb) {
        acDocManager->lockDocument(curDoc(), AcAp::kWrite);

        auto& dd = data_[acDocManager->document(pDb)];
        if (dd.dbReactor)
        {
            pDb->removeReactor(dd.dbReactor);
            delete dd.dbReactor;
            dd.dbReactor = nullptr;
            acutPrintf(_T("\nDetached DbReactor from the current database.\n"));
        }
        acDocManager->unlockDocument(curDoc());

        acedPostCommandPrompt();
    }

    void modeless_dialog::startDlg() {
        if (!dlg_) {
            assert(acedGetAcadFrame() == CWnd::FromHandle(adsw_acadMainWnd()));
            dlg_ = new MySimpleModelessDialog(acedGetAcadFrame());
            auto b = dlg_->Create(IDD_SIMPLE_MODELESS_DIALOG);
            assert(b == TRUE);
        }
        dlg_->ShowWindow(SW_SHOW);
    }

    void modeless_dialog::endDlg() {
        if (dlg_) {
            dlg_->DestroyWindow();
            delete dlg_;
            dlg_ = nullptr;
        }
    }

    void modeless_dialog::objDisplay(const TCHAR* file, const TCHAR* pWhat, const AcDbObject* pDbObj) {
        if (!dlg_)
            return;
        TCHAR hstr[20];
        TCHAR idstr[20];
        const TCHAR* pClsName = NULL;
        if (pDbObj)
        {
            AcDbObjectId id = pDbObj->objectId();
            AcDbHandle h;
            pClsName = pDbObj->isA()->name();

            pDbObj->getAcDbHandle(h);
            h.getIntoAsciiBuffer(hstr);

            acutPrintf(_T("\nDbReactor: obj %s: ")
                _T("class %s, id %lx, handle %s.\n"),
                pWhat, pClsName, id, hstr);

            _stprintf_s(idstr, _countof(idstr), _T("0x%08")PRIx64, id.handle().operator Adesk::UInt64());
        }
        else
        {
            pClsName = _T("");
            _tcscpy_s(hstr, _countof(hstr), _T(""));
            _tcscpy_s(idstr, _countof(idstr), _T(""));
        }

        dlg_->SetDlgItemText(IDC_EDIT_FILE, file);
        dlg_->SetDlgItemText(IDC_WHAT, pWhat);
        dlg_->SetDlgItemText(IDC_EDIT_CLASS, pClsName);
        dlg_->SetDlgItemText(IDC_EDIT_OBJID, idstr);
        dlg_->SetDlgItemText(IDC_EDIT_HANDLE, hstr);

        //enable UNDO dialog button only if UNDO not already underway and
        //either an entity is being generated or modified.
        auto& dd = data_[curDoc()];
        if (pDbObj && pDbObj->isKindOf(AcDbEntity::desc()) && !dd.undoCmd)
            dd.undoBtnAvailable = TRUE;
    }
}