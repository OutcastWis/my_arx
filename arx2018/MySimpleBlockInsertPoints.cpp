#include "pch.h"
#include "MySimpleBlockInsertPoints.h"

#include <AcString.h>

const TCHAR* MySimpleBlockInsertPoints::DynmicBlockName()
{
	return _T("WZJ_DYN_BLOCK");
}

Acad::ErrorStatus MySimpleBlockInsertPoints::getInsertionPoints(const AcDbBlockTableRecord* pBlock,
	const AcDbBlockReference* pBlkRef, AcGePoint3dArray& insPts, AcGeVector3dArray& alignmentDirections)
{
	(void*)pBlkRef;

	AcString sName;
	Acad::ErrorStatus es = pBlock->getName(sName);
	if (_tcsicmp(sName, DynmicBlockName()) != 0)
		return Acad::eNotApplicable;

	// INSERT命令时, 按CTRL会导致block的插入点按顺序变化. 从默认(0,0,0)开始, 沿insPts

	insPts.append(AcGePoint3d(1.0, 0.0, 0.0)); //midpoint of side
	insPts.append(AcGePoint3d(1.0, -1.0, 0.0)); //buffered midpoint
	insPts.append(AcGePoint3d(2.0, 0.0, 0.0));	//endpoint
	insPts.append(AcGePoint3d(2.0, 1.0, 0.0)); //midpoint of side
	insPts.append(AcGePoint3d(3.0, 1.0, 0.0)); //buffered midpoint
	insPts.append(AcGePoint3d(2.0, 2.0, 0.0)); //endpoint
	insPts.append(AcGePoint3d(1.0, 2.0, 0.0)); //midpoint of side
	insPts.append(AcGePoint3d(1.0, 3.0, 0.0)); //buffered midpoint
	insPts.append(AcGePoint3d(0.0, 2.0, 0.0)); //endpoint
	insPts.append(AcGePoint3d(0.0, 1.0, 0.0)); //midpoint of side
	insPts.append(AcGePoint3d(-1.0, 1.0, 0.0)); //buffered midpoint
	// 不同插入点的插入方向. 在插入时会影响block的翻转等
	alignmentDirections.append(AcGeVector3d(0.0, 1.0, 0.0));
	alignmentDirections.append(AcGeVector3d(0.0, 1.0, 0.0));
	alignmentDirections.append(AcGeVector3d(-1.0, 0.0, 0.0));
	alignmentDirections.append(AcGeVector3d(-1.0, 0.0, 0.0));
	alignmentDirections.append(AcGeVector3d(-1.0, 0.0, 0.0));
	alignmentDirections.append(AcGeVector3d(0.0, -1.0, 0.0));
	alignmentDirections.append(AcGeVector3d(0.0, -1.0, 0.0));
	alignmentDirections.append(AcGeVector3d(0.0, -1.0, 0.0));
	alignmentDirections.append(AcGeVector3d(1.0, 0.0, 0.0));
	alignmentDirections.append(AcGeVector3d(1.0, 0.0, 0.0));
	alignmentDirections.append(AcGeVector3d(1.0, 0.0, 0.0));


	return Acad::eOk;
}
