#pragma once
#include "icmd.h"

namespace wzj {
    class block_order : public icmd
    {
    public:
        static block_order* instance() {
            static block_order one;
            return &one;
        }

        const TCHAR* name() const override {
            return _T("block_order");
        }

        void init_impl() override;

        void stop_impl() override;

    private:

    };
}

