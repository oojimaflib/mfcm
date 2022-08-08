/***********************************************************************
 * mfcm Config/Config.cpp
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

#include "Config.hpp"

template<typename T>
std::vector<T> split_string(const std::string& in,
			    const std::string& delimiters)
{
  std::vector<std::string> vec;
  boost::algorithm::split(vec, in, boost::algorithm::is_any_of(delimiters));
  std::vector<T> res;
  for (auto&& item : vec) {
    res.push_back(boost::lexical_cast<T>(item));
  }
  return res;
}

template<>
std::vector<std::string> split_string(const std::string& in,
				      const std::string& delimiters)
{
  std::vector<std::string> vec;
  boost::algorithm::split(vec, in, boost::algorithm::is_any_of(delimiters));
  for (auto&& item : vec) {
    boost::algorithm::trim(item);
  }
  return vec;
}

template<typename T, unsigned long D>
std::array<T, D> split_string(const std::string& in,
			      const std::string& delimiters)
{
  std::vector<std::string> vec = split_string<std::string>(in, delimiters);
  if (vec.size() != D) {
    std::cerr << "Expected " << D
	      << " components in vector " << in << std::endl;
    std::cerr << "Actually got " << vec.size() << " components: " << std::endl;
    for (auto&& comp : vec) {
      std::cerr << "\"" << comp << "\"" << std::endl;
    }
    throw std::runtime_error("Unexpected number of components in vector");
  }
  
  std::array<T, D> arr;
  for (size_t i = 0; i < D; ++i) {
    arr[i] = boost::lexical_cast<T>(vec.at(i));
  }
  return arr;
}

GlobalConfig* GlobalConfig::global_config_ = nullptr;

GlobalConfig::GlobalConfig(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "ERROR: Expected one simulation file as argument."
	      << std::endl;
    throw std::runtime_error("Wrong argument list");
  }

  std::string config_file_path_str = argv[1];
  stdfs::path config_file_path(config_file_path_str);
  if (config_file_path.has_filename()) {
    try {
      config_filename_ = config_file_path.filename();
      std::cout << config_filename_ << std::endl;
      simulation_base_path_ = config_file_path.parent_path();
    } catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      throw e;
    }
  } else {
    std::cerr << "Could not determine filename from " << config_file_path << std::endl;
  }

  std::cout << "Simulation base directory: " << simulation_base_path_ << std::endl;
  std::cout << "Configuration file name: " << config_filename_ << std::endl;

  bpt::read_mf((simulation_base_path_ / config_filename_).native(),
	       config_);
}

GlobalConfig& GlobalConfig::instance(void)
{
  if (global_config_) {
    return *global_config_;
  }
  throw std::logic_error("Global configuration not initialized");
}

void GlobalConfig::init(int argc, char* argv[])
{
  if (global_config_) {
    throw std::logic_error("Multiple initializations of global configuration.");
  }
  std::cout << "Initialising configuration. "
	    << argc << " argument(s). Config file: " << argv[1] << std::endl;
  global_config_ = new GlobalConfig(argc, argv);
}

const Config& GlobalConfig::device_configuration(void) const
{
  return config_.get_child("device");
}

const Config& GlobalConfig::scheme_configuration(void) const
{
  return config_.get_child("scheme");
}

const Config& GlobalConfig::mesh_configuration(void) const
{
  return config_.get_child("mesh");
}

const Config& GlobalConfig::
time_series_configuration(const std::string& name) const
{
  auto ts_crange = config_.equal_range("time series");
  for (auto it = ts_crange.first; it != ts_crange.second; ++it) {
    std::string ts_name = it->second.get_value<std::string>();
    if (name == ts_name) {
      return it->second;
    }
  }
  std::cerr << "Could not find time series '" << name << "'."
	    << std::endl;
  throw std::runtime_error("Could not find time series.");
}

const Config& GlobalConfig::
raster_configuration(const std::string& name) const
{
  auto ts_crange = config_.equal_range("raster field");
  for (auto it = ts_crange.first; it != ts_crange.second; ++it) {
    std::string ts_name = it->second.get_value<std::string>();
    if (name == ts_name) {
      return it->second;
    }
  }
  std::cerr << "Could not find raster field '" << name << "'."
	    << std::endl;
  throw std::runtime_error("Could not find raster field.");
}

const Config& GlobalConfig::
field_configuration(const std::string& name)
{
  auto ts_crange = config_.equal_range("field");
  for (auto it = ts_crange.first; it != ts_crange.second; ++it) {
    std::string ts_name = it->second.get_value<std::string>();
    if (name == ts_name) {
      return it->second;
    }
  }
  std::cout << "Could not find field '" << name << "'."
	    << std::endl;
  std::cout << "Using default values for field "
	    << std::quoted(name) << "." << std::endl;
  return config_.add("field", name);
}

const Config& GlobalConfig::
boundary_configuration(const std::string& name)
{
  auto ts_crange = config_.equal_range("boundaries");
  for (auto it = ts_crange.first; it != ts_crange.second; ++it) {
    std::string ts_name = it->second.get_value<std::string>();
    if (name == ts_name) {
      return it->second;
    }
  }
  return config_.add("boundaries", name);
}

const Config& GlobalConfig::
check_file_configuration(const std::string& name)
{
  // If the user specified details of the check file, return those.
  auto crange = config_.equal_range("check");
  for (auto it = crange.first; it != crange.second; ++it) {
    std::string check_name = it->second.get_value<std::string>();
    if (name == check_name) {
      return it->second;
    }
  }

  // If we get here, create a configuration for the check file that
  // explicitly disables it.
  Config& conf = config_.add("check", name);
  conf.put("enabled", false);
  
  // Some check files should happen by default. If this is one of
  // those, set appropriate default parameters.
  if (name == "constants") {
    conf.put("enabled", true);
    conf.put("filename", stdfs::path("check") / "constants.csv");
    conf.put("delimiter", ",");
  }

  // Return the newly-created check file configuration.
  return conf;
}

const Config& GlobalConfig::
output_file_configuration(const std::string& name)
{
  // If the user specified details of the output file, return those.
  auto crange = config_.equal_range("output");
  for (auto it = crange.first; it != crange.second; ++it) {
    std::string check_name = it->second.get_value<std::string>();
    if (name == check_name) {
      return it->second;
    }
  }

  // If we get here, create a configuration for the output file that
  // explicitly disables it.
  Config& conf = config_.add("output", name);
  conf.put("enabled", false);
  
  // Some output files might want to happen by default. We can create
  // configurations for them here.
  if (name == "constants") {
    conf.put("enabled", true);
    conf.put("every seconds", 1e39);
    conf.put("filename", stdfs::path("check") / "constants.csv");
    conf.put("delimiter", ",");
  }

  // Return the newly-created output file configuration.
  return conf;
}

const Config& GlobalConfig::
measure_configuration(void)
{
  if (config_.count("measures") > 0) {
    return config_.get_child("measures");
  } else {
    return config_.add("measures", "");
  }
  // // If configuration data for the named measure exists, return it.
  // auto crange = config_.equal_range("measure");
  // for (auto it = crange.first; it != crange.second; ++it) {
  //   std::string check_name = it->second.get_value<std::string>();
  //   if (name == check_name) {
  //     return it->second;
  //   }
  // }

  // // If we get here, there's no user-supplied condfiguration. Create a
  // // default.
  // Config& conf = config_.add("measure", name);
  // conf.put("enabled", false);

  // // If we want to have some named measures enabled by default, test
  // // for them here.
  
  // return conf;
  
}
