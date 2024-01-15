#pragma once
#include "icmd.h"

namespace wzj {
    class custom_geo : public icmd
    {
    public:
        static custom_geo* instance() {
            static custom_geo one;
            return &one;
        }

        const TCHAR* name() const override {
            return _T("custom_geo");
        }

        void init_impl() override;

        void stop_impl() override;
    private:

    };
}

