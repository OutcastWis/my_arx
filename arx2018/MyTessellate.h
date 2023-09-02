#pragma once

#include <dbents.h>

class MyTessellate : public AcDbEntity {
public:
	ACRX_DECLARE_MEMBERS(MyTessellate);
protected:
    virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw*);
    void subViewportDraw(AcGiViewportDraw*);
    Acad::ErrorStatus subTransformBy(const AcGeMatrix3d&);
};