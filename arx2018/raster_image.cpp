#include "pch.h"
#include "raster_image.h"

#include <imgdef.h>
#include <imgent.h>

#include "command.h"


namespace wzj {

    namespace detail {
        const TCHAR image_name[] = _T("atoll");     // 图片在字典中的名字
        const TCHAR image_path[] = _T("atoll.jpg"); // 实际图片路径. 请将该图片放置在AUTOCAD的寻找路径中

        // 修剪id表示的image. 默认image的定位点为左下角, 宽x高=分辨率. 宽为+x轴, 高为+y轴
        Acad::ErrorStatus manipulate(AcDbObjectId id) {
            AcDbRasterImage* rid;
            auto es = acdbOpenObject(rid, id, AcDb::kForWrite);
            if (es != Acad::eOk)
                return es;

            // 对图形设置坐标, 确定大小, 进行变化. 流程应该是先将图形左下角平移到新的坐标, 然后变换长宽. 再进行边界处理
            {
                // 设置坐标点, 宽, 高.
                if (!rid->setOrientation(AcGePoint3d(100, 100, 100), AcGeVector3d(500, 0, 0), AcGeVector3d(0, 200, 0)))
                    ads_printf(_T("Error: Set orientation failed.\n"));

                // 变形. 这里的坐标是像素点, 而像素点的坐标原点是图形的左上角, x轴向右+, y轴向下+. 等价于UI界面的坐标
                AcGePoint2dArray boundry;
                AcGePoint2d pts[5] = { {100, 100}, {300, 0}, {600, 200}, {100, 400}, {100, 100} };
                for (int i = 0; i < 5; ++i)
                    boundry.append(pts[i]);

                // 可以使用getPixelToModelTransform来获取世界坐标
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

                // 图形会变形为由boundary为边界的样式. 注意不是裁剪
                es = rid->setClipBoundary(AcDbRasterImage::kPoly, boundry); 
                if (es != Acad::eOk)
                    ads_printf(_T("Error: Set orientation failed.\n"));

            }

            rid->close();
        }
        /**
        * 创建reactor. 其作用是在definition改变的时候通知image重画
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
        * 创建一个光栅图形. 
        * @param [in] def_id, 光栅定义对象的id
        * @param [out] *id, 所创建的光栅图形id
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
        * 创建一个definition, 保存到字典中. 字典由AcDbRasterImageDef::createImageDictionary确定
        * @param [out] id, 光栅定义对象id
        * @param [in] image_name, 光栅定义对象在字典中的key. value为对象本身
        * @param [in] image_path, 实际图形文件的路径
        * @remark 如果image_name已存在在字典中, 则返回Acad::eHandleInUse, 并不会发生覆盖
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
        * 生成definition, 和2组image和reactor. 返回第2组image的id
        */
        Acad::ErrorStatus attach(AcDbObjectId* image_id, bool overwrite) {
            // 载入图形,生成光栅定义
            AcDbObjectId def_id;
            auto es = create_def(&def_id, image_name, image_path, overwrite);
            if (es != Acad::eOk)
                return es;

            // 依据光栅定义, 创建2个对象
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
        // 和access_image本质相同. 但可以覆盖definition在字典中的key, 而前者不行
        void access_image_overwrite() {
            AcDbObjectId id;
            auto es = attach(&id, true);
            if (es != Acad::eOk)
            {
                ads_printf(_T("Error: Attach example image failed.\n"));
                return;
            }
            // 对attach中第二个图形进行变化
            es = manipulate(id);
            if (es != Acad::eOk)
                ads_printf(_T("Error: Manipulate example image failed.\n"));
        }
    }

    void raster_image::init_impl()
    {
        acrxDynamicLinker->loadModule(_T("acISMobj22.dbx"), true); // 需要载入, 否则AcDbRasterImage*这些类都不认识

        acedRegCmds->addCommand(_T("WZJ_COMMAND_RASTER_IMAGE"), _T("GLOBAL_RI"), _T("LOCAL_RI"), ACRX_CMD_MODAL, detail::access_image);

        acedRegCmds->addCommand(_T("WZJ_COMMAND_RASTER_IMAGE"), _T("GLOBAL_RI_OVERWRITE"), _T("LOCAL_RI_OVERWRITE"), ACRX_CMD_MODAL, detail::access_image_overwrite);
    }

    void raster_image::stop_impl()
    {
        acedRegCmds->removeGroup(_T("WZJ_COMMAND_RASTER_IMAGE"));

        acrxDynamicLinker->unloadModule(_T("acISMobj22.dbx"), true); // 如何载入, 如何卸载. 即保持第二个参数一致
    }
}