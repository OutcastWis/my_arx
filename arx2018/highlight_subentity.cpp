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

        // ����·��, ���⵽��. �������Ǳ�ѡ�еĶ�����
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
                        { // ��Щ����, ����Ƕ�����, ��������
                            eName[0] = rbWalk->resval.rlname[0];
                            eName[1] = rbWalk->resval.rlname[1];
                            AddtoIdArray(eName, ids);
                        }
                        rbWalk = rbWalk->rbnext;
                    }
                    acutRelRb(rb);
                }
            }
            ids.reverse(); // ��ת, Ϊ�ⲿAcDbFullSubentPathʹ��
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
                // ��ʱ, ids��Ӧ��Ӧ���� BlockReference, BlockReference,..., Entity
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
                        // ����face, edge, vertex, mline��Ҫʹ��getSubentPathsAtGsMarker
                        AcGeMatrix3d xform;
                        int numIds;
                        AcDbFullSubentPath* subentIds;
                        pOutermost->getSubentPathsAtGsMarker(st, gs, asPnt3d(pick_point), xform, numIds, subentIds,
                            ids.length() - 1, ids.reverse().asArrayPtr()); // ������Ҫע��, ��ȡ��subentIds�е�GS mark��һ���ʹ����gsһ��
                        for (int i = 0; i < numIds; ++i) 
                            pOutermost->highlight(subentIds[i]);    
                        delete[]subentIds;
                    }
                    pOutermost->close();
                }

                ads_ssfree(sset);
            }

            // ʹ��acedNEntSelP
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
            // ��ʱ����AddtoIdArray�������Ӧ��һ��. Ϊ���ڵ���
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