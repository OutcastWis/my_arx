#pragma once

#include "icmd.h"

namespace wzj {

    class docman : public icmd
    {
    public:

        static docman* instance() {
            static docman one;
            return &one;
        }

        const TCHAR* name() const override {
            return _T("docman");
        }

        void init_impl() override;

        void stop_impl() override;

    };
}

