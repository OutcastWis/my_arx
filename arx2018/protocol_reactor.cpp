#include "pch.h"
#include "protocol_reactor.h"

#include <rxprotevnt.h>

#include "MySimpleBlockInsertPoints.h"


namespace wzj {

	namespace detail {
		static MySimpleBlockInsertPoints* pPts = nullptr;

		void addInsertionPoints()
		{
			if (!pPts)
			{
				pPts = new MySimpleBlockInsertPoints();
				ACRX_PROTOCOL_REACTOR_LIST_AT(AcDbBlockTableRecord::desc(),
					MySimpleBlockInsertPoints::desc())->addReactor(pPts);
			}
		}

		void createBlock() {
			AcDbSymbolTable* pBT = NULL;
			acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBT, AcDb::kForWrite);

			AcDbBlockTableRecord* pBTR = new AcDbBlockTableRecord();
			pBTR->setName(MySimpleBlockInsertPoints::DynmicBlockName());
			pBT->add(pBTR);
			pBT->close();

			AcGePoint3d pt1(0.0, 0.0, 0.0);
			AcGePoint3d pt2(2.0, 0.0, 0.0);
			AcDbLine* pLine = new AcDbLine(pt1, pt2);
			pBTR->appendAcDbEntity(pLine);
			pLine->setColorIndex(1);
			pLine->close();

			pt1 = pt2;
			pt2.y = 2.0;
			pLine = new AcDbLine(pt1, pt2);
			pBTR->appendAcDbEntity(pLine);
			pLine->setColorIndex(2);
			pLine->close();

			pt1 = pt2;
			pt2.x = 0.0;
			pLine = new AcDbLine(pt1, pt2);
			pBTR->appendAcDbEntity(pLine);
			pLine->setColorIndex(3);
			pLine->close();

			pt1 = pt2;
			pt2.y = 0.0;
			pLine = new AcDbLine(pt1, pt2);
			pBTR->appendAcDbEntity(pLine);
			pLine->setColorIndex(4);
			pLine->close();

			pBTR->close();
		}
	}

	void protocol_reactor::init_impl()
	{
		acedRegCmds->addCommand(_T("WZJ_COMMAND_DYN_BLK"), _T("GLOBAL_INS_PT"), _T("LOCAL_INS_PT"), ACRX_CMD_MODAL,
			detail::addInsertionPoints);
		acedRegCmds->addCommand(_T("WZJ_COMMAND_DYN_BLK"), _T("GLOBAL_CREATE_DYN_BLK"), _T("LOCAL_CREATE_DYN_BLK"), ACRX_CMD_MODAL,
			detail::createBlock);
	}

	void protocol_reactor::stop_impl()
	{
		acedRegCmds->removeGroup(_T("WZJ_COMMAND_DYN_BLK"));
		if (detail::pPts)
		{
			ACRX_PROTOCOL_REACTOR_LIST_AT(AcDbBlockTableRecord::desc(),
				MySimpleBlockInsertPoints::desc())->removeReactor(detail::pPts);
			delete detail::pPts;
			detail::pPts = NULL;
		}
	}
}