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

#include <boost/program_options.hpp>
namespace bpo = boost::program_options;

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

GlobalConfig::GlobalConfig(const stdfs::path& base_path,
			   const Config& config)
  : simulation_base_path_(base_path),
    config_(config)
{
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
  bpo::options_description opts_desc("Command-line flags");
  opts_desc.add_options()
    ("help", "produce help message")
    ("base-path", bpo::value<std::string>(), "simulation base path")
    ("accel-platform", bpo::value<std::string>(), "accelerator platform")
    ("accel-device", bpo::value<std::string>(), "accelerator device")
    ;

  bpo::options_description hidden_desc("Hidden options");
  hidden_desc.add_options()
    ("config-file", bpo::value<std::vector<std::string>>(),
     "configuration file name")
    ;
  
  bpo::options_description all_desc("Allowed options");
  all_desc.add(opts_desc).add(hidden_desc);
  
  bpo::positional_options_description pos_opts;
  pos_opts.add("config-file", -1);
  
  bpo::variables_map bpo_vm;
  bpo::store(bpo::command_line_parser(argc, argv)
	     .options(all_desc).positional(pos_opts).run(), bpo_vm);
  bpo::notify(bpo_vm);

  if (bpo_vm.count("help")) {
    std::cout << opts_desc << std::endl;
    throw std::runtime_error("User did not request simulation.");
  }

  std::string config_file_path_str;
  if (bpo_vm.count("config-file")) {
    auto config_file_vec = bpo_vm["config-file"].as<std::vector<std::string>>();
    if (config_file_vec.size() > 1) {
      std::cerr << "Multiple input files not yet supported." << std::endl;
      throw std::runtime_error("Multiple input files not yet supported.");
    } else {
      config_file_path_str = config_file_vec.at(0);
    }
  } else {
    std::cerr << "No input configuration specified." << std::endl;
    throw std::runtime_error("No input configuration specified.");
  }
  
  stdfs::path config_file_path(config_file_path_str);
  stdfs::path base_path = ".";
  if (bpo_vm.count("base-path")) {
    base_path = bpo_vm["base-path"].as<std::string>();
    if (not config_file_path.is_absolute()) {
      config_file_path = base_path / config_file_path;
    }
  } else {
    base_path = config_file_path.parent_path();
  }
  
  std::cout << "Simulation base directory: " << base_path << std::endl;
  std::cout << "Configuration file name: " << config_file_path << std::endl;

  Config config;
  bpt::read_mf(config_file_path.native(), config);

  if (bpo_vm.count("accel-platform")) {
    config.put<std::string>("device.platform",
			    bpo_vm["accel-platform"].as<std::string>());
  }
  if (bpo_vm.count("accel-device")) {
    config.put<std::string>("device.device",
			    bpo_vm["accel-device"].as<std::string>());
  }
  
  global_config_ = new GlobalConfig(base_path, config);
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
  auto ts_crange = config_.equal_range("boundary");
  for (auto it = ts_crange.first; it != ts_crange.second; ++it) {
    std::string ts_name = it->second.get_value<std::string>();
    if (name == ts_name) {
      return it->second;
    }
  }
  return config_.add("boundary", name);
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

const std::vector<std::string> GlobalConfig::
output_files_list(void)
{
  std::vector<std::string> configs;

  // Add any defaults here
  // configs.push_back("h");
  
  // Populate the vector with configurations specified by the user
  auto crange = config_.equal_range("output");
  for (auto it = crange.first; it != crange.second; ++it) {
    configs.push_back(it->second.get_value<std::string>());
  }

  // TODO: make the list unique

  return configs;
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

const std::vector<std::reference_wrapper<Config>>
GlobalConfig::source_term_configurations(void)
{
  std::vector<std::reference_wrapper<Config>> vec;
  auto ts_crange = config_.equal_range("source term");
  for (auto it = ts_crange.first; it != ts_crange.second; ++it) {
    vec.push_back(it->second);
  }
  return vec;
}

const std::vector<std::reference_wrapper<Config>>
GlobalConfig::boundary_configurations(void)
{
  std::vector<std::reference_wrapper<Config>> vec;
  auto ts_crange = config_.equal_range("boundary");
  for (auto it = ts_crange.first; it != ts_crange.second; ++it) {
    vec.push_back(it->second);
  }
  return vec;
}
