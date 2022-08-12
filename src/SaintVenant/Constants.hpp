/***********************************************************************
 * mfcm SaintVenant/Constants.hpp
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

#ifndef mfcm_SaintVenant_Constants_hpp
#define mfcm_SaintVenant_Constants_hpp

#include "Field.hpp"
#include <memory>

template<typename T,
	 typename Mesh>
class SaintVenantConstants
{
public:

  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;

private:

  std::shared_ptr<MeshType> mesh_;

  FieldType z_bed_;
  FieldType dzdx_bed_;
  FieldType dzdy_bed_;

public:

  SaintVenantConstants(const std::shared_ptr<MeshType>& mesh,
		       bool on_device = true);

  const CellField<ValueType,MeshType>& z_bed(void) const { return z_bed_; }
  const CellField<ValueType,MeshType>& dzdx_bed(void) const { return dzdx_bed_; }
  const CellField<ValueType,MeshType>& dzdy_bed(void) const { return dzdy_bed_; }
  
};

#endif
