#include "pch.h"
#include "ssget.h"

#include <vector>
#include <geassign.h>
#include <AcString.h>

#include "command.h"

namespace wzj {

	namespace detail {
		/**
		* ÿ��ͼ��һ������. ͼ�����:
		*	LY_CIRCLE1, ���ز�����. �뾶Ϊ10��Բ
		*	LY_CIRCLE2, ����. �뾶Ϊ8��Բ
		*	LY_CIRCLE3, ����. �뾶Ϊ6��Բ
		*	LY_LINE1, ����. �����µ����ϵ�ֱ��
		*	LY_LINE2, ���᲻����. �����ϵ����µ�ֱ��
		*	LY_LINE3, ����. ���ϽǵĶ�ֱ��, ��LY_LINE1��ֱ
		*	LY_LINE4, ����. ���½ǵĶ�ֱ��, ��LY_LINE1��ֱ, ��ʾ��ɫ(����1)
		*
		* ��:
		*	GROUP_LINE1. ����LY_LINE1, LY_LINE2, LY_LINE3
		*	GROUP_LINE2. ����LY_LINE1, LY_LINE4
		*
		* ���ж��󱻴�����, ��ΪPICKFIRST
		*/
		void make_fake_data() {
			Acad::ErrorStatus es;
			// layer
			const AcString clayer1 = _T("LY_CIRCLE1");
			const AcString clayer2 = _T("LY_CIRCLE2");
			const AcString clayer3 = _T("LY_CIRCLE3");
			const AcString llayer1 = _T("LY_LINE1");
			const AcString llayer2 = _T("LY_LINE2");
			const AcString llayer3 = _T("LY_LINE3");
			const AcString llayer4 = _T("LY_LINE4");
			create_layer(clayer1, false, true); // off, but not frozen
			create_layer(clayer2, false, false); // off, but not frozen
			create_layer(clayer3, false, false); // off, but not frozen
			create_layer(llayer1, false, false); // normal
			create_layer(llayer2, true, false); // frozon, but not off
			create_layer(llayer3, false, false); // normal
			create_layer(llayer4, false, false); // normal

			//
			AcDbCircle* circle = new AcDbCircle(AcGePoint3d(), AcGeVector3d::kZAxis, 10);
			es = circle->setLayer(clayer1);
			AcDbObjectId id1;
			add_to_model_space(id1, circle, nullptr);
			circle->close();

			AcDbCircle* circle2 = new AcDbCircle(AcGePoint3d(), AcGeVector3d::kZAxis, 8);
			circle2->setLayer(clayer2);
			AcDbObjectId id4;
			add_to_model_space(id4, circle2, nullptr);
			circle2->close();

			AcDbCircle* circle3 = new AcDbCircle(AcGePoint3d(), AcGeVector3d::kZAxis, 6);
			circle3->setLayer(clayer3);
			AcDbObjectId id7;
			add_to_model_space(id7, circle3, nullptr);
			circle3->close();

			AcDbLine* line = new AcDbLine(AcGePoint3d(-10, -10, 0), AcGePoint3d(10, 10, 0));
			line->setLayer(llayer1);
			AcDbObjectId id2;
			add_to_model_space(id2, line, nullptr);
			line->close();

			AcDbLine* line2 = new AcDbLine(AcGePoint3d(-10, 10, 0), AcGePoint3d(10, -10, 0));
			line2->setLayer(llayer2);
			AcDbObjectId id3;
			add_to_model_space(id3, line2, nullptr);
			line2->close();

			AcDbLine* line3 = new AcDbLine(AcGePoint3d(10, 0, 0), AcGePoint3d(0, 10, 0));
			line3->setLayer(llayer3);
			AcDbObjectId id5;
			add_to_model_space(id5, line3, nullptr);
			line3->close();
			AcDbLine* line4 = new AcDbLine(AcGePoint3d(-10, 0, 0), AcGePoint3d(0, -10, 0));
			line4->setLayer(llayer4);
			line4->setColorIndex(1);
			AcDbObjectId id6;
			add_to_model_space(id6, line4, nullptr);
			line4->close();


			// group. ��1����line1, line2, line3
			auto ids1 = AcDbObjectIdArray();
			ids1.append(id2), ids1.append(id3), ids1.append(id5);
			create_group(ids1, _T("GROUP_LINE1"));
			// ��2����lin1, line4
			auto ids2 = AcDbObjectIdArray();
			ids2.append(id2), ids2.append(id6);
			create_group(ids2, _T("GROUP_LINE2"));
		}

		resbuf* keywork_cb(const TCHAR* keyword) {
			ads_printf(_T("You type keyword %s"), keyword);
			return nullptr;
		}

		resbuf* other_cb(const TCHAR* keyword) {
			ads_printf(_T("You type other %s"), keyword);
			return nullptr;
		}

		int ssget(const TCHAR* str, const void* pt1, const void* pt2, const resbuf* filter, bool release = true) {

			ads_name ss = {};
			auto res = ads_ssget(str, pt1, pt2, filter, ss);

			if (res == RTNORM) {
				Adesk::Int32 len = -1;
				ads_sslength(ss, &len);
				CString info;
				info.Format(_T("%s: ss's len is %d. {"), str, len);

				for (int i = 0; i < len; ++i) {
					ads_name ent = {};
					ads_ssname(ss, i, ent);
					AcDbObjectId id;
					acdbGetObjectId(id, ent);
					AcDbObject* obj = nullptr;
					if (acdbOpenObject(obj, id, AcDb::kForRead) == Acad::eOk) {
						if (obj->isKindOf(AcDbEntity::desc())) {
							AcDbEntity* ee = (AcDbEntity*)obj;
							info.Format(_T("%s%s, "), (const TCHAR*)info, ee->layer());
						}
						else {
#ifdef CAD2022
							info.Format(_T("%s%s, "), (const TCHAR*)info, obj->desc()->className());
#else
							info.Format(_T("%s%s, "), (const TCHAR*)info, obj->desc()->dxfName());
#endif
						}
						obj->close();
					}
				}
				info.Format(_T("%s%s"), (const TCHAR*)info, _T("}\n"));
				ads_printf(_T("%s"), (const TCHAR*)info);

				if (release) ads_ssfree(ss);
				len = -1;
			}
			else {
				// check <ol_errno.h> to see what the number mean is
				CString info;
				info.Format(_T("%s: errno is %d\n"), str, get_int_sysvar(_T("ERRNO")));
				ads_printf(_T("%s"), (const TCHAR*)info);
			}

			return res;
		}


		/**
		* ssget
		* 
		* Selection Method Mode Options:
		*	NULL		PICKFIRST. ����ṩ��pt1, ���ǵ�ѡ. û��PICKFIRSTʱ, ���н���
		*	I			PICKFIRST. ��Ҫ������ACRX_CMD_REDRAW. û��PICKFIRSTʱ,���ش���
		*	C, W		��ѡ. C��ʾ�ཻ����, W����Ҫ��Χ. ��Ҫͼ������ͼ�ڿɼ�
		*	CP, WP		��ѡ�Ķ���ΰ汾. ��Ҫͼ������ͼ�ڿɼ�
		*	F			��դ. ��ȡ��pt1��ʾ��ֱ���ཻ�Ķ���. ��Ҫͼ������ͼ�ڿɼ�
		*	L			��󴴽���Entity. �����GROUP, ����ȫ�����
		*	P			��һ�ε�ѡ��
		*	X			����. filter==nullptr, ����ȫͼ. ������������ݿ�,����ͼֽ�ռ�
		*
		* Colon Mode Options
		*	:$	��ʾ, pt1����2���ַ�������. 0��Addʱ����ʾ, 1��Removeʱ��ʾ
		*   :?  ���acedSSSetOtherCallbackPtr, ������ʶ�Ĺؼ���
		*	:A	ȫ��
		*	:D	���ظ�ѡ��
		*	:K	pt2�����ùؼ���. ���acedSSSetKwordCallbackPtr, ����ؼ���
		*	:N	Ƕ��. ��������ͨѡ��ûɶ����. ����acedSSNameX����ȷ��ѡ�е�Ƕ�׶���. ���highlight(). ʵ��ʹ����Ҳ����ѡ���Ӷ���
		*	:S	��ѡ
		*	:E  �ɼ�����
		*	:U	��סCTRL����ѡ���Ӷ���, ��������ͨѡ��. �Ӷ����ǿ��Ƕ�׶���, ��������Poly�еĸ��߶�, 3Dͼ�ε���ͱ�.
		*	:V	ǿ��ѡ���Ӷ���. ����ö���û���Ӷ���, ����ѡ��
		*
		* Keyword Filter Mode Options
		*	+	���ӹؼ���. ��ʱĬ��Ϊ"Add/Remove/Undo/Single"
		*	-   ���ٹؼ���. ��ʱĬ��Ϊ"Window/Last/Crossing/BOX/ALL/Fence/WPolygon/CPolygon/Group/Add/Remove/Multiple/Previous/Undo/AUto/Single"
		*	#	-ʱ����ȥ��"Last|All|Group|Previous". +ʱ��������"Last|All|Previous"
		*	B	-ʱȥ��"Box|AUTO". +ʱ����"Window|Crossing|BOX|WPolygon|CPolygon"
		*	D	-ʱȥ��"Add/Remove". +ʱ������
		*	A	��Ӧ"ALL"
		*	M	��Ӧ"Multiple"
		*   C	��Ӧ"Crossing|CPolygon"
		*   F	��Ӧ"Fence"
		*   L	��Ӧ"Last"
		*	P	��Ӧ"Previous"
		*	W	��Ӧ"Window|WPolygon"
		*	G	��Ӧ"Group"
		*	.   ����+, �û�����ѡ. ���û��'.', ��Ҫ��ָ��������ʽ, ����ѡ�����
		*
		* @remark  Keyword Filter Mode OptionsӦ�÷���һ��, �����ַ�������ǰ��. ����"_+.+F+G+W"
		*/
		void ssget() {
			make_fake_data();

#define POINT(d,x,y,z) (d[0]=(x), d[1]=(y), d[2]=(z), d)

			int res = RTERROR;
			double pt1[3], pt2[3];
			std::vector<AcDbEntity*> other_ents;
			// I 
			res = ssget(_T("I"), nullptr, nullptr, nullptr);

			// NULL. str, pt1, ptr2, filer are all null.
			res = ssget(nullptr, nullptr, nullptr, nullptr);

			// C. "W"�ȼ�. ֻ����������Ҫ�����������, �������ཻ����
			{
				AcDbPolyline* poly = new AcDbPolyline();
				poly->addVertexAt(0, AcGePoint2d(-1, 11));
				poly->addVertexAt(0, AcGePoint2d(3, 11));
				poly->addVertexAt(0, AcGePoint2d(3, 7));
				poly->addVertexAt(0, AcGePoint2d(-1, 7));
				poly->setClosed(true); // poly��pt1��pt2�����ľ��ο�
				other_ents.push_back(poly);
				// ���Կ���: 
				//		1. pt1��pt2��˳������ν. 
				//		2. poly��GROUP_LINE1, CIRCLE2�ཻ, ����ѡ��������4��ͼ��(GROUP_LINE1 �е� LINE1 �� LINE3, CIRCLE2)
				res = ssget(_T("C"), POINT(pt1, -1, 7, 0), POINT(pt2, 3, 11, 0), nullptr);
			}

			// "CP". "WP"�ȼ�. ֻ����������Ҫ�����������, �������ཻ����
			{
				double pts[12] = {
				0, 0, 0,
				8,0,0,
				8,-8,0,
				0,-8,0
				};
				auto list_pts1 = ads_buildlist(RT3DPOINT, pts, RT3DPOINT, pts + 3, RT3DPOINT, pts + 6, RT3DPOINT, pts + 9, 0);
				AcDbPolyline* poly = new AcDbPolyline();
				int i = 0;
				for (auto r = list_pts1; r != nullptr; r = r->rbnext) {
					poly->addVertexAt(i++, asPnt2d(r->resval.rpoint));
				}
				poly->setClosed(true);
				other_ents.push_back(poly);
				// ���Կ���: 
				//		1. poly��CIRCLE2, CIRCLE3�ཻ. ѡ�������ٺ�����2��
				//		2. (0,0)����LINE1����, Ϊ�������, ������Ҳ���ܲ���. ����(-0.01, 0.01)����LINE1��, �Ƴ���GROUP1��GROUP2��,
				//			��ʱ, ѡ���ж��⺬��LINE1, LINE3, LINE4
				res = ssget(_T("CP"), list_pts1, nullptr, nullptr);
				ads_relrb(list_pts1);
			}

			// "F"
			{
				double pts2[6] = {
				-10, 4, 0,
				0,4,0
				};
				auto list_pts2 = ads_buildlist(RT3DPOINT, pts2, RT3DPOINT, pts2 + 3, 0);
				AcDbLine* line = new AcDbLine();
				line->setStartPoint(asPnt3d(list_pts2->resval.rpoint));
				line->setEndPoint(asPnt3d(list_pts2->rbnext->resval.rpoint));
				other_ents.push_back(line);
				// ��ֱ��, ����CIRCLE2, CIRCLE3�ཻ, ����ѡ����ֻ����2��
				res = ssget(_T("F"), list_pts2, nullptr, nullptr);
				ads_relrb(list_pts2);
			}

			// "P". ��һ�ε�ѡ�񼯽��
			res = ssget(_T("P"), nullptr, nullptr, nullptr);

			// "L". ����������ݿ�Ķ���. ���������GROUP2, ����ѡ������GROUP2�еĶ���
			res = ssget(_T("L"), nullptr, nullptr, nullptr);

			// "X". 
			{
				// Ѱ������CIRCLE�ͷǺ�ɫLINE. ��ǰ�治ͬ, X��Զ��������ͼ����д���. 
				// ���ѡ����Ϊ6������. 3��CIRCLE, 3��LINE(LINE4Ϊ��ɫ, ���ų�)
				auto filter = ads_buildlist(
					-4, _T("<or"),
					RTDXF0, _T("CIRCLE"),
					-4, _T("<and"),
					RTDXF0, _T("LINE"), -4,
					_T("<not"), AcDb::kDxfColor, 1, -4, _T("not>"),
					-4, _T("and>"),
					-4, _T("or>"),
					0);
				res = ssget(_T("X"), nullptr, nullptr, filter);
				ads_relrb(filter);
			}

			// ":$:?:K"
			acedSSSetKwordCallbackPtr(keywork_cb); // :K����. ʶ��Ĺؼ���global�����callback
			acedSSSetOtherCallbackPtr(other_cb); // :?����. ����ʶ�Ĺؼ��ֽ����callback
			const TCHAR* prompts[2] = { _T("Add��ʾ:[KAey1 KBey2 KCey3]"), _T("Remove��ʾ") };
			res = ssget(_T(":$:?:K"), prompts, _T("KAey1 KBey2 KCey3 _ GK1 GK2 GK3"), nullptr, true);

			// ����. ����?���ɿ����ؼ����б�. Ӧ����"����(W)/��ѡ(F)/ȦΧ(WP)/����(G)/���(A)/ɾ��(R)/����(U)/����(SI)"
			// �����"_+F+W+G", ����Ҫ������ؼ��ֺ���ܽ��ж���ѡ��
			res = ssget(_T("_+F+W+G+."), nullptr, nullptr, nullptr);
#undef POINT

			{
				create_layer(_T("����"), false, false);

				int i = 1;
				AcDbObjectId id;
				for (auto ent : other_ents) {
					ent->setColorIndex(i++);
					ent->setLayer(_T("����"));
					if (i > 6) i = 1;
					add_to_model_space(id, ent, nullptr);
					ent->close();
				}
			}

		}
	}

	void ssget::init_impl()
	{
		acedRegCmds->addCommand(_T("WZJ_COMMAND_SSGET"), _T("GLOBAL_SSGET_DO"), _T("LOCAL_SSGET_DO"),
			ACRX_CMD_MODAL | ACRX_CMD_REDRAW, detail::ssget);
	}

	void ssget::stop_impl()
	{
		acedRegCmds->removeGroup(_T("WZJ_COMMAND_SSGET"));
	}
}