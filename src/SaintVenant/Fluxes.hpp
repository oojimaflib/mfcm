/***********************************************************************
 * mfcm SaintVenant/Fluxes.hpp
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

#ifndef mfcm_SaintVenant_Fluxes_hpp
#define mfcm_SaintVenant_Fluxes_hpp

#define MFCM_FLUX_BRANCH_OUTPUT 1

#include "State.hpp"
#include "Constants.hpp"

template<typename T,
	 typename Mesh>
class SaintVenantFluxes
{
public:

  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = FaceField<ValueType,MeshType>;

private:

  // The mesh
  std::shared_ptr<MeshType> mesh_;
  
  // Vectors at time now
  FieldType h_;
  FieldType u_;
  FieldType v_;
  FieldType z_;

#if MFCM_FLUX_BRANCH_OUTPUT
  FieldType branch_;
#endif
  
public:

  SaintVenantFluxes(const std::shared_ptr<MeshType>& mesh,
		    const std::string& prefix = "",
		    const std::string& suffix = "",
		    bool on_device = true);

  FieldType& h(void) { return h_; }
  FieldType& u(void) { return u_; }
  FieldType& v(void) { return v_; }
  FieldType& z(void) { return z_; }

  const FieldType& h(void) const { return h_; }
  const FieldType& u(void) const { return u_; }
  const FieldType& v(void) const { return v_; }
  const FieldType& z(void) const { return z_; }

  void update(const SaintVenantState<ValueType,MeshType>& U,
	      const SaintVenantConstants<ValueType,MeshType>& constants,
	      const SaintVenantState<ValueType,MeshType>& dUdx,
	      const SaintVenantState<ValueType,MeshType>& dUdy);

  template<typename OutputFieldType>
  OutputFieldType* get_output_field_ptr(const std::string& name)
  {
    return nullptr;
  }

  template<>
  FieldType* get_output_field_ptr<FieldType>(const std::string& name)
  {
    if (name == "flux-h") {
      return &h_;
    } else if (name == "flux-u") {
      return &u_;
    } else if (name == "flux-v") {
      return &v_;
    } else if (name == "flux-z") {
      return &z_;
#if MFCM_FLUX_BRANCH_OUTPUT
    } else if (name == "flux-branch") {
      return &branch_;
#endif
    }
    return nullptr;
  }
  
};

#endif
