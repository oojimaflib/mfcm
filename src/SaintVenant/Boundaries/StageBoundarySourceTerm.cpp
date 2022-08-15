/***********************************************************************
 * mfcm SaintVenant/Boundaries/StageBoundarySourceTerm.cpp
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

#include "StageBoundarySourceTerm.hpp"

template<typename T,
	 typename Mesh>
StageBoundarySourceKernel<T,Mesh>::
StageBoundarySourceKernel(sycl::handler& cgh,
			  const State& U,
			  const Constants& K,
			  const FieldType& hbdy0,
			  const FieldType& hbdy1,
			  State& dUdt,
			  const double& timestep,
			  const double& time_now,
			  const double& step_length)
  : z_bed_(K.z_bed(), cgh), h_(U.h(), cgh), u_(U.u(), cgh), v_(U.v(), cgh),
    hbdy0_(hbdy0, cgh), hbdy1_(hbdy1, cgh),
    dhdt_(dUdt.h(), cgh), dudt_(dUdt.u(), cgh), dvdt_(dUdt.v(), cgh),
    timestep_(timestep), time_now_(time_now), step_length_(step_length)
{}

template<typename T,
	 typename Mesh>
void
StageBoundarySourceKernel<T,Mesh>::
operator()(sycl::item<1> item) const
{
  size_t cell_c = item.get_linear_id();

  ValueType z = z_bed_.data()[cell_c];
  
  ValueType h0 = hbdy0_.data()[cell_c] - z;
  ValueType h1 = hbdy1_.data()[cell_c] - z;

  if (!(h0 != h0) and !(h1 != h1)) {
    ValueType h = h_.data()[cell_c];

    ValueType dhbdydt = (h1 - h0) / step_length_;
    ValueType hbdy_now = h0 + time_now_ * dhbdydt;
    ValueType hbdy_next = hbdy_now + timestep_ * dhbdydt;
    ValueType target_h = ValueType(0.5) * (hbdy_now + hbdy_next);

    if (target_h <= 0.0) {
      target_h = 0.0;
    }
    
    ValueType dhdt = (target_h - h) / timestep_;

    dhdt_.data()[cell_c] = dhdt;
  }
  
}

template<typename TT,
	 typename T,
	 typename Mesh>
std::shared_ptr<SaintVenantSourceTerm<TT,T,Mesh>>
StageBoundarySourceTerm<TT,T,Mesh>::
create_boundary(const Config& conf,
		const std::shared_ptr<MeshType>& mesh,
		bool on_device)
{
  return std::make_shared<StageBoundarySourceTerm<TT,T,Mesh>>(mesh, on_device);
}
