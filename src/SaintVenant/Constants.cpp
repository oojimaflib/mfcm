/***********************************************************************
 * mfcm SaintVenant/Constants.cpp
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

#include "Constants.hpp"
#include "FieldGenerator.hpp"
#include "../Output/CheckFile.hpp"
#include "SpatialDerivative.hpp"
#include "Minmod3.hpp"

template<typename T, typename Mesh>
SaintVenantConstants<T,Mesh>::
SaintVenantConstants(const std::shared_ptr<MeshType>& mesh,
		     bool on_device)
  : mesh_(mesh),
    z_bed_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
	   (mesh_->queue_ptr(), "z_bed", mesh_, 0.0f, on_device)()),
    dzdx_bed_(mesh_->queue_ptr(), "dzdx_bed", mesh_, 0.0f, on_device),
    dzdy_bed_(mesh_->queue_ptr(), "dzdy_bed", mesh_, 0.0f, on_device)
{
  using SpatialDerivative = SpatialDerivativeOperator<ValueType,
						      MeshType,
						      MeshComponent::Cell,
						      Minmod3<ValueType>>;
  SpatialDerivative::template apply<SpatialDerivativeAxis::X>(z_bed_, dzdx_bed_);
  SpatialDerivative::template apply<SpatialDerivativeAxis::Y>(z_bed_, dzdy_bed_);

  FieldCheckFile<FieldType> cf("constants");
  cf.output({&z_bed_, &dzdx_bed_, &dzdy_bed_});
}
