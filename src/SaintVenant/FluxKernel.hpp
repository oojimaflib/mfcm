/***********************************************************************
 * mfcm SaintVenant/FluxKernel.hpp
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

#ifndef mfcm_SaintVenant_FluxKernel_hpp
#define mfcm_SaintVenant_FluxKernel_hpp

template<typename T,
	 typename Mesh>
class SaintVenantFluxKernel
{
public:

  using ValueType = T;
  using MeshType = Mesh;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;
  
  using ReadAccessor = typename CellField<ValueType,MeshType>::
    template Accessor<sycl::access::mode::read,
		      sycl::access::target::global_buffer>;

  using WriteAccessor = typename FaceField<ValueType,MeshType>::
    template Accessor<sycl::access::mode::write,
		      sycl::access::target::global_buffer>;

private:

  ReadAccessor h_;
  ReadAccessor u_;
  ReadAccessor v_;

  ReadAccessor zb_;
  ReadAccessor dzbdx_;
  ReadAccessor dzbdy_;

  ReadAccessor dhdx_;
  ReadAccessor dudx_;
  ReadAccessor dvdx_;

  ReadAccessor dhdy_;
  ReadAccessor dudy_;
  ReadAccessor dvdy_;

  WriteAccessor hflux_;
  WriteAccessor uflux_;
  WriteAccessor vflux_;
  WriteAccessor zflux_;

public:

  SaintVenantFluxKernel(sycl::handler& cgh,
			const State& U, const Constants& K,
			const State& dUdx, const State& dUdy,
			FaceField<ValueType,MeshType>& hflux,
			FaceField<ValueType,MeshType>& uflux,
			FaceField<ValueType,MeshType>& vflux,
			FaceField<ValueType,MeshType>& zflux);

  void zero_flux(const size_t& fid) const;

  void operator()(sycl::item<1> item) const;
  
};

#endif
