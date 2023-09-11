#include "pch.h"
#include "icmd.h"

namespace wzj{

    void icmd::init() {
        if (start_ == true) {
            ads_printf(_T("\n%s already started\n"), name());
            return;
        }

        start_ = true;
        ads_printf(_T("\n%s start\n"), name());

        init_impl();
    }

    void icmd::stop() {
        if (start_ == false) {
            ads_printf(_T("\n%s already stoped\n"), name());
            return;
        }

        stop_impl();

        start_ = false;
        ads_printf(_T("\n%s stop\n"), name());
    }

}