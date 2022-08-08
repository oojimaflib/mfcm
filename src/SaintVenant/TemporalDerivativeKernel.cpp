/***********************************************************************
 * mfcm SaintVenant/TemporalDerivativeKernel.cpp
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

#include "TemporalDerivativeKernel.hpp"

template<typename T,
	 typename Mesh>
SaintVenantTemporalDerivativeKernel<T,Mesh>::
SaintVenantTemporalDerivativeKernel(sycl::handler& cgh,
				    const State& U,
				    const Constants& K,
				    const State& dUdx,
				    const State& dUdy,
				    const Fluxes& F,
				    State& dUdt,
				    const double& time_now,
				    const double& timestep)
  : h_(U.h(), cgh), u_(U.u(), cgh), v_(U.v(), cgh),
    zb_(K.z_bed(), cgh), dzbdx_(K.dzdx_bed(), cgh), dzbdy_(K.dzdy_bed(), cgh),
    dhdx_(dUdx.h(), cgh), dudx_(dUdx.u(), cgh), dvdx_(dUdx.v(), cgh),
    dhdy_(dUdy.h(), cgh), dudy_(dUdy.u(), cgh), dvdy_(dUdy.v(), cgh),
    hflux_(F.h(), cgh), uflux_(F.u(), cgh), vflux_(F.v(), cgh), zflux_(F.z(), cgh),
    dhdt_(dUdt.h(), cgh), dudt_(dUdt.u(), cgh), dvdt_(dUdt.v(), cgh),
    time_now_(time_now), timestep_(timestep)
{
}

template<typename T,
	 typename Mesh>
void
SaintVenantTemporalDerivativeKernel<T,Mesh>::
operator()(sycl::item<1> item) const
{
  size_t cell_c = item.get_linear_id();
    
  // Get the surrounding face IDs
  auto mesh_acc = h_.mesh();
  auto [ face_w, face_e, dx, face_s, face_n, dy ] =
    mesh_acc.get_adjacent_faces(cell_c);

  // Calculate the changes in each variable due to the h, u and v fluxes
  ValueType dhdt = (hflux_.data()[face_w] - hflux_.data()[face_e]) / dx
    + (hflux_.data()[face_s] - hflux_.data()[face_n]) / dy;
  ValueType dudt = (uflux_.data()[face_w] - uflux_.data()[face_e]) / dx
    + (uflux_.data()[face_s] - uflux_.data()[face_n]) / dy;
  ValueType dvdt = (vflux_.data()[face_w] - vflux_.data()[face_e]) / dx
    + (vflux_.data()[face_s] - vflux_.data()[face_n]) / dy;

  // Calculate the horizontal forces on the cell due to the water
  // depth slope:
  dudt += dhdx_.data()[cell_c] * ValueType(-9.81);
  dvdt += dhdy_.data()[cell_c] * ValueType(-9.81);
  
  // Calculate the cell's bed slope and apply the horizontal
  // component of the gravity reaction force. The magnitude of this
  // is limited to gh.
  ValueType dzdx = dzbdx_.data()[cell_c];
  if (sycl::fabs(dzdx) > h_.data()[cell_c] / dx) {
    dzdx = sycl::sign(dzdx) * h_.data()[cell_c] / dx;
  }
  ValueType dzdy = dzbdy_.data()[cell_c];
  if (sycl::fabs(dzdy) > h_.data()[cell_c] / dy) {
    dzdy = sycl::sign(dzdy) * h_.data()[cell_c] / dy;
  }
  ValueType dudt_bed = ValueType(-9.81) * dzdx;
  ValueType dvdt_bed = ValueType(-9.81) * dzdy;

  // Calculate the forces on the water in the cell due to vertical
  // walls at the cell faces (the zflux_ term) and add this to our
  // momentum terms d[uv]dt
  dudt += (zflux_.data()[face_w] - zflux_.data()[face_e]) / dx;
  dvdt += (zflux_.data()[face_s] - zflux_.data()[face_n]) / dy;

  dudt += dudt_bed;
  dvdt += dvdt_bed;

  // Boundary condition stuffs go here

  // Mannings/Friction stuffs go here

  dhdt_.data()[cell_c] = dhdt;
  dudt_.data()[cell_c] = dudt;
  dvdt_.data()[cell_c] = dvdt;
}
