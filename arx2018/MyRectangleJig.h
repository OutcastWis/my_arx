#pragma once
#include <dbjig.h>

/**
* 双边平行UCS的XY轴, 形状为Fillet或Chamfer的矩形
*/
class MyRectangleJig : public AcEdJig 
{
public:
    MyRectangleJig();
    // 使用acquirePoint获取坐标变化
    virtual DragStatus sampler() override;
    // 应用变化
    virtual Adesk::Boolean update() override;
    // CAD内部使用该对象进行图形更新. entity()->worldDraw()
    virtual AcDbEntity* entity() const override;

    void doRectangle();

    AcGePoint3d samplerCorner;  // 上一次m_BottomRightCorner
private:
    AcDbPolyline*   m_pLWPoly;
    // 4个角点, WCS
    AcGePoint3d     m_TopLeftCorner;
    AcGePoint3d     m_TopRightCorner;
    AcGePoint3d     m_BottomLeftCorner;
    AcGePoint3d     m_BottomRightCorner;
    // Unit vectors in the UCS plane
    AcGeVector3d    m_vecUnitZ;       
    AcGeVector3d    m_vecUnitX;
    AcGeVector3d    m_vecUnitY;

};

