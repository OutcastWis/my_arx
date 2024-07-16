#include "pch.h"
#include "MySimpleBlockInsertPoints.h"

const TCHAR* MySimpleBlockInsertPoints::DynmicBlockName()
{
	return _T("WZJ_DYN_BLOCK");
}

Acad::ErrorStatus MySimpleBlockInsertPoints::getInsertionPoints(const AcDbBlockTableRecord* pBlock,
	const AcDbBlockReference* pBlkRef, AcGePoint3dArray& insPts, AcGeVector3dArray& alignmentDirections)
{

	AcString sName;
	Acad::ErrorStatus es = pBlock->getName(sName);
	if (_tcsicmp(sName, DynmicBlockName()) != 0)
		return Acad::eNotApplicable;

	// INSERT����ʱ, ��CTRL�ᵼ��block�Ĳ���㰴˳��仯. ��Ĭ��(0,0,0)��ʼ, ��insPts

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
	// ��ͬ�����Ĳ��뷽��. �ڲ���ʱ��Ӱ��block�ķ�ת��
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
