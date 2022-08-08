/***********************************************************************
 * mfcm Field/FieldVector_impl_mesh_Value.cpp
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

template class FieldVector<Field<ValueType,MeshType,MeshComponent::Cell>, NFields>;
template class FieldVector<Field<ValueType,MeshType,MeshComponent::Face>, NFields>;
template class FieldVector<Field<ValueType,MeshType,MeshComponent::Vertex>, NFields>;

template class FieldVectorAccessor<Field<ValueType,MeshType,MeshComponent::Cell>, NFields, sycl::access::mode::read>;
template class FieldVectorAccessor<Field<ValueType,MeshType,MeshComponent::Cell>, NFields, sycl::access::mode::write>;
template class FieldVectorAccessor<Field<ValueType,MeshType,MeshComponent::Cell>, NFields, sycl::access::mode::read_write>;

template class FieldVectorAccessor<Field<ValueType,MeshType,MeshComponent::Face>, NFields, sycl::access::mode::read>;
template class FieldVectorAccessor<Field<ValueType,MeshType,MeshComponent::Face>, NFields, sycl::access::mode::write>;
template class FieldVectorAccessor<Field<ValueType,MeshType,MeshComponent::Face>, NFields, sycl::access::mode::read_write>;

template class FieldVectorAccessor<Field<ValueType,MeshType,MeshComponent::Vertex>, NFields, sycl::access::mode::read>;
template class FieldVectorAccessor<Field<ValueType,MeshType,MeshComponent::Vertex>, NFields, sycl::access::mode::write>;
template class FieldVectorAccessor<Field<ValueType,MeshType,MeshComponent::Vertex>, NFields, sycl::access::mode::read_write>;

