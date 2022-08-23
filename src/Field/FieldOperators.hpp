/***********************************************************************
 * mfcm Field/FieldOperators.hpp
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

#ifndef mfcm_Field_FieldOperators_hpp
#define mfcm_Field_FieldOperators_hpp

#include "Field.hpp"
#include "MeshSelection.hpp"

template<typename T,
	 typename SourceMesh,
	 typename DestMesh,
	 MeshComponent SourceFieldMapping,
	 MeshComponent DestFieldMapping>
class MapFieldOperator
{
public:

  using ValueType = T;
  using SourceMeshType = SourceMesh;
  using DestMeshType = DestMesh;
  static const MeshComponent SourceFieldMappingType = SourceFieldMapping;
  static const MeshComponent DestFieldMappingType = DestFieldMapping;

  using SourceFieldType = Field<ValueType,
				SourceMeshType,
				SourceFieldMappingType>;
  using DestFieldType = Field<ValueType,
			      DestMeshType,
			      DestFieldMappingType>;

  using DestSelectionType = MeshSelection<DestMeshType,
					  DestFieldMappingType>;
  
  static void apply(const SourceFieldType& s, DestFieldType& d);
  static void apply(const SourceFieldType& s, DestFieldType& d,
		    const DestSelectionType& selection);

  constexpr ValueType operator()(const ValueType& arg) const
  {
    return arg;
  }
  
};

template<typename MFO>
class MapFieldOperationKernel
{
public:

  using SourceFieldType = typename MFO::SourceFieldType;
  using DestFieldType = typename MFO::DestFieldType;
  using DestSelectionType = typename MFO::DestSelectionType;

  using SourceAccessor =
    typename SourceFieldType::template Accessor<sycl::access::mode::read>;
  using DestAccessor =
    typename DestFieldType::template Accessor<sycl::access::mode::write>;
  using DestSelectionAccessor =
    typename DestSelectionType::Accessor;
  
private:

  SourceAccessor s_ro_;
  DestAccessor d_wo_;

  DestSelectionAccessor d_sel_ro_;

public:

  MapFieldOperationKernel(sycl::handler& cgh,
			  const SourceFieldType& source,
			  const DestFieldType& dest,
			  const DestSelectionType& dest_selection)
    : s_ro_(source),
      d_wo_(dest),
      d_sel_ro_(dest_selection)
  {
    s_ro_.bind(cgh);
    d_wo_.bind(cgh);
    d_sel_ro_.bind(cgh);
  }

  void operator()(sycl::item<1> item) const
  {
    constexpr MeshComponent SourceFieldMappingType =
      SourceFieldType::FieldMappingType;
    constexpr MeshComponent DestFieldMappingType =
      DestFieldType::FieldMappingType;
    // Get the location of the item
    size_t i = d_sel_ro_(item.get_linear_id());
    size_t n_obj = s_ro_.mesh().template object_count<SourceFieldMappingType>();
    if (i < n_obj) {
      std::array<double,2> location =
	d_wo_.mesh().template get_object_location<DestFieldMappingType>(i);
      size_t j = s_ro_.mesh().template get_nearest_object_index<SourceFieldMappingType>(location);
      if (j < n_obj) {
	d_wo_.data()[item] = MFO()(s_ro_.data()[j]);
      }
    }
  }
  
};

template<typename Tsrc,
	 typename Tdest,
	 typename Mesh,
	 MeshComponent FieldMapping>
class CastFieldOperator
{
public:

  using SourceValueType = Tsrc;
  using DestValueType = Tdest;
  using MeshType = Mesh;
  static const MeshComponent FieldMappingType = FieldMapping;

  using SourceFieldType = Field<SourceValueType,MeshType,FieldMappingType>;
  using DestFieldType = Field<DestValueType,MeshType,FieldMappingType>;

  using DestSelectionType = MeshSelection<MeshType,
					  FieldMappingType>;
  
  static void apply(const SourceFieldType& s, DestFieldType& d);
  static void apply(const SourceFieldType& s, DestFieldType& d,
		    const DestSelectionType& selection);

  static void apply(const SourceValueType& s, DestFieldType& d);
  static void apply(const SourceValueType& s, DestFieldType& d,
		    const DestSelectionType& selection);

  constexpr DestValueType operator()(const SourceValueType& arg) const
  {
    return Tdest(arg);
  }
  
};

template<typename CFO>
class CastFieldOperationKernel
{
public:

  using SourceFieldType = typename CFO::SourceFieldType;
  using DestFieldType = typename CFO::DestFieldType;
  using DestSelectionType = typename CFO::DestSelectionType;

  using SourceAccessor =
    typename SourceFieldType::template Accessor<sycl::access::mode::read>;
  using DestAccessor =
    typename DestFieldType::template Accessor<sycl::access::mode::write>;

  using DestSelectionAccessor =
    typename DestSelectionType::Accessor;

private:

  SourceAccessor s_ro_;
  DestAccessor d_wo_;

  DestSelectionAccessor d_sel_ro_;

public:

  CastFieldOperationKernel(sycl::handler& cgh,
			   const SourceFieldType& source,
			   const DestFieldType& dest,
			   const DestSelectionType& dest_selection)
    : s_ro_(source),
      d_wo_(dest),
      d_sel_ro_(dest_selection)
  {
    s_ro_.bind(cgh);
    d_wo_.bind(cgh);
    d_sel_ro_.bind(cgh);
  }

  void operator()(sycl::item<1> item) const
  {
    constexpr MeshComponent SourceFieldMappingType =
      SourceFieldType::FieldMappingType;
    size_t i = d_sel_ro_(item.get_linear_id());
    if (i < s_ro_.mesh().template object_count<SourceFieldMappingType>()) {
      d_wo_.data()[i] = CFO()(s_ro_.data()[i]);
    }
  }
  
};

template<typename CFO>
class CastConstantOperationKernel
{
public:

  using SourceValueType = typename CFO::SourceValueType;
  using DestFieldType = typename CFO::DestFieldType;
  using DestSelectionType = typename CFO::DestSelectionType;

  using DestAccessor =
    typename DestFieldType::template Accessor<sycl::access::mode::write>;

  using DestSelectionAccessor =
    typename DestSelectionType::Accessor;

private:

  SourceValueType s_value_;
  DestAccessor d_wo_;

  DestSelectionAccessor d_sel_ro_;

public:

  CastConstantOperationKernel(sycl::handler& cgh,
			      const SourceValueType& source,
			      const DestFieldType& dest,
			      const DestSelectionType& dest_selection)
    : s_value_(source),
      d_wo_(dest),
      d_sel_ro_(dest_selection)
  {
    d_wo_.bind(cgh);
    d_sel_ro_.bind(cgh);
  }

  void operator()(sycl::item<1> item) const
  {
    constexpr MeshComponent FieldMappingType =
      DestFieldType::FieldMappingType;
    size_t i = d_sel_ro_(item.get_linear_id());
    if (i < d_wo_.mesh().template object_count<FieldMappingType>()) {
      d_wo_.data()[i] = CFO()(s_value_);
    }
  }
  
};

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
class UnaryFieldOperator
{
public:
  
  using ValueType = T;
  using MeshType = Mesh;
  static const MeshComponent FieldMappingType = FieldMapping;
  using Op = OperatorFn;
  
  using FieldType = Field<ValueType,MeshType,FieldMappingType>;

  using SelectionType = MeshSelection<MeshType,
				      FieldMappingType>;
  
  static void apply(const FieldType& s, FieldType& d);
  static void apply(const FieldType& s, FieldType& d,
		    const SelectionType& selection);
  
  constexpr ValueType operator()(const ValueType& arg) const
  {
    return Op()(arg);
  }
  
};

template<typename UFO>
class UnaryFieldOperationKernel
{
public:

  using FieldType = typename UFO::FieldType;
  using SelectionType = typename UFO::SelectionType;

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

  UnaryFieldOperationKernel(sycl::handler& cgh,
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
    constexpr MeshComponent SourceFieldMappingType =
      FieldType::FieldMappingType;
    size_t i = d_sel_ro_(item.get_linear_id());
    if (i < s_ro_.mesh().template object_count<SourceFieldMappingType>()) {
      d_wo_.data()[i] = UFO()(s_ro_.data()[i]);
    }
  }
  
};

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
class BinaryFieldCompoundAssignmentOperator
{
public:

  using ValueType = T;
  using MeshType = Mesh;
  static const MeshComponent FieldMappingType = FieldMapping;
  using Op = OperatorFn;

  using FieldType = Field<ValueType,MeshType,FieldMappingType>;
  using SelectionType = MeshSelection<MeshType,FieldMappingType>;

  static void apply(FieldType& lhs, const FieldType& rhs);
  static void apply(FieldType& lhs, const FieldType& rhs, const SelectionType& selection);

  static void apply(FieldType& lhs, const ValueType& rhs);
  static void apply(FieldType& lhs, const ValueType& rhs, const SelectionType& selection);

  constexpr ValueType operator()(ValueType& lhs,
				 const ValueType& rhs) const
  {
    return Op()(lhs, rhs);
  }
  
};


template<typename BCAO>
class BinaryFieldCompoundAssignmentOperationKernel
{
public:

  using FieldType = typename BCAO::FieldType;
  using SelectionType = typename BCAO::SelectionType;

  using LHSAccessor =
    typename FieldType::template Accessor<sycl::access::mode::read_write>;
  using RHSAccessor =
    typename FieldType::template Accessor<sycl::access::mode::read>;
  using SelectionAccessor = typename SelectionType::Accessor;

private:

  LHSAccessor lhs_;
  RHSAccessor rhs_;
  SelectionAccessor sel_;

public:

  BinaryFieldCompoundAssignmentOperationKernel(sycl::handler& cgh,
					       FieldType& lhs,
					       const FieldType& rhs,
					       const SelectionType& sel)
    : lhs_(lhs), rhs_(rhs), sel_(sel)
  {
    lhs_.bind(cgh);
    rhs_.bind(cgh);
    sel_.bind(cgh);
  }

  void operator()(sycl::item<1> item) const
  {
    constexpr MeshComponent SourceFieldMappingType =
      FieldType::FieldMappingType;
    size_t i = sel_(item.get_linear_id());
    if (i < lhs_.mesh().template object_count<SourceFieldMappingType>()) {
      lhs_.data()[i] = BCAO()(lhs_.data()[i], rhs_.data()[i]);
    }
  }
};

template<typename BCAO>
class BinaryFieldConstantCompoundAssignmentOperationKernel
{
public:

  using FieldType = typename BCAO::FieldType;
  using SelectionType = typename BCAO::SelectionType;
  using ValueType = typename FieldType::ValueType;

  using LHSAccessor =
    typename FieldType::template Accessor<sycl::access::mode::read_write>;
  using SelectionAccessor = typename SelectionType::Accessor;

private:

  LHSAccessor lhs_;
  ValueType rhs_;
  SelectionAccessor sel_;

public:

  BinaryFieldConstantCompoundAssignmentOperationKernel(sycl::handler& cgh,
						       FieldType& lhs,
						       const ValueType& rhs,
						       const SelectionType& sel)
    : lhs_(lhs), rhs_(rhs), sel_(sel)
  {
    lhs_.bind(cgh);
    sel_.bind(cgh);
  }

  void operator()(sycl::item<1> item) const
  {
    constexpr MeshComponent SourceFieldMappingType =
      FieldType::FieldMappingType;
    size_t i = sel_(item.get_linear_id());
    if (i < lhs_.mesh().template object_count<SourceFieldMappingType>()) {
      lhs_.data()[i] = BCAO()(lhs_.data()[i], rhs_);
    }
  }
};

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
class BinaryFieldOperator
{
public:
  
  using ValueType = T;
  using MeshType = Mesh;
  static const MeshComponent FieldMappingType = FieldMapping;
  using Op = OperatorFn;
  
  using FieldType = Field<ValueType,MeshType,FieldMappingType>;
  using SelectionType = MeshSelection<MeshType,
				      FieldMappingType>;

  static void apply(const FieldType& lhs, const FieldType& rhs, FieldType& d);
  static void apply(const FieldType& lhs, const FieldType& rhs, FieldType& d,
		    const SelectionType& selection);
  
  constexpr ValueType operator()(const ValueType& lhs,
				 const ValueType& rhs) const
  {
    return Op()(lhs, rhs);
  }
  
};

template<typename BFO>
class BinaryFieldOperationKernel
{
public:

  using FieldType = typename BFO::FieldType;
  using SelectionType = typename BFO::SelectionType;

  using SourceAccessor =
    typename FieldType::template Accessor<sycl::access::mode::read>;
  using DestAccessor =
    typename FieldType::template Accessor<sycl::access::mode::write>;
  using DestSelectionAccessor =
    typename SelectionType::Accessor;

private:

  SourceAccessor lhs_ro_;
  SourceAccessor rhs_ro_;
  DestAccessor d_wo_;
  DestSelectionAccessor d_sel_ro_;


public:

  BinaryFieldOperationKernel(sycl::handler& cgh,
			     const FieldType& lhs,
			     const FieldType& rhs,
			     const FieldType& dest,
			     const SelectionType& selection)
    : lhs_ro_(lhs),
      rhs_ro_(rhs),
      d_wo_(dest),
      d_sel_ro_(selection)
  {
    lhs_ro_.bind(cgh);
    rhs_ro_.bind(cgh);
    d_wo_.bind(cgh);
    d_sel_ro_.bind(cgh);
  }

  void operator()(sycl::item<1> item) const
  {
    constexpr MeshComponent SourceFieldMappingType =
      FieldType::FieldMappingType;
    size_t i = d_sel_ro_(item.get_linear_id());
    if (i < lhs_ro_.mesh().template object_count<SourceFieldMappingType>()) {
      d_wo_.data()[i] = BFO()(lhs_ro_.data()[i], rhs_ro_.data()[i]);
    }
  }
  
};


#endif
