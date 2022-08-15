/***********************************************************************
 * mfcm SaintVenant/InfiltrationSourceTerm.cpp
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

#include "InfiltrationSourceTerm.hpp"

template<typename TT,
	 typename T,
	 typename Mesh>
InfiltrationSourceKernel<TT,T,Mesh>::
InfiltrationSourceKernel(sycl::handler& cgh,
			 const State& U,
			 const Constants& K,
			 const FieldType& i_rate,
			 FieldType& i_cap,
			 State& dUdt,
			 const TT& timestep)
  : h_(U.h(), cgh),
    i_rate_(i_rate, cgh), i_cap_(i_cap, cgh),
    dhdt_(dUdt.h(), cgh),
    timestep_(timestep)
{}

template<typename TT,
	 typename T,
	 typename Mesh>
void InfiltrationSourceKernel<TT,T,Mesh>::
operator()(sycl::item<1> item) const
{
  size_t cell_c = item.get_linear_id();

  // Get the depth of water in the cell
  ValueType h = h_.data()[cell_c];

  // Calculate how much we want to infiltrate this timestep.
  ValueType dh = i_rate_.data()[cell_c] * timestep_;

  // Cannot take more water than is in the cell
  if (dh > h) {
    dh = h;
  }

  // Cannot take more water than there is space for in the soil
  if (dh > i_cap_.data()[cell_c]) {
    dh = i_cap_.data()[cell_c];
  }

  dhdt_.data()[cell_c] -= dh / timestep_;
  i_cap_.data()[cell_c] -= dh;
}

template<typename TT,
	 typename T,
	 typename Mesh>
std::shared_ptr<SaintVenantSourceTerm<TT,T,Mesh>>
InfiltrationSourceTerm<TT,T,Mesh>::
create_source_term(const Config& conf,
		   const std::shared_ptr<MeshType>& mesh,
		   bool on_device)
{
  ValueType i_rate_val = conf.get<ValueType>("default infiltration rate", 1e-6);
  ValueType i_cap_val = conf.get<ValueType>("default infiltration capacity", 0.1);
  return std::make_shared<InfiltrationSourceTerm<TT,T,Mesh>>(mesh,
							     i_rate_val,
							     i_cap_val,
							     on_device);
}
