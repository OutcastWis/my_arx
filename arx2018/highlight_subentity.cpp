#include "pch.h"
#include "command.h"

#include <acedsubsel.h>


void AddtoIdArray(ads_name eName, AcDbObjectIdArray& ids, const TCHAR* prefix=_T("")) {
    AcDbObjectId id;
    if (Acad::eOk == acdbGetObjectId(id, eName))
    {
        TCHAR hdl[16] = { };
        id.handle().getIntoAsciiBuffer(hdl, 16);
        ads_printf(_T("%s, hdl = %s\n"), prefix, hdl);
        ids.append(id);
    }
}


AcDbObjectIdArray ContainerIdsAndEntity(ads_name sset, ads_point pick_point, Adesk::GsMarker* gs)
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


void highlight_subentity() {
    ads_name sset = {};
    ads_point pick_point = {};

    int r = RTNORM;
    //Pick entity
    if (RTNORM ==(r =  acedSSGet(_T("_:s:n"), NULL, NULL, NULL, sset)))
    {
        Adesk::GsMarker gs;
        //Create Container and Entity ObjectIdArray
        AcDbObjectIdArray ids;
        ids = ContainerIdsAndEntity(sset, pick_point, &gs);
        // Retrieve the sub-entity path for this entity
        AcDbSubentId subid = AcDbSubentId(AcDb::kNullSubentType, 0);
        AcDbFullSubentPath path(ids, subid);
        // Open the outermost container
        AcDbEntity* pOutermost;
        if (Acad::eOk == acdbOpenObject(pOutermost, ids[0], AcDb::kForRead))
        {
            //Highlight the nested entity
           // pOutermost->highlight(path);
            pOutermost->close();
        }
        ads_ssfree(sset);

        auto rs = acedSSSubentAdd(path, NULL, sset);
        assert(rs == RTNORM);
        rs = ads_sssetfirst(sset, NULL);
        assert(rs == RTNORM);
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