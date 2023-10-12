#pragma once
#include <dbjig.h>

/**
* ˫��ƽ��UCS��XY��, ��״ΪFillet��Chamfer�ľ���
*/
class MyRectangleJig : public AcEdJig 
{
public:
    MyRectangleJig();
    // ʹ��acquirePoint��ȡ����仯
    virtual DragStatus sampler() override;
    // Ӧ�ñ仯
    virtual Adesk::Boolean update() override;
    // CAD�ڲ�ʹ�øö������ͼ�θ���. entity()->worldDraw()
    virtual AcDbEntity* entity() const override;

    void doRectangle();

    AcGePoint3d samplerCorner;  // ��һ��m_BottomRightCorner
private:
    AcDbPolyline*   m_pLWPoly;
    // 4���ǵ�, WCS
    AcGePoint3d     m_TopLeftCorner;
    AcGePoint3d     m_TopRightCorner;
    AcGePoint3d     m_BottomLeftCorner;
    AcGePoint3d     m_BottomRightCorner;
    // Unit vectors in the UCS plane
    AcGeVector3d    m_vecUnitZ;       
    AcGeVector3d    m_vecUnitX;
    AcGeVector3d    m_vecUnitY;

};

