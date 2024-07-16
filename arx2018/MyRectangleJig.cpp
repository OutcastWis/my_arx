#include "pch.h"
#include "MyRectangleJig.h"

#include <dbxutil.h>
#include <geassign.h>

#include "data_per_doc.h"

#define MAKEOCSCOORD(x, y)      \
    {\
        auto tmpPoint = (y); \
        acdbWcs2Ecs(asDblArray(tmpPoint), asDblArray(tmpPoint), asDblArray(m_vecUnitZ), Adesk::kFalse); \
        x[0] = tmpPoint[0]; x[1] = tmpPoint[1]; \
    }


MyRectangleJig::MyRectangleJig()
{
    m_pLWPoly = new AcDbPolyline();
    samplerCorner = AcGePoint3d();
    CRectInfo& plineInfo = wzj::data_per_doc::instance()->docData();
    plineInfo.m_cornerTreatment = plineInfo.m_first != 0.0
        || plineInfo.m_second != 0.0
        || plineInfo.m_radius != 0.0;

   // 仅考虑Model Space
   /* if (inPaperSpace()) {
        m_vecUnitX = acdbHostApplicationServices()->workingDatabase()->pucsxdir();
        m_vecUnitY = acdbHostApplicationServices()->workingDatabase()->pucsydir();
    }
    else */{
        m_vecUnitX = acdbHostApplicationServices()->workingDatabase()->ucsxdir();
        m_vecUnitY = acdbHostApplicationServices()->workingDatabase()->ucsydir();
    }
    m_vecUnitZ = m_vecUnitX.crossProduct(m_vecUnitY);

    // addVertexAt需要传入ECS( Entity Coordinate System)下的点, 所有将UCS转为ECS
    acdbUcs2Ecs(asDblArray(plineInfo.m_topLeftCorner), asDblArray(m_TopLeftCorner), asDblArray(m_vecUnitZ), Adesk::kFalse);
    acdbUcs2Ecs(asDblArray(plineInfo.m_topLeftCorner), asDblArray(plineInfo.m_topLeftCorner), asDblArray(m_vecUnitZ), Adesk::kFalse);
    AcGePoint2d initPoint;
    initPoint = AcGePoint2d(m_TopLeftCorner[X], m_TopLeftCorner[Y]);

    // If the user has set the elev option from the main command prompt,
    // then this will be the default until the user again sets it to 0.0.
    // If however the user simply picks a point with or without an object
    // snap, then use the Z value of the first point picked.
    if (plineInfo.m_elevHandSet == TRUE)
        m_pLWPoly->setElevation(plineInfo.m_elev);
    else
        m_pLWPoly->setElevation(m_TopLeftCorner[Z]);

    // 圆角需要8个点, 非圆角4个点
    int vn = plineInfo.m_cornerTreatment ? 8 : 4;
    for (int i = 0; i < vn; i++)
        m_pLWPoly->addVertexAt(i, initPoint);


    m_pLWPoly->setNormal(m_vecUnitZ);       // 设置法向, 需要WCS
    m_pLWPoly->setClosed(Adesk::kTrue);
    m_pLWPoly->setThickness(plineInfo.m_thick);
    m_pLWPoly->setConstantWidth(plineInfo.m_width);
    // 设置线性比例缩放因子
    m_pLWPoly->setLinetypeScale(acdbHostApplicationServices()->workingDatabase()->celtscale());
    // 设置完毕后, 变成WCS, 为后面的移动做准备
    acdbEcs2Wcs(asDblArray(m_TopLeftCorner), asDblArray(m_TopLeftCorner), asDblArray(m_vecUnitZ), Adesk::kFalse);
    acdbEcs2Wcs(asDblArray(plineInfo.m_topLeftCorner), asDblArray(plineInfo.m_topLeftCorner), asDblArray(m_vecUnitZ), Adesk::kFalse);
}

AcEdJig::DragStatus MyRectangleJig::sampler()
{
    DragStatus stat = kNormal;

    setUserInputControls((UserInputControls)
        (AcEdJig::kNoNegativeResponseAccepted | AcEdJig::kNoZeroResponseAccepted)
    );
    
    setSpecialCursorType(kCrosshair);

    CRectInfo& plineInfo = wzj::data_per_doc::instance()->docData();
    stat = acquirePoint(m_BottomRightCorner, plineInfo.m_topLeftCorner);

    // m_BottomRightCorner投影到planeParallelToUCS
    AcGePlane planeParallelToUCS(m_TopLeftCorner, m_vecUnitZ);
    m_BottomRightCorner = m_BottomRightCorner.project(planeParallelToUCS, m_vecUnitZ);

    // If the newly acquired point is the same as it was
    // in the last sample, then we return kNoChange so that
    // AsdkRectangleJig::update() will not be called and the
    // last update call will be able to finish thus allowing
    // the polyline to fully elaborate.
    if (samplerCorner != m_BottomRightCorner) {
        // m_BottomRightCorner will be used to update the remaining corners
        // in AsdkRectangleJig::update() below.
        samplerCorner = m_BottomRightCorner;
    }
    else if (stat == AcEdJig::kNormal)
        return AcEdJig::kNoChange;

    return stat;
}

Adesk::Boolean MyRectangleJig::update()
{
    AcGeLine3d lineX, lineY;
    // Top right corner is intersection of lineX and lineY.
    lineX.set(m_TopLeftCorner, m_vecUnitX);
    lineY.set(m_BottomRightCorner, m_vecUnitY);
    lineX.intersectWith(lineY, m_TopRightCorner);
    // Bottom left corner is intersection of lineX and lineY.
    lineX.set(m_BottomRightCorner, m_vecUnitX);
    lineY.set(m_TopLeftCorner, m_vecUnitY);
    lineX.intersectWith(lineY, m_BottomLeftCorner);

    // Check to see if we have flipped around the X or Y axis.
    // 因为m_TopLeftCorner和m_BottomRightCorner本质上只是用户选择的2个角点. m_TopLeftCorner
    // 即可能是左下角, 也可能是右下角
    AcGeVector3d tmpXVec, tmpYVec;
    bool bXFlip = m_vecUnitX.dotProduct(m_TopLeftCorner - m_TopRightCorner) > 0;
    bool bYFlip = m_vecUnitY.dotProduct(m_TopLeftCorner - m_BottomLeftCorner) < 0;
    tmpXVec = bXFlip ? -1 * m_vecUnitX : m_vecUnitX;
    tmpYVec = bYFlip ? -1 * m_vecUnitY : m_vecUnitY;

    CRectInfo& plineInfo = wzj::data_per_doc::instance()->docData();
    if (plineInfo.m_cornerTreatment) {
        // 画圆角矩阵
        AcGePoint2d point_TL, point_TR, point_BL;
        MAKEOCSCOORD(point_TL, m_TopLeftCorner);
        MAKEOCSCOORD(point_TR, m_TopRightCorner);
        MAKEOCSCOORD(point_BL, m_BottomLeftCorner);

        // 判断是否距离太近, 不能切角
        bool tooSmall = (point_TL.distanceTo(point_TR) < plineInfo.m_first + plineInfo.m_second) ||
            (point_TL.distanceTo(point_BL) < plineInfo.m_first + plineInfo.m_second);

        AcGePoint2d adjustedPoint;
        if (tooSmall) {
            m_pLWPoly->setBulgeAt(0, 0.0);
            MAKEOCSCOORD(adjustedPoint, m_TopLeftCorner);
            m_pLWPoly->setPointAt(0, adjustedPoint);
            m_pLWPoly->setPointAt(1, adjustedPoint);

            m_pLWPoly->setBulgeAt(2, 0.0);
            MAKEOCSCOORD(adjustedPoint, m_TopRightCorner);
            m_pLWPoly->setPointAt(2, adjustedPoint);
            m_pLWPoly->setPointAt(3, adjustedPoint);

            m_pLWPoly->setBulgeAt(4, 0.0);
            MAKEOCSCOORD(adjustedPoint, m_BottomRightCorner);
            m_pLWPoly->setPointAt(4, adjustedPoint);
            m_pLWPoly->setPointAt(5, adjustedPoint);

            m_pLWPoly->setBulgeAt(6, 0.0);
            MAKEOCSCOORD(adjustedPoint, m_BottomLeftCorner);
            m_pLWPoly->setPointAt(6, adjustedPoint);
            m_pLWPoly->setPointAt(7, adjustedPoint);
        }
        else {
            double tmpBulge = ((!bXFlip && !bYFlip) || (bXFlip && bYFlip))
                ? plineInfo.m_bulge : -plineInfo.m_bulge;

            // Now we will set adjustedPoint to the intersection of the rectangle
            // sides with the place where the new end points will be.
            m_pLWPoly->setBulgeAt(0, tmpBulge);
            MAKEOCSCOORD(adjustedPoint, m_TopLeftCorner + (-plineInfo.m_first * tmpYVec));
            m_pLWPoly->setPointAt(0, adjustedPoint);

            MAKEOCSCOORD(adjustedPoint, m_TopLeftCorner + plineInfo.m_second * tmpXVec);
            m_pLWPoly->setPointAt(1, adjustedPoint);

            m_pLWPoly->setBulgeAt(2, tmpBulge);
            MAKEOCSCOORD(adjustedPoint, m_TopRightCorner + (-plineInfo.m_first * tmpXVec));
            m_pLWPoly->setPointAt(2, adjustedPoint);

            MAKEOCSCOORD(adjustedPoint, m_TopRightCorner + (-plineInfo.m_second * tmpYVec));
            m_pLWPoly->setPointAt(3, adjustedPoint);

            m_pLWPoly->setBulgeAt(4, tmpBulge);
            MAKEOCSCOORD(adjustedPoint, m_BottomRightCorner + plineInfo.m_first * tmpYVec);
            m_pLWPoly->setPointAt(4, adjustedPoint);

            MAKEOCSCOORD(adjustedPoint, m_BottomRightCorner + (-plineInfo.m_second * tmpXVec));
            m_pLWPoly->setPointAt(5, adjustedPoint);

            m_pLWPoly->setBulgeAt(6, tmpBulge);
            MAKEOCSCOORD(adjustedPoint, m_BottomLeftCorner + plineInfo.m_first * tmpXVec);
            m_pLWPoly->setPointAt(6, adjustedPoint);

            MAKEOCSCOORD(adjustedPoint, m_BottomLeftCorner + plineInfo.m_second * tmpYVec);
            m_pLWPoly->setPointAt(7, AcGePoint2d(adjustedPoint[X], adjustedPoint[Y]));
        }
    }
    else {
        AcGePoint2d adjustedPoint;
        // 普通矩阵
        MAKEOCSCOORD(adjustedPoint, m_TopLeftCorner);
        m_pLWPoly->setPointAt(0, adjustedPoint);

        MAKEOCSCOORD(adjustedPoint, m_TopRightCorner);
        m_pLWPoly->setPointAt(1, adjustedPoint);

        MAKEOCSCOORD(adjustedPoint, m_BottomRightCorner);
        m_pLWPoly->setPointAt(2, adjustedPoint);

        MAKEOCSCOORD(adjustedPoint, m_BottomLeftCorner);
        m_pLWPoly->setPointAt(3, adjustedPoint);
    }
    return Adesk::kTrue;
}

AcDbEntity* MyRectangleJig::entity() const
{
    return m_pLWPoly;
}

void MyRectangleJig::doRectangle()
{
    setDispPrompt(_T("\nOther corner: "));
    auto stat = drag();
    if (stat == kNormal)
        append();
    else
        delete m_pLWPoly;
}
