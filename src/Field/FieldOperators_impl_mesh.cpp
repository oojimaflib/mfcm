/***********************************************************************
 * mfcm Field/FieldOperators_impl_mesh.cpp
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

template class CastFieldOperator<float, float, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<float, float, MeshType, MeshComponent::Face>;
template class CastFieldOperator<float, float, MeshType, MeshComponent::Vertex>;
template class CastFieldOperator<float, double, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<float, double, MeshType, MeshComponent::Face>;
template class CastFieldOperator<float, double, MeshType, MeshComponent::Vertex>;
template class CastFieldOperator<float, int32_t, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<float, int32_t, MeshType, MeshComponent::Face>;
template class CastFieldOperator<float, int32_t, MeshType, MeshComponent::Vertex>;
template class CastFieldOperator<float, uint32_t, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<float, uint32_t, MeshType, MeshComponent::Face>;
template class CastFieldOperator<float, uint32_t, MeshType, MeshComponent::Vertex>;

template class CastFieldOperator<double, float, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<double, float, MeshType, MeshComponent::Face>;
template class CastFieldOperator<double, float, MeshType, MeshComponent::Vertex>;
template class CastFieldOperator<double, double, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<double, double, MeshType, MeshComponent::Face>;
template class CastFieldOperator<double, double, MeshType, MeshComponent::Vertex>;
template class CastFieldOperator<double, int32_t, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<double, int32_t, MeshType, MeshComponent::Face>;
template class CastFieldOperator<double, int32_t, MeshType, MeshComponent::Vertex>;
template class CastFieldOperator<double, uint32_t, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<double, uint32_t, MeshType, MeshComponent::Face>;
template class CastFieldOperator<double, uint32_t, MeshType, MeshComponent::Vertex>;

template class CastFieldOperator<int32_t, float, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<int32_t, float, MeshType, MeshComponent::Face>;
template class CastFieldOperator<int32_t, float, MeshType, MeshComponent::Vertex>;
template class CastFieldOperator<int32_t, double, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<int32_t, double, MeshType, MeshComponent::Face>;
template class CastFieldOperator<int32_t, double, MeshType, MeshComponent::Vertex>;
template class CastFieldOperator<int32_t, int32_t, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<int32_t, int32_t, MeshType, MeshComponent::Face>;
template class CastFieldOperator<int32_t, int32_t, MeshType, MeshComponent::Vertex>;
template class CastFieldOperator<int32_t, uint32_t, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<int32_t, uint32_t, MeshType, MeshComponent::Face>;
template class CastFieldOperator<int32_t, uint32_t, MeshType, MeshComponent::Vertex>;

template class CastFieldOperator<uint32_t, float, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<uint32_t, float, MeshType, MeshComponent::Face>;
template class CastFieldOperator<uint32_t, float, MeshType, MeshComponent::Vertex>;
template class CastFieldOperator<uint32_t, double, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<uint32_t, double, MeshType, MeshComponent::Face>;
template class CastFieldOperator<uint32_t, double, MeshType, MeshComponent::Vertex>;
template class CastFieldOperator<uint32_t, int32_t, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<uint32_t, int32_t, MeshType, MeshComponent::Face>;
template class CastFieldOperator<uint32_t, int32_t, MeshType, MeshComponent::Vertex>;
template class CastFieldOperator<uint32_t, uint32_t, MeshType, MeshComponent::Cell>;
template class CastFieldOperator<uint32_t, uint32_t, MeshType, MeshComponent::Face>;
template class CastFieldOperator<uint32_t, uint32_t, MeshType, MeshComponent::Vertex>;


template class UnaryFieldOperator<float, MeshType, MeshComponent::Cell, std::negate<float>>;
template class UnaryFieldOperator<float, MeshType, MeshComponent::Face, std::negate<float>>;
template class UnaryFieldOperator<float, MeshType, MeshComponent::Vertex, std::negate<float>>;

template class UnaryFieldOperator<double, MeshType, MeshComponent::Cell, std::negate<double>>;
template class UnaryFieldOperator<double, MeshType, MeshComponent::Face, std::negate<double>>;
template class UnaryFieldOperator<double, MeshType, MeshComponent::Vertex, std::negate<double>>;

template class UnaryFieldOperator<int32_t, MeshType, MeshComponent::Cell, std::negate<int32_t>>;
template class UnaryFieldOperator<int32_t, MeshType, MeshComponent::Face, std::negate<int32_t>>;
template class UnaryFieldOperator<int32_t, MeshType, MeshComponent::Vertex, std::negate<int32_t>>;

template class UnaryFieldOperator<uint32_t, MeshType, MeshComponent::Cell, std::negate<uint32_t>>;
template class UnaryFieldOperator<uint32_t, MeshType, MeshComponent::Face, std::negate<uint32_t>>;
template class UnaryFieldOperator<uint32_t, MeshType, MeshComponent::Vertex, std::negate<uint32_t>>;

#define FNO std::plus
#include "BinaryFieldOperator_impl.hpp"
#undef FNO

#define FNO std::minus
#include "BinaryFieldOperator_impl.hpp"
#undef FNO

#define FNO std::multiplies
#include "BinaryFieldOperator_impl.hpp"
#undef FNO

#define FNO std::divides
#include "BinaryFieldOperator_impl.hpp"
#undef FNO
