#pragma once
#include "icmd.h"

namespace wzj {
    class jig : public icmd
    {
    public:
        static jig* instance() {
            static jig one;
            return &one;
        }

        const TCHAR* name() const override {
            return _T("jig");
        }

        void init_impl() override;

        void stop_impl() override;

    private:
    };
}

