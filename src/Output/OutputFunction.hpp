/***********************************************************************
 * mfcm Output/OutputFunction.hpp
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

#ifndef mfcm_Output_OutputFunction_hpp
#define mfcm_Output_OutputFunction_hpp

#include "Field/Field.hpp"
#include "Input/TimeSeries.hpp"
#include <fstream>
#include <iomanip>

class OutputFunction
{
public:

  OutputFunction(void) {}

  virtual ~OutputFunction(void) {}

  virtual size_t ncols(void) const = 0;

  virtual size_t nrows(void) const = 0;

  virtual bool rows_have_location(void) const
  {
    return false;
  }
  
  virtual std::array<double,2> location(const size_t& row) const
  {
    return { 0.0, 0.0 };
  }
  
  virtual std::string at_str(const size_t& col,
			     const size_t& row) = 0;

  virtual std::string column_name(const size_t& col) const = 0;

  virtual size_t binary_value_size(void) const = 0;
  
  virtual void set_binary_value_ptr(const size_t& col,
				    const size_t& row,
				    char* vptr) = 0;
};

template<typename T>
class TypedOutputFunction : public OutputFunction
{
public:

  using ValueType = T;

  TypedOutputFunction(void)
    : OutputFunction()
  {}

  virtual ~TypedOutputFunction(void) {}

  virtual size_t ncols(void) const = 0;

  virtual size_t nrows(void) const = 0;

  virtual bool rows_have_location(void) const
  {
    return false;
  }
  
  virtual std::array<double,2> location(const size_t& row) const
  {
    return { 0.0, 0.0 };
  }
  
  virtual std::string at_str(const size_t& col,
			     const size_t& row)
  {
    return std::to_string(this->at(col, row));
  }

  virtual ValueType at(const size_t& col,
		       const size_t& row) = 0;

  virtual std::string column_name(const size_t& col) const = 0;

  virtual size_t binary_value_size(void) const
  {
    return sizeof(ValueType);
  }
  
  virtual void set_binary_value_ptr(const size_t& col,
				    const size_t& row,
				    char* pointer)
  {
    ValueType* vptr = (ValueType*)pointer;
    *vptr = this->at(col, row);
  }
  
};


template<typename Field>
class FieldOutputFunction
  : public TypedOutputFunction<typename Field::ValueType>
{
public:

  using FieldType = Field;

  using ValueType = typename FieldType::ValueType;
  using MeshType = typename FieldType::MeshType;
  static const MeshComponent FieldMappingType = FieldType::FieldMappingType;

private:

  std::vector<FieldType*> field_ptrs_;
  std::vector<bool> field_locations_;

public:
  
  FieldOutputFunction(FieldType* field_ptr)
    : TypedOutputFunction<ValueType>(),
      field_ptrs_()
  {
    field_ptrs_.push_back(field_ptr);
    for (auto&& fptr : field_ptrs_) {
      bool on_device = fptr->is_on_device();
      field_locations_.push_back(on_device);
      if (on_device) fptr->move_to_host();
    }
  }

  FieldOutputFunction(const std::vector<FieldType*> field_ptrs)
    : TypedOutputFunction<ValueType>(),
      field_ptrs_(field_ptrs)
  {
    for (auto&& fptr : field_ptrs_) {
      bool on_device = fptr->is_on_device();
      field_locations_.push_back(on_device);
      if (on_device) fptr->move_to_host();
    }
  }

  virtual ~FieldOutputFunction(void)
  {
    for (size_t i = 0; i < field_ptrs_.size(); ++i) {
      if (field_locations_.at(i)) {
	field_ptrs_.at(i)->move_to_device();
      }
    }
  }

  virtual size_t ncols(void) const
  {
    return field_ptrs_.size();
  }

  virtual size_t nrows(void) const
  {
    if (ncols() > 0) {
      return field_ptrs_.at(0)->size();
    } else {
      return 0;
    }
  }

  virtual bool rows_have_location(void) const
  {
    return true;
  }
  
  virtual std::array<double,2> location(const size_t& row) const
  {
    return field_ptrs_.at(0)->mesh()->template get_object_location<FieldMappingType>(row);
  }
  
  virtual ValueType at(const size_t& col,
		       const size_t& row)
  {
    return field_ptrs_.at(col)->data().host_vector().at(row);
  }

  virtual std::string column_name(const size_t& col) const
  {
    return field_ptrs_.at(col)->name();
  }

  virtual const FieldType& field(const size_t& col) const
  {
    return *(field_ptrs_.at(col));
  }
  
};

template<typename Mesh,
	 MeshComponent FieldMappingType>
class MeshOutputFunction
  : public TypedOutputFunction<double>
{
public:

  using MeshType = Mesh;

private:

  std::shared_ptr<MeshType> mesh_ptr_;
  bool mesh_on_device_;

public:

  MeshOutputFunction(const std::shared_ptr<MeshType>& mesh_ptr)
    : TypedOutputFunction<double>(),
      mesh_ptr_(mesh_ptr)
  {
    mesh_on_device_ = mesh_ptr_->is_on_device();
    if (mesh_on_device_) {
      mesh_ptr_->move_to_host();
    }
  }

  ~MeshOutputFunction(void)
  {
    if (mesh_on_device_) {
      mesh_ptr_->move_to_device();
    }
  }

  virtual size_t ncols(void) const
  {
    return 2;
  }

  virtual size_t nrows(void) const
  {
    return mesh_ptr_->template object_count<FieldMappingType>();
  }

  virtual bool rows_have_location(void) const
  {
    return false;
  }

  virtual std::array<double,2> location(const size_t& row) const
  {
    return mesh_ptr_->template get_object_location<FieldMappingType>(row);
  }

  virtual double at(const size_t& col, const size_t& row)
  {
    return this->location(row)[col];
  }

  virtual std::string column_name(const size_t& col) const
  {
    const std::vector<std::string> column_names_ { "X", "Y" };
    return column_names_.at(col);
  }

};

template<typename MeshSelection>
class MeshSelectionOutputFunction : public TypedOutputFunction<size_t>
{
public:

  using SelectionType = MeshSelection;

  using ValueType = size_t;
  using MeshType = typename SelectionType::MeshType;
  static const MeshComponent FieldMappingType = SelectionType::FieldMappingType;

private:

  std::shared_ptr<MeshType> mesh_;
  std::vector<size_t> id_list_;
  bool is_global_;

  const std::string colname_ = "ID";

public:
  
  MeshSelectionOutputFunction(const SelectionType& sel)
    : TypedOutputFunction<ValueType>(),
      mesh_(sel.mesh()),
      id_list_(sel.get_id_list()),
      is_global_(sel.is_global())
  {
  }

  virtual ~MeshSelectionOutputFunction(void)
  {
  }

  virtual size_t ncols(void) const
  {
    return 1;
  }

  virtual size_t nrows(void) const
  {
    if (is_global_) {
      return mesh_->template object_count<FieldMappingType>();
    }
    return id_list_.size() - 1;
  }

  virtual ValueType at(const size_t& col,
		       const size_t& row)
  {
    return id_list_.at(row);
  }

  virtual std::string column_name(const size_t& col) const
  {
    return colname_;
  }
  
};

template<typename TimeSeries>
class TimeSeriesOutputFunction
  : public TypedOutputFunction<typename TimeSeries::ValueType>
{
public:

  using TimeSeriesType = TimeSeries;

  using TimeType = typename TimeSeries::TimeType;
  using ValueType = typename TimeSeries::ValueType;

private:

  std::string ts_name_;
  std::shared_ptr<TimeSeriesType> ts_;
  
  const std::string col0name_ = "ID";
  
public:
  
  TimeSeriesOutputFunction(const std::string& ts_name)
    : TypedOutputFunction<ValueType>(),
      ts_name_(ts_name),
      ts_(TimeSeriesDatabase<TimeType,ValueType>::instance().get_time_series_ptr(ts_name))
  {
  }

  TimeSeriesOutputFunction(const std::string& ts_name,
			   std::shared_ptr<TimeSeriesType>& ts_ptr)
    : TypedOutputFunction<ValueType>(),
      ts_name_(ts_name),
      ts_(ts_ptr)
  {}

  virtual ~TimeSeriesOutputFunction(void)
  {
  }

  virtual size_t ncols(void) const
  {
    return 2;
  }

  virtual size_t nrows(void) const
  {
    return ts_->get_time_array().host_vector().size();
  }

  virtual ValueType at(const size_t& col,
		       const size_t& row)
  {
    switch (col) {
    case 0:
      return ts_->get_time_array().host_vector().at(row);
    case 1:
      return ts_->get_value_array().host_vector().at(row);
    default:
      throw std::logic_error("Column out of range in time series output function.");
    };
  }

  virtual std::string column_name(const size_t& col) const
  {
    switch (col) {
    case 0:
      return col0name_;
    case 1:
      return ts_name_;
    default:
      throw std::logic_error("Column out of range in time series output function.");
    };
  }
  
};

#endif
