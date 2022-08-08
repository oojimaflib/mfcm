/***********************************************************************
 * mfcm Output/OutputFile.hpp
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

#ifndef mfcm_Output_OutputFile_hpp
#define mfcm_Output_OutputFile_hpp

#include "OutputFormat.hpp"
#include "TemporalScheme/TimeParameters.hpp"

template<typename OutputObject>
class OutputFile
{
protected:

  std::string name_;

  bool enabled_;

  stdfs::path output_filename_;
  std::string delimiter_;

public:

  OutputFile(const std::string& name)
    : name_(name)
  {
    const Config& conf =
      GlobalConfig::instance().output_file_configuration(name);
    enabled_ = conf.get<bool>("enabled", true);

    output_filename_ = conf.get<stdfs::path>("filename",
					     stdfs::path("output") /
					     name);
    if (not output_filename_.is_absolute()) {
      output_filename_ = GlobalConfig::instance().simulation_base_path() / output_filename_;
    }
    delimiter_ = conf.get<std::string>("delimiter", ", ");
  }

  virtual ~OutputFile(void) {}

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
  
  virtual void output(const std::shared_ptr<OutputFunction>& of,
		      const stdfs::path& output_filename) const
  {
    auto formatter = std::make_shared<CSVOutput>(of);
    formatter->output(output_filename);
  }
  
};

template<typename Mesh>
class MeshOutputFile : public OutputFile<Mesh>
{
public:

  MeshOutputFile(const std::string& name)
    : OutputFile<Mesh>(name)
  {}

  virtual ~MeshOutputFile(void) {}

  void mesh_output(const std::shared_ptr<Mesh>& mesh_ptr)
  {
    if (stdfs::create_directories(this->output_filename_)) {
      std::cout << "Created mesh output directory "
		<< std::quoted(this->output_filename_.native()) << std::endl;
    }

    stdfs::path cell_fn = this->output_filename_ / "cells.csv";
    auto cell_of = std::make_shared<MeshOutputFunction<Mesh,
						       MeshComponent::Cell>>(mesh_ptr);
    this->output(cell_of, cell_fn);

    stdfs::path face_fn = this->output_filename_ / "faces.csv";
    auto face_of = std::make_shared<MeshOutputFunction<Mesh,
						       MeshComponent::Face>>(mesh_ptr);
    this->output(face_of, face_fn);

    stdfs::path vertex_fn = this->output_filename_ / "vertices.csv";
    auto vertex_of = std::make_shared<MeshOutputFunction<Mesh,
							 MeshComponent::Vertex>>(mesh_ptr);
    this->output(vertex_of, vertex_fn);
  }
  
};

template<typename TT, typename OutputObject>
class TimedOutputFile : public OutputFile<OutputObject>
{
public:
  
  using TimeType = TT;

protected:

  std::shared_ptr<TimeParameters<TimeType>> time_parameters_;

  TimeType start_time_;
  TimeType end_time_;
  TimeType output_every_;
  mutable size_t output_no_;
  
public:
  
  TimedOutputFile(const std::string& name,
		  const std::shared_ptr<TimeParameters<TimeType>> tparams)
    : OutputFile<OutputObject>(name),
      time_parameters_(tparams)
  {
    const Config& conf =
      GlobalConfig::instance().output_file_configuration(name);

    const TimeParser<TimeType>& parser = time_parameters_->parser();

    try {
      start_time_ = parser.parse(conf.get<std::string>("start time"));
    } catch (bpt::ptree_bad_path e) {
      start_time_ = time_parameters_->start_time();
    }

    try {
      end_time_ = parser.parse(conf.get<std::string>("end time"));
    } catch (bpt::ptree_bad_path e) {
      end_time_ = time_parameters_->end_time();
    }

    try {
      output_every_ = parser.parse(conf.get<std::string>("every"));
    } catch (bpt::ptree_bad_path e) {
      output_every_ = conf.get<TimeType>("every seconds");
    }
    output_no_ = 0;
  }

  virtual ~TimedOutputFile(void) {}

  virtual void timed_output(const std::vector<OutputObject*> oo_ptrs,
			    const TimeType& time_now) const = 0;

};

template<typename TT,
	 typename Field>
class FieldOutputFile : public TimedOutputFile<TT, Field>
{
public:

  using TimeType = TT;
  using FieldType = Field;
  using FieldOutputFunctionType = FieldOutputFunction<Field>;

  FieldOutputFile(const std::string& name,
		  const std::shared_ptr<TimeParameters<TimeType>> tparams)
    : TimedOutputFile<TT,Field>(name, tparams)
  {}
  
  virtual ~FieldOutputFile(void) {}

  virtual void timed_output(const std::vector<FieldType*> field_ptrs,
			    const TimeType& time_now) const
  {
    if (this->enabled() and
	time_now >= this->start_time_ + this->output_every_ * this->output_no_) {
      stdfs::path fn = this->output_filename_.native() + "_" +
	std::to_string(time_now) + ".csv";
      std::cout << "Writing output file " << std::quoted(this->name())
		<< " to " << std::quoted(fn.native()) << std::endl;
      if (stdfs::create_directories(fn.parent_path())) {
	std::cout << "Created output directory "
		  << std::quoted(fn.parent_path().native()) << std::endl;
      }

      auto output_func = std::make_shared<FieldOutputFunctionType>(field_ptrs);
      this->output_no_ += 1;
      return this->output(output_func, fn);
    }
  }

};

#endif
