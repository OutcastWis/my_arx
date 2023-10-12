### 主要头文件
```c++
#include <rxobject.h>
#include <rxregsvc.h>
#include <aced.h>
#include <acedads.h>	// aced全局函数
#include <dbelipse.h>
#include <dbsymtb.h>
#include <adslib.h>		// RTNORM, acdb, acut全局函数
#include <geassign.h>
#include <dbapserv.h>	// acdbSetHostApplicationServices
#include <dbents.h>		// 所有实体
```


### 画图, AcGi
* CAD有多种AcGi图形显示实现规范
1. **2D-display pipe**用于生成显示在屏幕上的2D图像
2. **3D system**对于持久呈现的视图
3. **proxy generation**. 在保存自定义对象时使用，将图形存储在元文件中，该元文件可以在自定义应用程序不可用的系统上加载图形时重播
4. **explode operation**. 把对象炸开成ObjectDBX基本图元，用于发现复杂对象的边界

* AcGiWorldDraw::regenType()
1. **kAcGiStandardDisplay** 典型的绘图模式，在用户发出REGEN命令或编辑条目时使用。在这种模式下，实体应该在线框中呈现
2. **kAcGiHideOrShadeCommand** 执行隐藏线删除，并指示HIDE或SHADE命令生效。在这种模式下，实体应该使用面来渲染。
3. **kAcGiShadedDisplay** 使用材料和照明来创建3D模型的逼真阴影图像，并在用户发出RENDER命令时使用。在这种模式下，实体应该使用面来渲染
4. **kAcGiForExplode** 用于爆炸操作.
5. **kAcGiSaveWorldDrawForProxy** 用于生成代理图形。在这种情况下，所有的渲染都应该在subWorldDraw()中完成，因为subViewportDraw()不支持代理图形。
6. **eAcGiRegenTypeInvalid** 无效


### 嵌套对象, nested entity
* 嵌套对象的格式是: block ref -> block ref -> ... -> entity
* 嵌套对象的操作需要使用AcDbFullSubentPath. 否则使用的坐标是用户坐标, 而不是世界坐标, 会产生奇怪的BUG. 例如高亮时, 会在其他位置生成无法操作的高亮块.
* 对于face, edge, vertex等, 在此基础上, 需要使用getSubentPathsAtGsMarker
* 详见highlight_subentity


### 坐标系
1. WCS, World coordinate system. 作为参考坐标系存在, 其他所有坐标系皆是相对于它. 不会改变
2. UCS, User coordinate system. 用户界面交互使用的坐标. 但在DXF中, 存的是WCS
3. OCS, Object coordinate system(以前版本叫ECS). 主要用在2D对象放到3D空间中. CAD中使用标,1.系的有:  ARC, CIRCLE, TEXT, LWPOLYLINE, HATCH, SOLID, TRACE, INSERT, IMAGE
4. DCS, Display coordinate system. 模型再CAD内部, 经过三次转换Model coordinates + [Entity block transform] => World conordinates + [viewport/view transform] => Eye coordinates + [Perspective tranform] => Display coordinates. DCS的坐标原点被记录在系统变量TARGET中. 和viewport相关的画图, 应该使用该坐标系, 详见MyGlyph::subViewportDraw中注释



### 快捷菜单, AcEdUiContext
* 有5种:
1. **Hot Grip Cursor** 右键选中对象的grip
2. **Object Snap Cursor** 按住shift并且右键
3. **Default Mode** 既无对象选中, 也不在命令中时, 右键
4. **Command Mode** 在命令中时右键
5. **Edit Mode** 不在命令中, 选中对象后, 但没有选中grip, 右键

* 快捷菜单在cui里面有对应的别名
1. 默认菜单: POP501, CMDEFAULT
2. 编辑菜单: POP502, CMEDIT
3. 命令菜单: POP503, CMCOMMAND
上述三者必须存在, 否则对应的命令模式无法使用. 例如, cui中不存在POP501, CMDEFAULT时, Default Mode便不能使用, 即使在程序中进行了注册
* 详见context_menu

* 命令模式能否正确显示还依赖于右键的配置. 在options->用户系统配置->自定义右键单击->命令模式 中, 如果选择的是"快捷菜单: 命令选项存在时可用", 则在没有可用的选项时，右键与按 Enter 键的效果一样。. 选择"快捷菜单: 总是启用"则可以.
* "命令选项存在时可用"需要在使用acedget*中, 对参数prompt进行设定. prompt格式如下:
```
Description: 
Current instruction (descriptive info) or 
[Option1/oPtion2/opTion3/...] <default option>:
```
> 主要是第3行中, 对option的设置. 其他的可以随意. 注意每行不要超80个字符, 不要超过3行.
* 详见context_menu.cpp中detail::cmd函数


### CTRL+C work as cancel or copy ?
* CTRL+C在cad2018上的表现是复制. 在cui中取消改键盘快捷方式后, CTRL+C无任何效果. 不知道如何实现帮助文档中CTRL+C等于cancel的效果. 但公司软件可以用CTRL+C进行取消, 其效果和ESC不同. CTRL+C取消后, 需要用acedUsrBrk来擦除, 否则会影响同命令的下一个acedget*, 导致直接返回RTCAN. 而ESC则不会, 即不需要acedUsrBrk进行擦除


### 文档上下文的切换
* 旧版本CAD是fiber, 切换文档会导致原文档的命令挂起. 但近期的版本不是, 起码2018不是, 即当前文档命令不再挂起, 而是执行完成后, 在进行实际的文档切换. 
* 所有新建, 打开, 激活等都可视为文档上下文的切换
* **current document**指的是正在操作的上下文所属文档, 通过curDocument获取
* **activated document**指的是界面上最上层的文档, 通过mdiActiveDocument获取
* 上述二者可以不同. 但激活一个文档时, 它一定是current document. 后续可以通过setCurDocument进行改变, 从而使二者不同. 退出命令时, 确保二者统一


### Objectarx Reference
* 有2种:
1. **ownership reference**, 父子关系, 或者说包含关系
2. **pointer reference**, 普通引用关系
* 上述2种都存在hard和soft的区别. 
*
||soft|hard|
|-|-|-|
|ownership||owner死, 自身也死|
|pointer|类比C中的指针|类比引用计数. 计数不为0, 被引用的对象不能被清除|
* soft ownership举例:
1. 除*MODEL_SPACE, *PAPER_SPACE, *PAPER_SPACE0, layer 0外的其他符号表(sumbol table), 对其包含的元素都是soft ownership
2. 字典都是(2018下扩展字典是hard ownership, 旧版本扩展字典也是soft. 命名字典是soft)




### Deep clone和wblock, 统称深拷贝
* 主要分2步走, 首先拷贝所有有关的对象(cloning), 然后再更新对象之间的引用为拷贝后的新id(translating).
* 有三个主要函数来实现深拷贝:
```c++
AcDbDatabase::deepCloneObjects()	// 不能跨database使用
AcDbDatabase::wblock()
AcDbDatabase::insert()              // 配合wblock使用
```
* deep clone和wblock都用一样的数据填充的手段. new一个新对象, 旧对象通过dwgOut输出数据流, 新对象用dwgIn来读取该数据流
* 
|起作用|hard|soft|
|-|-|-|
|ownership|deep clone & wblock|deep clone|
|pointer|wblock||



### 注意点
* 宏ACRX_DXF_DEFINE_MEMBERS的倒数第二项一定要大写
* AcFile::fprintf存在BUG. 64位下, 不能正确输出wchar_t. fprintf(L"%s", L"ABC"), 只会显示A. 设置setCharFormat为UTF8后依旧无效, 或者使用已有的FILE*进行attach依旧不行
* acedSSGet中"C"和"W"模式, 需要框选对象在视图中, 否则返回RTERROR, 即不能选中对象
* 当你的命令需要使用MFC资源时, 确保用addCommand添加命令时, 用AC_DECLARE_EXTENSION_MODULE. 或者在命令实现中, 用CAcModuleResourceOverride
