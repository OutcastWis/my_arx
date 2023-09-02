#pragma once

#include <acadstrc.h>
#include <dbid.h>
#include <dbmain.h>




// common 
void register_class();
void unregister_class();
void init_app();
void unload_app();
// pEntity需要手动关闭
Acad::ErrorStatus add_to_model_space(AcDbObjectId& objId, AcDbEntity* pEntity);
void create_layer(const TCHAR* layer_name);
AcDbObjectId create_line();
AcDbObjectId create_circle();
void create_group(AcDbObjectIdArray& objIds, const TCHAR* pGroupName);
Acad::ErrorStatus change_color(AcDbObjectId entId, Adesk::UInt16 newColor);











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
// 统计命令运行
void cmd_count();