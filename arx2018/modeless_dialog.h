#pragma once

#include <aced.h>

#include "icmd.h"
#include "MySimpleModelessDialog.h"
#include "MySimpleDbReactor.h"
#include "MySimpleEditReactor.h"
#include "MySimpleDocReactor.h"

namespace wzj {

    class modeless_dialog : public icmd
    {
    public:
        static modeless_dialog* instance() {
            static modeless_dialog one;
            return &one;
        }

        const TCHAR* name() const override{
            return _T("modeless_dialog");
        }

        void init_impl() override;

        void stop_impl() override;
        // 初始化edit_reactor_, doc_reactor_
        void attach();
        // 卸载edit_reactor_, doc_reactor_
        void detach();
        // 根据display_dialog_, 关闭或者初始化db_reactor_
        void attachDbReactor(AcDbDatabase* pDb);
        // 卸载
        void detachDbReactor(AcDbDatabase* pDb);

        void startDlg();

        void endDlg();
        // dlg上显示最后一个变化的对象, 类,所在文件
        void objDisplay(const TCHAR* file, const TCHAR* pWhat, const AcDbObject* pDbObj);

    public:
        // false则所有都不显示. true则需要再当前文件存在dbReactor时才显示, 否则不显示
        bool display_dialog_;

        MySimpleModelessDialog* dlg_;
        MySimpleEditReactor* edit_reactor_;
        MySimpleDocReactor* doc_reactor_;

        struct DocData {
            bool undoBtnAvailable; 
            bool undoCmd; // true表示正在Undo命令中
            MySimpleDbReactor* dbReactor;
        };

        std::map<AcApDocument*, DocData> data_;
    };
}

