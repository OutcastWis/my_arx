#pragma once
#include "icmd.h"

#include <map>

#include "MySimpleDocReactor.h"


struct CRectInfo {
    AcGePoint3d     m_topLeftCorner;        // First point selection.
    double          m_first = 0;            // First Chamfer distance. 切角的长
    double          m_second = 0;           // Second Chamfer distance. 切角的宽
    double          m_bulge = 0;            // Bulge value. 
    double          m_elev = 0;             // WCS, Elevation.
    double          m_thick = 0;            // Thickness.
    double          m_width = 0;            // Width.
    double          m_radius = 0;           // Fillet radius.
    bool            m_cornerTreatment=false;  // Filleting or chamfering.
    bool            m_elevHandSet=false;
    // m_chamfDirUnitVec is the vector direction of the chamfer of the
    // top left corner of the rectangle. The chamfer opposite the top left
    // corner will have the same vector direction and the ones on the
    // alternate corners will be perpendicular to this one.
    AcGeVector3d    m_chamfDirUnitVec;
};

namespace wzj {
    class data_per_doc : public icmd
    {
    public:
        static data_per_doc* instance() {
            static data_per_doc one;
            return &one;
        }

        const TCHAR* name() const override {
            return _T("data_per_doc");
        }

        void init_impl() override;

        void stop_impl() override;

        CRectInfo& docData(AcApDocument* pDoc);
        CRectInfo& docData();

        void clear();

        void make_reactor();
    private:
        MySimpleDocReactor * doc_reactor_; // 响应documentToBeDestroyed
        std::map<AcApDocument*, CRectInfo> data_;
    };
}

