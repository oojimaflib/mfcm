/***********************************************************************
 * mfcm Output/CheckFile.hpp
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

#ifndef mfcm_Output_CheckFile_hpp
#define mfcm_Output_CheckFile_hpp

#include "OutputFormat.hpp"

class CheckFile
{
private:

  std::string name_;
  bool enabled_;
  stdfs::path output_filename_;

protected:

  CheckFile(const std::string& name)
    : name_(name)
  {
    const Config& conf =
      GlobalConfig::instance().check_file_configuration(name);
    enabled_ = conf.get<bool>("enabled", true);
    output_filename_ = conf.get<stdfs::path>("filename",
					     stdfs::path("check") /
					     (name + ".csv"));
    if (not output_filename_.is_absolute()) {
      output_filename_ = GlobalConfig::instance().simulation_base_path() / output_filename_;
    }
  }

public:

  const std::string& name(void) const { return name_; }
  
  bool enabled(void) const { return enabled_; }

  const stdfs::path& output_filename(void) const
  {
    return output_filename_;
  }

  void set_output_filename(const std::string& new_of)
  {
    output_filename_ = new_of;
  }

protected:
  
  const stdfs::path& get_valid_filename(void) const
  {
    const stdfs::path& fn = output_filename_;
    std::cout << "Writing check file " << std::quoted(name())
	      << " to " << std::quoted(fn.native()) << std::endl;
    if (stdfs::create_directories(fn.parent_path())) {
      std::cout << "Created output directory "
		<< std::quoted(fn.parent_path().native()) << std::endl;
    }
    return fn;
  }

  void output(const std::shared_ptr<OutputFunction>& of) const
  {
    if (enabled()) {
      const Config& conf =
	GlobalConfig::instance().check_file_configuration(name_);
      
      const stdfs::path& fn = this->get_valid_filename();
      
      std::shared_ptr<FormattedOutput> formatter;
      std::string format_type_str = conf.get<std::string>("format", "csv");
      if (format_type_str == "csv") {
	Config default_config;
	const Config& format_conf = conf.get_child("format", default_config);
	std::string delimiter = format_conf.get<std::string>("delimiter", ", ");
	formatter = std::make_shared<CSVOutput>(of, delimiter);
      } else {
	std::cerr << "Unknown output format: " << format_type_str << std::endl;
	throw std::runtime_error("Unknown output format");
      }
      formatter->output(fn);
    }
  }
  
};

template<typename Object,
	 typename ObjectOutputFunction>
class ObjectCheckFile : public CheckFile
{
public:

  using ObjectType = Object;
  using OutputFunctionType = ObjectOutputFunction;

  ObjectCheckFile(const std::string& name)
    : CheckFile(name)
  {
  }

  void output(const ObjectType& sel)
  {
    return CheckFile::output(std::make_shared<OutputFunctionType>(sel));
  }

};

template<typename MeshSelection>
using MeshSelectionCheckFile = ObjectCheckFile<MeshSelection, MeshSelectionOutputFunction<MeshSelection>>;

template<typename TimeSeries>
class TimeSeriesCheckFile : public CheckFile
{
public:

  using TimeSeriesType = TimeSeries;
  using OutputFunctionType = TimeSeriesOutputFunction<TimeSeriesType>;

  TimeSeriesCheckFile(const std::string& name)
    : CheckFile(name)
  {
  }

  void output(const std::string& ts_name)
  {
    const stdfs::path& ofn = output_filename();
    stdfs::path fn = ofn.parent_path() / ofn.stem();
    fn += std::string("_") + ts_name;
    fn += ofn.extension();
    set_output_filename(fn);
    return CheckFile::output(std::make_shared<OutputFunctionType>(ts_name));
  }
  
};

template<typename Field>
class FieldCheckFile : public CheckFile
{
public:

  using FieldType = Field;
  using OutputFunctionType = FieldOutputFunction<Field>;

  FieldCheckFile(const std::string& name)
    : CheckFile(name)
  {
  }

  void output(const std::vector<FieldType*>& field_ptrs)
  {
    return CheckFile::output(std::make_shared<OutputFunctionType>(field_ptrs));
  }
  
};

#endif
