/***********************************************************************
 * mfcm SaintVenant/FluxKernel.cpp
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

#include "FluxKernel.hpp"

template<typename T,
	 typename Mesh>
SaintVenantFluxKernel<T,Mesh>::
SaintVenantFluxKernel(sycl::handler& cgh,
		      const State& U, const Constants& K,
		      const State& dUdx, const State& dUdy,
		      FaceField<ValueType,MeshType>& hflux,
		      FaceField<ValueType,MeshType>& uflux,
		      FaceField<ValueType,MeshType>& vflux,
		      FaceField<ValueType,MeshType>& zflux)
  : h_(U.h(), cgh), u_(U.u(), cgh), v_(U.v(), cgh),
    zb_(K.z_bed(), cgh), dzbdx_(K.dzdx_bed(), cgh), dzbdy_(K.dzdy_bed(), cgh),
    dhdx_(dUdx.h(), cgh), dudx_(dUdx.u(), cgh), dvdx_(dUdx.v(), cgh),
    dhdy_(dUdy.h(), cgh), dudy_(dUdy.u(), cgh), dvdy_(dUdy.v(), cgh),
    hflux_(hflux, cgh), uflux_(uflux, cgh),
    vflux_(vflux, cgh), zflux_(zflux, cgh)
{
}

template<typename T,
	 typename Mesh>
void
SaintVenantFluxKernel<T,Mesh>::
zero_flux(const size_t& fid) const
{
  hflux_.data()[fid] = ValueType(0.0);
  uflux_.data()[fid] = ValueType(0.0);
  vflux_.data()[fid] = ValueType(0.0);
  zflux_.data()[fid] = ValueType(0.0);    
}
  
template<typename T,
	 typename Mesh>
void
SaintVenantFluxKernel<T,Mesh>::
operator()(sycl::item<1> item) const
{
  // Get the face ID
  size_t fid = item.get_linear_id();

  // Get the surrounding cell IDs
  auto mesh_acc = h_.mesh();
  auto [ lhs_id, rhs_id, edge, dir, dx ] = mesh_acc.get_adjacent_cells(fid);

  // Direction of flow across this face
  ValueType xdir = 1.0 - ValueType(dir);
  ValueType ydir = ValueType(dir);

  // Get the cell bed levels
  ValueType zb_L = zb_.data()[lhs_id];
  ValueType zb_R = zb_.data()[rhs_id];

  // Check if either of the surrounding cells are excluded from the
  // computation ("coded out")
  if (zb_L != zb_L) {
    // LHS cell bed level is NaN ("coded out")
    lhs_id = rhs_id;
    if (edge == 1) {
      // This face is between a coded out cell and the RHS of the
      // mesh. Move on.
      zero_flux(fid);
      return;
    }
    edge = -1;
  }
  if (zb_R != zb_R) {
    // RHS cell bed level is NaN ("coded out")
    if (edge == -1) {
      // Either this face is between a coded out cell and the LHS of
      // the mesh, or it's between a coded out cell and another
      // coded out cell. Move on.
      zero_flux(fid);
      return;
    }
    rhs_id = lhs_id;
    edge = 1;
  }

  // Get surrounding water depths. Zero the depth if the cell is
  // coded out.
  ValueType h_L = h_.data()[lhs_id] * (edge < 0 ? 0 : 1);
  ValueType h_R = h_.data()[rhs_id] * (edge > 0 ? 0 : 1);

  // Get the surrounding velocities in the x direction. Zero them if
  // the cell is coded out and the face is flowing horizontally.
  ValueType u_L = u_.data()[lhs_id] * (edge < 0 && xdir == 1 ? 0 : 1);
  ValueType u_R = u_.data()[rhs_id] * (edge > 0 && xdir == 1 ? 0 : 1);

  // Get the surrounding velocities in the y direction. Zero them if
  // the cell is coded out and the face is flowing vertically.
  ValueType v_L = v_.data()[lhs_id] * (edge < 0 && ydir == 1 ? 0 : 1);
  ValueType v_R = v_.data()[rhs_id] * (edge > 0 && ydir == 1 ? 0 : 1);

  // Get the water depth slopes. Zero if the cell is coded out.
  ValueType dhdx_L = dhdx_.data()[lhs_id] * (edge < 0 ? 0 : 1);
  ValueType dhdx_R = dhdx_.data()[rhs_id] * (edge > 0 ? 0 : 1);
  ValueType dhdy_L = dhdy_.data()[lhs_id] * (edge < 0 ? 0 : 1);
  ValueType dhdy_R = dhdy_.data()[rhs_id] * (edge > 0 ? 0 : 1);

  // Get the slopes of x-dir velocity. Zero if the face is flowing
  // horizontally and the cell is coded out
  ValueType dudx_L = dudx_.data()[lhs_id] * (edge < 0 && xdir == 1 ? 0 : 1);
  ValueType dudx_R = dudx_.data()[rhs_id] * (edge > 0 && xdir == 1 ? 0 : 1);
  ValueType dudy_L = dudy_.data()[lhs_id] * (edge < 0 && xdir == 1 ? 0 : 1);
  ValueType dudy_R = dudy_.data()[rhs_id] * (edge > 0 && xdir == 1 ? 0 : 1);

  // Get the slopes of y-dir velocity. Zero if the face is flowing
  // vertically and the cell is coded out
  ValueType dvdx_L = dvdx_.data()[lhs_id] * (edge < 0 && ydir == 1 ? 0 : 1);
  ValueType dvdx_R = dvdx_.data()[rhs_id] * (edge > 0 && ydir == 1 ? 0 : 1);
  ValueType dvdy_L = dvdy_.data()[lhs_id] * (edge < 0 && ydir == 1 ? 0 : 1);
  ValueType dvdy_R = dvdy_.data()[rhs_id] * (edge > 0 && ydir == 1 ? 0 : 1);

  // Get the bed elevation slopes. Zero if the cell is coded out.
  ValueType dzbdx_L = dzbdx_.data()[lhs_id] * (edge < 0 ? 0 : 1);
  ValueType dzbdx_R = dzbdx_.data()[rhs_id] * (edge > 0 ? 0 : 1);
  ValueType dzbdy_L = dzbdy_.data()[lhs_id] * (edge < 0 ? 0 : 1);
  ValueType dzbdy_R = dzbdy_.data()[rhs_id] * (edge > 0 ? 0 : 1);

  // If one of our cells is coded out, pretend its bed level is
  // above the water level in the other cell.
  if (edge < 0) {
    zb_L = zb_R + h_R * 2.0f;
  }
  if (edge > 0) {
    zb_R = zb_L + h_L * 2.0f;
  }

  // Project estimates of each variable from the lhs cell rightward
  // to the lhs of the face
  ValueType zb_m = zb_L +
    ValueType(0.5) * dx * dzbdx_L * xdir +
    ValueType(0.5) * dx * dzbdy_L * ydir;
  ValueType h_m = h_L +
    ValueType(0.5) * dx * dhdx_L * xdir +
    ValueType(0.5) * dx * dhdy_L * ydir;
  ValueType u_m = u_L +
    ValueType(0.5) * dx * dudx_L * xdir +
    ValueType(0.5) * dx * dudy_L * ydir;
  ValueType v_m = v_L +
    ValueType(0.5) * dx * dvdx_L * xdir +
    ValueType(0.5) * dx * dvdy_L * ydir;

  // Project estimates of each variable from the rhs cell leftward
  // to the rhs of the face
  ValueType zb_p = zb_R +
    ValueType(-0.5) * dx * dzbdx_R * xdir +
    ValueType(-0.5) * dx * dzbdy_R * ydir;
  ValueType h_p = h_R +
    ValueType(-0.5) * dx * dhdx_R * xdir +
    ValueType(-0.5) * dx * dhdy_R * ydir;
  ValueType u_p = u_R +
    ValueType(-0.5) * dx * dudx_R * xdir +
    ValueType(-0.5) * dx * dudy_R * ydir;
  ValueType v_p = v_R +
    ValueType(-0.5) * dx * dvdx_R * xdir +
    ValueType(-0.5) * dx * dvdy_R * ydir;

  // Calculate the bed level of the face (the maximum of the two
  // projected bed levels)
  ValueType zb_f = sycl::fmax(zb_m, zb_p);

  // Calculate the water levels at the face
  ValueType y_m = zb_m + h_m;
  ValueType y_p = zb_p + h_p;

  // Limit the depths on each side of the face such that they are
  // never negative.
  h_m = sycl::fmax(h_m, ValueType(0.0));
  h_p = sycl::fmax(h_p, ValueType(0.0));

  // Calculate the wave speed on each side of the face
  ValueType c_m = sycl::sqrt(ValueType(9.81) * h_m);
  ValueType c_p = sycl::sqrt(ValueType(9.81) * h_p);

  ValueType branch = 5;
  // Calculate the face fluxes:
  if (y_m > zb_f and y_p > zb_f) {
    branch = 1;
    // Step is fully submerged
    ValueType spd_m = u_m * xdir + v_m * ydir;
    ValueType spd_p = u_p * xdir + v_p * ydir;

    ValueType hf_m = h_m * spd_m;
    ValueType hf_p = h_p * spd_p;
    ValueType uf_m = u_m * (ValueType(1.0 - 0.5 * xdir) * spd_m)
      + ValueType(9.81) * h_m * xdir;
    ValueType uf_p = u_p * (ValueType(1.0 - 0.5 * xdir) * spd_p)
      - ValueType(9.81) * h_p * xdir;
    ValueType vf_m = v_m * (ValueType(1.0 - 0.5 * ydir) * spd_m)
      + ValueType(9.81) * h_m * ydir;
    ValueType vf_p = v_p * (ValueType(1.0 - 0.5 * ydir) * spd_p)
      - ValueType(9.81) * h_p * ydir;

    ValueType a = sycl::fmax(sycl::fabs(spd_p + sycl::sign(spd_p) * c_p),
			     sycl::fabs(spd_m + sycl::sign(spd_m) * c_m));
      
    hflux_.data()[fid] = ValueType(0.5) * (hf_p + hf_m) -
      ValueType(0.5) * a * (h_p - h_m);
    uflux_.data()[fid] = ValueType(0.5) * (uf_p + uf_m) -
      ValueType(0.5) * a * (u_p - u_m);
    vflux_.data()[fid] = ValueType(0.5) * (vf_p + vf_m) -
      ValueType(0.5) * a * (v_p - v_m);
    zflux_.data()[fid] = (zb_m - zb_p) * ValueType(9.81);
  } else if (y_m <= zb_f and y_p <= zb_f) {
    branch = 2;
    // Both water levels below the face, but we could have some
    // water in the lower cell
    hflux_.data()[fid] = ValueType(0.0);
    if (zb_p > zb_m) {
      ValueType uf_m = ValueType(9.81) * h_m * xdir;
      ValueType vf_m = ValueType(9.81) * h_m * ydir;
      uflux_.data()[fid] = ValueType(0.5) * uf_m;
      vflux_.data()[fid] = ValueType(0.5) * vf_m;
      zflux_.data()[fid] = -h_m * ValueType(0.5) * ValueType(9.81);
    } else {
      ValueType uf_p = ValueType(9.81) * h_p * xdir;
      ValueType vf_p = ValueType(9.81) * h_p * ydir;
      uflux_.data()[fid] = ValueType(-0.5) * uf_p;
      vflux_.data()[fid] = ValueType(-0.5) * vf_p;
      zflux_.data()[fid] = h_p * ValueType(0.5) * ValueType(9.81);
    }
  } else if (y_m > zb_f) {
    branch = 3;
    // Water level above the face on the LHS but not on the right.
    ValueType spd_m = u_m * xdir + v_m * ydir;
    ValueType hf_m = h_m * spd_m;
    ValueType uf_m = u_m * (ValueType(1.0 - 0.5 * xdir) * spd_m)
      + ValueType(9.81) * h_m * xdir;
    ValueType vf_m = v_m * (ValueType(1.0 - 0.5 * ydir) * spd_m)
      + ValueType(9.81) * h_m * ydir;
    ValueType a = sycl::fabs(spd_m + sycl::sign(spd_m) * c_m);
    hflux_.data()[fid] = ValueType(0.5) * hf_m -
      ValueType(0.5) * a * (-h_m);
    uflux_.data()[fid] = ValueType(0.5) * uf_m -
      ValueType(0.5) * a * (-u_m);
    vflux_.data()[fid] = ValueType(0.5) * vf_m -
      ValueType(0.5) * a * (-v_m);
    zflux_.data()[fid] = h_p / dx;
  } else {
    branch = 4;
    // Water level above the face on the RHS but not on the left
    ValueType spd_p = u_p * xdir + v_p * ydir;
    ValueType hf_p = h_p * spd_p;
    ValueType uf_p = u_p * (ValueType(1.0 - 0.5 * xdir) * spd_p)
      + ValueType(9.81) * h_p * xdir;
    ValueType vf_p = v_p * (ValueType(1.0 - 0.5 * ydir) * spd_p)
      + ValueType(9.81) * h_p * ydir;      
    ValueType a = sycl::fabs(spd_p + sycl::sign(spd_p) * c_p);
    hflux_.data()[fid] = ValueType(0.5) * hf_p -
      ValueType(0.5) * a * (h_p);
    uflux_.data()[fid] = ValueType(0.5) * uf_p -
      ValueType(0.5) * a * (u_p);
    vflux_.data()[fid] = ValueType(0.5) * vf_p -
      ValueType(0.5) * a * (v_p);
    zflux_.data()[fid] = h_m / dx;
  }
    
  // zflux_.data()[fid] = ValueType(branch);
}
