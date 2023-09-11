#pragma once

#include <tchar.h>

namespace wzj {


    class icmd
    {
    public:
        void init();

        void stop();

        bool is_start() const { return start_; }

    public:

        virtual const TCHAR* name() const = 0;
        virtual void init_impl() = 0;
        virtual void stop_impl() = 0;

    protected:
        bool start_;
    };
}

