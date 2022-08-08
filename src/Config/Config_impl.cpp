/***********************************************************************
 * mfcm Config/Config_impl.cpp
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

#include "mf_parser.cpp"
#include "Config.cpp"

namespace boost {
  namespace property_tree {
    namespace mf_parser {

      template void read_mf<Config, char>(std::basic_istream<char>&, Config&);
      template void read_mf<Config, char>(std::basic_istream<char>&, Config&,
					  const Config&);
      template void read_mf<Config>(const std::string&, Config&,
				    const std::locale&);
      template void read_mf<Config>(const std::string&, Config&,
				    const Config&, const std::locale&);

      template void write_mf<Config, char>(std::basic_ostream<char>&,
					   const Config&,
					   const mf_writer_settings<char>&);
      template void write_mf<Config>(const std::string&, const Config&,
				     const std::locale& loc,
				     const mf_writer_settings<Config::key_type::value_type>&);
      
    }
  }
}

template
std::vector<float> split_string(const std::string& in,
				const std::string& delimiters = ",");
template
std::vector<double> split_string(const std::string& in,
				 const std::string& delimiters = ",");
template
std::vector<int32_t> split_string(const std::string& in,
				  const std::string& delimiters = ",");
template
std::vector<uint32_t> split_string(const std::string& in,
				   const std::string& delimiters = ",");
template
std::vector<size_t> split_string(const std::string& in,
				 const std::string& delimiters = ",");

template
std::array<float, 2> split_string<float, 2>(const std::string& in,
				  const std::string& delimiters = ",");
template
std::array<float, 3> split_string<float, 3>(const std::string& in,
				  const std::string& delimiters = ",");
template
std::array<float, 4> split_string<float, 4>(const std::string& in,
				  const std::string& delimiters = ",");
template
std::array<float, 5> split_string<float, 5>(const std::string& in,
				  const std::string& delimiters = ",");
template
std::array<float, 6> split_string<float, 6>(const std::string& in,
				  const std::string& delimiters = ",");


template
std::array<double, 2> split_string<double, 2>(const std::string& in,
				   const std::string& delimiters = ",");
template
std::array<double, 3> split_string<double, 3>(const std::string& in,
				   const std::string& delimiters = ",");
template
std::array<double, 4> split_string<double, 4>(const std::string& in,
				   const std::string& delimiters = ",");
template
std::array<double, 5> split_string<double, 5>(const std::string& in,
				   const std::string& delimiters = ",");
template
std::array<double, 6> split_string<double, 6>(const std::string& in,
				   const std::string& delimiters = ",");



template
std::array<int32_t, 2> split_string<int32_t, 2>(const std::string& in,
				    const std::string& delimiters = ",");
template
std::array<int32_t, 3> split_string<int32_t, 3>(const std::string& in,
				    const std::string& delimiters = ",");
template
std::array<int32_t, 4> split_string<int32_t, 4>(const std::string& in,
				    const std::string& delimiters = ",");
template
std::array<int32_t, 5> split_string<int32_t, 5>(const std::string& in,
				    const std::string& delimiters = ",");
template
std::array<int32_t, 6> split_string<int32_t, 6>(const std::string& in,
				    const std::string& delimiters = ",");



template
std::array<uint32_t, 2> split_string<uint32_t, 2>(const std::string& in,
				     const std::string& delimiters = ",");
template
std::array<uint32_t, 3> split_string<uint32_t, 3>(const std::string& in,
				     const std::string& delimiters = ",");
template
std::array<uint32_t, 4> split_string<uint32_t, 4>(const std::string& in,
				     const std::string& delimiters = ",");
template
std::array<uint32_t, 5> split_string<uint32_t, 5>(const std::string& in,
				     const std::string& delimiters = ",");
template
std::array<uint32_t, 6> split_string<uint32_t, 6>(const std::string& in,
				     const std::string& delimiters = ",");


template
std::array<unsigned long, 2> split_string<unsigned long, 2>(const std::string& in,
				     const std::string& delimiters = ",");
template
std::array<unsigned long, 3> split_string<unsigned long, 3>(const std::string& in,
				     const std::string& delimiters = ",");
template
std::array<unsigned long, 4> split_string<unsigned long, 4>(const std::string& in,
				     const std::string& delimiters = ",");
template
std::array<unsigned long, 5> split_string<unsigned long, 5>(const std::string& in,
				     const std::string& delimiters = ",");
template
std::array<unsigned long, 6> split_string<unsigned long, 6>(const std::string& in,
				     const std::string& delimiters = ",");


