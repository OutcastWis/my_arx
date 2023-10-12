#include "pch.h"
#include "osnap.h"

#include "command.h"

namespace wzj {

    static osnap* global_one = osnap::instance();

    namespace detail {

        void snapToPolyline() {
            if (getYorN(_T("Snap to polyline segments"))) {
                global_one->setSnapToSegments(true);
            }
            else
                global_one->setSnapToSegments(false);
        }
    }

    void osnap::init_impl() {

        acedRegCmds->addCommand(_T("MY_COMMAND_OSNAP"), _T("GLOBAL_STP"), _T("LOCAL_STP"), ACRX_CMD_TRANSPARENT, &detail::snapToPolyline);

        MyOsnapInfo::rxInit();
        acrxBuildClassHierarchy();

        osnap_entity_ = new MyOsnapEntityInfo;
        osnap_curve_ = new MyOsnapCurveInfo;
        osnap_pline_ = new MyOsnapPolylineInfo;

        AcDbEntity::desc()->addX(MyOsnapEntityInfo::desc(), osnap_entity_);
        AcDbCurve::desc()->addX(MyOsnapCurveInfo::desc(), osnap_curve_);
        AcDbPolyline::desc()->addX(MyOsnapPolylineInfo::desc(), osnap_pline_);

        acdbCustomOsnapManager()->addCustomOsnapMode(&mode_);
        acdbCustomOsnapManager()->activateOsnapMode(_T("_WZJOsnapmode")); // ¶ÔÓ¦AcDbCustomOsnapModeµÄmode string
    }

    void osnap::stop_impl() {
        auto obj = AcDbEntity::desc()->delX(MyOsnapEntityInfo::desc());
        assert(obj == osnap_entity_);
        obj = AcDbCurve::desc()->delX(MyOsnapCurveInfo::desc());
        assert(obj == osnap_curve_);
        obj = AcDbPolyline::desc()->delX(MyOsnapPolylineInfo::desc());
        assert(obj == osnap_pline_);

        delete osnap_entity_;
        osnap_entity_ = nullptr;
        delete osnap_curve_;
        osnap_curve_ = nullptr;
        delete osnap_pline_;
        osnap_pline_ = nullptr;

        acdbCustomOsnapManager()->removeCustomOsnapMode(&mode_);

        deleteAcRxClass(MyOsnapInfo::desc());

        acedRegCmds->removeGroup(_T("MY_COMMAND_OSNAP"));
    }

    void osnap::setSnapToSegments(bool v)
    {
        if (osnap_pline_)
            osnap_pline_->setSnapToSegments(v);
    }
}