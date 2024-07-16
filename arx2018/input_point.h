#pragma once

#include "icmd.h"

namespace wzj {
    class input_point : public icmd
    {
    public:
        static input_point* instance() {
            static input_point one;
            return &one;
        }

        const TCHAR* name() const override {
            return _T("input_point");
        }

        void init_impl() override;

        void stop_impl() override;
    private:

    };
}