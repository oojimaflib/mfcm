/***********************************************************************
 * mfcm Mesh/Mesh_impl.cpp
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

#include "Cartesian2DMesh.cpp"
#include "MeshSelection.cpp"

template class MeshSelection<Cartesian2DMesh,MeshComponent::Cell>;
template class MeshSelection<Cartesian2DMesh,MeshComponent::Face>;
template class MeshSelection<Cartesian2DMesh,MeshComponent::Vertex>;

template class MeshSelectionAccessor<Cartesian2DMesh,MeshComponent::Cell>;
template class MeshSelectionAccessor<Cartesian2DMesh,MeshComponent::Face>;
template class MeshSelectionAccessor<Cartesian2DMesh,MeshComponent::Vertex>;

