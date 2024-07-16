#include "pch.h"
#include "data_per_doc.h"

#include <geassign.h>

#include "MyRectangleJig.h"

namespace wzj {
    static data_per_doc* global_one = data_per_doc::instance();
    const double ZERO = 1e-10;
    const double PI = 3.14159265358979323846;

    namespace detail {

        bool get_input() {
            CRectInfo& plineInfo = global_one->docData();

            TCHAR keyWord[10] = {};
            bool terminated = false;
            int stat;
            while (!terminated) {
                // Main prompt for user input.
                //
                acedInitGet(RSG_NONULL, _T("Chamfer Elevation Fillet Thickness Width"));
                if ((stat = acedGetPoint(NULL,
                    _T("\nChamfer/Elevation/Fillet/Thickness/Width/<First corner>: "),
                    asDblArray(plineInfo.m_topLeftCorner))) == RTKWORD)
                {
                    acedGetInput(keyWord);
                }
                else {
                    if (stat == RTCAN)
                        terminated = TRUE;
                    break;
                }
                switch (keyWord[0])
                {
                    // Chamfer;
                case _T('C'):
                    // Retrieve the first chamfer distance. 
                    acutPrintf(_T("\nFirst chamfer distance for rectangles <%.16q0>: "),
                        plineInfo.m_first == 0.0 ? plineInfo.m_radius : plineInfo.m_first);
                    if ((stat = acedGetDist(NULL, NULL, &plineInfo.m_first)) == RTCAN)
                    {
                        terminated = TRUE;
                        break;
                    }
                    else if (stat == RTNONE && plineInfo.m_first == 0.0)
                        plineInfo.m_second = plineInfo.m_radius;

                    // Retrieve the second chamfer distance. 
                    acutPrintf(_T("\nSecond chamfer distance for rectangles <%.16q0>: "),
                        plineInfo.m_second == 0.0
                        ? plineInfo.m_first : plineInfo.m_second);
                    if ((stat = acedGetDist(NULL, NULL, &plineInfo.m_second)) == RTCAN)
                    {
                        plineInfo.m_first = 0.0;
                        plineInfo.m_second = 0.0;
                        terminated = TRUE;
                    }
                    else {
                        if (stat == RTNONE && plineInfo.m_second == 0.0)
                            plineInfo.m_second = plineInfo.m_first;
                        // If we actually set the chamfer distances, then zero out the
                        // radius and bulge.
                        //
                        plineInfo.m_radius = 0.0;
                        plineInfo.m_bulge = 0.0;
                    }
                    break;
                    // Elevation;
                case _T('E'):
                    // Retrieve the radius to apply to the filleting of the corners. 
                    // 
                    acutPrintf(_T("\nElevation for rectangles <%.16q0>: "),
                        plineInfo.m_elev);
                    if ((stat = acedGetDist(NULL, NULL, &plineInfo.m_elev)) == RTCAN)
                        terminated = TRUE;
                    plineInfo.m_elevHandSet = (plineInfo.m_elev == 0.0) ? FALSE : TRUE;
                    break;
                    // Fillet;
                case _T('F'):
                    // Retrieve the radius to apply to the filleting of the corners. 
                    // If the user has previously used the chamfer, then use the 
                    // first disance as the default for the radius.
                    acutPrintf(_T("\nFillet radius for rectangles <%.16q0>: "),
                        plineInfo.m_radius == 0.0
                        ? plineInfo.m_first : plineInfo.m_radius);
                    if ((stat = acedGetDist(NULL, NULL, &plineInfo.m_radius)) == RTCAN)
                    {
                        terminated = TRUE;
                    }
                    else {
                        if (stat == RTNONE && plineInfo.m_radius == 0.0)
                            plineInfo.m_radius = plineInfo.m_first;

                        plineInfo.m_second = plineInfo.m_first = plineInfo.m_radius;

                        // Bulge is tangent of 1/4 of the included angle.
                        // We'll assume normal[Z] > 0. & clock wise for now, 
                        // hence the '-'.
                        plineInfo.m_bulge = -tan(PI / 8.0);
                    }
                    break;
                    // Thickness;
                case _T('T'):
                    // Retrieve the thickness to apply to the polyline. 
                    // 
                    acutPrintf(_T("\nThickness for rectangles <%.16q0>: "),
                        plineInfo.m_thick);
                    if ((stat = acedGetDist(NULL, NULL, &plineInfo.m_thick)) == RTCAN)
                    {
                        terminated = TRUE;
                    }
                    break;
                    // Width;
                case _T('W'):
                    // Retrieve the width to apply to the polyline. 
                    // 
                    acutPrintf(_T("\nWidth for rectangles <%.16q0>: "),
                        plineInfo.m_width);
                    if ((stat = acedGetDist(NULL, NULL, &plineInfo.m_width)) == RTCAN)
                    {
                        terminated = TRUE;
                    }
                    break;
                    // Just in case;
                default:
                    terminated = TRUE;
                    break;
                }
            }

            return terminated;
        }

        void create_rect() {
            CRectInfo& plineInfo = global_one->docData();

            plineInfo.m_elevHandSet = plineInfo.m_elev != 0.0;

            // Since it looks quite strange to have orthomode on while trying to draw a
            // rectangle, we'll temporarily turn it off. Remembering the current setting
            // and resetting it when we leave.
            struct resbuf rb;
            acedGetVar(_T("ORTHOMODE"), &rb);
            int oldOrthoMode = rb.resval.rint;
            rb.resval.rint = 0;
            acedSetVar(_T("ORTHOMODE"), &rb);
            // 显示信息
            if ((plineInfo.m_first != 0.0 && plineInfo.m_second != 0 && plineInfo.m_radius == 0.0) ||
                (plineInfo.m_elev != 0) || (plineInfo.m_radius != 0) || (plineInfo.m_thick != 0) ||
                (plineInfo.m_width != 0))
            {
                acutPrintf(_T("\nRectangle modes:  "));
                if (plineInfo.m_first != 0.0 && plineInfo.m_second != 0. && plineInfo.m_radius == 0.0)
                {
                    acutPrintf(_T("Chamfer=%.16q0 x %.16q0 "), plineInfo.m_first, plineInfo.m_second);
                }
                if (plineInfo.m_elev != 0.)
                    acutPrintf(_T("Elevation=%.16q0  "), plineInfo.m_elev);
                if (plineInfo.m_radius != 0.)
                    acutPrintf(_T("Fillet=%.16q0  "), plineInfo.m_radius);
                if (plineInfo.m_thick != 0.)
                    acutPrintf(_T("Thickness=%.16q0  "), plineInfo.m_thick);
                if (plineInfo.m_width != 0.)
                    acutPrintf(_T("Width=%.16q0  "), plineInfo.m_width);
                acutPrintf(_T("\n"));
            }
            // 
            if (!get_input()) {
                if (plineInfo.m_first != 0.0) {
                    // If we are treating the corners, then calculate the unit vector
                    // of the corners. Note for filleting the angle is 45 degrees. 
                    plineInfo.m_chamfDirUnitVec = AcGeVector3d(plineInfo.m_second, plineInfo.m_first, plineInfo.m_elev);
                    plineInfo.m_chamfDirUnitVec.normalize();
                }

                // Now start up the jig to interactively get the opposite corner.
                MyRectangleJig* pJig = new MyRectangleJig();
                pJig->doRectangle();
                delete pJig;
            }
        }
    }

    void data_per_doc::init_impl()
    {
        acedRegCmds->addCommand(_T("MY_COMMNAD_DATAPD"), _T("GLOBAL_DATAPD_CREATE"), _T("LOCAL_DATAPD_CREATE"), ACRX_CMD_MODAL, &detail::create_rect);

        make_reactor();
    }

    void data_per_doc::stop_impl()
    {
        acedRegCmds->removeGroup(_T("MY_COMMNAD_DATAPD"));

        clear();
    }

    CRectInfo& data_per_doc::docData(AcApDocument* pDoc)
    {
        auto i = data_.find(pDoc);
        if (i == data_.end())
            return data_[pDoc];
        else
            return (*i).second;
    }

    CRectInfo& data_per_doc::docData()
    {
        return docData(acDocManager->curDocument());
    }

    void data_per_doc::clear()
    {
        delete doc_reactor_;
        doc_reactor_ = nullptr;
    }

    void data_per_doc::make_reactor()
    {
        doc_reactor_ = new MySimpleDocReactor;
        doc_reactor_->ops_[_T("documentToBeDestroyed")] = [&](const TCHAR*, void* data) {
            data_.erase((AcApDocument*)(data));
        };
    }

}