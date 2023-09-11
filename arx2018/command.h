#pragma once

#include <acadstrc.h>
#include <dbid.h>
#include <dbmain.h>




// common 
void register_class();
void unregister_class();
void init_app(void* appId);
void unload_app();
// pEntity��Ҫ�ֶ��ر�
Acad::ErrorStatus add_to_model_space(AcDbObjectId& objId, AcDbEntity* pEntity);
void create_layer(const TCHAR* layer_name);
AcDbObjectId create_line();
AcDbObjectId create_circle();
void create_group(AcDbObjectIdArray& objIds, const TCHAR* pGroupName);
Acad::ErrorStatus change_color(AcDbObjectId entId, Adesk::UInt16 newColor);
Adesk::Boolean get_yorno(const TCHAR* pStr);






// ���ຯ��, ���ޱ�Ҫ, ��Ҫ�Ķ�. ��Ϊ��׼ʹ��. ��Ҫ��ʱ�޸�, ʹ���������
void test();



// used in acedRegcmds

// Beginner
// ѡ�����. ѡ����Բ��ʾ�ɹ�
void curves();
// ��ʾ. ����ͼ��, line, circle. �ı�circle��ɫ. ����������������
void mk_ents();
// ��ʾһ��MFC��ģ��Ի���, �̳���CAcUiDialog. ��һЩAcUi�ؼ���ʹ��
void model_dialog();
// ��ʾ������
void progress_meter();
// �Զ�������ʵ��, �а�Χ��, ��Ӱ
void custom_text();
// ����һ�����Ӹ��ӵ�ͼ��, ָ��������, ��ɫ��
void complex_layer();
// һ����Ӧ�ӿ�, �ܹ������Լ��������ö����ģ��ĵ�λԲ
void tessellate();



// Medium
// ͳ����������, �ĵ�״̬. ������AcEditorReactor��AcApDocManagerReactor��ʹ��
void cmd_count();
// ѡ��һ��������и���. ������block���ж���. ʹ��acedSSGet(_T("_:s:n")...)����acedNEntSelP
void highlight();
// ʹ�ö������չ�ֵ�, �洢һ���Զ������MyInventoryData
void ex_dict();
// ��դͼ�ε�ʹ��
void raster_image();
// �Զ���˵�
void context_menu(void* appId);



// High
// MDI��, ��, �ر�, ������ĵ�����
void docman();