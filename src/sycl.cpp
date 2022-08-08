/***********************************************************************
 * mfcm sycl.cpp
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

#include "sycl.hpp"
#include "Config.hpp"

std::shared_ptr<sycl::queue> get_sycl_queue(void)
{
  const Config& device_conf = GlobalConfig::instance().device_configuration();

  std::vector<sycl::platform> platforms = sycl::platform::get_platforms();
  std::vector<std::string> platform_names;
  for (size_t i = 1; i <= platforms.size(); ++i) {
    const sycl::platform& p = platforms.at(i-1);
    platform_names.push_back(std::string("#") + std::to_string(i) + " " +
			     p.get_info<sycl::info::platform::name>() +
			     " " +
			     p.get_info<sycl::info::platform::version>() +
			     " " +
			     p.get_info<sycl::info::platform::vendor>());
  }

  size_t platform_id = platforms.size();
  std::string platform_str =
    device_conf.get<std::string>("platform", "list");
  if (platform_str == "list") {
    std::cout << "List of available computational platforms:" << std::endl;
    for (auto&& pn : platform_names) {
      std::cout << "  " << pn << std::endl;
    }
  } else {
    for (size_t i = 0; i < platforms.size(); ++i) {
      size_t pos = platform_names.at(i).find(platform_str);
      if (pos != std::string::npos) {
	// We matched this platform's name
	if (platform_id == platforms.size()) {
	  platform_id = i;
	} else {
	  // More than one platform matches!
	  std::cerr << "More than one available platform matches '"
		    << platform_str << "':" << std::endl
		    << "  " << platform_names.at(platform_id) << std::endl
		    << "  " << platform_names.at(i) << std::endl;
	  throw std::runtime_error("Platform is ambiguous.");
	}
      }
    }
  }
  if (platform_id == platforms.size()) {
    // We didn't match any platform
    std::cerr << "No platform available matching '"
	      << platform_str << "'" << std::endl;
    std::cerr << "List of available computational platforms:"
	      << std::endl;
    for (auto&& pn : platform_names) {
      std::cerr << "  " << pn << std::endl;
    }
    throw std::runtime_error("Platform not available.");
  }

  sycl::platform& platform = platforms.at(platform_id);

  std::vector<sycl::device> devices = platform.get_devices();
  std::vector<std::string> device_names;
  for (size_t i = 1; i <= devices.size(); ++i) {
    const sycl::device& d = devices.at(i-1);
    device_names.push_back(std::string("#") + std::to_string(i) + " " +
			   d.get_info<sycl::info::device::name>() + " " +
			   d.get_info<sycl::info::device::vendor>());
  }

  size_t device_id = devices.size();
  std::string device_str =
    device_conf.get<std::string>("device", "list");
  if (device_str == "list") {
    std::cout << "List of available computational devices:" << std::endl;
    for (auto&& dn : device_names) {
      std::cout << "  " << dn << std::endl;
    }
  } else {
    for (size_t i = 0; i < devices.size(); ++i) {
      size_t pos = device_names.at(i).find(device_str);
      if (pos != std::string::npos) {
	// We matched this device's name
	if (device_id == devices.size()) {
	  device_id = i;
	} else {
	  // More than one device matches!
	  std::cerr << "More than one available device matches '"
		    << device_str << "':" << std::endl
		    << "  " << device_names.at(device_id) << std::endl
		    << "  " << device_names.at(i) << std::endl;
	  throw std::runtime_error("Device is ambiguous.");
	}
      }
    }
  }
  if (device_id == devices.size()) {
    // We didn't match any device
    std::cerr << "No device available matching '"
	      << device_str << "'" << std::endl;
    std::cerr << "List of available computational devices:"
	      << std::endl;
    for (auto&& dn : device_names) {
      std::cerr << "  " << dn << std::endl;
    }
    throw std::runtime_error("Device not available.");
  }    
  
  return std::make_shared<sycl::queue>(devices.at(device_id));
}
