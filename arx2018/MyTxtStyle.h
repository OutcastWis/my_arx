#pragma once
#include <dbents.h>


class MyTxtStyle : public AcDbEntity
{
public:
    ACRX_DECLARE_MEMBERS(MyTxtStyle);

    static void SetStyle(const AcGiTextStyle& style);
    static const AcGiTextStyle& GetStyle();

protected:
    virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw*);
    Acad::ErrorStatus subTransformBy(const AcGeMatrix3d&);
};

