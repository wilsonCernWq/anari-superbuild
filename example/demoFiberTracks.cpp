// Copyright 2021-2024 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <array>
// anari
#define ANARI_EXTENSION_UTILITY_IMPL
#include "anari/anari_cpp.hpp"
#include "anari/anari_cpp/ext/std.h"
// stb_image
#include "stb_image_write.h"

#include <vtkm/io/VTKDataSetReader.h>
#include <vtkm/cont/CellSetSingleType.h>
#include <vtkm/cont/ArrayCopy.h>
#include <vtkm/cont/ColorTable.h>
#include <vtkm/cont/ColorTableSamples.h>

#include "PolyLineDataReader.h"

using uvec2 = std::array<unsigned int, 2>;
using uvec3 = std::array<unsigned int, 3>;
using vec3 = std::array<float, 3>;
using vec4 = std::array<float, 4>;
using box3 = std::array<vec3, 2>;

static void statusFunc(const void *userData,
    ANARIDevice device,
    ANARIObject source,
    ANARIDataType sourceType,
    ANARIStatusSeverity severity,
    ANARIStatusCode code,
    const char *message)
{
  (void)userData;
  (void)device;
  (void)source;
  (void)sourceType;
  (void)code;
  if (severity == ANARI_SEVERITY_FATAL_ERROR) {
    fprintf(stderr, "[FATAL] %s\n", message);
  } else if (severity == ANARI_SEVERITY_ERROR) {
    fprintf(stderr, "[ERROR] %s\n", message);
  } else if (severity == ANARI_SEVERITY_WARNING) {
    fprintf(stderr, "[WARN ] %s\n", message);
  } else if (severity == ANARI_SEVERITY_PERFORMANCE_WARNING) {
    fprintf(stderr, "[PERF ] %s\n", message);
  } else if (severity == ANARI_SEVERITY_INFO) {
    fprintf(stderr, "[INFO ] %s\n", message);
  } else if (severity == ANARI_SEVERITY_DEBUG) {
    fprintf(stderr, "[DEBUG] %s\n", message);
  }
}

static void onFrameCompletion(const void *, anari::Device d, anari::Frame f)
{
  printf("anari::Device(%p) finished rendering anari::Frame(%p)!\n", d, f);
}

template <typename T>
static T getPixelValue(uvec2 coord, int width, const T *buf)
{
  return buf[coord[1] * width + coord[0]];
}

int main(int argc, const char **argv)
{
  (void)argc;
  (void)argv;
  stbi_flip_vertically_on_write(1);

  // load dataset
  printf("loading ... %s", argv[1]);
  vtkm::io::PolyLineDataReader reader(argv[1]);
  auto ds = reader.ReadDataSet();
  ds.PrintSummary(std::cout);
  std::cout << std::endl;

  auto coords = ds.GetCoordinateSystem().GetData().AsArrayHandle<vtkm::cont::ArrayHandle<vtkm::Vec3f_32>>();
  auto curves = ds.GetCellSet().AsCellSet<vtkm::cont::CellSetExplicit<>>();

  vtkm::cont::ArrayHandle<vtkm::UInt32> curve_offsets;
  vtkm::cont::ArrayCopyShallowIfPossible(curves.GetOffsetsArray(vtkm::TopologyElementTagCell(), vtkm::TopologyElementTagPoint()), curve_offsets);
  auto curve_offsets_reader = curve_offsets.ReadPortal();

  std::vector<uint32_t> curve_indices;
  for (int i = 1; i < curve_offsets.GetNumberOfValues(); i++) {
    auto offset_curr = curve_offsets_reader.Get(i - 1);
    auto offset_next = curve_offsets_reader.Get(i);
    auto num_segments = offset_next - offset_curr - 1;
    for (int j = 0; j < num_segments; j++)
      curve_indices.push_back(offset_curr+j);
  }

  std::vector<uint32_t> curve_lengths;
  curve_lengths.resize(curve_offsets.GetNumberOfValues()-1);
  uint32_t curve_length_max = 0;
  for (int i = 1; i < curve_offsets.GetNumberOfValues(); i++) {
    auto offset_curr = curve_offsets_reader.Get(i - 1);
    auto offset_next = curve_offsets_reader.Get(i);
    curve_lengths[i-1] = offset_next - offset_curr;
    curve_length_max = std::max(curve_length_max, curve_lengths[i-1]);
  }
  std::cout << "max length: " << curve_length_max << std::endl;

  vtkm::cont::ColorTable color_table(vtkm::cont::ColorTable::Preset::CoolToWarm);
  color_table.SetColorSpace(vtkm::ColorSpace::Diverging);

  vtkm::cont::ColorTableSamplesRGB color_table_samples;
  color_table.Sample(256, color_table_samples);
  auto color_table_portal = color_table_samples.Samples.ReadPortal();

  std::vector<vec3> vertex_colors;
  vertex_colors.resize(coords.GetNumberOfValues());
  int primID = 0;
  for (int i = 0; i < coords.GetNumberOfValues(); i++) {
    auto end = curve_offsets_reader.Get(primID+1);
    if (i == end) {
      primID++;
    }
    auto v = curve_lengths[primID] / (float)curve_length_max;
    auto c = color_table_portal.Get(v * color_table_samples.NumberOfSamples);
    vertex_colors[i] = {
      c[0] / 255.f,
      c[1] / 255.f,
      c[2] / 255.f
    };
  }

  // image size
  uvec2 img_size = {1400 /*width*/, 2000 /*height*/};

  // camera
  vec3 cam_pos = {6.9f, 30.3f, 200.f};
  vec3 cam_focal = {1.086f, 19.150f, 103.543f};
  vec3 cam_up = {-0.221713f, -0.97421f, 0.0419261f};
  vec3 cam_view = {
    cam_focal[0] - cam_pos[0],
    cam_focal[1] - cam_pos[1],
    cam_focal[2] - cam_pos[2],
  };

  printf("initialize ANARI...");
  anari::Library lib = anari::loadLibrary("helide", statusFunc);
  anari::Extensions extensions = anari::extension::getDeviceExtensionStruct(lib, "default");

  if (!extensions.ANARI_KHR_GEOMETRY_TRIANGLE)
    printf("WARNING: device doesn't support ANARI_KHR_GEOMETRY_TRIANGLE\n");
  if (!extensions.ANARI_KHR_CAMERA_PERSPECTIVE)
    printf("WARNING: device doesn't support ANARI_KHR_CAMERA_PERSPECTIVE\n");
  if (!extensions.ANARI_KHR_MATERIAL_MATTE)
    printf("WARNING: device doesn't support ANARI_KHR_MATERIAL_MATTE\n");
  if (!extensions.ANARI_KHR_FRAME_COMPLETION_CALLBACK) {
    printf(
        "INFO: device doesn't support ANARI_KHR_FRAME_COMPLETION_CALLBACK\n");
  }

  anari::Device d = anari::newDevice(lib, "default");

  printf("done!\n");
  printf("setting up camera...");

  // create and setup camera
  auto camera = anari::newObject<anari::Camera>(d, "perspective");
  anari::setParameter(d, camera, "aspect", (float)img_size[0] / (float)img_size[1]);
  anari::setParameter(d, camera, "position", cam_pos);
  anari::setParameter(d, camera, "direction", cam_view);
  anari::setParameter(d, camera, "up", cam_up);
  anari::commitParameters(d, camera); // commit objects to indicate setting parameters is done

  printf("done!\n");
  printf("setting up scene...");

  // The world to be populated with renderable objects
  auto world = anari::newObject<anari::World>(d);

  // create and setup surface and mesh
  auto mesh = anari::newObject<anari::Geometry>(d, "curve");
  {
    vtkm::cont::Token token;
    auto* ptr = (vec3*)coords.GetBuffers()[0].ReadPointerHost(token);
    anari::setParameterArray1D(d, mesh, "vertex.position", ptr, coords.GetNumberOfValues());
  }
  // {
  //   vtkm::cont::Token token;
  //   auto* ptr = (uint32_t*)curve_offsets.GetBuffers()[0].ReadPointerHost(token);
  //   anari::setParameterArray1D(d, mesh, "primitive.index", ptr, 2);
  // }
  anari::setParameterArray1D(d, mesh, "primitive.index", curve_indices.data(), curve_indices.size());

  anari::setParameterArray1D(d, mesh, "vertex.color", vertex_colors.data(), vertex_colors.size());

  anari::setParameter(d, mesh, "radius", 0.25f);
  anari::commitParameters(d, mesh);

  auto mat = anari::newObject<anari::Material>(d, "matte");
  anari::setParameter(d, mat, "color", "color");
  // anari::setParameter(d, mat, "color", vec3{0.9f, 0.9f, 0.9f});
  anari::commitParameters(d, mat);

  // put the mesh into a surface
  auto surface = anari::newObject<anari::Surface>(d);
  anari::setAndReleaseParameter(d, surface, "geometry", mesh);
  anari::setAndReleaseParameter(d, surface, "material", mat);
  anari::setParameter(d, surface, "id", 2u);
  anari::commitParameters(d, surface);

  // put the surface directly onto the world
  anari::setParameterArray1D(d, world, "surface", &surface, 1);
  anari::setParameter(d, world, "id", 3u);
  anari::release(d, surface);

  anari::Light light = anari::newObject<anari::Light>(d, "directional");
  anari::setParameter(d, light, "direction", vec3{0.f, -0.5f, 1.f});
  anari::commitParameters(d, light);

  anari::setAndReleaseParameter(d, world, "light", anari::newArray1D(d, &light));
  anari::release(d, light);

  anari::commitParameters(d, world);

  printf("done!\n");

  // print out world bounds
  box3 worldBounds;
  if (anari::getProperty(d, world, "bounds", worldBounds, ANARI_WAIT)) {
    printf("\nworld bounds: ({%f, %f, %f}, {%f, %f, %f}\n\n",
        worldBounds[0][0],
        worldBounds[0][1],
        worldBounds[0][2],
        worldBounds[1][0],
        worldBounds[1][1],
        worldBounds[1][2]);
  } else {
    printf("\nworld bounds not returned\n\n");
  }

  printf("setting up renderer...");

  // create renderer
  auto renderer = anari::newObject<anari::Renderer>(d, "default");
  // objects can be named for easier identification in debug output etc.
  anari::setParameter(d, renderer, "name", "MainRenderer");
  anari::setParameter(d, renderer, "ambientRadiance", 1.f);
  anari::commitParameters(d, renderer);

  printf("done!\n");

  // create and setup frame
  auto frame = anari::newObject<anari::Frame>(d);
  anari::setParameter(d, frame, "size", img_size);
  anari::setParameter(d, frame, "channel.color", ANARI_UFIXED8_RGBA_SRGB);
  anari::setParameter(d, frame, "channel.primitiveId", ANARI_UINT32);
  anari::setParameter(d, frame, "channel.objectId", ANARI_UINT32);
  anari::setParameter(d, frame, "channel.instanceId", ANARI_UINT32);

  anari::setAndReleaseParameter(d, frame, "renderer", renderer);
  anari::setAndReleaseParameter(d, frame, "camera", camera);
  anari::setAndReleaseParameter(d, frame, "world", world);

  anari::setParameter(d,
      frame,
      "frameCompletionCallback",
      (anari::FrameCompletionCallback)onFrameCompletion);

  anari::commitParameters(d, frame);

  printf("rendering frame to firstFrame.png...\n");

  // render one frame
  for (int i = 0; i < 1; i++)
    anari::render(d, frame);
  anari::wait(d, frame);

  // access frame and write its content as PNG file
  auto fb = anari::map<uint32_t>(d, frame, "channel.color");
  stbi_write_png("demo_output.png",
      int(fb.width),
      int(fb.height),
      4,
      fb.data,
      4 * int(fb.width));
  anari::unmap(d, frame, "channel.color");

  printf("...done!\n");

  printf("\ncleaning up objects...");

  // final cleanups
  anari::release(d, frame);
  anari::release(d, d);
  anari::unloadLibrary(lib);

  printf("done!\n");

  return 0;
}
