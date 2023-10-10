#pragma once

#include "icmd.h"
#include "MySimpleEditReactor.h"

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

        void clear();

        void make_reactor();
    private:
        MySimpleEditReactor* edit_reactor_;
        
    };
}

