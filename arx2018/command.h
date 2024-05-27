#pragma once

#include <acadstrc.h>
#include <dbid.h>
#include <dbmain.h>
#include <type_traits>



// common 
void register_class();
void unregister_class();
void init_app(void* appId);
void unload_app();
/**
* 添加对象到图纸中
* @param objId [out]
* @param *pEntity [in]
* @param *pDb [in]
* @remark pEntity需要手动关闭. 
*/
Acad::ErrorStatus add_to_model_space(AcDbObjectId& objId, AcDbEntity* pEntity, AcDbDatabase* pDb = nullptr);
void create_layer(const TCHAR* layer_name);
AcDbObjectId create_line();
AcDbObjectId create_circle();
void create_group(AcDbObjectIdArray& objIds, const TCHAR* pGroupName);
Acad::ErrorStatus change_color(AcDbObjectId entId, Adesk::UInt16 newColor);
Adesk::Boolean getYorN(const TCHAR* pStr);
// 获取桌面的路径
CString desktop_url();
// 获取和设置CAD系统变量
int get_int_sysvar(const TCHAR* var);
void set_int_sysvar(const TCHAR* var, int value);
/**
* 解析sset中的对象. sset必须是single selection
* @param sset [in]
* @param pick_point [out], 选中对象的坐标
* @param *gs [out], gs marker
* @param *pick [out], 选中对象的id
* @return 对于内嵌对象, 返回最外层到最内层(*pick). 对于普通对象, 返回自身(*pick)
*/
AcDbObjectIdArray ContainerIdsAndEntity(ads_name sset, ads_point pick_point, Adesk::GsMarker* gs, AcDbObjectId* pick);





// 其余函数, 若无必要, 不要改动. 皆为标准使用. 需要临时修改, 使用下面这个
void test();



// used in acedRegcmds

// Beginner
// 选择对象. 选中椭圆提示成功
void curves();
// 演示. 创建图层, line, circle. 改变circle颜色. 并将他俩创建成组
void mk_ents();
// 显示一个MFC下模块对话框, 继承自CAcUiDialog. 和一些AcUi控件的使用
void model_dialog();
// 显示进度条
void progress_meter();
// 自定义文字实体, 有包围框, 重影
void custom_text();
// 创建一个更加复杂的图层, 指定了线型, 颜色等
void complex_layer();
// 一个适应视口, 能够调整自己边数的用多边形模拟的单位圆
void tessellate();



// Medium
// 统计命令运行, 文档状态. 本质是AcEditorReactor和AcApDocManagerReactor的使用
void cmd_count();
// 选择一个对象进行高亮. 可以是block的中对象. 使用acedSSGet(_T("_:s:n")...)或者acedNEntSelP
void highlight();
// 使用对象的扩展字典, 存储一个自定义对象MyInventoryData
void ex_dict();
// 光栅图形的使用
void raster_image();
// 自定义右键菜单
void context_menu(void* appId);
// 向option对话框中添加tab
void extend_tabs(void* appId);
// 非模态对话框
void modeless_dialog();



// High
// MDI下, 打开, 关闭, 激活等文档操作. 和一些application context和document context的解释
void docman();
// 使用block_order.dbx, 实现块中对象遍历顺序的改变
void block_order();
// 浅拷贝和深拷贝
void clone_work();
// jig的简单使用. 用了drag() + AcGiDrawable, 而不是sampler()->update()->entity()的流程
void jig();
// 维护各文档的数据. 同时又提供了正规的jig流程, 即sampler()->update()->entity()
void data_per_doc();
// 为arc和pline增加三分点作为捕捉点. 使用自定义的捕捉点显示
void osnap();


// Advanced
// 使用AcGi
void custom_geo();