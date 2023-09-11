#include "pch.h"

#include <acedsubsel.h>
#include <geassign.h>

#include "highlight_subentity.h"



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

        // 返回路径, 从外到内. 最里面是被选中的对象本身
        AcDbObjectIdArray ContainerIdsAndEntity(ads_name sset, ads_point pick_point, Adesk::GsMarker* gs, AcDbObjectId* pick)
        {
            AcDbObjectIdArray ids;
            ads_name eName;
            AcDbObjectId id;
            Adesk::Int32 len = 0;
            acedSSLength(sset, &len);
            for (long i = 0; i < len; i++)
            {
                resbuf* rb = NULL;
                if (RTNORM == acedSSNameX(&rb, sset, i))
                {
                    resbuf* rbWalk = rb;
                    int cnt = 0;

                    // skip RTLB
                    rbWalk = rbWalk->rbnext;
                    // select method
                    ads_printf(_T("selectin method: %d\n"), rbWalk->resval.rint);
                    // picked entity's entity name
                    rbWalk = rbWalk->rbnext;
                    AddtoIdArray(rbWalk->resval.rlname, ids);
                    acdbGetObjectId(*pick, rbWalk->resval.rlname);
                    // GS maker
                    rbWalk = rbWalk->rbnext;
                    *gs = rbWalk->resval.rint;
                    // RTLB
                    rbWalk = rbWalk->rbnext;
                    // point descriptor
                    rbWalk = rbWalk->rbnext;
                    auto pt_desc = rbWalk->resval.rint;
                    // point on pick line (in WCS)
                    rbWalk = rbWalk->rbnext;
                    ads_point_set(rbWalk->resval.rpoint, pick_point);

                    while (NULL != rbWalk)
                    {
                        if (RTENAME == rbWalk->restype)
                        { // 这些对象, 按照嵌套深度, 由内至外
                            eName[0] = rbWalk->resval.rlname[0];
                            eName[1] = rbWalk->resval.rlname[1];
                            AddtoIdArray(eName, ids);
                        }
                        rbWalk = rbWalk->rbnext;
                    }
                    acutRelRb(rb);
                }
            }
            ids.reverse(); // 翻转, 为外部AcDbFullSubentPath使用
            return ids;
        }

        void highlight(AcDb::SubentType st) {
            ads_name sset = {};
            ads_point pick_point = {};

            int r = RTNORM;
            //Pick entity
            if (RTNORM == (r = acedSSGet(_T("_:s:n"), NULL, NULL, NULL, sset)))
            {
                Adesk::GsMarker gs;
                //Create Container and Entity ObjectIdArray
                AcDbObjectId pick_id;
                AcDbObjectIdArray ids;
                // 此时, ids对应的应该是 BlockReference, BlockReference,..., Entity
                ids = ContainerIdsAndEntity(sset, pick_point, &gs, &pick_id);

                AcDbEntity* pOutermost;
                if (Acad::eOk == acdbOpenObject(pOutermost, ids[0], AcDb::kForRead))
                {
                    if (st == AcDb::kNullSubentType) {
                        AcDbSubentId subid = AcDbSubentId(st, gs);
                        AcDbFullSubentPath path(ids, subid);
                        pOutermost->highlight(path);
                    }
                    else 
                    {
                        // 对于face, edge, vertex, mline需要使用getSubentPathsAtGsMarker
                        AcGeMatrix3d xform;
                        int numIds;
                        AcDbFullSubentPath* subentIds;
                        pOutermost->getSubentPathsAtGsMarker(st, gs, asPnt3d(pick_point), xform, numIds, subentIds,
                            ids.length() - 1, ids.reverse().asArrayPtr()); // 这里需要注意, 获取的subentIds中的GS mark不一定和传入的gs一致
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
            AcDbObjectIdArray ids;
            if (RTNORM == (r = acedNEntSelP(NULL, ent, pick_point, TRUE, mat, &insStack))) {
                AddtoIdArray(ent, ids, _T("acedNEntSelP"));

                struct resbuf* res = insStack;
                while (res) {
                    assert(res->restype == RTENAME);
                    AddtoIdArray(res->resval.rlname, ids, _T("acedNEntSelP"));
                    res = res->rbnext;
                }
                if (insStack)
                    ads_relrb(insStack);
            }
            // 此时俩者AddtoIdArray中输出的应该一致. 为由内到外
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