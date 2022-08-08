/***********************************************************************
 * mfcm SaintVenant/TemporalDerivativeKernel.hpp
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

#ifndef mfcm_SaintVenant_TemporalDerivativeKernel_hpp
#define mfcm_SaintVenant_TemporalDerivativeKernel_hpp

template<typename T,
	 typename Mesh>
class SaintVenantTemporalDerivativeKernel
{
public:

  using ValueType = T;
  using MeshType = Mesh;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;
  using Fluxes = SaintVenantFluxes<ValueType,MeshType>;
  
  using CellReadAccessor = typename CellField<ValueType,MeshType>::
    template Accessor<sycl::access::mode::read,
		      sycl::access::target::global_buffer>;

  using FaceReadAccessor = typename FaceField<ValueType,MeshType>::
    template Accessor<sycl::access::mode::read,
		      sycl::access::target::global_buffer>;

  using WriteAccessor = typename CellField<ValueType,MeshType>::
    template Accessor<sycl::access::mode::write,
		      sycl::access::target::global_buffer>;

private:

  CellReadAccessor h_;
  CellReadAccessor u_;
  CellReadAccessor v_;

  CellReadAccessor zb_;
  CellReadAccessor dzbdx_;
  CellReadAccessor dzbdy_;

  FaceReadAccessor hflux_;
  FaceReadAccessor uflux_;
  FaceReadAccessor vflux_;
  FaceReadAccessor zflux_;

  CellReadAccessor dhdx_;
  CellReadAccessor dudx_;
  CellReadAccessor dvdx_;

  CellReadAccessor dhdy_;
  CellReadAccessor dudy_;
  CellReadAccessor dvdy_;
  
  WriteAccessor dhdt_;
  WriteAccessor dudt_;
  WriteAccessor dvdt_;

  double time_now_;
  double timestep_;

public:
  
  SaintVenantTemporalDerivativeKernel(sycl::handler& cgh,
				      const State& U,
				      const Constants& K,
				      const State& dUdx,
				      const State& dUdy,
				      const Fluxes& F,
				      State& dUdt,
				      const double& time_now,
				      const double& timestep);

  void operator()(sycl::item<1> item) const;
  
};

#endif
