#pragma once

#include <dbappgrip.h>
#include <gevc3dar.h>

class MySimpleBlockInsertPoints : public AcDbBlockInsertionPoints {
public:
	MySimpleBlockInsertPoints() {}
	~MySimpleBlockInsertPoints() {}

	// 根据这个名字来判断, 是否要对BLOCK进行扩展
	static const TCHAR* DynmicBlockName();

	virtual Acad::ErrorStatus getInsertionPoints(
		const AcDbBlockTableRecord* pBlock,
		const AcDbBlockReference* pBlkRef,
		AcGePoint3dArray& insPts,
		AcGeVector3dArray& alignmentDirections);
};