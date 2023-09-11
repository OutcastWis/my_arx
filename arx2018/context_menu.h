#pragma once

#include "icmd.h"

#include "MyCmdContext.h"
#include "MyDefaultContext.h"
#include "MyEditContext.h"

namespace wzj {
    class context_menu : public icmd
    {
    public:

        static context_menu* instance() {
            static context_menu one;
            return &one;
        }

        const TCHAR* name() const override {
            return _T("context_menu");
        }

        void set_data(void* appId) {
            appId_ = appId;
        }

        void init_impl() override;

        void stop_impl() override;

    private:

        void* appId_; // ���ڳ�ʼ��ʱ��Ч. ���ʧЧ. ΪacrxEntryPoint�ĵڶ�������

        MyCmdContext* cc_;
        MyDefaultContext* dc_;
        MyEntityContext* ec_;
        MyLineContext* linec_;
        MyCircleContext* circlec_;

    };
}

