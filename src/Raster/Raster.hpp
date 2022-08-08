/***********************************************************************
 * mfcm Raster/Raster.hpp
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

#ifndef mfcm_Raster_Raster_hpp
#define mfcm_Raster_Raster_hpp

#include "RasterFormat.hpp"
#include <map>

template<typename T>
class RasterDatabase
{
private:

  std::map<std::string, std::shared_ptr<RasterField<T>>> db_;

protected:

  static RasterDatabase<T>* rdb_;

  RasterDatabase(void) {}

public:

  RasterDatabase(RasterDatabase<T>&) = delete;
  void operator=(const RasterDatabase<T>&) = delete;

  static RasterDatabase<T>& instance(void);

  const std::shared_ptr<RasterField<T>>&
  get_raster_field_ptr(const std::shared_ptr<sycl::queue>& queue,
		       const std::string& name);
  
};

#endif
