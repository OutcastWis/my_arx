#include "pch.h"
#include "custom_geo.h"

#include <geassign.h>
#include <acestext.h>
#include <dbregion.h>
#include <dbbody.h>

#include "command.h"
//#include "MyCustomWorldDraw.h"
#include "MyCustomDrawEntity.h"

namespace wzj {

    namespace detail {

        // Select faces of solids or regions. It will create a region or body
        // and return its id
        bool getSubEntPtr(AcDbObjectId* objId) {
            ads_name sset = { };
            if (ads_ssget(NULL, NULL, NULL, NULL, sset) != RTNORM) // single-point selection
                return false;

            AcDbObjectId entId;
            ads_point pick_point;
            Adesk::GsMarker gs;
            auto ids = ContainerIdsAndEntity(sset, pick_point, &gs, &entId);

            AcDbEntity* pOutermost = nullptr;
            if (acdbOpenAcDbEntity(pOutermost, ids[0], AcDb::kForRead) != Acad::eOk)
                return false;

            AcGeMatrix3d xform;
            int numIds;
            AcDbFullSubentPath* subentIds;
            auto es = pOutermost->getSubentPathsAtGsMarker(AcDb::kFaceSubentType, gs, asPnt3d(pick_point), xform,
                numIds, subentIds, ids.length() - 1, ids.reverse().asArrayPtr());
            if (es != Acad::eOk) {
                ads_printf(_T("\nFailed for gsMarkerToSubentIds: %s"), acadErrorStatusText(es));
                pOutermost->close();
                return false;
            }

            if (numIds == 0) {
                ads_printf(_T("No face subent found"));
                return false;
            }

            // select face
            int i = 0;
            pOutermost->highlight(subentIds[i]);
            while (!getYorN(_T("\nIs this the face <Yes>/No: "))) {
                pOutermost->unhighlight(subentIds[i]);
                if (++i >= numIds)
                    i = 0;
                pOutermost->highlight(subentIds[i]);
            }
            
            // 
            pOutermost->unhighlight(subentIds[i]);
            AcDbEntity* pEnt = pOutermost->subentPtr(subentIds[i]);
            delete[] subentIds;
            if (pEnt == NULL) {
                ads_printf(_T("\nFailed: subentPtr"));
                pOutermost->close();
                return false;
            }
            pOutermost->close();

            add_to_model_space(*objId, pEnt);
            pEnt->close();
            return true;
        }

        void shadeface() {
            AcDbObjectId id;
            //while (getSubEntPtr(&id)) {
            //    AcDbEntity* pEntity = NULL;
            //    if (acdbOpenAcDbEntity(pEntity, id, AcDb::kForRead) != Acad::eOk)
            //        continue;

            //    if (!pEntity->isKindOf(AcDbRegion::desc()) && !pEntity->isKindOf(AcDbBody::desc())) {
            //        ads_printf(_T("\nNot a region or a body"));
            //        ads_printf(_T("\nThe entity is %s\n"), (pEntity->isA())->name());
            //        pEntity->close();
            //        return;
            //    }

            //    MyCustomWorldDraw* wd = new MyCustomWorldDraw;
            //    wd->setDeviation(0.01); // 0.005 <= value <= 0.1
            //    //  Call the entity's worldDraw with MyCustomWorldDraw
            //    pEntity->worldDraw(wd);
            //    //  set the FaceData by entering the colorIndex.  All faces
            //    //  will get this color.
            //    wd->shellData()->setFaceData(1);

            //    pEntity->upgradeOpen();
            //    pEntity->erase();
            //    pEntity->downgradeOpen();
            //    pEntity->close();

            //    MyCustomDrawEntity* cde = new MyCustomDrawEntity;
            //    cde->mpShellData = wd->shellData();
            //    AcDbObjectId cId;
            //    add_to_model_space(cId, cde);
            //    cde->close();

            //    delete wd;
            //}
        }
    }


    void custom_geo::init_impl() {
        acedRegCmds->addCommand(_T("MY_COMMAND_CGEO"), _T("GOBAL_CGEO"), _T("LOCAL_CGEO"), ACRX_CMD_MODAL, &detail::shadeface);
    }

    void custom_geo::stop_impl() {
        acedRegCmds->removeGroup(_T("MY_COMMAND_CGEO"));
    }
}