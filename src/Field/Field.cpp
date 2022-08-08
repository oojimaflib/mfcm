/***********************************************************************
 * mfcm Field/Field.cpp
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

#include "Field.hpp"
#include "FieldOperators.hpp"

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>::
Field(const std::shared_ptr<sycl::queue>& queue,
      const std::string& name,
      const std::shared_ptr<MeshType>& mesh_p,
      const T& init_value,
      bool on_device)
  : name_(name),
    data_(queue,
	  mesh_p->template object_count<FieldMappingType>(),
	  init_value, on_device),
    mesh_p_(mesh_p)
{
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>::
Field(const std::shared_ptr<sycl::queue>& queue,
      const std::string& name,
      const std::shared_ptr<MeshType>& mesh_p,
      const std::vector<T>& init_values,
      bool on_device)
  : name_(name),
    data_(queue, init_values, on_device),
    mesh_p_(mesh_p)
{
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>::
Field(const Field<ValueType, MeshType, FieldMappingType>& f)
  : name_(f.name_),
    mesh_p_(f.mesh_p_),
    data_(f.data_)
{}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>::
Field(const std::string& prefix,
      const Field<ValueType, MeshType, FieldMappingType>& f,
      const std::string& suffix)
  : name_(prefix + f.name_ + suffix),
    mesh_p_(f.mesh_p_),
    data_(f.data_)
{
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>&
Field<T,Mesh,FieldMapping>::operator=(const Field<T,Mesh,FieldMapping>& rhs)
{
  CastFieldOperator<T,T,Mesh,FieldMapping>::apply(rhs, *this);
  return *this;
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>&
Field<T,Mesh,FieldMapping>::operator=(const ValueType& rhs)
{
  (*this) *= 0.0;
  (*this) += rhs;
  return *this;
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>
Field<T,Mesh,FieldMapping>::operator-(void)
{
  Field<T,Mesh,FieldMapping> dest(data_.queue_ptr(),
				  std::string("-") + name_,
				  mesh_p_, T(), is_on_device());
  UnaryFieldOperator<T,Mesh,FieldMapping,std::negate<T>>::apply(*this, dest);
  return dest;
}


template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>&
Field<T,Mesh,FieldMapping>::operator+=(const Field<T,Mesh,FieldMapping>& rhs)
{
  BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,std::plus<T>>::apply(*this, rhs);
  return *this;
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>&
Field<T,Mesh,FieldMapping>::operator+=(const ValueType& rhs)
{
  BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,std::plus<T>>::apply(*this, rhs);
  return *this;
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>&
Field<T,Mesh,FieldMapping>::operator-=(const Field<T,Mesh,FieldMapping>& rhs)
{
  BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,std::minus<T>>::apply(*this, rhs);
  return *this;
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>&
Field<T,Mesh,FieldMapping>::operator-=(const ValueType& rhs)
{
  BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,std::minus<T>>::apply(*this, rhs);
  return *this;
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>&
Field<T,Mesh,FieldMapping>::operator*=(const Field<T,Mesh,FieldMapping>& rhs)
{
  BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,std::multiplies<T>>::apply(*this, rhs);
  return *this;
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>&
Field<T,Mesh,FieldMapping>::operator*=(const ValueType& rhs)
{
  BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,std::multiplies<T>>::apply(*this, rhs);
  return *this;
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>&
Field<T,Mesh,FieldMapping>::operator/=(const Field<T,Mesh,FieldMapping>& rhs)
{
  BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,std::divides<T>>::apply(*this, rhs);
  return *this;
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
Field<T,Mesh,FieldMapping>&
Field<T,Mesh,FieldMapping>::operator/=(const ValueType& rhs)
{
  BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,std::divides<T>>::apply(*this, rhs);
  return *this;
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 sycl::access::mode Mode,
	 sycl::access::target Target>
FieldAccessor<T,Mesh,FieldMapping,Mode,Target>::
FieldAccessor(const Field<ValueType,MeshType,FieldMappingType>& f)
  : mesh_ro_(MeshAccessor(*(f.mesh()))),
    data_acc_(f.data().template get_placeholder_accessor<Mode,Target>())
{}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 sycl::access::mode Mode,
	 sycl::access::target Target>
FieldAccessor<T,Mesh,FieldMapping,Mode,Target>::
FieldAccessor(const Field<ValueType,MeshType,FieldMappingType>& f,
	      sycl::handler& cgh)
  : mesh_ro_(MeshAccessor(*(f.mesh()))),
    data_acc_(f.data().template get_placeholder_accessor<Mode,Target>())
{
  mesh_ro_.bind(cgh);
  cgh.require(data_acc_);
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 sycl::access::mode Mode,
	 sycl::access::target Target>
void
FieldAccessor<T,Mesh,FieldMapping,Mode,Target>::bind(sycl::handler& cgh)
{
  mesh_ro_.bind(cgh);
  cgh.require(data_acc_);
}
