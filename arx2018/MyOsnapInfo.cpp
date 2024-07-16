#include "pch.h"
#include "MyOsnapInfo.h"

ACRX_NO_CONS_DEFINE_MEMBERS(MyOsnapInfo, AcDbCustomOsnapInfo);


Acad::ErrorStatus MyOsnapCurveInfo::getOsnapInfo(AcDbEntity* pickedObject, Adesk::GsMarker gsSelectionMark, const AcGePoint3d& pickPoint, 
    const AcGePoint3d& lastPoint, const AcGeMatrix3d& viewXform, AcArray<AcGePoint3d>& snapPoints, 
    AcArray<int>& geomIdsForPts, AcArray<AcGeCurve3d*>& snapCurves, AcArray<int>& geomIdsForLines)
{
    assert(pickedObject->isKindOf(AcDbCurve::desc()));
    AcDbCurve* pCurve = (AcDbCurve*)pickedObject;

    double startParam, endParam;
    auto es = pCurve->getStartParam(startParam);
    es = pCurve->getEndParam(endParam);
    // 获取1/3处弧上坐标
    AcGePoint3d pt;
    es = pCurve->getPointAtParam(startParam + ((endParam - startParam) / 3), pt);
    assert(Acad::eOk == es);
    snapPoints.append(pt);
    // 获取2/3处弧上坐标
    es = pCurve->getPointAtParam(startParam + ((endParam - startParam) * 2 / 3), pt);
    assert(Acad::eOk == es);
    snapPoints.append(pt);

    if (pCurve->isClosed())
    {
        es = pCurve->getStartPoint(pt);
        assert(Acad::eOk == es);
        snapPoints.append(pt);
    }

    return Acad::eOk;
}

Acad::ErrorStatus MyOsnapPolylineInfo::getOsnapInfo(AcDbEntity* pickedObject, Adesk::GsMarker gsSelectionMark, const AcGePoint3d& pickPoint, 
    const AcGePoint3d& lastPoint, const AcGeMatrix3d& viewXform, AcArray<AcGePoint3d>& snapPoints,
    AcArray<int>& geomIdsForPts, AcArray<AcGeCurve3d*>& snapCurves, AcArray<int>& geomIdsForLines)
{
    assert(pickedObject->isKindOf(AcDbPolyline::desc()));
    AcDbPolyline* pPline = (AcDbPolyline*)pickedObject;

    Acad::ErrorStatus es;
    if (m_bSnapToSegments) {
        // Snap to a third of each of the segments
        unsigned int numSegs = pPline->numVerts() - 1;
        AcGeLineSeg3d segLn;
        AcGeCircArc3d segArc;
        double startParam, endParam, newParam, dist;
        AcGePoint3d pt;

        for (unsigned int idx = 0; idx < numSegs; idx++)
        {
            switch (pPline->segType(idx))
            {
            case AcDbPolyline::kLine:
                es = pPline->getLineSegAt(idx, segLn);
                startParam = segLn.paramOf(segLn.startPoint());
                endParam = segLn.paramOf(segLn.endPoint());
                snapPoints.append(segLn.evalPoint(startParam + ((endParam - startParam) / 3)));
                snapPoints.append(segLn.evalPoint(startParam + ((endParam - startParam) * 2 / 3)));
                break;
            case AcDbPolyline::kArc:
                es = pPline->getArcSegAt(idx, segArc);
                startParam = segArc.paramOf(segArc.startPoint());
                endParam = segArc.paramOf(segArc.endPoint());
                dist = segArc.length(startParam, endParam);
                newParam = segArc.paramAtLength(startParam, dist / 3);
                snapPoints.append(segArc.evalPoint(newParam));
                newParam = segArc.paramAtLength(startParam, dist * 2 / 3);
                snapPoints.append(segArc.evalPoint(newParam));
                break;
            default:
                break;
            }
        }
    }
    else {
        double endParam;
        AcGePoint3d pt;
        double dist;

        es = pPline->getEndParam(endParam);
        es = pPline->getDistAtParam(endParam, dist);
        es = pPline->getPointAtDist(dist / 3, pt);
        assert(Acad::eOk == es);
        snapPoints.append(pt);
        es = pPline->getPointAtDist(dist * 2 / 3, pt);
        assert(Acad::eOk == es);
        snapPoints.append(pt);
        if (pPline->isClosed())
        {
            es = pPline->getStartPoint(pt);
            snapPoints.append(pt);
        }
    }
    return Acad::eOk;
}
