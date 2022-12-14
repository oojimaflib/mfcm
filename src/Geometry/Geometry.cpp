/***********************************************************************
 * mfcm Geometry/Geometry.cpp
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

#include "Geometry.hpp"
#include "GeometryParser.hpp"

GeometryCollection::
GeometryCollection(const Config& config)
  : Geometry(), std::vector<std::shared_ptr<Geometry>>()
{
  if (config.count("wkt") == 1) {
    read_wkt_geometry(config.get<std::string>("wkt"), *this);
  } else if (config.count("source") == 1) {
    read_gdal_geometry(config, *this);
  }
}

void read_wkt_geometry(const std::string& str, GeometryCollection& gc)
{
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;

  using it_type = std::string::const_iterator;
  wkt_parser::WKT<it_type> parser;
  
  // GeometryCollection gc;
  it_type it = str.begin();
  it_type end = str.end();
  if (qi::phrase_parse(it, end, parser, ascii::space, gc) and it == end) {
    std::cout << "Parsed " << gc.size() << " geometries from WKT" << std::endl;
    // success
    // return gc;
  } else {
    std::cerr << "ERROR: failed to parse WKT: "
	      << std::string(it, end) << std::endl << std::endl;
    throw std::runtime_error("Failed to parse WKT string");
  }
}

#if MFCM_HAS_GDAL

#include "ogrsf_frmts.h"

void read_gdal_geometry(const Config& config, GeometryCollection& gc)
{
  stdfs::path user_filepath = config.get<stdfs::path>("source");
  stdfs::path filepath;
  if (user_filepath.is_absolute()) {
    filepath = user_filepath;
  } else {
    filepath = GlobalConfig::instance().simulation_base_path()
      / user_filepath;
  }
  
  GDALAllRegister();

  std::string filename = filepath.native();
  GDALDataset* ds_ptr = (GDALDataset*) GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
  if (ds_ptr == nullptr) {
    std::cerr << "Could not open " << filename << " using GDAL." << std::endl;
    throw std::runtime_error("Could not open vector dataset");
  }

  OGRLayer* layer_ptr = nullptr;
  std::string layer_name = "<not specified>";
  if (ds_ptr->GetLayerCount() > 1) {
    layer_name = config.get<std::string>("layer");
    layer_ptr = ds_ptr->GetLayerByName(layer_name.c_str());
  } else {
    layer_ptr = ds_ptr->GetLayer(0);
    layer_name = layer_ptr->GetName();
  }
  if (layer_ptr == nullptr) {
    std::cerr << "Could not get layer with name " << layer_name << std::endl;
    throw std::runtime_error("Dataset does not have specified layer");
  }

  std::cout << "Reading features from " << filename << ", layer " << layer_name << std::endl;

  for (auto&& feat_ptr : *layer_ptr) {
    OGRGeometry* geom_ptr = feat_ptr->GetGeometryRef();
    if (geom_ptr == nullptr) {
      // Ignore features with no geometry
      continue;
    }
    
    std::cout << "Reading feature from file." << std::endl;
    char* wkt_char_ptr;
    geom_ptr->exportToWkt(&wkt_char_ptr);
    if (wkt_char_ptr == nullptr) {
      std::cerr << "Did not get WKT from GDAL library" << std::endl;
      throw std::runtime_error("Could not get WKT");
    }
    std::string wkt = wkt_char_ptr;
    CPLFree(wkt_char_ptr);
    read_wkt_geometry(wkt, gc);
  }

  GDALClose(ds_ptr);
}
#else

void read_gdal_geometry(const Config& config, GeometryCollection& gc)
{
  throw std::runtime_error("GDAL geometry is not supported.");
}

#endif
