/***********************************************************************
 * mfcm SaintVenant/EnergyLossSourceTerm.cpp
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

#include "EnergyLossSourceTerm.hpp"

template<typename TT,
	 typename T,
	 typename Mesh>
EnergyLossSourceKernel<TT,T,Mesh>::
EnergyLossSourceKernel(sycl::handler& cgh,
		       const State& U,
		       const Constants& K,
		       const FieldType& fdx,
		       const FieldType& fdy,
		       State& dUdt,
		       const TT& timestep)
  : h_(U.h(), cgh), u_(U.u(), cgh), v_(U.v(), cgh),
    fdx_(fdx, cgh), fdy_(fdy, cgh),
    dudt_(dUdt.u(), cgh), dvdt_(dUdt.v(), cgh),
    timestep_(timestep)
{}

template<typename TT,
	 typename T,
	 typename Mesh>
void
EnergyLossSourceKernel<TT,T,Mesh>::
operator()(sycl::item<1> item) const
{
  size_t cell_c = item.get_linear_id();

  ValueType h = h_.data()[cell_c];
  ValueType u = u_.data()[cell_c];
  ValueType v = v_.data()[cell_c];
  
  if (h > 1e-6) {
    ValueType hyp = sycl::sqrt(u*u + v*v);
    ValueType dudt = -ValueType(0.5) * u * fdx_.data()[cell_c] * hyp;
    ValueType dvdt = -ValueType(0.5) * v * fdy_.data()[cell_c] * hyp;

    // If the change in velocity is enough to push the water backwards
    // relative to it's current velocity, cap it.
    if (sycl::fabs(dudt * timestep_) > sycl::fabs(u) and
	sycl::sign(dudt * timestep_) != sycl::sign(u)) {
      dudt = -u / timestep_;
    }
    if (sycl::fabs(dvdt * timestep_) > sycl::fabs(v) and
	sycl::sign(dvdt * timestep_) != sycl::sign(v)) {
      dvdt = -v / timestep_;
    }

    dudt_.data()[cell_c] += dudt;
    dvdt_.data()[cell_c] += dvdt;
  }
}

template<typename TT,
	 typename T,
	 typename Mesh>
std::shared_ptr<SaintVenantSourceTerm<TT,T,Mesh>>
EnergyLossSourceTerm<TT,T,Mesh>::
create_source_term(const Config& conf,
		   const std::shared_ptr<MeshType>& mesh,
		   bool on_device)
{
  ValueType fdx = conf.get<ValueType>("default fx/m", 0.0);
  ValueType fdy = conf.get<ValueType>("default fy/m", 0.0);
  return std::make_shared<EnergyLossSourceTerm<TT,T,Mesh>>(mesh,
							   fdx, fdy,
							   on_device);
}

