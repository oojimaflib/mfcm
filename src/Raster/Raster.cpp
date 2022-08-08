/***********************************************************************
 * mfcm Raster/Raster.cpp
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

#include "Raster.hpp"
#include "RasterFormats/GDAL.hpp"
#include "RasterFormats/NIMROD.hpp"

#include "Config.hpp"

template<typename T>
RasterDatabase<T>&
RasterDatabase<T>::instance(void)
{
  if (not rdb_) {
    rdb_ = new RasterDatabase<T>();
  }
  return *rdb_;
}

template<typename T>
const std::shared_ptr<RasterField<T>>&
RasterDatabase<T>::
get_raster_field_ptr(const std::shared_ptr<sycl::queue>& queue,
		     const std::string& name)
{
  using boost::algorithm::to_lower_copy;
  if (db_.count(name) == 0) {
    const GlobalConfig& gc = GlobalConfig::instance();
    const Config& conf = gc.raster_configuration(name);
    
    stdfs::path filepath = conf.get<stdfs::path>("filename", ".");
    if (not filepath.is_absolute()) {
      filepath = gc.simulation_base_path() / filepath;
    }
    
    std::string source_type_str =
      to_lower_copy(conf.get<std::string>("source"));
    if (source_type_str == "gdal") {
#if MFCM_HAS_GDAL
      db_[name] = GDALRasterFormat<T>(filepath, conf)(queue, name);
#else
      throw std::runtime_error("GDAL not supported in this build.");
#endif
    } else if (source_type_str == "nimrod") {
      db_[name] = NIMRODRasterFormat<T>(filepath, conf)(queue, name);
    } else {
      std::cerr << "Unknown source type '" << source_type_str
		<< "' for raster field: " << name << std::endl;
      throw std::runtime_error("Unknown source type for raster field");
    }
  }
  return db_[name];
}
