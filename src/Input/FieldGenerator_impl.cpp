/***********************************************************************
 * mfcm Input/FieldGenerator_impl.cpp
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

#include "FieldGenerator.cpp"
#include "Cartesian2DMesh.hpp"

template class FieldGenerator<float,Cartesian2DMesh,MeshComponent::Cell>;

template class FieldGenerator<double,Cartesian2DMesh,MeshComponent::Cell>;

template class FieldGenerator<int32_t,Cartesian2DMesh,MeshComponent::Cell>;

template class FieldGenerator<uint32_t,Cartesian2DMesh,MeshComponent::Cell>;

