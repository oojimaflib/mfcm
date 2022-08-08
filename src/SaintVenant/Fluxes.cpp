/***********************************************************************
 * mfcm SaintVenant/Fluxes.cpp
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

#include "Fluxes.hpp"
#include "FluxKernel.hpp"

template<typename T,
	 typename Mesh>
SaintVenantFluxes<T,Mesh>::
SaintVenantFluxes(const std::shared_ptr<MeshType>& mesh,
		  const std::string& prefix,
		  const std::string& suffix,
		  bool on_device)
  : mesh_(mesh),
    h_(Field<ValueType,MeshType,MeshComponent::Face>
       (mesh_->queue_ptr(), "h_flux", mesh_, 0.0f, on_device)),
    u_(Field<ValueType,MeshType,MeshComponent::Face>
       (mesh_->queue_ptr(), "u_flux", mesh_, 0.0f, on_device)),
    v_(Field<ValueType,MeshType,MeshComponent::Face>
       (mesh_->queue_ptr(), "v_flux", mesh_, 0.0f, on_device)),
    z_(Field<ValueType,MeshType,MeshComponent::Face>
       (mesh_->queue_ptr(), "z_flux", mesh_, 0.0f, on_device))
{}

template<typename T,
	 typename Mesh>
void
SaintVenantFluxes<T,Mesh>::
update(const SaintVenantState<ValueType,MeshType>& U,
       const SaintVenantConstants<ValueType,MeshType>& constants,
       const SaintVenantState<ValueType,MeshType>& dUdx,
       const SaintVenantState<ValueType,MeshType>& dUdy)
{
  using FluxKernel = SaintVenantFluxKernel<ValueType,MeshType>;
    
  size_t nfaces = mesh_->template object_count<MeshComponent::Face>();
  mesh_->queue_ptr()->submit([&] (sycl::handler& cgh) {
    auto kernel = FluxKernel(cgh, U, constants, dUdx, dUdy,
			     h_, u_, v_, z_);
    cgh.parallel_for(sycl::range<1>(nfaces), kernel);
  });
}
