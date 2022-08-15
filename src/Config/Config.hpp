/***********************************************************************
 * mfcm Config/Config.hpp
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

#ifndef mfcm_Config_Config_hpp
#define mfcm_Config_Config_hpp

#include <stdexcept>
#include <iostream>
#include <filesystem>
namespace stdfs = std::filesystem;

#include "mf_parser.hpp"
namespace bpt = boost::property_tree;
typedef bpt::iptree Config;

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

template<typename T>
std::vector<T> split_string(const std::string& in,
			    const std::string& delimiters = ",");

template<>
std::vector<std::string> split_string(const std::string& in,
				      const std::string& delimiters);

template<typename T, unsigned long D>
std::array<T, D> split_string(const std::string& in,
			      const std::string& delimiters = ",");

class GlobalConfig
{
protected:

  /**
     Constructor.

     Get configuration from command-line arguments and files specified
     on the command line.
   */
  GlobalConfig(const stdfs::path& base_path,
	       const Config& config);

  /**
     Static pointer to the singleton GlobalConfig object.
   */
  static GlobalConfig* global_config_;

  /**
     Filename from which the configuration has been read.
   */
  // stdfs::path config_filename_;
  
  /**
     "Base" path of the simulation. File paths in the configuration
     file will be relative to this.
   */
  stdfs::path simulation_base_path_;
  
  /**
     Config object holding the user-supplied key-value pairs.
   */
  Config config_;

public:

  /**
     No public copy constructor to enforce singleton.
   */
  GlobalConfig(GlobalConfig&) = delete;
  /**
     No public assignment operator to enforce singleton.
   */
  void operator=(const GlobalConfig&) = delete;

  /**
     Retrieve the singleton instance.
   */
  static GlobalConfig& instance(void);

  /**
     Create the singleton instance.
   */
  static void init(int argc, char* argv[]);

  /**
     Get the base path for the simulation.
   */
  const stdfs::path& simulation_base_path(void) const
  {
    return simulation_base_path_;
  }

  /**
     Get the device configuration
  */
  const Config& device_configuration(void) const;

  /**
     Get the scheme configuration
  */
  const Config& scheme_configuration(void) const;
  
  /**
     Get the mesh configuration
   */
  const Config& mesh_configuration(void) const;

  /**
     Get the configuration for a time series by name
   */
  const Config& time_series_configuration(const std::string& name) const;
  
  /**
     Get the configuration for a time series by name
   */
  const Config& raster_configuration(const std::string& name) const;
  
  /**
     Get the configuration for a field by name
   */
  const Config& field_configuration(const std::string& name);

  /**
     Get the configuration for a boundary condition by type
   */
  const Config& boundary_configuration(const std::string& name);
  
  /**
     Get the configuration for a check file
  */
  const Config& check_file_configuration(const std::string& name);
  
  /**
     Get the configuration for an output file
  */
  const std::vector<std::string> output_files_list(void);

  /**
     Get the configuration for an output file
  */
  const Config& output_file_configuration(const std::string& name);

  /**
     Get the configuration for a measure
  */
  const Config& measure_configuration(void);

  const std::vector<std::reference_wrapper<Config>>
  source_term_configurations(void);
};

#endif
