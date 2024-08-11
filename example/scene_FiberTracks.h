#pragma once

#include "PolyLineDataReader.h"

// #include <anari_test_scenes.h>

#define ANARI_EXTENSION_UTILITY_IMPL
#include "anari/anari_cpp.hpp"
#include "anari/anari_cpp/ext/std.h"

#include <vtkm/io/VTKDataSetReader.h>
#include <vtkm/cont/CellSetSingleType.h>
#include <vtkm/cont/ArrayCopy.h>
#include <vtkm/cont/ColorTable.h>
#include <vtkm/cont/ColorTableSamples.h>

#include <iostream>
#include <string>
#include <array>

namespace anari {
namespace scenes {

// FiberTrack definitions //////////////////////////////////////////////////////
inline anari::World CreateFiberTracks(anari::Device d, std::string filename)
{
  using vec3 = std::array<float, 3>;
  using vec4 = std::array<float, 4>;
  using box3 = std::array<vec3, 2>;

  // Load Dataset
  printf("loading ... %s", filename.c_str());
  vtkm::io::PolyLineDataReader reader(filename.c_str());
  static auto ds = reader.ReadDataSet();
  ds.PrintSummary(std::cout);
  std::cout << std::endl;

  auto coords = ds.GetCoordinateSystem().GetData().AsArrayHandle<vtkm::cont::ArrayHandle<vtkm::Vec3f_32>>();
  auto curves = ds.GetCellSet().AsCellSet<vtkm::cont::CellSetExplicit<>>();
  auto curve_offsets = curves.GetOffsetsArray(vtkm::TopologyElementTagCell(), vtkm::TopologyElementTagPoint());
  auto curve_offsets_reader = curve_offsets.ReadPortal();

  uint32_t curve_length_max = 0;
  uint32_t curve_length_min = std::numeric_limits<uint32_t>::max();
  std::vector<uint32_t> curve_lengths(curve_offsets.GetNumberOfValues()-1);
  // Curves are defined by a sequence of segments. 
  // The i-th segment is defined by two points: vertex[prim[i]], vertex[prim[i]+1].
  // Thus if a curve has n+1 vertices, it has n segments (aka n primitives).
  std::vector<uint32_t> segment_indices;
  for (int i = 1; i < curve_offsets.GetNumberOfValues(); i++) {
    auto offset_curr = curve_offsets_reader.Get(i - 1);
    auto offset_next = curve_offsets_reader.Get(i);
    auto num_segments = offset_next - offset_curr - 1;
    curve_lengths[i-1] = offset_next - offset_curr;
    curve_length_max = std::max(curve_length_max, curve_lengths[i-1]);
    curve_length_min = std::min(curve_length_min, curve_lengths[i-1]);
    for (int j = 0; j < num_segments; j++) {
      segment_indices.push_back(offset_curr+j);
    }
  }
  segment_indices.shrink_to_fit();
  std::cout << "max length: " << curve_length_max << std::endl;
  std::cout << "min length: " << curve_length_min << std::endl;

  // Color Mapping //
  vtkm::cont::ColorTable color_table(vtkm::cont::ColorTable::Preset::RainbowDesaturated);
  color_table.SetColorSpace(vtkm::ColorSpace::RGB);

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
    int v = 2.5 * (curve_lengths[primID] - curve_length_min) / (float)(curve_length_max - curve_length_min) 
      * color_table_samples.NumberOfSamples;
    auto c = color_table_portal.Get(std::min(v, color_table_samples.NumberOfSamples-1));
    vertex_colors[i] = {
      c[0] / 255.f,
      c[1] / 255.f,
      c[2] / 255.f
    };
  }

  auto world = anari::newObject<anari::World>(d);

  // Create and setup surface and mesh
  auto mesh = anari::newObject<anari::Geometry>(d, "curve");
  {
    vtkm::cont::Token token;
    auto* ptr = (vec3*)coords.GetBuffers()[0].ReadPointerHost(token);
    anari::setParameterArray1D(d, mesh, "vertex.position", ptr, coords.GetNumberOfValues());
  }
  anari::setParameterArray1D(d, mesh, "vertex.color", vertex_colors.data(), vertex_colors.size());
  anari::setParameterArray1D(d, mesh, "primitive.index", segment_indices.data(), segment_indices.size());

  anari::setParameter(d, mesh, "radius", 0.25f);
  anari::commitParameters(d, mesh);

  auto mat = anari::newObject<anari::Material>(d, "matte");
  anari::setParameter(d, mat, "color", "color");
  // anari::setParameter(d, mat, "color", vec3{0.9f, 0.9f, 0.9f});
  anari::commitParameters(d, mat);

  // Put the mesh into a surface
  auto surface = anari::newObject<anari::Surface>(d);
  anari::setAndReleaseParameter(d, surface, "geometry", mesh);
  anari::setAndReleaseParameter(d, surface, "material", mat);
  anari::setParameter(d, surface, "id", 2u);
  anari::commitParameters(d, surface);

  // Put the surface directly onto the world
  anari::setParameterArray1D(d, world, "surface", &surface, 1);
  anari::setParameter(d, world, "id", 3u);
  anari::release(d, surface);

  // Light source
  anari::Light light;

  // Fix this up with new check
  if (false/*anari::deviceImplements(d, "ANARI_KHR_AREA_LIGHTS")*/) {
    light = anari::newObject<anari::Light>(d, "quad");
    anari::setParameter(d, light, "color", vec3{0.78f, 0.551f, 0.183f});
    anari::setParameter(d, light, "intensity", 47.f);
    anari::setParameter(d, light, "position", vec3{-0.23f, 0.98f, -0.16f});
    anari::setParameter(d, light, "edge1", vec3{0.47f, 0.0f, 0.0f});
    anari::setParameter(d, light, "edge2", vec3{0.0f, 0.0f, 0.38f});
  } else {
    light = anari::newObject<anari::Light>(d, "directional");
    anari::setParameter(d, light, "direction", vec3{0.f, -0.5f, 1.f});
  }

  anari::commitParameters(d, light);

  anari::setAndReleaseParameter(
      d, world, "light", anari::newArray1D(d, &light));

  anari::release(d, light);

  anari::commitParameters(d, world);

  return world;
}

}
}
