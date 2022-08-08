/***********************************************************************
 * mfcm Raster/RasterFormat.hpp
 *
 * Copyright (C) Edenvale Young Associates 2022
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

#ifndef mfcm_Raster_RasterFormat_hpp
#define mfcm_Raster_RasterFormat_hpp

#include "Field.hpp"
#include "Cartesian2DMesh.hpp"

template<typename T>
using RasterField = Field<T,Cartesian2DMesh,MeshComponent::Cell>;

/**
   Base class for raster format parsers.

   @tparam T Type of data to read.
 */
template<typename T>
class RasterFormat {
protected:

  /**
     Return the raster data.
   */
  virtual const std::vector<T>& values(void) const = 0;
  /**
     Return the number of X pixels.
   */
  virtual size_t ncols(void) const = 0;
  /**
     Return the number of Y pixels.
   */
  virtual size_t nrows(void) const = 0;
  /**
     Return the geographic transformation coefficients.
   */
  virtual const std::array<double, 6>& geo_transform(void) const = 0;
  /**
     Return the data value indicating missing data.
   */
  virtual T nodata_value(void) const = 0;
  
public:

  /**
     Constructor (implementations will read and parse the file here).
   */
  RasterFormat(void) {}

  /**
     Destructor.
   */
  virtual ~RasterFormat(void) {}

  /**
     Create a Field object from a raster

     @param queue Pointer to the SYCL queue object.
   */
  std::shared_ptr<RasterField<T>> operator()(const std::shared_ptr<sycl::queue>& queue,
					     const std::string& name)
  {
    std::array<size_t,2> ncells = {this->nrows(), this->ncols()};
    auto mesh = std::make_shared<Cartesian2DMesh>(queue, ncells,
						  this->geo_transform());
    return std::make_shared<RasterField<T>>(queue, name, mesh,
					    this->values(), true);
  }
  
};

#endif
