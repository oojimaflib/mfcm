/***********************************************************************
 * mfcm SpatialDerivative/SpatialDerivative.cpp
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

#include "SpatialDerivative.hpp"

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
template<SpatialDerivativeAxis Axis>
void
SpatialDerivativeOperator<T,Mesh,FieldMapping,OperatorFn>::
apply(const FieldType& s, FieldType& d)
{
  MeshSelection<Mesh,FieldMapping> ms(d.mesh());
  return SpatialDerivativeOperator<T,
				   Mesh,
				   FieldMapping,
				   OperatorFn>::apply<Axis>(s, d, ms);
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping,
	 typename OperatorFn>
template<SpatialDerivativeAxis Axis>
void
SpatialDerivativeOperator<T,Mesh,FieldMapping,OperatorFn>::
apply(const FieldType& s, FieldType& d,
      const SelectionType& selection)
{
  if (s.is_on_device()) {
    if (not d.is_on_device()) {
      throw std::logic_error("Input and output fields must be on the same "
			     "device in spatial derivative operator");
    }

    auto& op_queue = s.data().queue();
    if (op_queue != d.data().queue()) {
      throw std::logic_error("Input and output fields must be in the same "
			     "context in spatial derivative operator");
    }

    op_queue.submit([&](sycl::handler& cgh)
    {
      auto op_kernel =
	SpatialDerivativeOperationKernel<SpatialDerivativeOperator<ValueType,
								   MeshType,
								   FieldMappingType,
								   OperatorFn>,
					 Axis>
	(cgh, s, d, selection);
      cgh.parallel_for(sycl::range<1>(selection.size()), op_kernel);
    });
  } else {
    if (d.is_on_device()) {
      throw std::logic_error("Input and output fields must be on the same "
			     "device in spatial derivative operator");
    }

    throw std::logic_error("Operators not currently supported on the host.");
    /*
    for (size_t i = 0; i < op_size; ++i) {
      d.host_vector().at(i) = Op()(s.host_vector().at(i));
    }
    */
  }
}
