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
  
  FieldType& h(void) { return h_; }
  FieldType& u(void) { return u_; }
  FieldType& v(void) { return v_; }

  const FieldType& h(void) const { return h_; }
  const FieldType& u(void) const { return u_; }
  const FieldType& v(void) const { return v_; }

  template<typename OutputFieldType>
  OutputFieldType* get_output_field_ptr(const std::string& name)
  {
    return nullptr;
  }

  template<>
  FieldType* get_output_field_ptr<FieldType>(const std::string& name)
  {
    if (name == "h") {
      return &h_;
    } else if (name == "u") {
      return &u_;
    } else if (name == "v") {
      return &v_;
    }
    return nullptr;
  }
  
  void calculate_spatial_derivatives(SaintVenantState<ValueType,MeshType>& dUdx,
				     SaintVenantState<ValueType,MeshType>& dUdy);

  ValueType max_control_number(const double& timestep);

};

#endif
