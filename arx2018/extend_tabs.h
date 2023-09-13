#pragma once

#include "icmd.h"
#include "MyOptionsTab.h"

namespace wzj {
    class extend_tabs : public icmd
    {
    public:

        static extend_tabs* instance() {
            static extend_tabs one;
            return &one;
        }

        const TCHAR* name() const override {
            return _T("extend_tabs");
        }

        void init_impl() override;

        void stop_impl() override;

        void add_tab(void *appId);

    private:
        MyOptionsTab* m_tab;
    };
}

