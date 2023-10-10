#pragma once

#include "icmd.h"

namespace wzj {
    class clone_work : public icmd
    {
    public:
        static clone_work* instance() {
            static clone_work one;
            return &one;
        }

        const TCHAR* name() const override {
            return _T("clone_work");
        }

        void init_impl() override;

        void stop_impl() override;
    private:
    };
}

