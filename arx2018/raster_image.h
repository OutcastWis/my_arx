#pragma once

#include "icmd.h"

namespace wzj {

    class raster_image : public icmd
    {
    public:
        static raster_image* instance() {
            static raster_image one;
            return &one;
        }

        const TCHAR* name() const override {
            return _T("raster_image");
        }

        void init_impl() override;

        void stop_impl() override;
    };

}
