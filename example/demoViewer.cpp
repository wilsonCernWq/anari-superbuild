// Copyright 2023-2024 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "anari_viewer/Application.h"
#include "anari_viewer/windows/LightsEditor.h"
#include "anari_viewer/windows/SceneSelector.h"
#include "anari_viewer/windows/Viewport.h"
// anari
#include <anari_test_scenes.h>
// std
#include <iostream>

static const bool g_true = true;
static bool g_verbose = false;
static bool g_enableDebug = false;
static std::string g_libraryName = "environment";
static anari::Library g_debug = nullptr;
static anari::Device g_device = nullptr;
static const char *g_traceDir = nullptr;
extern const char *g_defaultLayout;

namespace anari {
namespace scenes {

// quad mesh data
static std::vector<math::float3> vertices = {
    // Floor
    {1.00f, -1.00f, -1.00f},
    {-1.00f, -1.00f, -1.00f},
    {-1.00f, -1.00f, 1.00f},
    {1.00f, -1.00f, 1.00f},
    // Ceiling
    {1.00f, 1.00f, -1.00f},
    {1.00f, 1.00f, 1.00f},
    {-1.00f, 1.00f, 1.00f},
    {-1.00f, 1.00f, -1.00f},
    // Backwall
    {1.00f, -1.00f, 1.00f},
    {-1.00f, -1.00f, 1.00f},
    {-1.00f, 1.00f, 1.00f},
    {1.00f, 1.00f, 1.00f},
    // RightWall
    {-1.00f, -1.00f, 1.00f},
    {-1.00f, -1.00f, -1.00f},
    {-1.00f, 1.00f, -1.00f},
    {-1.00f, 1.00f, 1.00f},
    // LeftWall
    {1.00f, -1.00f, -1.00f},
    {1.00f, -1.00f, 1.00f},
    {1.00f, 1.00f, 1.00f},
    {1.00f, 1.00f, -1.00f},
    // ShortBox Top Face
    {-0.53f, -0.40f, -0.75f},
    {-0.70f, -0.40f, -0.17f},
    {-0.13f, -0.40f, -0.00f},
    {0.05f, -0.40f, -0.57f},
    // ShortBox Left Face
    {0.05f, -1.00f, -0.57f},
    {0.05f, -0.40f, -0.57f},
    {-0.13f, -0.40f, -0.00f},
    {-0.13f, -1.00f, -0.00f},
    // ShortBox Front Face
    {-0.53f, -1.00f, -0.75f},
    {-0.53f, -0.40f, -0.75f},
    {0.05f, -0.40f, -0.57f},
    {0.05f, -1.00f, -0.57f},
    // ShortBox Right Face
    {-0.70f, -1.00f, -0.17f},
    {-0.70f, -0.40f, -0.17f},
    {-0.53f, -0.40f, -0.75f},
    {-0.53f, -1.00f, -0.75f},
    // ShortBox Back Face
    {-0.13f, -1.00f, -0.00f},
    {-0.13f, -0.40f, -0.00f},
    {-0.70f, -0.40f, -0.17f},
    {-0.70f, -1.00f, -0.17f},
    // ShortBox Bottom Face
    {-0.53f, -1.00f, -0.75f},
    {-0.70f, -1.00f, -0.17f},
    {-0.13f, -1.00f, -0.00f},
    {0.05f, -1.00f, -0.57f},
    // TallBox Top Face
    {0.53f, 0.20f, -0.09f},
    {-0.04f, 0.20f, 0.09f},
    {0.14f, 0.20f, 0.67f},
    {0.71f, 0.20f, 0.49f},
    // TallBox Left Face
    {0.53f, -1.00f, -0.09f},
    {0.53f, 0.20f, -0.09f},
    {0.71f, 0.20f, 0.49f},
    {0.71f, -1.00f, 0.49f},
    // TallBox Front Face
    {0.71f, -1.00f, 0.49f},
    {0.71f, 0.20f, 0.49f},
    {0.14f, 0.20f, 0.67f},
    {0.14f, -1.00f, 0.67f},
    // TallBox Right Face
    {0.14f, -1.00f, 0.67f},
    {0.14f, 0.20f, 0.67f},
    {-0.04f, 0.20f, 0.09f},
    {-0.04f, -1.00f, 0.09f},
    // TallBox Back Face
    {-0.04f, -1.00f, 0.09f},
    {-0.04f, 0.20f, 0.09f},
    {0.53f, 0.20f, -0.09f},
    {0.53f, -1.00f, -0.09f},
    // TallBox Bottom Face
    {0.53f, -1.00f, -0.09f},
    {-0.04f, -1.00f, 0.09f},
    {0.14f, -1.00f, 0.67f},
    {0.71f, -1.00f, 0.49f}};

static std::vector<math::uint3> indices = {
    {0, 1, 2}, // Floor
    {0, 2, 3}, // Floor
    {4, 5, 6}, // Ceiling
    {4, 6, 7}, // Ceiling
    {8, 9, 10}, // Backwall
    {8, 10, 11}, // Backwall
    {12, 13, 14}, // RightWall
    {12, 14, 15}, // RightWall
    {16, 17, 18}, // LeftWall
    {16, 18, 19}, // LeftWall
    {20, 22, 23}, // ShortBox Top Face
    {20, 21, 22}, // ShortBox Top Face
    {24, 25, 26}, // ShortBox Left Face
    {24, 26, 27}, // ShortBox Left Face
    {28, 29, 30}, // ShortBox Front Face
    {28, 30, 31}, // ShortBox Front Face
    {32, 33, 34}, // ShortBox Right Face
    {32, 34, 35}, // ShortBox Right Face
    {36, 37, 38}, // ShortBox Back Face
    {36, 38, 39}, // ShortBox Back Face
    {40, 41, 42}, // ShortBox Bottom Face
    {40, 42, 43}, // ShortBox Bottom Face
    {44, 45, 46}, // TallBox Top Face
    {44, 46, 47}, // TallBox Top Face
    {48, 49, 50}, // TallBox Left Face
    {48, 50, 51}, // TallBox Left Face
    {52, 53, 54}, // TallBox Front Face
    {52, 54, 55}, // TallBox Front Face
    {56, 57, 58}, // TallBox Right Face
    {56, 58, 59}, // TallBox Right Face
    {60, 61, 62}, // TallBox Back Face
    {60, 62, 63}, // TallBox Back Face
    {64, 65, 66}, // TallBox Bottom Face
    {64, 66, 67}, // TallBox Bottom Face
};

static std::vector<math::float4> colors = {
    // Floor
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // Ceiling
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // Backwall
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // RightWall
    {0.140f, 0.450f, 0.091f, 1.0f},
    {0.140f, 0.450f, 0.091f, 1.0f},
    {0.140f, 0.450f, 0.091f, 1.0f},
    {0.140f, 0.450f, 0.091f, 1.0f},
    // LeftWall
    {0.630f, 0.065f, 0.05f, 1.0f},
    {0.630f, 0.065f, 0.05f, 1.0f},
    {0.630f, 0.065f, 0.05f, 1.0f},
    {0.630f, 0.065f, 0.05f, 1.0f},
    // ShortBox Top Face
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // ShortBox Left Face
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // ShortBox Front Face
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // ShortBox Right Face
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // ShortBox Back Face
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // ShortBox Bottom Face
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // TallBox Top Face
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // TallBox Left Face
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // TallBox Front Face
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // TallBox Right Face
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // TallBox Back Face
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    // TallBox Bottom Face
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f},
    {0.725f, 0.710f, 0.68f, 1.0f}};

// CornelBox definitions //////////////////////////////////////////////////////

anari::World CreateCornellBox(anari::Device d)
{
  auto m_world = anari::newObject<anari::World>(d);

  auto geom = anari::newObject<anari::Geometry>(d, "triangle");

  anari::setAndReleaseParameter(d,
      geom,
      "vertex.position",
      anari::newArray1D(d, vertices.data(), vertices.size()));
  anari::setAndReleaseParameter(d,
      geom,
      "vertex.color",
      anari::newArray1D(d, colors.data(), colors.size()));
  anari::setAndReleaseParameter(d,
      geom,
      "primitive.index",
      anari::newArray1D(d, indices.data(), indices.size()));

  anari::commitParameters(d, geom);

  auto surface = anari::newObject<anari::Surface>(d);
  anari::setAndReleaseParameter(d, surface, "geometry", geom);

  auto mat = anari::newObject<anari::Material>(d, "matte");
  anari::setParameter(d, mat, "color", "color");
  anari::commitParameters(d, mat);
  anari::setAndReleaseParameter(d, surface, "material", mat);

  anari::commitParameters(d, surface);

  anari::setAndReleaseParameter(
      d, m_world, "surface", anari::newArray1D(d, &surface));
  anari::release(d, surface);

  anari::Light light;

  // fix this up with new check
  if (false/*anari::deviceImplements(d, "ANARI_KHR_AREA_LIGHTS")*/) {
    light = anari::newObject<anari::Light>(d, "quad");
    anari::setParameter(d, light, "color", math::float3(0.78f, 0.551f, 0.183f));
    anari::setParameter(d, light, "intensity", 47.f);
    anari::setParameter(d, light, "position", math::float3(-0.23f, 0.98f, -0.16f));
    anari::setParameter(d, light, "edge1", math::float3(0.47f, 0.0f, 0.0f));
    anari::setParameter(d, light, "edge2", math::float3(0.0f, 0.0f, 0.38f));
  } else {
    light = anari::newObject<anari::Light>(d, "directional");
    anari::setParameter(d, light, "direction", math::float3(0.f, -0.5f, 1.f));
  }

  anari::commitParameters(d, light);

  anari::setAndReleaseParameter(
      d, m_world, "light", anari::newArray1D(d, &light));

  anari::release(d, light);

  anari::commitParameters(d, m_world);

  return m_world;
}

}
}

namespace viewer {

struct AppState
{
  manipulators::Orbit manipulator;
  anari::Device device{nullptr};
};

static void statusFunc(const void *userData,
    ANARIDevice device,
    ANARIObject source,
    ANARIDataType sourceType,
    ANARIStatusSeverity severity,
    ANARIStatusCode code,
    const char *message)
{
  const bool verbose = userData ? *(const bool *)userData : false;
  if (severity == ANARI_SEVERITY_FATAL_ERROR) {
    fprintf(stderr, "[FATAL][%p] %s\n", source, message);
    std::exit(1);
  } else if (severity == ANARI_SEVERITY_ERROR) {
    fprintf(stderr, "[ERROR][%p] %s\n", source, message);
  } else if (severity == ANARI_SEVERITY_WARNING) {
    fprintf(stderr, "[WARN ][%p] %s\n", source, message);
  } else if (verbose && severity == ANARI_SEVERITY_PERFORMANCE_WARNING) {
    fprintf(stderr, "[PERF ][%p] %s\n", source, message);
  } else if (verbose && severity == ANARI_SEVERITY_INFO) {
    fprintf(stderr, "[INFO ][%p] %s\n", source, message);
  } else if (verbose && severity == ANARI_SEVERITY_DEBUG) {
    fprintf(stderr, "[DEBUG][%p] %s\n", source, message);
  }
}

static void initializeANARI()
{
  auto library =
      anariLoadLibrary(g_libraryName.c_str(), statusFunc, &g_verbose);
  if (!library)
    throw std::runtime_error("Failed to load ANARI library");

  if (g_enableDebug)
    g_debug = anariLoadLibrary("debug", statusFunc, &g_true);

  anari::Device dev = anariNewDevice(library, "default");

  anari::unloadLibrary(library);

  if (g_enableDebug)
    anari::setParameter(dev, dev, "glDebug", true);

#ifdef USE_GLES2
  anari::setParameter(dev, dev, "glAPI", "OpenGL_ES");
#else
  anari::setParameter(dev, dev, "glAPI", "OpenGL");
#endif

  if (g_enableDebug) {
    anari::Device dbg = anariNewDevice(g_debug, "debug");
    anari::setParameter(dbg, dbg, "wrappedDevice", dev);
    if (g_traceDir) {
      anari::setParameter(dbg, dbg, "traceDir", g_traceDir);
      anari::setParameter(dbg, dbg, "traceMode", "code");
    }
    anari::commitParameters(dbg, dbg);
    anari::release(dev, dev);
    dev = dbg;
  }

  anari::commitParameters(dev, dev);

  g_device = dev;
}

// Application definition /////////////////////////////////////////////////////

struct Application : public anari_viewer::Application
{
  Application() = default;
  ~Application() override = default;

  anari_viewer::WindowArray setupWindows() override
  {
    ui::init();

    // ANARI //

    m_state.device = g_device;

    // ImGui //

    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.5f;
    io.IniFilename = nullptr;

    ImGui::LoadIniSettingsFromMemory(g_defaultLayout);

    auto *viewport = new windows::Viewport(g_device, "Viewport");
    viewport->setManipulator(&m_state.manipulator);

    auto *leditor = new windows::LightsEditor(g_device);

    // Create a scene //

    auto w = anari::scenes::CreateCornellBox(g_device);
    viewport->setWorld(w, true);
    leditor->setWorlds({w});
    anari::release(g_device, w);

    // Create Windows //

    anari_viewer::WindowArray windows;
    windows.emplace_back(viewport);
    windows.emplace_back(leditor);

    return windows;
  }

  void uiFrameStart() override
  {
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("print ImGui ini")) {
          const char *info = ImGui::SaveIniSettingsToMemory();
          printf("%s\n", info);
        }

        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }
  }

  void teardown() override
  {
    anari::release(m_state.device, m_state.device);
    ui::shutdown();
  }

 private:
  AppState m_state;
};

} // namespace viewer

///////////////////////////////////////////////////////////////////////////////

static void printUsage()
{
  std::cout << "./anariViewer [{--help|-h}]\n"
            << "   [{--verbose|-v}] [{--debug|-g}]\n"
            << "   [{--library|-l} <ANARI library>]\n"
            << "   [{--trace|-t} <directory>]\n";
}

static void parseCommandLine(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "-v" || arg == "--verbose")
      g_verbose = true;
    if (arg == "--help" || arg == "-h") {
      printUsage();
      std::exit(0);
    }
    else if (arg == "-l" || arg == "--library")
      g_libraryName = argv[++i];
    else if (arg == "--debug" || arg == "-g")
      g_enableDebug = true;
    else if (arg == "--trace" || arg == "-t")
      g_traceDir = argv[++i];
  }
}

int main(int argc, char *argv[])
{
  parseCommandLine(argc, argv);
  viewer::initializeANARI();
  if (!g_device)
    return 1;
  viewer::Application app;
  app.run(1920, 1200, "ANARI Demo Viewer");
  return 0;
}

const char *g_defaultLayout =
R"layout(
[Window][MainDockSpace]
Pos=0,25
Size=1920,1174
Collapsed=0

[Window][Viewport]
Pos=551,25
Size=1369,1174
Collapsed=0
DockId=0x00000002,0

[Window][Lights Editor]
Pos=0,310
Size=549,889
Collapsed=0
DockId=0x00000004,0

[Window][Debug##Default]
Pos=60,60
Size=400,400
Collapsed=0

[Window][Scene]
Pos=0,25
Size=549,283
Collapsed=0
DockId=0x00000003,0

[Docking][Data]
DockSpace     ID=0x782A6D6B Window=0xDEDC5B90 Pos=0,25 Size=1920,1174 Split=X
  DockNode    ID=0x00000001 Parent=0x782A6D6B SizeRef=549,1174 Split=Y Selected=0x5098EBE6
    DockNode  ID=0x00000003 Parent=0x00000001 SizeRef=549,283 Selected=0xE192E354
    DockNode  ID=0x00000004 Parent=0x00000001 SizeRef=549,889 Selected=0x5098EBE6
  DockNode    ID=0x00000002 Parent=0x782A6D6B SizeRef=1369,1174 CentralNode=1 Selected=0x13926F0B
)layout";
