/***********************************************************************
 * mfcm Raster/RasterFormats/GDAL.cpp
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

#include "GDAL.hpp"
#if MFCM_HAS_GDAL

#include "gdal_priv.h"

/**
   Get the value of the GDALDataType enum corresponding to a type.

   @tparam T The type to get the corresponding value for.
 */
template<typename T>
GDALDataType get_gdal_buf_type(void);

template<>
GDALDataType get_gdal_buf_type<float>(void) { return GDT_Float32; }
template<>
GDALDataType get_gdal_buf_type<double>(void) { return GDT_Float64; }
template<>
GDALDataType get_gdal_buf_type<int32_t>(void) { return GDT_Int32; }
template<>
GDALDataType get_gdal_buf_type<uint32_t>(void) { return GDT_UInt32; }

template<typename T>
GDALRasterFormat<T>::
GDALRasterFormat(const stdfs::path& filepath,
		 const Config& conf)
  : RasterFormat<T>()
{
  GDALAllRegister();

  std::string filename = filepath.native();
  GDALDataset* ds = (GDALDataset*) GDALOpen(filename.c_str(), GA_ReadOnly);
  if (ds == nullptr) {
    std::cerr << "Could not open GDAL Dataset: " << filename << std::endl;
    throw std::runtime_error("Could not open GDAL dataset");
  }
  std::cout << "Opened GDAL dataset from " << filename << std::endl;
  std::cout << "  driver: " << ds->GetDriver()->GetDescription()
	    << " (" << ds->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME)
	    << ")" << std::endl;

    
  // Get the band number
  size_t nbands = ds->GetRasterCount();
  size_t band_no = conf.get<size_t>("band", 1);
  if (band_no > nbands) {
    std::cerr << "Dataset has " << nbands
	      << " bands, but we requested band " << band_no << std::endl;
    throw std::runtime_error("Band out of range");
  }
  std::cout << "  bands: " << nbands << std::endl;

  GDALRasterBand* band = ds->GetRasterBand(band_no);

  // Get the no-data value
  int has_nodata = 0;
  nodata_value_ = band->GetNoDataValue(&has_nodata);
  if (!has_nodata) {
    GDALDataType band_type = band->GetRasterDataType();
    switch (band_type) {
    case GDT_Float32:
      nodata_value_ = T(std::numeric_limits<float>::quiet_NaN());
      break;
    case GDT_Float64:
      nodata_value_ = T(std::numeric_limits<double>::quiet_NaN());
      break;
    case GDT_Int32:
      nodata_value_ = T(std::numeric_limits<int32_t>::max());
      break;
    case GDT_UInt32:
      nodata_value_ = T(std::numeric_limits<uint32_t>::max());
      break;
    default:
      throw std::runtime_error("Unsupported band type.");
    };
    std::cout << "  no no-data value available. Using "
	      << nodata_value_ << std::endl;
  } else {
    std::cout << "  no-data value: " << nodata_value_ << std::endl;
  }

  // Get raster size
  nxpx_ = ds->GetRasterXSize();
  nypx_ = ds->GetRasterYSize();
  std::cout << "  size: " << nxpx_ << "??" << nypx_ << std::endl;

  // Show the user the projection
  std::string projection_str = "None";
  if (ds->GetProjectionRef() != nullptr) {
    projection_str = ds->GetProjectionRef();
    if (projection_str.size() == 0) { projection_str = "None"; }
  }
  std::cout << "  projection: " << projection_str << std::endl;

  // Get the transformation
  if (ds->GetGeoTransform(geotrans_.data()) != CE_None) {
    std::cerr << "Dataset does not have a valid geographic transform" << std::endl;
    throw std::runtime_error("Could not get geotransform");
  }
  std::cout << "  origin: " << geotrans_[0] << ", " << geotrans_[3] << std::endl;
  std::cout << "  pixel size: " << std::sqrt(geotrans_[1]*geotrans_[1] +
					     geotrans_[2]*geotrans_[2])
	    << "??" << std::sqrt(geotrans_[4]*geotrans_[4] +
				geotrans_[5]*geotrans_[5]) << std::endl;

  double pdfMin, pdfMax, pdfMean, pdfStdDev;
  if (band->GetStatistics(FALSE, TRUE,
			  &pdfMin, &pdfMax, &pdfMean, &pdfStdDev) != CE_None) {
    std::cerr << "Could not get band statistics." << std::endl;
    throw std::runtime_error("Could not read data from raster.");
  } else {
    std::cout << "  band statistics:" << std::endl
	      << "    min = " << pdfMin << std::endl
	      << "    max = " << pdfMax << std::endl
	      << "    mean = " << pdfMean << std::endl
	      << "    std. dev. = " << pdfStdDev << std::endl;
  }
    
  buffer_.resize(nxpx_*nypx_);

  GDALDataType T_gdal = get_gdal_buf_type<T>();
  if (band->RasterIO(GF_Read, 0, 0, nxpx_, nypx_, buffer_.data(),
		     nxpx_, nypx_, T_gdal, 0, 0) != CE_None) {
    std::cerr << "Could not read " << nxpx_ * nypx_
	      << " data from raster." << std::endl;
    throw std::runtime_error("Could not read data from raster.");
  } else {
    std::cout << "Read " << nxpx_*nypx_ << " data ("
	      << nxpx_*nypx_*sizeof(T) << " bytes) from raster." << std::endl;
  }

  GDALClose(ds);
}

#endif

