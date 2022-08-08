/***********************************************************************
 * mfcm Field/FieldOperators.cpp
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

#include "FieldOperators.hpp"

template<typename T,
	 typename SourceMesh,
	 typename DestMesh,
	 MeshComponent SourceFieldMapping,
	 MeshComponent DestFieldMapping>
void
MapFieldOperator<T,SourceMesh,DestMesh,SourceFieldMapping,DestFieldMapping>::
apply(const SourceFieldType& s, DestFieldType& d)
{
  MeshSelection<DestMesh,DestFieldMapping> ms(d.mesh());
  return MapFieldOperator<T,
			  SourceMesh,
			  DestMesh,
			  SourceFieldMapping,
			  DestFieldMapping>::apply(s, d, ms);
}

      
template<typename T,
	 typename SourceMesh,
	 typename DestMesh,
	 MeshComponent SourceFieldMapping,
	 MeshComponent DestFieldMapping>
void
MapFieldOperator<T,SourceMesh,DestMesh,SourceFieldMapping,DestFieldMapping>::
apply(const SourceFieldType& s, DestFieldType& d,
      const DestSelectionType& selection)
{
  if (s.is_on_device()) {
    if (not d.is_on_device()) {
      throw std::logic_error("Input and output fields must be on the same "
			     "device in map operator");
    }

    auto& op_queue = s.data().queue();
    if (op_queue != d.data().queue()) {
      throw std::logic_error("Input and output fields must be in the same "
			     "context in map operator");
    }

    op_queue.submit([&](sycl::handler& cgh)
    {
      auto op_kernel =
	MapFieldOperationKernel<MapFieldOperator<ValueType,
						 SourceMeshType,
						 DestMeshType,
						 SourceFieldMappingType,
						 DestFieldMappingType>>
	(cgh, s, d, selection);
      cgh.parallel_for(sycl::range<1>(selection.size()), op_kernel);
    });
  } else {
    if (d.is_on_device()) {
      throw std::logic_error("Input and output fields must be on the same "
			     "device in unary operator");
    }

    throw std::logic_error("Operators not currently supported on the host.");
  }
}

template<typename Tsrc,
	 typename Tdest,
	 typename Mesh,
	 MeshComponent FieldMapping>
void
CastFieldOperator<Tsrc,Tdest,Mesh,FieldMapping>::
apply(const SourceFieldType& s, DestFieldType& d)
{
  MeshSelection<Mesh,FieldMapping> ms(d.mesh());
  return CastFieldOperator<Tsrc,
			   Tdest,
			   Mesh,
			   FieldMapping>::apply(s, d, ms);
}


template<typename Tsrc,
	 typename Tdest,
	 typename Mesh,
	 MeshComponent FieldMapping>
void
CastFieldOperator<Tsrc,Tdest,Mesh,FieldMapping>::
apply(const SourceFieldType& s, DestFieldType& d,
      const DestSelectionType& selection)
{
  if (s.size() != d.size()) {
    throw std::logic_error("Output field size must match input field "
			   "size in unary operator");
  }
  if (s.is_on_device()) {
    if (not d.is_on_device()) {
      throw std::logic_error("Input and output fields must be on the same "
			     "device in unary operator");
    }

    auto& op_queue = s.data().queue();
    if (op_queue != d.data().queue()) {
      throw std::logic_error("Input and output fields must be in the same "
			     "context in unary operator");
    }

    op_queue.submit([&](sycl::handler& cgh)
    {
      auto op_kernel = CastFieldOperationKernel<CastFieldOperator<SourceValueType,DestValueType,MeshType,FieldMappingType>>(cgh, s, d, selection);
      cgh.parallel_for(sycl::range<1>(selection.size()), op_kernel);
    });
  } else {
    if (d.is_on_device()) {
      throw std::logic_error("Input and output fields must be on the same "
			     "device in unary operator");
    }

    throw std::logic_error("Operators not currently supported on the host.");
  }
}

template<typename Tsrc,
	 typename Tdest,
	 typename Mesh,
	 MeshComponent FieldMapping>
void
CastFieldOperator<Tsrc,Tdest,Mesh,FieldMapping>::
apply(const SourceValueType& s, DestFieldType& d)
{
  MeshSelection<Mesh,FieldMapping> ms(d.mesh());
  return CastFieldOperator<Tsrc,
			   Tdest,
			   Mesh,
			   FieldMapping>::apply(s, d, ms);
}


template<typename Tsrc,
	 typename Tdest,
	 typename Mesh,
	 MeshComponent FieldMapping>
void
CastFieldOperator<Tsrc,Tdest,Mesh,FieldMapping>::
apply(const SourceValueType& s, DestFieldType& d,
      const DestSelectionType& selection)
{
  if (d.is_on_device()) {
    auto& op_queue = d.data().queue();
    op_queue.submit([&](sycl::handler& cgh)
    {
      auto op_kernel = CastConstantOperationKernel<CastFieldOperator<SourceValueType,DestValueType,MeshType,FieldMappingType>>(cgh, s, d, selection);
      cgh.parallel_for(sycl::range<1>(selection.size()), op_kernel);
    });
  } else {
    throw std::logic_error("Operators not currently supported on the host.");
  }
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
void
UnaryFieldOperator<T,Mesh,FieldMapping,OperatorFn>::
apply(const FieldType& s, FieldType& d)
{
  MeshSelection<Mesh,FieldMapping> ms(d.mesh());
  return UnaryFieldOperator<T,
			    Mesh,
			    FieldMapping,
			    OperatorFn>::apply(s, d, ms);
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
void
UnaryFieldOperator<T,Mesh,FieldMapping,OperatorFn>::
apply(const FieldType& s, FieldType& d,
      const SelectionType& selection)
{
  size_t op_size = s.size();
  if (op_size != d.size()) {
    throw std::logic_error("Output field size must match input field "
			   "size in unary operator");
  }
  if (s.is_on_device()) {
    if (not d.is_on_device()) {
      throw std::logic_error("Input and output fields must be on the same "
			     "device in unary operator");
    }

    auto& op_queue = s.data().queue();
    if (op_queue != d.data().queue()) {
      throw std::logic_error("Input and output fields must be in the same "
			     "context in unary operator");
    }

    op_queue.submit([&](sycl::handler& cgh)
    {
      auto op_kernel = UnaryFieldOperationKernel<UnaryFieldOperator<ValueType,MeshType,FieldMappingType,Op>>(cgh, s, d, selection);
      cgh.parallel_for(sycl::range<1>(selection.size()), op_kernel);
    });
  } else {
    if (d.is_on_device()) {
      throw std::logic_error("Input and output fields must be on the same "
			     "device in unary operator");
    }

    throw std::logic_error("Operators not currently supported on the host.");
  }
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
void
BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,OperatorFn>::
apply(FieldType& lhs, const FieldType& rhs)
{
  MeshSelection<Mesh,FieldMapping> ms(lhs.mesh());
  return BinaryFieldCompoundAssignmentOperator<T,
					       Mesh,
					       FieldMapping,
					       OperatorFn>::apply(lhs, rhs, ms);
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
void
BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,OperatorFn>::
apply(FieldType& lhs, const FieldType& rhs,
      const SelectionType& selection)
{
  size_t op_size = lhs.size();
  if (op_size != rhs.size()) {
    throw std::logic_error("Field sizes must match input field "
			   "size in binary operator");
  }
  if (lhs.is_on_device()) {
    if (not rhs.is_on_device()) {
      throw std::logic_error("Fields must be on the same "
			     "device in binary operator");
    }

    auto& op_queue = lhs.data().queue();
    if (op_queue != rhs.data().queue()) {
      throw std::logic_error("Fields must be in the same "
			     "context in binary operator");
    }

    op_queue.submit([&](sycl::handler& cgh)
    {
      auto op_kernel = BinaryFieldCompoundAssignmentOperationKernel<BinaryFieldCompoundAssignmentOperator<ValueType,MeshType,FieldMappingType,Op>>(cgh, lhs, rhs, selection);
      cgh.parallel_for(sycl::range<1>(selection.size()), op_kernel);
    });
  } else {
    throw std::logic_error("Operators not currently supported on the host.");
  }
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
void
BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,OperatorFn>::
apply(FieldType& lhs, const ValueType& rhs)
{
  MeshSelection<Mesh,FieldMapping> ms(lhs.mesh());
  return BinaryFieldCompoundAssignmentOperator<T,
					       Mesh,
					       FieldMapping,
					       OperatorFn>::apply(lhs, rhs, ms);
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
void
BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,OperatorFn>::
apply(FieldType& lhs, const ValueType& rhs,
      const SelectionType& selection)
{
  if (lhs.is_on_device()) {
    auto& op_queue = lhs.data().queue();
    op_queue.submit([&](sycl::handler& cgh)
    {
      auto op_kernel = BinaryFieldConstantCompoundAssignmentOperationKernel<BinaryFieldCompoundAssignmentOperator<ValueType,MeshType,FieldMappingType,Op>>(cgh, lhs, rhs, selection);
      cgh.parallel_for(sycl::range<1>(selection.size()), op_kernel);
    });
  } else {
    throw std::logic_error("Operators not currently supported on the host.");
  }
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
void
BinaryFieldOperator<T,Mesh,FieldMapping,OperatorFn>::
apply(const FieldType& lhs, const FieldType& rhs, FieldType& d)
{
  MeshSelection<Mesh,FieldMapping> ms(d.mesh());
  return BinaryFieldOperator<T,
			     Mesh,
			     FieldMapping,
			     OperatorFn>::apply(lhs, rhs, d, ms);
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
void
BinaryFieldOperator<T,Mesh,FieldMapping,OperatorFn>::
apply(const FieldType& lhs, const FieldType& rhs, FieldType& d,
      const SelectionType& selection)
{
  size_t op_size = lhs.size();
  if (op_size != rhs.size() or op_size != d.size()) {
    throw std::logic_error("Field sizes must match input field "
			   "size in binary operator");
  }
  if (lhs.is_on_device()) {
    if (not rhs.is_on_device() or not d.is_on_device()) {
      throw std::logic_error("Fields must be on the same "
			     "device in binary operator");
    }

    auto& op_queue = lhs.data().queue();
    if (op_queue != rhs.data().queue() or op_queue != d.data().queue()) {
      throw std::logic_error("Fields must be in the same "
			     "context in binary operator");
    }

    op_queue.submit([&](sycl::handler& cgh)
    {
      auto op_kernel = BinaryFieldOperationKernel<BinaryFieldOperator<ValueType,MeshType,FieldMappingType,Op>>(cgh, lhs, rhs, d, selection);
      cgh.parallel_for(sycl::range<1>(selection.size()), op_kernel);
    });
  } else {
    throw std::logic_error("Operators not currently supported on the host.");
  }
}

