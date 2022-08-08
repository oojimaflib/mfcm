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

#include "TimeParser.hpp"

#include <map>

std::map<std::string, double> time_unit_factors =
  {
    { "seconds", 1.0 },
    { "second", 1.0 },
    { "secs", 1.0 },
    { "sec", 1.0 },
    { "s", 1.0 },
      
    { "minutes", 60.0 },
    { "minute", 60.0 },
    { "mins", 60.0 },
    { "min", 60.0 },
    { "m", 60.0 },
    
    { "hours", 3600.0 },
    { "hour", 3600.0 },
    { "hrs", 3600.0 },
    { "hr", 3600.0 },
    { "h", 3600.0 },
  };

template<typename T>
void TimeParser<T>::parse_configuration(const Config& conf)
{
  time_format_str_ = conf.get<std::string>("time format", time_format_str_);
  formatted_time_ = (time_format_str_.size() > 0);
  if (formatted_time_) {
    std::istringstream tzss(conf.get<std::string>("time zero"));
    std::tm tz = {};
    tzss >> std::get_time(&tz, time_format_str_.c_str());
    time_zero_ = std::mktime(&tz);
  }
  
  // Get the time units
  std::string time_unit_str = conf.get<std::string>("time unit", "default");
  std::cout << "time unit str == " << time_unit_str << std::endl;
  if (time_unit_factors.count(time_unit_str) > 0) {
    // User specified a unit
    time_unit_factor_ = TimeType(time_unit_factors[time_unit_str]);
  } else {
    double user_value;
    auto [ptr, ec] {
      std::from_chars(time_unit_str.data(),
		      time_unit_str.data() + time_unit_str.size(),
		      user_value) };
    if (ec == std::errc()) {
      // User specified a factor
      time_unit_factor_ = user_value;
    } else if (time_unit_str != "default") {
      std::cerr << "Unknown time unit: "
		<< std::quoted(time_unit_str) << std::endl;
      std::cerr << "Valid time units: " << std::endl;
      for (auto&& kv : time_unit_factors) {
	std::cerr << "  " << kv.first << " == " << kv.second << std::endl;
      }
      throw std::runtime_error("Unknown time unit.");
    }
    throw std::runtime_error("Unknown time unit.");
  }
}

template<typename T>
TimeParser<T>::TimeParser(const Config& conf)
  : time_format_str_(""),
    formatted_time_(false),
    time_zero_(0),
    time_unit_factor_(3600.0)
{
  parse_configuration(conf);
  std::cout << "Created time parser with " << std::endl
	    << "  time format == " << std::quoted(time_format_str_) << std::endl
	    << "  [formatted time] == " << formatted_time_ << std::endl
	    << "  time zero == " << time_zero_ << std::endl
	    << "  [time unit factor] == " << time_unit_factor_ << std::endl;
    
}

template<typename T>
TimeParser<T>::TimeParser(const TimeParser& tp,
			  const Config& conf)
  : time_format_str_(tp.time_format_str_),
    formatted_time_(tp.formatted_time_),
    time_zero_(tp.time_zero_),
    time_unit_factor_(tp.time_unit_factor_)
{
  std::cout << "Created time parser with " << std::endl
	    << "  time format == " << std::quoted(time_format_str_) << std::endl
	    << "  [formatted time] == " << formatted_time_ << std::endl
	    << "  time zero == " << time_zero_ << std::endl
	    << "  [time unit factor] == " << time_unit_factor_ << std::endl;
  parse_configuration(conf);
  std::cout << "Updated time parser with " << std::endl
	    << "  time format == " << std::quoted(time_format_str_) << std::endl
	    << "  [formatted time] == " << formatted_time_ << std::endl
	    << "  time zero == " << time_zero_ << std::endl
	    << "  [time unit factor] == " << time_unit_factor_ << std::endl;
}

template<typename T>
T TimeParser<T>::parse(const std::string& time_str) const
{
  if (formatted_time_) {
    std::istringstream ss(time_str);
    std::tm t = {};
    std::tm* tptr = &t;
    ss >> std::get_time(&t, time_format_str_.c_str());
    if (not ss.fail()) {
      std::time_t tt = std::mktime(&t);
      return TimeType(tt - time_zero_);
    }
  }
  return TimeType(boost::lexical_cast<TimeType>(time_str) * time_unit_factor_);
}

template<typename T>
std::string TimeParser<T>::format(const TimeType& time) const
{
  if (formatted_time_) {
    std::time_t tt = time_zero_ + int(time);
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&tt), time_format_str_.c_str());
    return ss.str();
  } else {
    return std::to_string(TimeType(time / time_unit_factor_));
  }
}
