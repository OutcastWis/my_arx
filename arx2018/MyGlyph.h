#pragma once

#include <dbosnap.h>

/**
* 配合osnap使用, 自定义的osnap图形
*/
class MyGlyph : public AcGiGlyph
{
public:
    // 注意dscPoint, 需要的是DSC下的坐标
    virtual Acad::ErrorStatus setLocation(const AcGePoint3d& dcsPoint) override;
    virtual void subViewportDraw(AcGiViewportDraw* vportDrawContext) override;

private:
    AcGePoint3d m_center;
};

