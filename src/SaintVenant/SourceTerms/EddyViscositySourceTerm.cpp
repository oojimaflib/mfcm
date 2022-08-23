/***********************************************************************
 * mfcm SaintVenant/EddyViscositySourceTerm.cpp
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

#include "EddyViscositySourceTerm.hpp"

template<typename TT,
	 typename T,
	 typename Mesh>
EddyViscositySourceKernel<TT,T,Mesh>::
EddyViscositySourceKernel(sycl::handler& cgh,
			  const State& U,
			  const Constants& K,
			  const FieldType& mu,
			  const FieldType& d2udx2,
			  const FieldType& d2udy2,
			  const FieldType& d2vdx2,
			  const FieldType& d2vdy2,
			  State& dUdt,
			  const TT& timestep)
  : h_(U.h(), cgh), u_(U.u(), cgh), v_(U.v(), cgh),
    mu_(mu, cgh),
    d2udx2_(d2udx2, cgh), d2udy2_(d2udy2, cgh),
    d2vdx2_(d2vdx2, cgh), d2vdy2_(d2vdy2, cgh),
    dudt_(dUdt.u(), cgh), dvdt_(dUdt.v(), cgh),
    timestep_(timestep)
{}

template<typename TT,
	 typename T,
	 typename Mesh>
void
EddyViscositySourceKernel<TT,T,Mesh>::
operator()(sycl::item<1> item) const
{
  size_t cell_c = item.get_linear_id();

  ValueType h = h_.data()[cell_c];
  ValueType u = u_.data()[cell_c];
  ValueType v = v_.data()[cell_c];
  
  if (h > 1e-6) {
    ValueType dudt = -mu_.data()[cell_c] *
      (d2udx2_.data()[cell_c] + d2udy2_.data()[cell_c]);
    ValueType dvdt = -mu_.data()[cell_c] *
      (d2vdx2_.data()[cell_c] + d2vdy2_.data()[cell_c]);

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

template<typename T,
	 typename Mesh>
SmagorinskyCoefficientKernel<T,Mesh>::
SmagorinskyCoefficientKernel(sycl::handler& cgh,
			     const ValueType& mu_const,
			     const ValueType& mu_smag,
			     const FieldType& dudx,
			     const FieldType& dudy,
			     const FieldType& dvdx,
			     const FieldType& dvdy,
			     FieldType& mu)
  : mu_const_(mu_const), mu_smag_(mu_smag),
    dudx_(dudx, cgh), dudy_(dudy, cgh),
    dvdx_(dvdx, cgh), dvdy_(dvdy, cgh),
    mu_(mu, cgh)
{}

template<typename T,
	 typename Mesh>
void
SmagorinskyCoefficientKernel<T,Mesh>::
operator()(sycl::item<1> item) const
{
  size_t cell_c = item.get_linear_id();

  ValueType cell_area = dudx_.mesh().cell_area(cell_c);
  
  mu_.data()[cell_c] = mu_const_ + mu_smag_ * cell_area *
    sycl::sqrt(dudx_.data()[cell_c] * dudx_.data()[cell_c] +
	       dvdy_.data()[cell_c] * dvdy_.data()[cell_c] +
	       0.5 * sycl::pow(sycl::fabs(dudy_.data()[cell_c]) +
			       sycl::fabs(dvdx_.data()[cell_c]), 2));
}

template<typename TT,
	 typename T,
	 typename Mesh>
std::shared_ptr<SaintVenantSourceTerm<TT,T,Mesh>>
EddyViscositySourceTerm<TT,T,Mesh>::
create_source_term(const Config& conf,
		   const std::shared_ptr<MeshType>& mesh,
		   bool on_device)
{
  std::string visc_type = conf.get<std::string>("viscosity type", "constant");
  if (visc_type == "constant") {
    ValueType mu = conf.get<ValueType>("default mu", 1.0);
    return std::make_shared<EddyViscositySourceTerm<TT,T,Mesh>>(mesh, mu,
								on_device);
  } else if (visc_type == "smagorinsky") {
    ValueType mu_const = conf.get<ValueType>("constant coefficient", 0.05);
    ValueType mu_smag = conf.get<ValueType>("dynamic coefficient", 0.5);
    return std::make_shared<EddyViscositySourceTerm<TT,T,Mesh>>(mesh,
								mu_const, mu_smag,
								on_device);
  } else {
    std::cerr << "ERROR: Unknown viscosity type: " << std::quoted(visc_type)
	      << std::endl;
    throw std::runtime_error("Unknown viscosity type.");
  }
}

