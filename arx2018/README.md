### 主要头文件
```c++
#include <rxobject.h>
#include <rxregsvc.h>
#include <aced.h>
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


### 注意点
* 宏ACRX_DXF_DEFINE_MEMBERS的倒数第二项一定要大写