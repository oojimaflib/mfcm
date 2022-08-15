/***********************************************************************
 * mfcm SaintVenant/ManningRoughnessSourceTerm.cpp
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

#include "ManningRoughnessSourceTerm.hpp"

template<typename TT,
	 typename T,
	 typename Mesh>
ManningRoughnessSourceKernel<TT,T,Mesh>::
ManningRoughnessSourceKernel(sycl::handler& cgh,
			     const State& U,
			     const Constants& K,
			     const FieldType& n_shallow,
			     const FieldType& n_deep,
			     const FieldType& d_shallow,
			     const FieldType& d_deep,
			     FieldType& nh,
			     FieldType& Sf,
			     State& dUdt,
			     const TT& timestep)
  : h_(U.h(), cgh), u_(U.u(), cgh), v_(U.v(), cgh),
    n_shallow_(n_shallow, cgh), n_deep_(n_deep, cgh),
    d_shallow_(d_shallow, cgh), d_deep_(d_deep, cgh),
    nh_(nh, cgh), Sf_(Sf, cgh),
    dudt_(dUdt.u(), cgh), dvdt_(dUdt.v(), cgh),
    timestep_(timestep)
{}

template<typename TT,
	 typename T,
	 typename Mesh>
void
ManningRoughnessSourceKernel<TT,T,Mesh>::
operator()(sycl::item<1> item) const
{
  size_t cell_c = item.get_linear_id();

  ValueType h = h_.data()[cell_c];
  ValueType u = u_.data()[cell_c];
  ValueType v = v_.data()[cell_c];
  
  ValueType manning_n = sycl::mix(n_shallow_.data()[cell_c],
				  n_deep_.data()[cell_c],
				  sycl::smoothstep(d_shallow_.data()[cell_c],
						   d_deep_.data()[cell_c], h));
  nh_.data()[cell_c] = manning_n;
  if (h > 1e-6) {
    ValueType inv_h = h / (h*h + 1e-3);
    ValueType Sf = manning_n * manning_n * sycl::sqrt(u*u + v*v)
      * sycl::pow(inv_h, ValueType(4.0)/ValueType(3.0));

    Sf_.data()[cell_c] = Sf;

    ValueType dudt = -ValueType(9.81) * Sf * u;
    ValueType dvdt = -ValueType(9.81) * Sf * v;

    // If the change in velocity due to friction is enough to push the
    // water backwards relative to it's current velocity, cap it.
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
  } else {
    Sf_.data()[cell_c] = 0.0;
  }
}

template<typename TT,
	 typename T,
	 typename Mesh>
std::shared_ptr<SaintVenantSourceTerm<TT,T,Mesh>>
ManningRoughnessSourceTerm<TT,T,Mesh>::
create_source_term(const Config& conf,
		   const std::shared_ptr<MeshType>& mesh,
		   bool on_device)
{
  ValueType n_shallow_val = conf.get<ValueType>("default shallow n", 0.03);
  ValueType n_deep_val = conf.get<ValueType>("default deep n", 0.3);
  ValueType d_shallow_val = conf.get<ValueType>("default shallow depth", 0.1);
  ValueType d_deep_val = conf.get<ValueType>("default deep depth", 0.3);
  return std::make_shared<ManningRoughnessSourceTerm<TT,T,Mesh>>(mesh,
								 n_shallow_val,
								 n_deep_val,
								 d_shallow_val,
								 d_deep_val,
								 on_device);
}

