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

        void* appId_; // 仅在初始化时有效. 随后失效. 为acrxEntryPoint的第二个参数

        MyCmdContext* cc_;
        MyDefaultContext* dc_;
        MyEntityContext* ec_;
        MyLineContext* linec_;
        MyCircleContext* circlec_;

    };
}

