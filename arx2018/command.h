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
* ��Ӷ���ͼֽ��
* @param objId [out]
* @param *pEntity [in]
* @param *pDb [in]
* @remark pEntity��Ҫ�ֶ��ر�. 
*/
Acad::ErrorStatus add_to_model_space(AcDbObjectId& objId, AcDbEntity* pEntity, AcDbDatabase* pDb = nullptr);
void create_layer(const TCHAR* layer_name);
AcDbObjectId create_line();
AcDbObjectId create_circle();
void create_group(AcDbObjectIdArray& objIds, const TCHAR* pGroupName);
Acad::ErrorStatus change_color(AcDbObjectId entId, Adesk::UInt16 newColor);
Adesk::Boolean getYorN(const TCHAR* pStr);
// ��ȡ�����·��
CString desktop_url();
// ��ȡ������CADϵͳ����
int get_int_sysvar(const TCHAR* var);
void set_int_sysvar(const TCHAR* var, int value);
/**
* ����sset�еĶ���. sset������single selection
* @param sset [in]
* @param pick_point [out], ѡ�ж��������
* @param *gs [out], gs marker
* @param *pick [out], ѡ�ж����id
* @return ������Ƕ����, ��������㵽���ڲ�(*pick). ������ͨ����, ��������(*pick)
*/
AcDbObjectIdArray ContainerIdsAndEntity(ads_name sset, ads_point pick_point, Adesk::GsMarker* gs, AcDbObjectId* pick);





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
// �Զ����Ҽ��˵�
void context_menu(void* appId);
// ��option�Ի��������tab
void extend_tabs(void* appId);
// ��ģ̬�Ի���
void modeless_dialog();



// High
// MDI��, ��, �ر�, ������ĵ�����. ��һЩapplication context��document context�Ľ���
void docman();
// ʹ��block_order.dbx, ʵ�ֿ��ж������˳��ĸı�
void block_order();
// ǳ���������
void clone_work();
// jig�ļ�ʹ��. ����drag() + AcGiDrawable, ������sampler()->update()->entity()������
void jig();
// ά�����ĵ�������. ͬʱ���ṩ�������jig����, ��sampler()->update()->entity()
void data_per_doc();
// Ϊarc��pline�������ֵ���Ϊ��׽��. ʹ���Զ���Ĳ�׽����ʾ
void osnap();


// Advanced
// ʹ��AcGi
void custom_geo();