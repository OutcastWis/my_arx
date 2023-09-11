#pragma once

#include "icmd.h"

namespace wzj
{
    class ex_dict : public icmd
    {
    public:

        static ex_dict* instance() {
            static ex_dict one;
            return &one;
        }

        const TCHAR* name() const override{
            return _T("ex_dict");
        }

        void init_impl() override;

        void stop_impl() override;
    };
}