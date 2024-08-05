//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//============================================================================
#pragma once

#include <vtkm/io/VTKDataSetReaderBase.h>
#include <vtkm/io/internal/VTKDataSetCells.h>

#include <vtkm/cont/ArrayPortalToIterators.h>

#include <iterator>

namespace vtkm {
namespace io {

class PolyLineDataReader : public VTKDataSetReaderBase {
public:
  explicit PolyLineDataReader(const char* fileName);
  explicit PolyLineDataReader(const std::string& fileName);

private:
  void Read() override;
};

}
} // namespace vtkm::io
