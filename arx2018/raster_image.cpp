#include "pch.h"
#include "raster_image.h"

#include <imgdef.h>
#include <imgent.h>

#include "command.h"


namespace wzj {

    namespace detail {
        const TCHAR image_name[] = _T("atoll");     // ͼƬ���ֵ��е�����
        const TCHAR image_path[] = _T("atoll.jpg"); // ʵ��ͼƬ·��. �뽫��ͼƬ������AUTOCAD��Ѱ��·����

        // �޼�id��ʾ��image. Ĭ��image�Ķ�λ��Ϊ���½�, ��x��=�ֱ���. ��Ϊ+x��, ��Ϊ+y��
        Acad::ErrorStatus manipulate(AcDbObjectId id) {
            AcDbRasterImage* rid;
            auto es = acdbOpenObject(rid, id, AcDb::kForWrite);
            if (es != Acad::eOk)
                return es;

            // ��ͼ����������, ȷ����С, ���б仯. ����Ӧ�����Ƚ�ͼ�����½�ƽ�Ƶ��µ�����, Ȼ��任����. �ٽ��б߽紦��
            {
                // ���������, ��, ��.
                if (!rid->setOrientation(AcGePoint3d(100, 100, 100), AcGeVector3d(500, 0, 0), AcGeVector3d(0, 200, 0)))
                    ads_printf(_T("Error: Set orientation failed.\n"));

                // ����. ��������������ص�, �����ص������ԭ����ͼ�ε����Ͻ�, x������+, y������+. �ȼ���UI���������
                AcGePoint2dArray boundry;
                AcGePoint2d pts[5] = { {100, 100}, {300, 0}, {600, 200}, {100, 400}, {100, 100} };
                for (int i = 0; i < 5; ++i)
                    boundry.append(pts[i]);

                // ����ʹ��getPixelToModelTransform����ȡ��������
                {
                    AcGeMatrix3d mtx;
                    rid->getPixelToModelTransform(mtx);
                    for (int i = 0; i < 5; ++i) {
                        AcGePoint3d pt(pts[i].x, pts[i].y, 0);
                        pt = pt.transformBy(mtx);
                        ads_printf(_T("\nPixel vs WCS: {%.3f, %.3f} => {%.3f, %.3f, %.3f}\n"), pts[i].x, pts[i].y,
                            pt.x, pt.y, pt.z);
                    }
                }

                // ͼ�λ����Ϊ��boundaryΪ�߽����ʽ. ע�ⲻ�ǲü�
                es = rid->setClipBoundary(AcDbRasterImage::kPoly, boundry); 
                if (es != Acad::eOk)
                    ads_printf(_T("Error: Set orientation failed.\n"));

            }

            rid->close();
        }
        /**
        * ����reactor. ����������definition�ı��ʱ��֪ͨimage�ػ�
        */
        Acad::ErrorStatus create_image_reactor(AcDbObjectId def_id, AcDbObjectId id) {
            // Disable image definition notification while changing the definition reactor list
            AcDbRasterImageDefReactor::setEnable(Adesk::kFalse);

            // open definition
            AcDbRasterImageDef* rid;
            auto es = acdbOpenObject(rid, def_id, AcDb::kForWrite);
            if (es != Acad::eOk) {
                rid->close();
                return es;
            }

            // open entity
            AcDbRasterImage* ri;
            es = acdbOpenObject(ri, id, AcDb::kForWrite);
            if (es != Acad::eOk) {
                rid->close();
                ri->close();
                return es;
            }

            // create reactor and set it's owner
            AcDbRasterImageDefReactor* ridr = new AcDbRasterImageDefReactor;   
            es = ridr->setOwnerId(id);

            // add to model space
            AcDbObjectId reactor_id;
            es = acdbHostApplicationServices()->workingDatabase()->addAcDbObject(reactor_id, ridr);

            //
            ri->setReactorId(reactor_id);
            rid->addPersistentReactor(reactor_id);

            //
            ridr->close();
            rid->close();
            ri->close();

            AcDbRasterImageDefReactor::setEnable(Adesk::kTrue);
            return es;
        }

        /**
        * ����һ����դͼ��. 
        * @param [in] def_id, ��դ��������id
        * @param [out] *id, �������Ĺ�դͼ��id
        */
        Acad::ErrorStatus create_image(AcDbObjectId def_id, AcDbObjectId* id) {
            AcDbRasterImage* ri = new AcDbRasterImage;
            if (ri == nullptr)
                return Acad::eNullEntityPointer;

            auto es = ri->setImageDefId(def_id);
            if (es != Acad::eOk)
            {
                delete ri;
                return es;
            }

            es = add_to_model_space(*id, ri);
            if (es != Acad::eOk) {
                delete ri;
                return es;
            }

            ri->close();
            return es;
        }

        /**
        * ����һ��definition, ���浽�ֵ���. �ֵ���AcDbRasterImageDef::createImageDictionaryȷ��
        * @param [out] id, ��դ�������id
        * @param [in] image_name, ��դ����������ֵ��е�key. valueΪ������
        * @param [in] image_path, ʵ��ͼ���ļ���·��
        * @remark ���image_name�Ѵ������ֵ���, �򷵻�Acad::eHandleInUse, �����ᷢ������
        */
        Acad::ErrorStatus create_def(AcDbObjectId* id, const TCHAR* image_name, const TCHAR* image_path, bool overwrite) {
            AcDbRasterImageDef* rid = new AcDbRasterImageDef;
            if (rid == nullptr)
                return Acad::eNullObjectPointer;

            // set source
            auto es = rid->setSourceFileName(image_path);
            if (es != Acad::eOk) {
                ads_printf(_T("Error: Could not find the image file.\n"));
                delete rid;
                return es;
            }
            // load image
            es = rid->load();
            if (es != Acad::eOk) {
                ads_printf(_T("Error: Could not open the image file.\n"));
                delete rid;
                return es;
            }
            // get dictionary id
            auto dict_id = AcDbRasterImageDef::imageDictionary(acdbHostApplicationServices()->workingDatabase());
            if (dict_id.asOldId() == 0)
            { // no dictionary open
                es = AcDbRasterImageDef::createImageDictionary(acdbHostApplicationServices()->workingDatabase(), dict_id);
                if (es != Acad::eOk) {
                    delete rid;
                    return es;
                }
            }
            // open the dictionary
            AcDbObjectPointer<AcDbDictionary> dict(dict_id, AcDb::kForWrite);
            if (dict.openStatus() != Acad::eOk) {
                delete rid;
                return es;
            }
            // check if the image name is already in use
            if (!dict->has(image_name))
                es = dict->setAt(image_name, rid, *id);
            else {
                delete rid;
                return overwrite ? dict->getAt(image_name, *id) : Acad::eHandleInUse;
            }

            rid->close();
            return es;
        }

        /**
        * ����definition, ��2��image��reactor. ���ص�2��image��id
        */
        Acad::ErrorStatus attach(AcDbObjectId* image_id, bool overwrite) {
            // ����ͼ��,���ɹ�դ����
            AcDbObjectId def_id;
            auto es = create_def(&def_id, image_name, image_path, overwrite);
            if (es != Acad::eOk)
                return es;

            // ���ݹ�դ����, ����2������
            const int count = 2;
            for (int i = 0; i < count; ++i) {
                es = create_image(def_id, image_id);
                if (es != Acad::eOk)
                    return es;

                es = create_image_reactor(def_id, *image_id);
                if (es != Acad::eOk)
                    return es;
            }

            return es;
        }

        void access_image() {
            AcDbObjectId id;
            auto es = attach(&id, false);
            if (es != Acad::eOk)
            {
                ads_printf(_T("Error: Attach example image failed.\n"));
                return;
            }

            es = manipulate(id);
            if (es != Acad::eOk)
                ads_printf(_T("Error: Manipulate example image failed.\n"));
        }
        // ��access_image������ͬ. �����Ը���definition���ֵ��е�key, ��ǰ�߲���
        void access_image_overwrite() {
            AcDbObjectId id;
            auto es = attach(&id, true);
            if (es != Acad::eOk)
            {
                ads_printf(_T("Error: Attach example image failed.\n"));
                return;
            }
            // ��attach�еڶ���ͼ�ν��б仯
            es = manipulate(id);
            if (es != Acad::eOk)
                ads_printf(_T("Error: Manipulate example image failed.\n"));
        }
    }

    void raster_image::init_impl()
    {
        acrxDynamicLinker->loadModule(_T("acISMobj22.dbx"), true); // ��Ҫ����, ����AcDbRasterImage*��Щ�඼����ʶ

        acedRegCmds->addCommand(_T("WZJ_COMMAND_RASTER_IMAGE"), _T("GLOBAL_RI"), _T("LOCAL_RI"), ACRX_CMD_MODAL, detail::access_image);

        acedRegCmds->addCommand(_T("WZJ_COMMAND_RASTER_IMAGE"), _T("GLOBAL_RI_OVERWRITE"), _T("LOCAL_RI_OVERWRITE"), ACRX_CMD_MODAL, detail::access_image_overwrite);
    }

    void raster_image::stop_impl()
    {
        acedRegCmds->removeGroup(_T("WZJ_COMMAND_RASTER_IMAGE"));

        acrxDynamicLinker->unloadModule(_T("acISMobj22.dbx"), true); // �������, ���ж��. �����ֵڶ�������һ��
    }
}