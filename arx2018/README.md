### ��Ҫͷ�ļ�
```c++
#include <rxobject.h>
#include <rxregsvc.h>
#include <aced.h>
#include <dbelipse.h>
#include <dbsymtb.h>
#include <adslib.h>		// RTNORM, acdb, acutȫ�ֺ���
#include <geassign.h>
#include <dbapserv.h>	// acdbSetHostApplicationServices
#include <dbents.h>		// ����ʵ��
```


### ��ͼ, AcGi
* CAD�ж���AcGiͼ����ʾʵ�ֹ淶
1. **2D-display pipe**����������ʾ����Ļ�ϵ�2Dͼ��
2. **3D system**���ڳ־ó��ֵ���ͼ
3. **proxy generation**. �ڱ����Զ������ʱʹ�ã���ͼ�δ洢��Ԫ�ļ��У���Ԫ�ļ��������Զ���Ӧ�ó��򲻿��õ�ϵͳ�ϼ���ͼ��ʱ�ز�
4. **explode operation**. �Ѷ���ը����ObjectDBX����ͼԪ�����ڷ��ָ��Ӷ���ı߽�

* AcGiWorldDraw::regenType()
1. **kAcGiStandardDisplay** ���͵Ļ�ͼģʽ�����û�����REGEN�����༭��Ŀʱʹ�á�������ģʽ�£�ʵ��Ӧ�����߿��г���
2. **kAcGiHideOrShadeCommand** ִ��������ɾ������ָʾHIDE��SHADE������Ч��������ģʽ�£�ʵ��Ӧ��ʹ��������Ⱦ��
3. **kAcGiShadedDisplay** ʹ�ò��Ϻ�����������3Dģ�͵ı�����Ӱͼ�񣬲����û�����RENDER����ʱʹ�á�������ģʽ�£�ʵ��Ӧ��ʹ��������Ⱦ
4. **kAcGiForExplode** ���ڱ�ը����.
5. **kAcGiSaveWorldDrawForProxy** �������ɴ���ͼ�Ρ�����������£����е���Ⱦ��Ӧ����subWorldDraw()����ɣ���ΪsubViewportDraw()��֧�ִ���ͼ�Ρ�
6. **eAcGiRegenTypeInvalid** ��Ч


### ע���
* ��ACRX_DXF_DEFINE_MEMBERS�ĵ����ڶ���һ��Ҫ��д