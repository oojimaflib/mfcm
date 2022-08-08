/***********************************************************************
 * mfcm Output/Measure.hpp
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

#ifndef mfcm_Output_Measure_hpp
#define mfcm_Output_Measure_hpp

template<typename TT,
	 typename S>
class Measure
{
public:

  using TimeType = TT;

  using StateType = S;
  using FieldType = typename StateType::FieldType;
  using ValueType = typename FieldType::ValueType;
  using MeshType = typename FieldType::MeshType;
  static const MeshComponent FieldMappingType = FieldType::FieldMappingType;

private:

  std::shared_ptr<TimeSeries<TimeType, ValueType>> values_;

  size_t next_uninitialized_value_;

  std::string name_;

  stdfs::path output_filename_;
  
protected:

  virtual void set_point(const TimeType& t, const ValueType& v)
  {
    values_->set_point(next_uninitialized_value_++, t, v);
  }
  
public:
  
  Measure(const std::shared_ptr<sycl::queue>& queue,
	  const std::shared_ptr<TimeParameters<TimeType>>& tparams,
	  const Config& config)
    : values_(std::make_shared<TimeSeries<TimeType,ValueType>>(queue, tparams->num_steps() + 1, false)),
      next_uninitialized_value_(0),
      name_(config.get_value<std::string>())
  {
    output_filename_ = config.get<stdfs::path>("filename",
					       stdfs::path("output") / "measures" / (name_ + ".csv"));
    if (not output_filename_.is_absolute()) {
      output_filename_ = GlobalConfig::instance().simulation_base_path() / output_filename_;
    }
  }

  virtual ~Measure(void)
  {}

  const std::string& name(void) const
  {
    return name_;
  }
  
  virtual void update(const TimeType& time_now,
		      const StateType& state) = 0;

  virtual const std::shared_ptr<TimeSeries<TimeType, ValueType>>& result(void) const
  {
    return values_;
  }

  void output(void) const
  {
    std::cout << "Writing measure " << std::quoted(name()) << " to "
	      << std::quoted(output_filename_.native()) << std::endl;
    if (stdfs::create_directories(output_filename_.parent_path())) {
      std::cout << "Created output directory "
		<< std::quoted(output_filename_.parent_path().native()) << std::endl;
    }

    auto formatter = std::make_shared<CSVOutput>(TimeSeriesOutputFunction<TimeSeries<TimeType,ValueType>>(name_, values_));
    formatter->output(output_filename_);
  }
  
};

#endif
