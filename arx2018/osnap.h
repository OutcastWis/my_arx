#pragma once
#include "icmd.h"

#include "MyOsnapInfo.h"
#include "MyOsnapMode.h"

namespace wzj {
    class osnap : public icmd
    {
    public:
        static osnap* instance() {
            static osnap one;
            return &one;
        }

        const TCHAR* name() const override {
            return _T("osnap");
        }

        void init_impl() override;

        void stop_impl() override;

        void setSnapToSegments(bool v);
    private:
        MyOsnapEntityInfo* osnap_entity_;
        MyOsnapCurveInfo* osnap_curve_;
        MyOsnapPolylineInfo* osnap_pline_;
        MyOsnapMode mode_;
    };
}

