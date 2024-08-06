//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================

#include "PolyLineDataReader.h"

#include <vtkm/cont/ConvertNumComponentsToOffsets.h>

namespace {

inline void FixCellSet(vtkm::cont::ArrayHandle<vtkm::Id>& connectivity,
                         vtkm::cont::ArrayHandle<vtkm::IdComponent>& numIndices,
                         vtkm::cont::ArrayHandle<vtkm::UInt8>& shapes,
                         vtkm::cont::ArrayHandle<vtkm::Id>& permutation)
{
  using namespace vtkm::io::internal;

  std::vector<vtkm::Id> newConnectivity;
  std::vector<vtkm::IdComponent> newNumIndices;
  std::vector<vtkm::UInt8> newShapes;
  std::vector<vtkm::Id> permutationVec;

  vtkm::Id connIdx = 0;
  auto shapesPortal = shapes.ReadPortal();
  auto indicesPortal = numIndices.ReadPortal();
  auto connPortal = connectivity.ReadPortal();
  for (vtkm::Id i = 0; i < shapes.GetNumberOfValues(); ++i)
  {
    vtkm::UInt8 shape = shapesPortal.Get(i);
    vtkm::IdComponent numInds = indicesPortal.Get(i);
    switch (shape)
    {
      case vtkm::CELL_SHAPE_VERTEX:
      case vtkm::CELL_SHAPE_LINE:
      case vtkm::CELL_SHAPE_TRIANGLE:
      case vtkm::CELL_SHAPE_QUAD:
      case vtkm::CELL_SHAPE_TETRA:
      case vtkm::CELL_SHAPE_HEXAHEDRON:
      case vtkm::CELL_SHAPE_WEDGE:
      case vtkm::CELL_SHAPE_PYRAMID:
      case CELL_SHAPE_POLY_LINE:
      {
        newShapes.push_back(shape);
        newNumIndices.push_back(numInds);
        for (vtkm::IdComponent j = 0; j < numInds; ++j)
        {
          newConnectivity.push_back(connPortal.Get(connIdx++));
        }
        permutationVec.push_back(i);
        break;
      }
      case vtkm::CELL_SHAPE_POLYGON:
      {
        vtkm::IdComponent numVerts = numInds;
        vtkm::UInt8 newShape = vtkm::CELL_SHAPE_POLYGON;
        if (numVerts == 3)
        {
          newShape = vtkm::CELL_SHAPE_TRIANGLE;
        }
        else if (numVerts == 4)
        {
          newShape = vtkm::CELL_SHAPE_QUAD;
        }
        newShapes.push_back(newShape);
        newNumIndices.push_back(numVerts);
        for (vtkm::IdComponent j = 0; j < numVerts; ++j)
        {
          newConnectivity.push_back(connPortal.Get(connIdx++));
        }
        permutationVec.push_back(i);
        break;
      }
      case CELL_SHAPE_POLY_VERTEX:
      {
        vtkm::IdComponent numVerts = numInds;
        for (vtkm::IdComponent j = 0; j < numVerts; ++j)
        {
          newShapes.push_back(vtkm::CELL_SHAPE_VERTEX);
          newNumIndices.push_back(1);
          newConnectivity.push_back(connPortal.Get(connIdx));
          permutationVec.push_back(i);
          ++connIdx;
        }
        break;
      }
      // case CELL_SHAPE_POLY_LINE:
      // {
      //   vtkm::IdComponent numLines = numInds - 1;
      //   for (vtkm::IdComponent j = 0; j < numLines; ++j)
      //   {
      //     newShapes.push_back(vtkm::CELL_SHAPE_LINE);
      //     newNumIndices.push_back(2);
      //     newConnectivity.push_back(connPortal.Get(connIdx));
      //     newConnectivity.push_back(connPortal.Get(connIdx + 1));
      //     permutationVec.push_back(i);
      //     ++connIdx;
      //   }
      //   connIdx += 1;
      //   break;
      // }
      case CELL_SHAPE_TRIANGLE_STRIP:
      {
        vtkm::IdComponent numTris = numInds - 2;
        for (vtkm::IdComponent j = 0; j < numTris; ++j)
        {
          newShapes.push_back(vtkm::CELL_SHAPE_TRIANGLE);
          newNumIndices.push_back(3);
          if (j % 2)
          {
            newConnectivity.push_back(connPortal.Get(connIdx));
            newConnectivity.push_back(connPortal.Get(connIdx + 1));
            newConnectivity.push_back(connPortal.Get(connIdx + 2));
          }
          else
          {
            newConnectivity.push_back(connPortal.Get(connIdx + 2));
            newConnectivity.push_back(connPortal.Get(connIdx + 1));
            newConnectivity.push_back(connPortal.Get(connIdx));
          }
          permutationVec.push_back(i);
          ++connIdx;
        }
        connIdx += 2;
        break;
      }
      case CELL_SHAPE_PIXEL:
      {
        newShapes.push_back(vtkm::CELL_SHAPE_QUAD);
        newNumIndices.push_back(numInds);
        newConnectivity.push_back(connPortal.Get(connIdx + 0));
        newConnectivity.push_back(connPortal.Get(connIdx + 1));
        newConnectivity.push_back(connPortal.Get(connIdx + 3));
        newConnectivity.push_back(connPortal.Get(connIdx + 2));
        permutationVec.push_back(i);
        connIdx += 4;
        break;
      }
      case CELL_SHAPE_VOXEL:
      {
        newShapes.push_back(vtkm::CELL_SHAPE_HEXAHEDRON);
        newNumIndices.push_back(numInds);
        newConnectivity.push_back(connPortal.Get(connIdx + 0));
        newConnectivity.push_back(connPortal.Get(connIdx + 1));
        newConnectivity.push_back(connPortal.Get(connIdx + 3));
        newConnectivity.push_back(connPortal.Get(connIdx + 2));
        newConnectivity.push_back(connPortal.Get(connIdx + 4));
        newConnectivity.push_back(connPortal.Get(connIdx + 5));
        newConnectivity.push_back(connPortal.Get(connIdx + 7));
        newConnectivity.push_back(connPortal.Get(connIdx + 6));
        permutationVec.push_back(i);
        connIdx += 8;
        break;
      }
      default:
      {
        throw vtkm::io::ErrorIO("Encountered unsupported cell type");
      }
    }
  }

  if (newShapes.size() == static_cast<std::size_t>(shapes.GetNumberOfValues()))
  {
    permutationVec.clear();
  }
  else
  {
    permutation.Allocate(static_cast<vtkm::Id>(permutationVec.size()));
    std::copy(permutationVec.begin(),
              permutationVec.end(),
              vtkm::cont::ArrayPortalToIteratorBegin(permutation.WritePortal()));
  }

  shapes.Allocate(static_cast<vtkm::Id>(newShapes.size()));
  std::copy(newShapes.begin(),
            newShapes.end(),
            vtkm::cont::ArrayPortalToIteratorBegin(shapes.WritePortal()));
  numIndices.Allocate(static_cast<vtkm::Id>(newNumIndices.size()));
  std::copy(newNumIndices.begin(),
            newNumIndices.end(),
            vtkm::cont::ArrayPortalToIteratorBegin(numIndices.WritePortal()));
  connectivity.Allocate(static_cast<vtkm::Id>(newConnectivity.size()));
  std::copy(newConnectivity.begin(),
            newConnectivity.end(),
            vtkm::cont::ArrayPortalToIteratorBegin(connectivity.WritePortal()));
}

template <typename T>
inline vtkm::cont::ArrayHandle<T> ConcatinateArrayHandles(
  const std::vector<vtkm::cont::ArrayHandle<T>>& arrays)
{
  vtkm::Id size = 0;
  for (std::size_t i = 0; i < arrays.size(); ++i)
  {
    size += arrays[i].GetNumberOfValues();
  }

  vtkm::cont::ArrayHandle<T> out;
  out.Allocate(size);

  auto outp = vtkm::cont::ArrayPortalToIteratorBegin(out.WritePortal());
  for (std::size_t i = 0; i < arrays.size(); ++i)
  {
    std::copy(vtkm::cont::ArrayPortalToIteratorBegin(arrays[i].ReadPortal()),
              vtkm::cont::ArrayPortalToIteratorEnd(arrays[i].ReadPortal()),
              outp);
    using DifferenceType = typename std::iterator_traits<decltype(outp)>::difference_type;
    std::advance(outp, static_cast<DifferenceType>(arrays[i].GetNumberOfValues()));
  }

  return out;
}
}

namespace vtkm {
namespace io {

PolyLineDataReader::PolyLineDataReader(const char* fileName)
  : VTKDataSetReaderBase(fileName)
{
}

PolyLineDataReader::PolyLineDataReader(const std::string& fileName)
  : VTKDataSetReaderBase(fileName)
{
}

void PolyLineDataReader::Read()
{
  if (this->DataFile->Structure != vtkm::io::internal::DATASET_POLYDATA)
  {
    throw vtkm::io::ErrorIO("Incorrect DataSet type");
  }

  // We need to be able to handle VisIt files which dump Field data
  // at the top of a VTK file
  std::string tag;
  this->DataFile->Stream >> tag;
  if (tag == "FIELD")
  {
    this->ReadGlobalFields();
    this->DataFile->Stream >> tag;
  }

  // Read the points
  internal::parseAssert(tag == "POINTS");
  this->ReadPoints();

  vtkm::Id numPoints = this->DataSet.GetNumberOfPoints();

  // Read the cellset
  std::vector<vtkm::cont::ArrayHandle<vtkm::Id>> connectivityArrays;
  std::vector<vtkm::cont::ArrayHandle<vtkm::IdComponent>> numIndicesArrays;
  std::vector<vtkm::UInt8> shapesBuffer;
  while (!this->DataFile->Stream.eof())
  {
    vtkm::UInt8 shape = vtkm::CELL_SHAPE_EMPTY;
    this->DataFile->Stream >> tag;
    if (tag == "VERTICES")
    {
      shape = vtkm::io::internal::CELL_SHAPE_POLY_VERTEX;
    }
    else if (tag == "LINES")
    {
      shape = vtkm::io::internal::CELL_SHAPE_POLY_LINE;
    }
    else if (tag == "POLYGONS")
    {
      shape = vtkm::CELL_SHAPE_POLYGON;
    }
    else if (tag == "TRIANGLE_STRIPS")
    {
      shape = vtkm::io::internal::CELL_SHAPE_TRIANGLE_STRIP;
    }
    else
    {
      this->DataFile->Stream.seekg(-static_cast<std::streamoff>(tag.length()), std::ios_base::cur);
      break;
    }

    vtkm::cont::ArrayHandle<vtkm::Id> cellConnectivity;
    vtkm::cont::ArrayHandle<vtkm::IdComponent> cellNumIndices;
    this->ReadCells(cellConnectivity, cellNumIndices);

    connectivityArrays.push_back(cellConnectivity);
    numIndicesArrays.push_back(cellNumIndices);
    shapesBuffer.insert(
      shapesBuffer.end(), static_cast<std::size_t>(cellNumIndices.GetNumberOfValues()), shape);
  }

  vtkm::cont::ArrayHandle<vtkm::Id> connectivity = ConcatinateArrayHandles(connectivityArrays);
  vtkm::cont::ArrayHandle<vtkm::IdComponent> numIndices = ConcatinateArrayHandles(numIndicesArrays);
  vtkm::cont::ArrayHandle<vtkm::UInt8> shapes;
  shapes.Allocate(static_cast<vtkm::Id>(shapesBuffer.size()));
  std::copy(shapesBuffer.begin(),
            shapesBuffer.end(),
            vtkm::cont::ArrayPortalToIteratorBegin(shapes.WritePortal()));

  vtkm::cont::ArrayHandle<vtkm::Id> permutation;
  FixCellSet(connectivity, numIndices, shapes, permutation);
  this->SetCellsPermutation(permutation);

  {
    auto offsets = vtkm::cont::ConvertNumComponentsToOffsets(numIndices);
    vtkm::cont::CellSetExplicit<> cellSet;
    cellSet.Fill(numPoints, shapes, connectivity, offsets);
    this->DataSet.SetCellSet(cellSet);
  }

  // this->DataSet.PrintSummary(std::cout);

  // Read points and cell attributes
  this->ReadAttributes();
}

}
} // namespace vtkm::io
