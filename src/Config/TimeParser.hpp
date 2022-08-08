/***********************************************************************
 * mfcm Config/TimeParser.hpp
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

#ifndef mfcm_Config_TimeParser_hpp
#define mfcm_Config_TimeParser_hpp

#include "Config.hpp"
#include <charconv>

template<typename T>
class TimeParser {
public:

  using TimeType = T;
  
private:
  /**
     Time format string in "strptime" format
  */
  std::string time_format_str_;
  
  /**
     Boolean indicating whether we expect a formatted string to
     represent time points (true) or a number in seconds (false).
  */
  bool formatted_time_;

  /**
     Time point matching zero seconds.
  */
  std::time_t time_zero_;
  
  /**
     Factor converting the units of time specified (if not a
     formatted time) to seconds.
  */
  double time_unit_factor_;

  void parse_configuration(const Config& conf);
  
public:

  TimeParser(const Config& conf);

  TimeParser(const TimeParser& tp,
	     const Config& conf);

  TimeType parse(const std::string& time_str) const;

  std::string format(const TimeType& time) const;
  
};

#endif
