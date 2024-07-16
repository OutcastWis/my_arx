#include "pch.h"
#include "MyGlyph.h"

#include <geassign.h>

Acad::ErrorStatus MyGlyph::setLocation(const AcGePoint3d& dcsPoint)
{
    m_center = dcsPoint;
    return Acad::eOk;
}

void MyGlyph::subViewportDraw(AcGiViewportDraw* vportDrawContext)
{
    // 获取拾取框大小(单位像素). 可由用户在Options界面设置
    int glyphPixels = acdbCustomOsnapManager()->osnapGlyphSize();
    // 获取m_center为中心单位平方内像素点. X轴上像素数存入glyphSize[X], Y轴上的存入glyphSize[Y]
    AcGePoint2d glyphSize;
    vportDrawContext->viewport().getNumPixelsInUnitSquare(m_center, glyphSize);
    // Calculate the size of the glyph in WCS (use for text height factor)
    double glyphHeight = glyphPixels / glyphSize[Y];

    // Get the extents of the glyph text, so we can centre it
    AcGiTextStyle style;
    AcGePoint2d ptExt = style.extents(_T("1/3"), Adesk::kFalse, -1, Adesk::kFalse);

    struct resbuf rbFrom, rbTo;
    rbFrom.restype = RTSHORT;
    rbFrom.resval.rint = 2; // From DCS
    rbTo.restype = RTSHORT;
    rbTo.resval.rint = 0; // To WCS

    // Translate the centre of the glyph from DCS to WCS co-ordinates
    AcGePoint3d ptPos, ptCen;
    AcGeVector3d vecExt(ptExt[X] / 2, ptExt[Y] / 2, 0);
    ptPos = m_center - vecExt / 2;
    if (RTNORM != acedTrans(asDblArray(ptPos), &rbFrom, &rbTo, 0, asDblArray(ptCen)))
        ptCen = m_center;

    // Translate the X-axis of the DCS to WCS co-ordinates (as a displacement vector)
    AcGeVector3d ptDir;
    acedTrans(asDblArray(AcGeVector3d::kXAxis), &rbFrom, &rbTo, 1, asDblArray(ptDir));

    // Draw the centred text representing the glyph
    vportDrawContext->geometry().text(ptCen, vportDrawContext->viewport().viewDir(), ptDir, glyphHeight, 1.0, 0.0, _T("1/3"));
}
