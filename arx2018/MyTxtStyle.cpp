#include "pch.h"
#include "MyTxtStyle.h"

namespace detail {
    static AcGiTextStyle AsdkStyle;
}


ACRX_DXF_DEFINE_MEMBERS(MyTxtStyle, AcDbEntity,
    AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 0, \
    MYTXTSTYLE, MyTxtStyle Sample);


void MyTxtStyle::SetStyle(const AcGiTextStyle& style) {
    detail::AsdkStyle = style;
}

const AcGiTextStyle& MyTxtStyle::GetStyle() {
    return detail::AsdkStyle;
}


Acad::ErrorStatus MyTxtStyle::subTransformBy(const AcGeMatrix3d& xfm)
{
    return Acad::eOk;
}


Adesk::Boolean MyTxtStyle::subWorldDraw(AcGiWorldDraw* pW)
{
    AcGePoint3d pos(0.0, 0.0, 0.0);  // 文字的插入点是左下角
    AcGeVector3d norm(0.0, 0.0, 1.0);
    AcGeVector3d dir(1.0, 0.2, 0.0); // 倾斜
    const TCHAR* pStr = _T("Class MyTxtStyle");
    size_t len = _tcslen(pStr);

    // 调整坐标系为直角坐标系. dir为x, vec为y, norm为z
    AcGeVector3d vec = norm;
    vec = vec.crossProduct(dir);
    dir = vec.crossProduct(norm);

    AcGePoint2d ext1 = detail::AsdkStyle.extents(pStr, Adesk::kFalse, len, Adesk::kFalse);
    pW->geometry().text(pos, norm, dir, pStr, (int)len, Adesk::kFalse, detail::AsdkStyle);

    // 创建重影
    pos.x += 0.1, pos.y += 0.1;
    AcGiTextStyle shadow_style = detail::AsdkStyle;
    shadow_style.setTrackingPercent(0.8);   // 设置字间距
    shadow_style.setObliquingAngle(0.5);    // 设置倾斜角度. 角度为与垂直方向的夹角. 即斜体字

    // You must call loadStyleRec() again after changing 
    // the style's properties in order for the current style 
    // settings to be loaded into the graphics system. 
    // Otherwise, the extents calculation may be incorrect.
    shadow_style.loadStyleRec();
    AcGePoint2d ext2 = shadow_style.extents(pStr, Adesk::kFalse, len, Adesk::kFalse);
    pW->geometry().text(pos, norm, dir, pStr, len, Adesk::kFalse, shadow_style);

    // 合并框
    ext2.x = ext2.x > ext1.x ? ext2.x : ext1.x;
    ext2.y = ext2.y > ext1.y ? ext2.y : ext1.y;

    // 画矩形框
    norm.normalize();
    dir.normalize();
    AcGeVector3d yAxis = norm;
    yAxis = yAxis.crossProduct(dir);
    yAxis.normalize();
    AcGeMatrix3d textMat;
    textMat.setCoordSystem(AcGePoint3d(0.0, 0.0, 0.0), dir, yAxis, norm);

    // Create the bounding box and enlarge it somewhat.
    double offset = ext2.y / 2.0;
    AcGePoint3d verts[5];
    verts[0] = verts[4] = AcGePoint3d(-offset, -offset, 0.0); // 左下角(0,0,0)为起点
    verts[1] = AcGePoint3d(ext2.x + offset, -offset, 0.0);
    verts[2] = AcGePoint3d(ext2.x + offset, ext2.y + offset, 0.0);
    verts[3] = AcGePoint3d(-offset, ext2.y + offset, 0.0);

    // Orient and then translate each point in the bounding box.
    for (int i = 0; i < 5; i++) {
        verts[i].transformBy(textMat);
        verts[i].x += pos.x;
        verts[i].y += pos.y;
        verts[i].z += pos.z;
    }
    pW->geometry().polyline(5, verts);

    return Adesk::kTrue;
}
