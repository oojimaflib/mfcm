/***********************************************************************
 * mfcm Field/Field_impl_mesh_value.cpp
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

template class Field<ValueType,MeshType,MeshComponent::Cell>;
template class Field<ValueType,MeshType,MeshComponent::Face>;
template class Field<ValueType,MeshType,MeshComponent::Vertex>;

template class FieldAccessor<ValueType,MeshType,MeshComponent::Cell,sycl::access::mode::read>;
template class FieldAccessor<ValueType,MeshType,MeshComponent::Cell,sycl::access::mode::write>;
template class FieldAccessor<ValueType,MeshType,MeshComponent::Cell,sycl::access::mode::read_write>;

template class FieldAccessor<ValueType,MeshType,MeshComponent::Face,sycl::access::mode::read>;
template class FieldAccessor<ValueType,MeshType,MeshComponent::Face,sycl::access::mode::write>;
template class FieldAccessor<ValueType,MeshType,MeshComponent::Face,sycl::access::mode::read_write>;

template class FieldAccessor<ValueType,MeshType,MeshComponent::Vertex,sycl::access::mode::read>;
template class FieldAccessor<ValueType,MeshType,MeshComponent::Vertex,sycl::access::mode::write>;
template class FieldAccessor<ValueType,MeshType,MeshComponent::Vertex,sycl::access::mode::read_write>;

#define NFields 2
#include "FieldVector_impl_mesh_value.cpp"
#undef NFields

#define NFields 3
#include "FieldVector_impl_mesh_value.cpp"
#undef NFields

#define NFields 4
#include "FieldVector_impl_mesh_value.cpp"
#undef NFields

#define NFields 5
#include "FieldVector_impl_mesh_value.cpp"
#undef NFields

#define NFields 6
#include "FieldVector_impl_mesh_value.cpp"
#undef NFields
