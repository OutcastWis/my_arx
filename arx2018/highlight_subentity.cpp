#include "pch.h"

#include <acedsubsel.h>
#include <geassign.h>

#include "highlight_subentity.h"

#include "command.h"

namespace wzj {
    namespace detail {
        void AddtoIdArray(ads_name eName, AcDbObjectIdArray& ids, const TCHAR* prefix = _T("")) {
            AcDbObjectId id;
            if (Acad::eOk == acdbGetObjectId(id, eName))
            {
                TCHAR hdl[16] = { };
                id.handle().getIntoAsciiBuffer(hdl, 16);
                ads_printf(_T("%s, hdl = %s\n"), prefix, hdl);
                ids.append(id);
            }
        }

        void highlight(AcDb::SubentType st) {
            ads_name sset = {};
            ads_point pick_point = {};

            int r = RTNORM;
            AcDbObjectIdArray ids1;
            //Pick entity
            if (RTNORM == (r = acedSSGet(_T("_:s:n"), NULL, NULL, NULL, sset)))
            {
                Adesk::GsMarker gs;
                //Create Container and Entity ObjectIdArray
                AcDbObjectId pick_id;
                
                // 此时, ids对应的应该是 BlockReference, BlockReference,..., Entity
                ids1 = ContainerIdsAndEntity(sset, pick_point, &gs, &pick_id);

                AcDbEntity* pOutermost;
                if (Acad::eOk == acdbOpenObject(pOutermost, ids1[0], AcDb::kForRead))
                {
                    if (st == AcDb::kNullSubentType) {
                        AcDbSubentId subid = AcDbSubentId(st, gs);
                        AcDbFullSubentPath path(ids1, subid);
                        pOutermost->highlight(path);
                    }
                    else 
                    {
                        // 对于face, edge, vertex, mline需要使用getSubentPathsAtGsMarker
                        AcGeMatrix3d xform;
                        int numIds;
                        AcDbFullSubentPath* subentIds;
                        pOutermost->getSubentPathsAtGsMarker(st, gs, asPnt3d(pick_point), xform, numIds, subentIds,
                            ids1.length() - 1, ids1.reverse().asArrayPtr()); // 这里需要注意, 获取的subentIds中的GS mark不一定和传入的gs一致
                        for (int i = 0; i < numIds; ++i) 
                            pOutermost->highlight(subentIds[i]);    
                        delete[]subentIds;
                    }
                    pOutermost->close();
                }

                ads_ssfree(sset);
            }

            // 使用acedNEntSelP
            if (r != RTNORM)
                return;

            ads_name ent;
            ads_matrix mat;
            struct resbuf* insStack;
            AcDbObjectIdArray ids2;
            if (RTNORM == (r = acedNEntSelP(NULL, ent, pick_point, TRUE, mat, &insStack))) {
                AddtoIdArray(ent, ids2, _T("acedNEntSelP"));

                struct resbuf* res = insStack;
                while (res) {
                    assert(res->restype == RTENAME);
                    AddtoIdArray(res->resval.rlname, ids2, _T("acedNEntSelP"));
                    res = res->rbnext;
                }
                if (insStack)
                    ads_relrb(insStack);
            }
            // 此时ids1和ids2应该相反. ids2为由内到外, ids1为由外到内
            assert(ids1.length() == ids2.length());
            for (int i = 0; i < ids1.length(); ++i) {
                assert(ids1[i] == ids2[ids1.length() - 1 - i]);
            }
        }

        void highlight_subentity() {
            highlight(AcDb::kNullSubentType);
        }

        void highlight_face() {
            highlight(AcDb::kFaceSubentType);
        }
        
        void highlight_edge() {
            highlight(AcDb::kEdgeSubentType);
        }
    }


    void highlight_subentity::init_impl() {
        acedRegCmds->addCommand(_T("WZJ_COMMAND_HLSUB"), _T("GLOBAL_HL_ENT"), _T("LOCAL_HL_ENT"), ACRX_CMD_MODAL, &detail::highlight_subentity);
        acedRegCmds->addCommand(_T("WZJ_COMMAND_HLSUB"), _T("GLOBAL_HL_FACE"), _T("LOCAL_HL_FACE"), ACRX_CMD_MODAL, &detail::highlight_face);
        acedRegCmds->addCommand(_T("WZJ_COMMAND_HLSUB"), _T("GLOBAL_HL_EDGE"), _T("LOCAL_HL_EDGE"), ACRX_CMD_MODAL, &detail::highlight_edge);

    }

    void highlight_subentity::stop_impl() {
        acedRegCmds->removeGroup(_T("WZJ_COMMAND_HLSUB"));

    }
}