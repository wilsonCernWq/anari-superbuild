#pragma once

#include <anari_test_scenes.h>
#include <iostream>

namespace anari {
namespace scenes {

// CornellBox definitions //////////////////////////////////////////////////////
inline anari::World CreateObjFile(anari::Device d)
{
  /*
    读取.obj文件以构建scene
    使用到的接口均来自于："anari_test_scenes.h"
  */
  std::string category = "file";
  const char *category_cstr = category.c_str();
  std::string name = "obj";
  const char *name_cstr = name.c_str();
  /*
    1. 创建场景
      在scenes的init函数中，已经为file-obj绑定了创建函数（见anari_test_scenes.cpp）：
      registerScene("file", "obj", sceneFileObj);
      sceneFileObj()会new一个FileObj实例
  */
  anari::scenes::SceneHandle scene = anari::scenes::createScene(d, category_cstr, name_cstr);
  if (!scene) {
    std::cerr << "Failed to create scene: " << category << "/" << name << std::endl;
    throw std::runtime_error("Failed to create scene");
  }
  /*
    2. 设置filename参数
      此处最好写绝对路径
      如果写相对路径, obj.cpp中用于解析basic_path的pathof()函数容易出问题
      导致后续找不到.mtl文件和纹理的png文件
      std::string filename = "./sponza/sponza.obj";
  */
 // 另，此处用多个单行注释会出错，非常诡异，渲染后得到一片漆黑，但是用多行注释就没事
  // std::string filename =
  //     "E:\\vis\\ANARI\\build\\anari\\src\\examples\\simple\\sponza\\sponza.obj";

    // std::string filename =
    //   "E:\\vis\\ANARI\\build\\anari\\src\\examples\\simple\\yucong\\yuqiong_diffuse_normal_dm_model.obj";
        std::string filename =
      "/mnt/scratch/fast0/qadwu/optix7course/models/sponza.obj";
  const char *filename_cstr = filename.c_str();
  anari::scenes::setParameter(scene, "fileName", filename_cstr);
  /*
    3. 提交scenes
      FileObj类继承自TestScene类，重写了commit方法
      FileObj commit时，会根据2中设置的filename参数去寻找.obj文件及配套文件
      然后调用obj.cpp中的loadObj方法来将它们加载进来
      加载obj、加载texture和偏移量对齐的工作都在loadObj方法中完成
  */
  printf("Load OBJ File...");
  anari::scenes::commit(scene);
  /*
    4. 通过scene句柄来生成world
      Get the ANARI world handle from the underlying scene: do not need to release
  */
  auto world = anari::scenes::getWorld(scene);
  return world;
}

}
}
