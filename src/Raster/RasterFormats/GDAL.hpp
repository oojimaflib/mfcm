/***********************************************************************
 * mfcm Raster/RasterFormats/GDAL.hpp
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

#ifndef mfcm_Raster_RasterFormats_GDAL_hpp
#define mfcm_Raster_RasterFormats_GDAL_hpp

#include "GDAL_config.hpp"
#if MFCM_HAS_GDAL

#include "../RasterFormat.hpp"
#include "Config.hpp"

/**
   Raster data parser for formats supported by GDAL.

   @tparam T The type of data to extract from the raster data files.
 */
template<typename T>
class GDALRasterFormat : public RasterFormat<T>
{
private:

  /**
     Buffer to store the raw raster data.
   */
  std::vector<T> buffer_;
  /**
     Number of X pixels.
   */
  size_t nxpx_;
  /**
     Number of Y pixels.
   */
  size_t nypx_;
  /**
     Geographic transform coefficients.
   */
  std::array<double, 6> geotrans_;
  /**
     Value indicating missing data.
   */
  T nodata_value_;

protected:

  virtual const std::vector<T>& values(void) const
  {
    return buffer_;
  }
  
  virtual size_t ncols(void) const
  {
    return nxpx_;
  }

  virtual size_t nrows(void) const
  {
    return nypx_;
  }

  virtual const std::array<double, 6>& geo_transform(void) const
  {
    return geotrans_;
  }
  
  virtual T nodata_value(void) const
  {
    return nodata_value_;
  }

public:

  /**
     Constructor. Reads and stores the data from the source files.

     @param filepath File or path of raster data to read.
     @param conf User-supplied configuration.
   */
  GDALRasterFormat(const stdfs::path& filepath,
		   const Config& conf);

  /**
     Destructor.
   */
  virtual ~GDALRasterFormat(void) {}

};

#endif

#endif
