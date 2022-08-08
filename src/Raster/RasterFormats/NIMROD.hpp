/***********************************************************************
 * mfcm Raster/RasterFormats/NIMROD.hpp
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

#ifndef mfcm_Raster_RasterFormats_NIMROD_hpp
#define mfcm_Raster_RasterFormats_NIMROD_hpp

#include "../RasterFormat.hpp"
#include "Config.hpp"
#include <iomanip>

/**
   Raster data parser for the MET Office NIMROD raster format.
 */
template<typename T>
class NIMRODRasterFormat : public RasterFormat<T>
{
private:

  std::array<int16_t,31> h1_;
  std::array<float,28> h2_;
  std::array<float,45> h3_;
  std::array<char,56> h4_;
  std::array<int16_t,51> h5_;
  
  std::vector<T> buffer_;
  size_t nxpx_;
  size_t nypx_;
  std::array<double,6> geotrans_;
  T nodata_value_;

  size_t ulc_xpx_;
  size_t ulc_ypx_;
  size_t lrc_xpx_;
  size_t lrc_ypx_;
  std::vector<T> values_;

  size_t total_cols(void) const
  {
    return nxpx_;
  }
  
  size_t total_rows(void) const
  {
    return nypx_;
  }

  std::array<double,2> llc(void) const
  {
    return {
      geotrans_[0] + ulc_xpx_ * geotrans_[1],
      geotrans_[3] + (nypx_ - lrc_ypx_ - 1) * geotrans_[5],
    };
  }

  T value(size_t col, size_t row)
  {
    size_t j = col + ulc_xpx_;
    size_t i = row + ulc_ypx_;
    return buffer_.at(i * nxpx_ + j);
  }
  
  template<typename U>
  void read_nimrod_vector(const bool& sil,
			  std::istream& in_file);

protected:

  virtual const std::vector<T>& values(void) const
  {
    return values_;
  }
  
  virtual size_t ncols(void) const
  {
    return 1 + lrc_xpx_ - ulc_xpx_;
  }

  virtual size_t nrows(void) const
  {
    return 1 + lrc_ypx_ - ulc_ypx_;
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
  NIMRODRasterFormat(const stdfs::path& filepath,
		     const Config& conf);

  /**
     Destructor.
   */
  virtual ~NIMRODRasterFormat(void) {}
  
};

#endif
