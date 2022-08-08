/***********************************************************************
 * mfcm Field/FieldVector.hpp
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

#ifndef mfcm_Field_FieldVector_hpp
#define mfcm_Field_FieldVector_hpp

#include "Field.hpp"

/**
   A group of related Field objects of the same type.
 */
template<typename Field, size_t N>
class FieldVector : public std::vector<Field>
{
public:

  /**
     The data type of the field objects.
   */
  using ValueType = typename Field::ValueType;
  /**
     The mesh type of the field objects.
   */
  using MeshType = typename Field::MeshType;
  /**
     The field mapping type of the field objects.
   */
  static const MeshComponent FieldMappingType = Field::FieldMappingType;
  /**
     The type of the field objects.
   */
  using FieldType = Field;

  template<sycl::access::mode Mode,
	   sycl::access::target Target = sycl::access::target::global_buffer>
  using FieldAccessor = typename FieldType::template Accessor<Mode,Target>;

  template<sycl::access::mode Mode,
	   sycl::access::target Target = sycl::access::target::global_buffer>
  using Accessor = std::array<FieldAccessor<Mode,Target>,N>;
   
private:

  /**
     Private default constructor to prevent creation of zero-length
     vectors.
   */
  FieldVector(void);

public:

  /**
     Construct the vector of fields given a vector of field names and
     a default value.
   */
  FieldVector(const std::shared_ptr<sycl::queue>& queue,
	      const std::array<std::string,N>& names,
	      const std::shared_ptr<MeshType>& mesh_p,
	      const ValueType& init_value = ValueType(),
	      bool on_device = false);

  /**
     Construct the vector of fields from an initializer list of
     fields.
   */
  FieldVector(const std::initializer_list<FieldType>& il);

  /**
     Construct the vector as a copy of another vector with a prefix
     and suffix added to each field name.
   */
  FieldVector(const std::string& prefix,
	      const FieldVector<FieldType,N>& cfv,
	      const std::string& suffix);

  /**
     Move all of the fields in the vector to the device.
   */
  void move_to_device(void);

  /**
     Copy back all of the fields in the vector from the device.
   */
  void move_to_host(void);

  /*
  template<size_t NA, sycl::access::mode Mode,
	   sycl::access::target Target = sycl::access::target::global_buffer>
  Accessor<Mode,Target> get_accessor(void);

  template<sycl::access::mode Mode,
	   sycl::access::target Target = sycl::access::target::global_buffer>
  Accessor<Mode,Target> get_accessor<2>(void)
  {
    return {
      FieldAccessor<Mode,Target>(this->at(0)),
      FieldAccessor<Mode,Target>(this->at(1))
    };
  }
  */
  
};


template<typename Field,
	 size_t N,
	 sycl::access::mode Mode,
	 sycl::access::target Target = sycl::access::target::global_buffer>
class FieldVectorAccessor
{
public:

  using ValueType = typename Field::ValueType;
  using MeshType = typename Field::MeshType;
  static const MeshComponent FieldMappingType = Field::FieldMappingType;

  using MeshAccessor = typename MeshType::Accessor;
  using DataArrayType = DataArray<ValueType>;
  using DataAccessor = typename DataArray<ValueType>::
    template Accessor<Mode,Target,sycl::access::placeholder::true_t>;

private:

  MeshAccessor mesh_ro_;

  std::array<DataAccessor,N> data_acc_;

public:
  
  FieldVectorAccessor(const FieldVector<Field,N>& fv);

  void bind(sycl::handler& cgh);

  const MeshAccessor& mesh(void) const
  {
    return mesh_ro_;
  }

  const DataAccessor& data(const size_t& i) const
  {
    return data_acc_[i];
  }
  
};


#endif
