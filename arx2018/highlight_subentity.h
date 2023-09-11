#pragma once

#include "icmd.h"

namespace wzj {
    class highlight_subentity : public icmd
    {
    public:
        static highlight_subentity* instance() {
            static highlight_subentity one;
            return &one;
        }

        const TCHAR* name() const override{
            return _T("highlight_subentity");
        }

        void init_impl() override;

        void stop_impl() override;
    };
}

