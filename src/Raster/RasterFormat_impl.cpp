/***********************************************************************
 * mfcm Raster/RasterFormat_impl.cpp
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

#if __INCLUDE_LEVEL__
#error "This file should not be included."
#endif

#include "RasterFormat.hpp"

template class RasterFormat<float>;
template class RasterFormat<double>;
template class RasterFormat<int32_t>;
template class RasterFormat<uint32_t>;

#include "RasterFormats/GDAL.cpp"
#if MFCM_HAS_GDAL

template class GDALRasterFormat<float>;
template class GDALRasterFormat<double>;
template class GDALRasterFormat<int32_t>;
template class GDALRasterFormat<uint32_t>;

#endif

#include "RasterFormats/NIMROD.cpp"

template class NIMRODRasterFormat<float>;
template class NIMRODRasterFormat<double>;
template class NIMRODRasterFormat<int32_t>;
template class NIMRODRasterFormat<uint32_t>;

// #include "RasterFormats/GDAL_impl.cpp"
// #include "RasterFormats/NIMROD_impl.cpp"

