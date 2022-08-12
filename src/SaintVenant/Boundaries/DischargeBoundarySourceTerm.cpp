/***********************************************************************
 * mfcm SaintVenant/Boundaries/DischargeBoundarySourceTerm.cpp
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

#include "DischargeBoundarySourceTerm.hpp"

template<typename T,
	 typename Mesh>
DischargeBoundarySourceKernel<T,Mesh>::
DischargeBoundarySourceKernel(sycl::handler& cgh,
			      const State& U,
			      const Constants& K,
			      const FieldType& qbdy0,
			      const FieldType& qbdy1,
			      State& dUdt,
			      const double& timestep,
			      const double& time_now,
			      const double& step_length)
  : h_(U.h(), cgh), u_(U.u(), cgh), v_(U.v(), cgh),
    qbdy0_(qbdy0, cgh), qbdy1_(qbdy1, cgh),
    dhdt_(dUdt.h(), cgh), dudt_(dUdt.u(), cgh), dvdt_(dUdt.v(), cgh),
    timestep_(timestep), time_now_(time_now), step_length_(step_length)
{}

template<typename T,
	 typename Mesh>
void
DischargeBoundarySourceKernel<T,Mesh>::
operator()(sycl::item<1> item) const
{
  size_t cell_c = item.get_linear_id();

  ValueType q0 = qbdy0_.data()[cell_c];
  ValueType q1 = qbdy1_.data()[cell_c];

  if (q0 != 0.0 or q1 != 0.0) {
    // ValueType cell_area = h_.mesh().cell_area(cell_c);
    
    ValueType h = h_.data()[cell_c];
    ValueType u = u_.data()[cell_c];
    ValueType v = v_.data()[cell_c];
    
    ValueType dqdt = (q1 - q0) / step_length_;
    ValueType qnow = q0 + time_now_ * dqdt;
    ValueType qnext = qnow + timestep_ * dqdt;
    ValueType dhdt = ValueType(0.5) * (qnow + qnext); // / cell_area;

    if (h - dhdt * timestep_ <= 0.0) {
      dhdt = -h / timestep_;
    }

    dhdt_.data()[cell_c] += dhdt;
  }
  
}
