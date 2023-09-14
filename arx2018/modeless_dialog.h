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
        // ��ʼ��edit_reactor_, doc_reactor_
        void attach();
        // ж��edit_reactor_, doc_reactor_
        void detach();
        // ����display_dialog_, �رջ��߳�ʼ��db_reactor_
        void attachDbReactor(AcDbDatabase* pDb);
        // ж��
        void detachDbReactor(AcDbDatabase* pDb);

        void startDlg();

        void endDlg();
        // dlg����ʾ���һ���仯�Ķ���, ��,�����ļ�
        void objDisplay(const TCHAR* file, const TCHAR* pWhat, const AcDbObject* pDbObj);

    public:
        // false�����ж�����ʾ. true����Ҫ�ٵ�ǰ�ļ�����dbReactorʱ����ʾ, ������ʾ
        bool display_dialog_;

        MySimpleModelessDialog* dlg_;
        MySimpleEditReactor* edit_reactor_;
        MySimpleDocReactor* doc_reactor_;

        struct DocData {
            bool undoBtnAvailable; 
            bool undoCmd; // true��ʾ����Undo������
            MySimpleDbReactor* dbReactor;
        };

        std::map<AcApDocument*, DocData> data_;
    };
}

