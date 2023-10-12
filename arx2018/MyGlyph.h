#pragma once

#include <dbosnap.h>

/**
* ���osnapʹ��, �Զ����osnapͼ��
*/
class MyGlyph : public AcGiGlyph
{
public:
    // ע��dscPoint, ��Ҫ����DSC�µ�����
    virtual Acad::ErrorStatus setLocation(const AcGePoint3d& dcsPoint) override;
    virtual void subViewportDraw(AcGiViewportDraw* vportDrawContext) override;

private:
    AcGePoint3d m_center;
};

