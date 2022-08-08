/***********************************************************************
 * mfcm Field/Field_impl_mesh.cpp
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

#define ValueType float
#include "Field_impl_mesh_value.cpp"
#undef ValueType

#define ValueType double
#include "Field_impl_mesh_value.cpp"
#undef ValueType

#define ValueType int32_t
#include "Field_impl_mesh_value.cpp"
#undef ValueType

#define ValueType uint32_t
#include "Field_impl_mesh_value.cpp"
#undef ValueType
