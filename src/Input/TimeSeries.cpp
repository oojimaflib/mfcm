/***********************************************************************
 * mfcm Input/TimeSeries.cpp
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

#include "TimeSeries.hpp"
#include "TimeParser.hpp"
#include "../Output/CheckFile.hpp"

#include "../rapidcsv/rapidcsv.h"
#include <ctime>

template<typename TT, typename T, typename TimesType, typename ValuesType>
T ts_linterp(const TimesType& time_ro_,
	     const ValuesType& values_ro_,
	     const TT& time)
{
  size_t i1 = 1;
  for (; i1 < time_ro_.size(); ++i1) {
    if (time_ro_[i1] > time) break;
  }
  size_t i0 = i1 - 1;

  TT t0 = time_ro_[i0];
  TT t1 = time_ro_[i1];
  T v0 = values_ro_[i0];
  T v1 = values_ro_[i1];

  TT dvdt = (v1 - v0) / (t1 - t0);
  TT dt = time - t0;
  T result = v0 + dvdt * dt;
    
  return result;
}

template<typename TT,
	 typename T>
TimeSeries<TT,T>::TransformData::TransformData(const Config& conf)
  : time_factor(conf.get<TimeType>("time factor", 1.0)),
    time_offset(conf.get<TimeType>("time offset", 0.0)),
    value_factor(conf.get<ValueType>("value factor", 1.0)),
    value_offset(conf.get<ValueType>("value offset", 0.0))
{
}

template<typename TT,
	 typename T>
TimeSeries<TT,T>::TimeSeries(const std::shared_ptr<sycl::queue>& queue,
			     const std::vector<TimeType>& times,
			     const std::vector<ValueType>& values,
			     bool on_device)
  : time_(queue, times, on_device),
    value_(queue, values, on_device)
{
}

template<typename TT,
	 typename T>
TimeSeries<TT,T>::TimeSeries(const std::shared_ptr<sycl::queue>& queue,
			     const size_t& ndata,
			     bool on_device)
  : time_(queue, ndata,
	  std::numeric_limits<TimeType>::quiet_NaN(), on_device),
    value_(queue, ndata,
	   std::numeric_limits<ValueType>::quiet_NaN(), on_device)
{
}

template<typename TT,
	 typename T>
void TimeSeries<TT,T>::
apply_transform(const TimeSeries<TT,T>::TransformData& td)
{
  // If the data is on the device, move it back to the host to apply
  // the transformation.
  bool time_on_device = time_.is_on_device();
  if (time_on_device) time_.move_to_host();
  bool value_on_device = value_.is_on_device();
  if (value_on_device) value_.move_to_host();

  // Apply the transformation
  for (auto&& t : time_.host_vector()) {
    t = (t + td.time_offset) * td.time_factor;
  }
  for (auto&& v : value_.host_vector()) {
    v = (v + td.value_offset) * td.value_factor;
  }

  // If the data was originally on the device, move it back.
  if (time_on_device) time_.move_to_device();
  if (value_on_device) value_.move_to_device();
}

template<typename TT,
	 typename T>
T TimeSeries<TT,T>::at(const TimeType& time) const
{
  return ts_linterp<TimeType,ValueType,
		    std::vector<TimeType>,
		    std::vector<ValueType>>(time_.host_vector(),
					    value_.host_vector(), time);
}

template<typename TT,
	 typename T>
void TimeSeries<TT,T>::set_point(size_t index,
				 const TimeType& time,
				 const ValueType& value)
{
  // If the data is on the device, move it back to the host to set the
  // point
  bool time_on_device = time_.is_on_device();
  if (time_on_device) time_.move_to_host();
  bool value_on_device = value_.is_on_device();
  if (value_on_device) value_.move_to_host();

  time_.host_vector().at(index) = time;
  value_.host_vector().at(index) = value;

  // If the data was originally on the device, move it back.
  if (time_on_device) time_.move_to_device();
  if (value_on_device) value_.move_to_device();
}

template<typename TT,
	 typename T>
void TimeSeries<TT,T>::sanity_checks(void) const
{
  // Check that the time and values arrays have the same size.
  if (time_.size() != value_.size()) {
    std::cerr << "Mismatched counts of times and values in time series."
	      << std::endl;
    throw std::logic_error("Mismatched time and value counts in time series.");
  }
    
  // Check that the host and device data are at least the same size.
  if (time_.host_vector().size() != time_.get_buffer().get_count()) {
    std::cerr << "Mismatched counts of times on the host and device."
	      << std::endl;
    throw std::logic_error("Mismatched counts of times on the host and device.");
  }
  if (value_.host_vector().size() != value_.get_buffer().get_count()) {
    std::cerr << "Mismatched counts of values on the host and device."
	      << std::endl;
    throw std::logic_error("Mismatched counts of values on the host and device.");
  }

  // Check that the length of the time series is at least 2
  if (size() < 2) {
    std::cerr << "Time series does not have at least two data."
	      << std::endl;
    throw std::runtime_error("Time series does not have at least two data.");
  }

  TimeType t_old = std::numeric_limits<TimeType>::lowest();
  for (auto&& t : time_.host_vector()) {
    if (t <= t_old) {
      std::cerr << "Time points do not increase in a time series."
		<< std::endl;
      throw std::logic_error("Time points do not increase in a time series.");
    }
    t_old = t;
  }
}

template<typename TT,
	 typename T>
std::shared_ptr<TimeSeries<TT,T>>
TimeSeries<TT,T>::load_inline(const std::shared_ptr<sycl::queue>& queue,
			      const TimeParser<TimeType>& parser,
			      const Config& conf)
{
  using boost::algorithm::to_lower_copy;
  std::vector<TimeType> times;
  std::vector<ValueType> values;

  TimeParser<TimeType> tparse(parser, conf);
    
  for (auto&& tsp : conf.get_child("data")) {
    std::string key = to_lower_copy(tsp.first);
    TimeType time = tparse.parse(key);
    if (times.size() > 0 and time <= times.back()) {
      std::cerr << "Times in time series must increase." << std::endl;
      std::cerr << time << " < " << times.back() << std::endl;
      throw std::runtime_error("Times in time series must increase.");
    }
    times.push_back(time);
    values.push_back(tsp.second.get_value<ValueType>());
  }
  return std::make_shared<TimeSeries<TimeType,ValueType>>(queue, times, values, false);
}

template<typename TT,
	 typename T>
std::shared_ptr<TimeSeries<TT,T>>
TimeSeries<TT,T>::load_csv(const std::shared_ptr<sycl::queue>& queue,
			   const TimeParser<TimeType>& parser,
			   const Config& conf)
{
  namespace CSV = rapidcsv;
    
  stdfs::path user_filepath = conf.get<stdfs::path>("filename");
  stdfs::path filepath;
  if (user_filepath.is_absolute()) {
    filepath = user_filepath;
  } else {
    filepath = GlobalConfig::instance().simulation_base_path()
      / user_filepath;
  }
    
  char separator = conf.get<char>("separator", ',');
  char comment_char = conf.get<char>("comment character", '#');
  bool headers = conf.get<bool>("headers", true);
  int skip_rows = conf.get<int>("skip rows", -1);
  int skip_cols = conf.get<int>("skip cols", -1);
  if (headers) {
    skip_rows += 1;
  }

  TimeParser<TimeType> tparse(parser, conf);
    
  size_t time_col;
  size_t value_col;
    
  CSV::Document doc(filepath.native(),
		    CSV::LabelParams(skip_rows, skip_cols),
		    CSV::SeparatorParams(separator, true),
		    CSV::ConverterParams(true),
		    CSV::LineReaderParams(true, comment_char, true));
    
  if (headers) {
    std::string time_header = conf.get<std::string>("time column");
    std::string value_header = conf.get<std::string>("value column");
    time_col = doc.GetColumnIdx(time_header);
    value_col = doc.GetColumnIdx(value_header);
  } else {
    time_col = conf.get<size_t>("time column", 1) - 1;
    value_col = conf.get<size_t>("value column", 2) - 1;
  }

  std::vector<std::string> time_strings =
    doc.GetColumn<std::string>(time_col);
  std::vector<TimeType> times;
  for (auto&& ts : time_strings) {
    times.push_back(tparse.parse(ts));
  }
  std::vector<ValueType> values = doc.GetColumn<ValueType>(value_col);
  return std::make_shared<TimeSeries<TimeType,ValueType>>(queue, times, values, false);
}

template<typename TT,
	 typename T>
std::shared_ptr<TimeSeries<TT,T>>
TimeSeries<TT,T>::load(const std::shared_ptr<sycl::queue>& queue,
		       const TimeParser<TimeType>& parser,
		       const Config& conf,
		       bool on_device)
{
  std::string source_type_str =
    boost::algorithm::to_lower_copy(conf.get<std::string>("source",
							  "not specified"));

  std::shared_ptr<TimeSeries<TimeType,ValueType>> ts_ptr;
  if (source_type_str == "inline") {
    ts_ptr = TimeSeries<TimeType,ValueType>::
      load_inline(queue, parser, conf.get_child("source"));
  } else if (source_type_str == "csv") {
    ts_ptr = TimeSeries<TimeType,ValueType>::
      load_csv(queue, parser, conf);
  } else {
    std::cerr << "Unknown source type '" << source_type_str
	      << "' for time series." << std::endl;
    throw std::runtime_error("Unknown source type for time series");
  }

  // Apply any transformation specified by the user.
  Config empty;
  TransformData td(conf.get_child("transform", empty));
  ts_ptr->apply_transform(td);

  if (on_device) ts_ptr->move_to_device();
  return ts_ptr;
}

template<typename TT,
	 typename T,
	 sycl::access::mode AM>
TimeSeriesAccessor<TT,T,AM>::
TimeSeriesAccessor(const TimeSeries<TimeType,ValueType>& ts)
  : time_acc_(ts.get_time_array().template get_placeholder_accessor<AM,sycl::access::target::global_buffer>()),
    values_acc_(ts.get_value_array().template get_placeholder_accessor<AM,sycl::access::target::global_buffer>())
{
}

template<typename TT,
	 typename T,
	 sycl::access::mode AM>
void TimeSeriesAccessor<TT,T,AM>::bind(sycl::handler& cgh)
{
  cgh.require(time_acc_);
  cgh.require(values_acc_);
}

template<typename TT,
	 typename T,
	 sycl::access::mode AM>
T TimeSeriesAccessor<TT,T,AM>::operator()(const TimeType& time) const
{
  return ts_linterp<TimeType,
		    ValueType,
		    TimeAccessor,
		    ValueAccessor>(time_acc_,
				   values_acc_, time);
}

template<typename TT,
	 typename T,
	 sycl::access::mode AM>
void TimeSeriesAccessor<TT,T,AM>::set_point(size_t index,
					    const TimeType& time,
					    const ValueType& value)
{
  if constexpr (AM == sycl::access::mode::write or
		AM == sycl::access::mode::read_write) {
    time_acc_[index] = time;
    values_acc_[index] = value;
  }
}

template<typename TT,
	 typename T>
TimeSeriesDatabase<TT,T>&
TimeSeriesDatabase<TT,T>::instance(void)
{
  if (not tsdb_) {
    tsdb_ = new TimeSeriesDatabase<TimeType,ValueType>();
  }
  return *tsdb_;
}

template<typename TT,
	 typename T>
const std::shared_ptr<TimeSeries<TT,T>>&
TimeSeriesDatabase<TT,T>::
get_time_series_ptr(const std::shared_ptr<sycl::queue>& queue,
		    const TimeParser<TimeType>& parser,
		    const std::string& name)
{
  if (db_.count(name) == 0) {
    const GlobalConfig& gc = GlobalConfig::instance();
    const Config& conf = gc.time_series_configuration(name);
    db_[name] = TimeSeries<TimeType,ValueType>::load(queue, parser, conf);

    db_[name]->sanity_checks();
    
    std::cout << "Loaded time series " << name << std::endl;
    TimeSeriesCheckFile<TimeSeries<TimeType,ValueType>> cf("time series");
    cf.output(name);
  }
  return db_[name];
}

template<typename TT,
	 typename T>
const std::shared_ptr<TimeSeries<TT,T>>&
TimeSeriesDatabase<TT,T>::
get_time_series_ptr(const std::string& name)
{
  if (db_.count(name) == 0) {
    throw std::logic_error("No queue available in time series database.");
  }
  return db_[name];
}
