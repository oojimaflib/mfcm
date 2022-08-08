/***********************************************************************
 * mfcm SpatialDerivative/SpatialDerivative.hpp
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

#ifndef mfcm_SpatialDerivative_SpatialDerivative_hpp
#define mfcm_SpatialDerivative_SpatialDerivative_hpp

#include "Field.hpp"
#include "MeshSelection.hpp"

enum class SpatialDerivativeAxis {
  X,
  Y
};

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
class SpatialDerivativeOperator
{
public:
  
  using ValueType = T;
  using MeshType = Mesh;
  static const MeshComponent FieldMappingType = FieldMapping;
  using Op = OperatorFn;
  
  using FieldType = Field<ValueType,MeshType,FieldMappingType>;

  using SelectionType = MeshSelection<MeshType,
				      FieldMappingType>;

  template<SpatialDerivativeAxis Axis>
  static void apply(const FieldType& s, FieldType& d);

  template<SpatialDerivativeAxis Axis>
  static void apply(const FieldType& s, FieldType& d,
		    const SelectionType& selection);
  
  constexpr ValueType operator()(const ValueType& l,
				 const double& dxl,
				 const ValueType& c,
				 const double& dxr,
				 const ValueType& r) const
  {
    return Op()(l,dxl,c,dxr,r);
  }
  
};

template<typename SDO,
	 SpatialDerivativeAxis Axis>
class SpatialDerivativeOperationKernel
{
public:

  using FieldType = typename SDO::FieldType;
  static const MeshComponent FieldMappingType = FieldType::FieldMappingType;
  using SelectionType = typename SDO::SelectionType;

  using SourceAccessor =
    typename FieldType::template Accessor<sycl::access::mode::read>;
  using DestAccessor =
    typename FieldType::template Accessor<sycl::access::mode::write>;
  using DestSelectionAccessor =
    typename SelectionType::Accessor;

private:

  SourceAccessor s_ro_;
  DestAccessor d_wo_;
  DestSelectionAccessor d_sel_ro_;

public:

  SpatialDerivativeOperationKernel(sycl::handler& cgh,
				   const FieldType& source,
				   const FieldType& dest,
				   const SelectionType& selection)
    : s_ro_(source),
      d_wo_(dest),
      d_sel_ro_(selection)
  {
    s_ro_.bind(cgh);
    d_wo_.bind(cgh);
    d_sel_ro_.bind(cgh);
  }

  void operator()(sycl::item<1> item) const
  {
    size_t i = d_sel_ro_(item.get_linear_id());
    const auto& mesh_ro = s_ro_.mesh();
    if constexpr (Axis == SpatialDerivativeAxis::X) {
      auto [il, dxl] { mesh_ro.template get_object_west<FieldMappingType>(i) };
      auto [ir, dxr] { mesh_ro.template get_object_east<FieldMappingType>(i) };
      d_wo_.data()[i] = SDO()(s_ro_.data()[il], dxl,
			      s_ro_.data()[i], dxr,
			      s_ro_.data()[ir]);			      
    } else {
      auto [il, dxl] { mesh_ro.template get_object_north<FieldMappingType>(i) };
      auto [ir, dxr] { mesh_ro.template get_object_south<FieldMappingType>(i) };
      d_wo_.data()[i] = SDO()(s_ro_.data()[il], dxl,
			      s_ro_.data()[i], dxr,
			      s_ro_.data()[ir]);
    }

  }
  
};

#endif
