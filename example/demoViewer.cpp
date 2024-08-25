// Copyright 2023-2024 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "scene_CornellBox.h"
#include "scene_FiberTracks.h"
#include "scene_ObjFile.h"

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

    // auto w = anari::scenes::CreateCornellBox(g_device);
    // auto w = anari::scenes::CreateFiberTracks(g_device, "H:/fibertracks.vtk");
    auto w = anari::scenes::CreateObjFile(g_device);

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
  if (!g_device) return 1;
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
