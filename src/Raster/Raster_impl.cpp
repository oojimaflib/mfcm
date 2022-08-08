/***********************************************************************
 * mfcm Raster/Raster_impl.cpp
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

#include "Raster.cpp"

template<>
RasterDatabase<float>* RasterDatabase<float>::rdb_ = nullptr;

template<>
RasterDatabase<double>* RasterDatabase<double>::rdb_ = nullptr;

template<>
RasterDatabase<int32_t>* RasterDatabase<int32_t>::rdb_ = nullptr;

template<>
RasterDatabase<uint32_t>* RasterDatabase<uint32_t>::rdb_ = nullptr;

template class RasterDatabase<float>;
template class RasterDatabase<double>;
template class RasterDatabase<int32_t>;
template class RasterDatabase<uint32_t>;
