/***********************************************************************
 * mfcm Field/Field.hpp
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

#ifndef mfcm_Field_Field_hpp
#define mfcm_Field_Field_hpp

#include "DataArray.hpp"
#include "Mesh.hpp"

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 sycl::access::mode Mode,
	 sycl::access::target Target = sycl::access::target::global_buffer>
class FieldAccessor;

/**
   Class representing a field, i.e. an array of data where each datum
   maps to some part of a mesh.
 */
template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
class Field
{
public:

  /**
     The type of data stored in the underlying array.
   */
  using ValueType = T;

  /**
     The type of mesh that the data maps to.
   */
  using MeshType = Mesh;

  using MeshAccessor = typename Mesh::Accessor;

  /**
     Enumeration type indicating what type of mesh component each
     datum is associated with.
   */
  static const MeshComponent FieldMappingType = FieldMapping;

  template<sycl::access::mode Mode,
	   sycl::access::target Target = sycl::access::target::global_buffer>
  using Accessor = FieldAccessor<ValueType, MeshType, FieldMappingType,
				 Mode, Target>;

private:

  /**
     Name of this field.
   */
  std::string name_;

  std::shared_ptr<MeshType> mesh_p_;

  DataArray<ValueType> data_;

public:

  Field(const std::shared_ptr<sycl::queue>& queue,
	const std::string& name,
	const std::shared_ptr<MeshType>& mesh_p,
	const T& init_value = T(),
	bool on_device = false);

  Field(const std::shared_ptr<sycl::queue>& queue,
	const std::string& name,
	const std::shared_ptr<MeshType>& mesh_p,
	const std::vector<T>& init_values,
	bool on_device = false);

  Field(const Field<ValueType, MeshType, FieldMappingType>& f);
  
  /**
     Construct as a copy of another field with a prefix and suffix
     added to the name.
   */
  Field(const std::string& prefix,
	const Field<ValueType, MeshType, FieldMappingType>& f,
	const std::string& suffix);

  /**
     Return a const reference to the field's name.
   */
  const std::string& name(void) const
  {
    return name_;
  }

  void rename(const std::string& name)
  {
    name_ = name;
  }

  size_t size(void) const
  {
    return data_.size();
  }

  bool is_on_device(void) const
  {
    return data_.is_on_device();
  }

  void set_value(const size_t i, const ValueType& v)
  {
    assert(not this->is_on_device());
    data_.host_vector().at(i) = v;
  }

  /**
     Return a pointer to the field's device queue.
  */
  const std::shared_ptr<sycl::queue>& queue_ptr(void) const
  {
    return data_.queue_ptr();
  }

  /**
     Return a pointer to the field's associated mesh.
   */
  const std::shared_ptr<MeshType>& mesh(void) const
  {
    return mesh_p_;
  }

  /**
     Copies the data in this field from the host to the device. If the
     data is already on the device this method does nothing.
   */
  void move_to_device(void)
  {
    mesh_p_->move_to_device();
    data_.move_to_device();
  }

  /**
     Copies the data in this field from the device to the host,
     removing it from the device.
   */
  void move_to_host(void)
  {
    mesh_p_->move_to_host();
    data_.move_to_host();
  }

  /**
     If the field is not on the device, move it to the device and
     return a reference to the field.
   */
  Field<ValueType, MeshType, FieldMappingType>& on_device(void)
  {
    this->move_to_device();
    return *this;
  }

  const DataArray<ValueType>& data(void) const
  {
    return data_;
  }
  
  Field<T,Mesh,FieldMapping>& operator=(const Field<T,Mesh,FieldMapping>& rhs);
  Field<T,Mesh,FieldMapping>& operator=(const ValueType& rhs);

  Field<T,Mesh,FieldMapping> operator-(void);
  // Field<T,Mesh,FieldMapping> operator+(const Field<T,Mesh,FieldMapping>& rhs);
  Field<T,Mesh,FieldMapping>& operator+=(const Field<T,Mesh,FieldMapping>& rhs);
  Field<T,Mesh,FieldMapping>& operator+=(const ValueType& rhs);
  friend Field<T,Mesh,FieldMapping> operator+(Field<T,Mesh,FieldMapping> lhs,
					      const Field<T,Mesh,FieldMapping>& rhs)
  {
    lhs += rhs;
    lhs.rename(std::string("(") + lhs.name() + "+" + rhs.name() + ")");
    return lhs;
  }
  friend Field<T,Mesh,FieldMapping> operator+(Field<T,Mesh,FieldMapping> lhs,
					      const ValueType& rhs)
  {
    lhs += rhs;
    lhs.rename(std::string("(") + lhs.name() + "+" + std::to_string(rhs) + ")");
    return lhs;
  }
  Field<T,Mesh,FieldMapping>& operator-=(const Field<T,Mesh,FieldMapping>& rhs);
  Field<T,Mesh,FieldMapping>& operator-=(const ValueType& rhs);
  friend Field<T,Mesh,FieldMapping> operator-(Field<T,Mesh,FieldMapping> lhs,
					      const Field<T,Mesh,FieldMapping>& rhs)
  {
    lhs -= rhs;
    lhs.rename(std::string("(") + lhs.name() + "-" + rhs.name() + ")");
    return lhs;
  }
  friend Field<T,Mesh,FieldMapping> operator-(Field<T,Mesh,FieldMapping> lhs,
					      const ValueType& rhs)
  {
    lhs -= rhs;
    lhs.rename(std::string("(") + lhs.name() + "-" + std::to_string(rhs) + ")");
    return lhs;
  }
  Field<T,Mesh,FieldMapping>& operator*=(const Field<T,Mesh,FieldMapping>& rhs);
  Field<T,Mesh,FieldMapping>& operator*=(const ValueType& rhs);
  friend Field<T,Mesh,FieldMapping> operator*(Field<T,Mesh,FieldMapping> lhs,
					      const Field<T,Mesh,FieldMapping>& rhs)
  {
    lhs *= rhs;
    lhs.rename(std::string("(") + lhs.name() + "×" + rhs.name() + ")");
    return lhs;
  }
  friend Field<T,Mesh,FieldMapping> operator*(Field<T,Mesh,FieldMapping> lhs,
					      const ValueType& rhs)
  {
    lhs *= rhs;
    lhs.rename(std::string("(") + lhs.name() + "×" + std::to_string(rhs) + ")");
    return lhs;
  }
  Field<T,Mesh,FieldMapping>& operator/=(const Field<T,Mesh,FieldMapping>& rhs);
  Field<T,Mesh,FieldMapping>& operator/=(const ValueType& rhs);
  friend Field<T,Mesh,FieldMapping> operator/(Field<T,Mesh,FieldMapping> lhs,
					      const Field<T,Mesh,FieldMapping>& rhs)
  {
    lhs /= rhs;
    lhs.rename(std::string("(") + lhs.name() + "÷" + rhs.name() + ")");
    return lhs;
  }
  friend Field<T,Mesh,FieldMapping> operator/(Field<T,Mesh,FieldMapping> lhs,
					      const ValueType& rhs)
  {
    lhs /= rhs;
    lhs.rename(std::string("(") + lhs.name() + "÷" + std::to_string(rhs) + ")");
    return lhs;
  }

};

template<typename T,
	 typename Mesh>
using CellField = Field<T,Mesh,MeshComponent::Cell>;

template<typename T,
	 typename Mesh>
using FaceField = Field<T,Mesh,MeshComponent::Face>;

template<typename T,
	 typename Mesh>
using VertexField = Field<T,Mesh,MeshComponent::Vertex>;

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 sycl::access::mode Mode,
	 sycl::access::target Target>
class FieldAccessor
{
public:

  /**
     The type of data stored in the underlying array.
   */
  using ValueType = T;

  /**
     The type of mesh that the data maps to.
   */
  using MeshType = Mesh;

  /**
     Enumeration type indicating what type of mesh component each
     datum is associated with.
   */
  static const MeshComponent FieldMappingType = FieldMapping;

  using MeshAccessor = typename MeshType::Accessor;
  
  using DataArrayType = DataArray<T>;

  using DataAccessor = typename DataArray<T>::
    template Accessor<Mode, Target, sycl::access::placeholder::true_t>;

private:

  MeshAccessor mesh_ro_;

  DataAccessor data_acc_;

public:

  FieldAccessor(const Field<ValueType,MeshType,FieldMappingType>& f);

  FieldAccessor(const Field<ValueType,MeshType,FieldMappingType>& f,
		sycl::handler& cgh);

  void bind(sycl::handler& cgh);

  const MeshAccessor& mesh(void) const
  {
    return mesh_ro_;
  }

  const DataAccessor& data(void) const
  {
    return data_acc_;
  }
  
};

#endif
