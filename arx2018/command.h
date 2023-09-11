#pragma once

#include <acadstrc.h>
#include <dbid.h>
#include <dbmain.h>




// common 
void register_class();
void unregister_class();
void init_app(void* appId);
void unload_app();
// pEntity需要手动关闭
Acad::ErrorStatus add_to_model_space(AcDbObjectId& objId, AcDbEntity* pEntity);
void create_layer(const TCHAR* layer_name);
AcDbObjectId create_line();
AcDbObjectId create_circle();
void create_group(AcDbObjectIdArray& objIds, const TCHAR* pGroupName);
Acad::ErrorStatus change_color(AcDbObjectId entId, Adesk::UInt16 newColor);
Adesk::Boolean get_yorno(const TCHAR* pStr);






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
// 自定义菜单
void context_menu(void* appId);



// High
// MDI下, 打开, 关闭, 激活等文档操作
void docman();