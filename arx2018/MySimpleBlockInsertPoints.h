#pragma once

#include <dbappgrip.h>
#include <gevc3dar.h>

class MySimpleBlockInsertPoints : public AcDbBlockInsertionPoints {
public:
	MySimpleBlockInsertPoints() {}
	~MySimpleBlockInsertPoints() {}

	// ��������������ж�, �Ƿ�Ҫ��BLOCK������չ
	static const TCHAR* DynmicBlockName();

	virtual Acad::ErrorStatus getInsertionPoints(
		const AcDbBlockTableRecord* pBlock,
		const AcDbBlockReference* pBlkRef,
		AcGePoint3dArray& insPts,
		AcGeVector3dArray& alignmentDirections);
};