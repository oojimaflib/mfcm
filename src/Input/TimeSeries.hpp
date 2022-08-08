/***********************************************************************
 * mfcm Input/TimeSeries.hpp
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

#ifndef mfcm_Input_TimeSeries_hpp
#define mfcm_Input_TimeSeries_hpp

#include "Config.hpp"
#include "DataArray.hpp"
#include "TimeParameters.hpp"
/**
   Time series. Stores a series of data of type T, each associated
   with a time point represented by a double.

   @tparam T The type of data stored in the time series.
 */
template<typename TT, typename T>
class TimeSeries
{
public:

  using TimeType = TT;
  using ValueType = T;

  struct TransformData {
    TimeType time_factor;
    TimeType time_offset;
    ValueType value_factor;
    ValueType value_offset;

    TransformData(const Config& conf);
  };
  
private:

  DataArray<TimeType> time_;
  DataArray<ValueType> value_;

public:

  /**
     Construct a time series.

     @param queue Pointer to the SYCL queue object.
     @param times Array of times at which values are stored.
     @param values Array of values associated with each time.

     It is expected that the times and values arrays are of equal
     length.
   */
  TimeSeries(const std::shared_ptr<sycl::queue>& queue,
	     const std::vector<TimeType>& times,
	     const std::vector<ValueType>& values,
	     bool on_device = true);

  /**
     Construct an empty time series.

     @param queue Pointer to the SYCL queue object
     @param ndata Length of time series
  */
  TimeSeries(const std::shared_ptr<sycl::queue>& queue,
	     const size_t& ndata,
	     bool on_device = true);
  
  /**
     Copy the time series data to the device
  */
  void move_to_device(void)
  {
    time_.move_to_device();
    value_.move_to_device();
  }

  /**
     Copy the time series data from the device.
  */
  void move_to_host(void)
  {
    time_.move_to_host();
    value_.move_to_host();
  }
  
  /**
     Return the number of data in the time series.
  */
  size_t size(void) const
  {
    return time_.size();
  }
  
  /**
     Look-up function to return the value from the time series at a
     given time point. Uses linear interpolation between time points.

     @param time Time point to return the value for.
  */
  T at(const TimeType& time) const;

  /** 
      Set a particular point in the time series
  */
  void set_point(size_t index,
		 const TimeType& time,
		 const ValueType& value);
  
  /**
     Return a reference to the array of times.
  */
  const DataArray<TimeType>& get_time_array(void) const
  {
    return time_;
  }

  /**
     Return a reference to the array of values.
  */
  const DataArray<ValueType>& get_value_array(void) const
  {
    return value_;
  }

  /**
     Apply a transformation to the time series data (in the form of a
     scale and offset to each of the time and values column).
  */
  void apply_transform(const TransformData& td);

  /**
     Perform a series of checks on the time series to ensure that it
     is valid. This does not ensure that the data is the same between
     host and device (which it should), but it does ensure that the
     data has matching lengths, that there are at least two data
     points in the time series, and that time points increase through
     the series.
  */
  void sanity_checks(void) const;

  /**
     Load a time series specified "in-line" within the user-supplied
     configuration.
     
     @param queue Pointer to the SYCL queue.
     @param parser TimeParser object for the simulation as a whole.
     @param conf User-supplied configuration.
   */
  static std::shared_ptr<TimeSeries<TimeType,ValueType>>
  load_inline(const std::shared_ptr<sycl::queue>& queue,
	      const TimeParser<TimeType>& parser,
	      const Config& conf);

  /**
     Load a time series from a CSV file specified through the
     user-supplied configuration.

     @param queue Pointer to the SYCL queue.
     @param parser TimeParser object for the simulation as a whole.
     @param conf User-supplied configuration.
   */
  static std::shared_ptr<TimeSeries<TimeType,ValueType>>
  load_csv(const std::shared_ptr<sycl::queue>& queue,
	   const TimeParser<TimeType>& parser,
	   const Config& conf);

  /**
     Load a time series specified by the user-supplied configuration.
     
     @param queue Pointer to the SYCL queue.
     @param parser TimeParser object for the simulation as a whole.
     @param conf User-supplied configuration.
  */
  static std::shared_ptr<TimeSeries<TimeType,ValueType>>
  load(const std::shared_ptr<sycl::queue>& queue,
       const TimeParser<TimeType>& parser,
       const Config& conf,
       bool on_device = true);
  
};


/**
   Accessor class to interact with time series data on a GPU device.

   @tparam T Type of data stored in the time series.
 */
template<typename TT,
	 typename T,
	 sycl::access::mode AM = sycl::access::mode::read>
class TimeSeriesAccessor
{
public:

  using TimeType = TT;
  using ValueType = T;
  
private:

  /**
     Read-only accessor type for the DataArray<double> object storing
     the time points.
   */
  using TimeAccessor =
    typename DataArray<TimeType>::
    template Accessor<AM,
		      sycl::access::target::global_buffer,
		      sycl::access::placeholder::true_t>;
  /**
     Read-only accessor type for the DataArray<T> object storing the
     values.
   */
  using ValueAccessor =
    typename DataArray<ValueType>::
    template Accessor<AM,
		      sycl::access::target::global_buffer,
		      sycl::access::placeholder::true_t>;

  /**
     Read-only accessor to the time points array.
   */
  TimeAccessor time_acc_;
  /**
     Read-only accessor to the values array.
   */
  ValueAccessor values_acc_;
  
public:

  /**
     Construct the accessor object from a TimeSeries object.
   */
  TimeSeriesAccessor(const TimeSeries<TimeType,ValueType>& ts);

  /**
     Bind the accessors to the current context.
   */
  void bind(sycl::handler& cgh);

  /**
     Look-up function to return the value from the time series at a
     given time point. Uses linear interpolation between time points.

     @param time Time point to return the value for.
   */
  T operator()(const TimeType& time) const;

  /** 
      Set a particular point in the time series
  */
  void set_point(size_t index,
		 const TimeType& time,
		 const ValueType& value);
  
};

template<typename TT,
	 typename T>
class TimeSeriesDatabase
{
public:

  using TimeType = TT;
  using ValueType = T;
  
private:

  std::map<std::string, std::shared_ptr<TimeSeries<TimeType,ValueType>>> db_;

protected:

  static TimeSeriesDatabase<TimeType,ValueType>* tsdb_;
  
  TimeSeriesDatabase(void) {}

public:

  TimeSeriesDatabase(TimeSeriesDatabase<TimeType,ValueType>&) = delete;
  void operator=(const TimeSeriesDatabase<TimeType,ValueType>&) = delete;

  static TimeSeriesDatabase<TimeType,ValueType>& instance(void);

  const std::shared_ptr<TimeSeries<TimeType,ValueType>>&
  get_time_series_ptr(const std::shared_ptr<sycl::queue>& queue,
		      const TimeParser<TimeType>& parser,
		      const std::string& name);
  
  const std::shared_ptr<TimeSeries<TimeType,ValueType>>&
  get_time_series_ptr(const std::string& name);
  
};

#endif
