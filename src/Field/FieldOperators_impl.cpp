/***********************************************************************
 * mfcm Field/FieldOperators_impl.cpp
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

#if __INCLUDE_LEVEL__
#error "This file should not be included."
#endif

#include "FieldOperators.cpp"
#include "Cartesian2DMesh.hpp"
#include <functional>

template class MapFieldOperator<float, Cartesian2DMesh, Cartesian2DMesh, MeshComponent::Cell, MeshComponent::Cell>;
template class MapFieldOperator<double, Cartesian2DMesh, Cartesian2DMesh, MeshComponent::Cell, MeshComponent::Cell>;
template class MapFieldOperator<int32_t, Cartesian2DMesh, Cartesian2DMesh, MeshComponent::Cell, MeshComponent::Cell>;
template class MapFieldOperator<uint32_t, Cartesian2DMesh, Cartesian2DMesh, MeshComponent::Cell, MeshComponent::Cell>;

#define MeshType Cartesian2DMesh
#include "FieldOperators_impl_mesh.cpp"
#undef MeshType

