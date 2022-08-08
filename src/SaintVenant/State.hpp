/***********************************************************************
 * mfcm SaintVenant/State.hpp
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

#ifndef mfcm_SaintVenant_State_hpp
#define mfcm_SaintVenant_State_hpp

#include "Field.hpp"

template<typename T,
	 typename Mesh>
class SaintVenantState
{
public:

  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;
  
private:

  // The mesh
  std::shared_ptr<MeshType> mesh_;
  
  // Vectors at time now
  FieldType h_;
  FieldType u_;
  FieldType v_;

public:

  SaintVenantState(const std::shared_ptr<MeshType>& mesh,
		   const std::string& prefix = "",
		   const std::string& suffix = "",
		   bool on_device = true);

  SaintVenantState(const SaintVenantState<ValueType,MeshType>& state,
		   const std::string& prefix = "",
		   const std::string& suffix = "");

  SaintVenantState& operator=(const SaintVenantState& state);

  
  SaintVenantState& operator+=(const SaintVenantState& rhs);
  SaintVenantState& operator-=(const SaintVenantState& rhs);
  SaintVenantState& operator*=(const SaintVenantState& rhs);
  SaintVenantState& operator/=(const SaintVenantState& rhs);

  SaintVenantState& operator+=(const ValueType& rhs);
  SaintVenantState& operator-=(const ValueType& rhs);
  SaintVenantState& operator*=(const ValueType& rhs);
  SaintVenantState& operator/=(const ValueType& rhs);

  friend SaintVenantState operator+(SaintVenantState lhs,
				    const SaintVenantState& rhs)
  {
    lhs += rhs;
    return lhs;
  }
  friend SaintVenantState operator-(SaintVenantState lhs,
				    const SaintVenantState& rhs)
  {
    lhs -= rhs;
    return lhs;
  }
  friend SaintVenantState operator*(SaintVenantState lhs,
				    const SaintVenantState& rhs)
  {
    lhs *= rhs;
    return lhs;
  }
  friend SaintVenantState operator/(SaintVenantState lhs,
				    const SaintVenantState& rhs)
  {
    lhs /= rhs;
    return lhs;
  }
  
  friend SaintVenantState operator+(SaintVenantState lhs,
				    const ValueType& rhs)
  {
    lhs += rhs;
    return lhs;
  }
  friend SaintVenantState operator-(SaintVenantState lhs,
				    const ValueType& rhs)
  {
    lhs -= rhs;
    return lhs;
  }
  friend SaintVenantState operator*(SaintVenantState lhs,
				    const ValueType& rhs)
  {
    lhs *= rhs;
    return lhs;
  }
  friend SaintVenantState operator/(SaintVenantState lhs,
				    const ValueType& rhs)
  {
    lhs /= rhs;
    return lhs;
  }
  
  CellField<ValueType,MeshType>& h(void) { return h_; }
  CellField<ValueType,MeshType>& u(void) { return u_; }
  CellField<ValueType,MeshType>& v(void) { return v_; }

  const CellField<ValueType,MeshType>& h(void) const { return h_; }
  const CellField<ValueType,MeshType>& u(void) const { return u_; }
  const CellField<ValueType,MeshType>& v(void) const { return v_; }

  void calculate_spatial_derivatives(SaintVenantState<ValueType,MeshType>& dUdx,
				     SaintVenantState<ValueType,MeshType>& dUdy);

  ValueType max_control_number(const double& timestep);

};

#endif
