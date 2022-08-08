/***********************************************************************
 * mfcm Field/BinaryFieldOperator_impl.hpp
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

template class BinaryFieldCompoundAssignmentOperator<float, MeshType, MeshComponent::Cell, FNO<float>>;
template class BinaryFieldCompoundAssignmentOperator<float, MeshType, MeshComponent::Face, FNO<float>>;
template class BinaryFieldCompoundAssignmentOperator<float, MeshType, MeshComponent::Vertex, FNO<float>>;

template class BinaryFieldCompoundAssignmentOperator<double, MeshType, MeshComponent::Cell, FNO<double>>;
template class BinaryFieldCompoundAssignmentOperator<double, MeshType, MeshComponent::Face, FNO<double>>;
template class BinaryFieldCompoundAssignmentOperator<double, MeshType, MeshComponent::Vertex, FNO<double>>;

template class BinaryFieldCompoundAssignmentOperator<int32_t, MeshType, MeshComponent::Cell, FNO<int32_t>>;
template class BinaryFieldCompoundAssignmentOperator<int32_t, MeshType, MeshComponent::Face, FNO<int32_t>>;
template class BinaryFieldCompoundAssignmentOperator<int32_t, MeshType, MeshComponent::Vertex, FNO<int32_t>>;

template class BinaryFieldCompoundAssignmentOperator<uint32_t, MeshType, MeshComponent::Cell, FNO<uint32_t>>;
template class BinaryFieldCompoundAssignmentOperator<uint32_t, MeshType, MeshComponent::Face, FNO<uint32_t>>;
template class BinaryFieldCompoundAssignmentOperator<uint32_t, MeshType, MeshComponent::Vertex, FNO<uint32_t>>;


template class BinaryFieldOperator<float, MeshType, MeshComponent::Cell, FNO<float>>;
template class BinaryFieldOperator<float, MeshType, MeshComponent::Face, FNO<float>>;
template class BinaryFieldOperator<float, MeshType, MeshComponent::Vertex, FNO<float>>;

template class BinaryFieldOperator<double, MeshType, MeshComponent::Cell, FNO<double>>;
template class BinaryFieldOperator<double, MeshType, MeshComponent::Face, FNO<double>>;
template class BinaryFieldOperator<double, MeshType, MeshComponent::Vertex, FNO<double>>;

template class BinaryFieldOperator<int32_t, MeshType, MeshComponent::Cell, FNO<int32_t>>;
template class BinaryFieldOperator<int32_t, MeshType, MeshComponent::Face, FNO<int32_t>>;
template class BinaryFieldOperator<int32_t, MeshType, MeshComponent::Vertex, FNO<int32_t>>;

template class BinaryFieldOperator<uint32_t, MeshType, MeshComponent::Cell, FNO<uint32_t>>;
template class BinaryFieldOperator<uint32_t, MeshType, MeshComponent::Face, FNO<uint32_t>>;
template class BinaryFieldOperator<uint32_t, MeshType, MeshComponent::Vertex, FNO<uint32_t>>;
