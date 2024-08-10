// Copyright 2021-2024 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "scene_FiberTracks.h"

#include "stb_image_write.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <array>

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

  // image size
  uvec2 img_size = {1400 /*width*/, 2000 /*height*/};

  // camera
  vec3 cam_pos = {0.0f, 19.150f, 200.f};
  vec3 cam_focal = {1.086f, 19.150f, 103.543f};
  vec3 cam_up = {-0.221713f, -0.97421f, 0.0419261f};
  vec3 cam_view = {
    cam_focal[0] - cam_pos[0],
    cam_focal[1] - cam_pos[1],
    cam_focal[2] - cam_pos[2],
  };

  printf("initialize ANARI...");
  anari::Library lib = anari::loadLibrary("visrtx", statusFunc);
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
  auto world = anari::scenes::CreateFiberTracks(d, argv[1]);

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
  for (int i = 0; i < 32; i++)
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
