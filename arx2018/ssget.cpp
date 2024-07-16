#include "pch.h"
#include "ssget.h"

#include <vector>
#include <geassign.h>
#include <AcString.h>

#include "command.h"

namespace wzj {

	namespace detail {
		/**
		* 每个图层一个对象. 图层见下:
		*	LY_CIRCLE1, 隐藏不冻结. 半径为10的圆
		*	LY_CIRCLE2, 正常. 半径为8的圆
		*	LY_CIRCLE3, 正常. 半径为6的圆
		*	LY_LINE1, 正常. 从左下到右上的直线
		*	LY_LINE2, 冻结不隐藏. 从左上到右下的直线
		*	LY_LINE3, 正常. 右上角的短直线, 和LY_LINE1垂直
		*	LY_LINE4, 正常. 左下角的短直线, 和LY_LINE1垂直, 显示红色(索引1)
		*
		* 组:
		*	GROUP_LINE1. 含有LY_LINE1, LY_LINE2, LY_LINE3
		*	GROUP_LINE2. 含有LY_LINE1, LY_LINE4
		*
		* 所有对象被创建后, 设为PICKFIRST
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


			// group. 组1包含line1, line2, line3
			auto ids1 = AcDbObjectIdArray();
			ids1.append(id2), ids1.append(id3), ids1.append(id5);
			create_group(ids1, _T("GROUP_LINE1"));
			// 组2包含lin1, line4
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
		*	NULL		PICKFIRST. 如果提供了pt1, 则是点选. 没有PICKFIRST时, 进行交互
		*	I			PICKFIRST. 需要命令有ACRX_CMD_REDRAW. 没有PICKFIRST时,返回错误
		*	C, W		框选. C表示相交即可, W则需要包围. 需要图形在试图内可见
		*	CP, WP		框选的多边形版本. 需要图形在试图内可见
		*	F			格栅. 获取和pt1表示的直线相交的对象. 需要图形在试图内可见
		*	L			最后创建的Entity. 如果是GROUP, 则是全组对象
		*	P			上一次的选择集
		*	X			过滤. filter==nullptr, 则是全图. 会遍历整个数据库,包括图纸空间
		*
		* Colon Mode Options
		*	:$	提示, pt1长度2的字符串数组. 0是Add时的提示, 1是Remove时提示
		*   :?  配合acedSSSetOtherCallbackPtr, 处理不认识的关键字
		*	:A	全部
		*	:D	可重复选择
		*	:K	pt2可设置关键字. 配合acedSSSetKwordCallbackPtr, 处理关键字
		*	:N	嵌套. 交互和普通选择没啥区别. 借助acedSSNameX才能确定选中的嵌套对象. 详见highlight(). 实际使用它也可以选择子对象
		*	:S	单选
		*	:E  可见对象
		*	:U	按住CTRL可以选择子对象, 否则是普通选择. 子对象不是块的嵌套对象, 而是例如Poly中的各线段, 3D图形的面和边.
		*	:V	强制选择子对象. 如果该对象没有子对象, 则不能选择
		*
		* Keyword Filter Mode Options
		*	+	增加关键字. 此时默认为"Add/Remove/Undo/Single"
		*	-   减少关键字. 此时默认为"Window/Last/Crossing/BOX/ALL/Fence/WPolygon/CPolygon/Group/Add/Remove/Multiple/Previous/Undo/AUto/Single"
		*	#	-时代表去掉"Last|All|Group|Previous". +时代表增加"Last|All|Previous"
		*	B	-时去掉"Box|AUTO". +时增加"Window|Crossing|BOX|WPolygon|CPolygon"
		*	D	-时去掉"Add/Remove". +时无作用
		*	A	对应"ALL"
		*	M	对应"Multiple"
		*   C	对应"Crossing|CPolygon"
		*   F	对应"Fence"
		*   L	对应"Last"
		*	P	对应"Previous"
		*	W	对应"Window|WPolygon"
		*	G	对应"Group"
		*	.   用于+, 用户鼠标点选. 如果没有'.', 需要先指定交互方式, 才能选择对象
		*
		* @remark  Keyword Filter Mode Options应该放在一起, 并在字符串的最前面. 例如"_+.+F+G+W"
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

			// C. "W"等价. 只不过后者需要包含对象才算, 而不是相交就行
			{
				AcDbPolyline* poly = new AcDbPolyline();
				poly->addVertexAt(0, AcGePoint2d(-1, 11));
				poly->addVertexAt(0, AcGePoint2d(3, 11));
				poly->addVertexAt(0, AcGePoint2d(3, 7));
				poly->addVertexAt(0, AcGePoint2d(-1, 7));
				poly->setClosed(true); // poly是pt1和pt2决定的矩形框
				other_ents.push_back(poly);
				// 可以看到: 
				//		1. pt1和pt2的顺序无所谓. 
				//		2. poly和GROUP_LINE1, CIRCLE2相交, 所以选择集里面有4个图形(GROUP_LINE1 中的 LINE1 和 LINE3, CIRCLE2)
				res = ssget(_T("C"), POINT(pt1, -1, 7, 0), POINT(pt2, 3, 11, 0), nullptr);
			}

			// "CP". "WP"等价. 只不过后者需要包含对象才算, 而不是相交就行
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
				// 可以看到: 
				//		1. poly与CIRCLE2, CIRCLE3相交. 选择集中至少含有这2个
				//		2. (0,0)点与LINE1相切, 为极端情况, 可能算也可能不算. 若是(-0.01, 0.01)则与LINE1交, 推出与GROUP1和GROUP2交,
				//			此时, 选择集中额外含有LINE1, LINE3, LINE4
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
				// 该直线, 仅与CIRCLE2, CIRCLE3相交, 所以选择集中只有这2个
				res = ssget(_T("F"), list_pts2, nullptr, nullptr);
				ads_relrb(list_pts2);
			}

			// "P". 上一次的选择集结果
			res = ssget(_T("P"), nullptr, nullptr, nullptr);

			// "L". 最近进入数据库的对象. 由于最近是GROUP2, 所以选择集中是GROUP2中的对象
			res = ssget(_T("L"), nullptr, nullptr, nullptr);

			// "X". 
			{
				// 寻找所有CIRCLE和非红色LINE. 和前面不同, X会对冻结和隐藏图层进行处理. 
				// 结果选择集中为6个对象. 3个CIRCLE, 3个LINE(LINE4为红色, 被排除)
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
			acedSSSetKwordCallbackPtr(keywork_cb); // :K控制. 识别的关键字global进入该callback
			acedSSSetOtherCallbackPtr(other_cb); // :?控制. 不认识的关键字进入该callback
			const TCHAR* prompts[2] = { _T("Add提示:[KAey1 KBey2 KCey3]"), _T("Remove提示") };
			res = ssget(_T(":$:?:K"), prompts, _T("KAey1 KBey2 KCey3 _ GK1 GK2 GK3"), nullptr, true);

			// 交互. 输入?即可看到关键字列表. 应该是"窗口(W)/栏选(F)/圈围(WP)/编组(G)/添加(A)/删除(R)/放弃(U)/单个(SI)"
			// 如果是"_+F+W+G", 则需要先输入关键字后才能进行对象选择
			res = ssget(_T("_+F+W+G+."), nullptr, nullptr, nullptr);
#undef POINT

			{
				create_layer(_T("辅助"), false, false);

				int i = 1;
				AcDbObjectId id;
				for (auto ent : other_ents) {
					ent->setColorIndex(i++);
					ent->setLayer(_T("辅助"));
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