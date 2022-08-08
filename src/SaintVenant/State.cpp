/***********************************************************************
 * mfcm SaintVenant/State.cpp
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

#include "State.hpp"
#include "FieldGenerator.hpp"
#include "SpatialDerivative.hpp"
#include "Minmod3.hpp"

template<typename T,
	 typename Mesh>
SaintVenantState<T,Mesh>::
SaintVenantState(const std::shared_ptr<MeshType>& mesh,
		 const std::string& prefix,
		 const std::string& suffix,
		 bool on_device)
  : mesh_(mesh),
    h_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
       (mesh_->queue_ptr(), prefix + "h" + suffix, mesh_, 0.0f, on_device)()),
    u_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
       (mesh_->queue_ptr(), prefix + "u" + suffix, mesh_, 0.0f, on_device)()),
    v_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
       (mesh_->queue_ptr(), prefix + "v" + suffix, mesh_, 0.0f, on_device)())
{}

template<typename T,
	 typename Mesh>
SaintVenantState<T,Mesh>::
SaintVenantState(const SaintVenantState<ValueType,MeshType>& state,
		 const std::string& prefix,
		 const std::string& suffix)
  : mesh_(state.mesh_),
    h_(CellField<ValueType,MeshType>(prefix, state.h_, suffix)),
    u_(CellField<ValueType,MeshType>(prefix, state.u_, suffix)),
    v_(CellField<ValueType,MeshType>(prefix, state.v_, suffix))
{}

template<typename T,
	 typename Mesh>
SaintVenantState<T,Mesh>&
SaintVenantState<T,Mesh>::operator=(const SaintVenantState& state)
{
  if (this == &state) {
    return *this;
  }
  
  mesh_ = state.mesh_;
  h_ = state.h_;
  u_ = state.u_;
  v_ = state.v_;
  return *this;
}

template<typename T,
	 typename Mesh>
SaintVenantState<T,Mesh>&
SaintVenantState<T,Mesh>::operator+=(const SaintVenantState& rhs)
{
  h_ += rhs.h_;
  u_ += rhs.u_;
  v_ += rhs.v_;
  return *this;
}

template<typename T,
	 typename Mesh>
SaintVenantState<T,Mesh>&
SaintVenantState<T,Mesh>::operator-=(const SaintVenantState& rhs)
{
  h_ -= rhs.h_;
  u_ -= rhs.u_;
  v_ -= rhs.v_;
  return *this;
}

template<typename T,
	 typename Mesh>
SaintVenantState<T,Mesh>&
SaintVenantState<T,Mesh>::operator*=(const SaintVenantState& rhs)
{
  h_ *= rhs.h_;
  u_ *= rhs.u_;
  v_ *= rhs.v_;
  return *this;
}

template<typename T,
	 typename Mesh>
SaintVenantState<T,Mesh>&
SaintVenantState<T,Mesh>::operator/=(const SaintVenantState& rhs)
{
  h_ /= rhs.h_;
  u_ /= rhs.u_;
  v_ /= rhs.v_;
  return *this;
}

template<typename T,
	 typename Mesh>
SaintVenantState<T,Mesh>&
SaintVenantState<T,Mesh>::operator+=(const ValueType& rhs)
{
  h_ += rhs;
  u_ += rhs;
  v_ += rhs;
  return *this;
}

template<typename T,
	 typename Mesh>
SaintVenantState<T,Mesh>&
SaintVenantState<T,Mesh>::operator-=(const ValueType& rhs)
{
  h_ -= rhs;
  u_ -= rhs;
  v_ -= rhs;
  return *this;
}

template<typename T,
	 typename Mesh>
SaintVenantState<T,Mesh>&
SaintVenantState<T,Mesh>::operator*=(const ValueType& rhs)
{
  h_ *= rhs;
  u_ *= rhs;
  v_ *= rhs;
  return *this;
}

template<typename T,
	 typename Mesh>
SaintVenantState<T,Mesh>&
SaintVenantState<T,Mesh>::operator/=(const ValueType& rhs)
{
  h_ /= rhs;
  u_ /= rhs;
  v_ /= rhs;
  return *this;
}

template<typename T,
	 typename Mesh>
void
SaintVenantState<T,Mesh>::
calculate_spatial_derivatives(SaintVenantState<ValueType,MeshType>& dUdx,
			      SaintVenantState<ValueType,MeshType>& dUdy)
{
  using SpatialDerivative = SpatialDerivativeOperator<ValueType,
						      MeshType,
						      MeshComponent::Cell,
						      Minmod3<ValueType>>;
  SpatialDerivative::template apply<SpatialDerivativeAxis::X>(h_, dUdx.h());
  SpatialDerivative::template apply<SpatialDerivativeAxis::X>(u_, dUdx.u());
  SpatialDerivative::template apply<SpatialDerivativeAxis::X>(v_, dUdx.v());
  SpatialDerivative::template apply<SpatialDerivativeAxis::Y>(h_, dUdy.h());
  SpatialDerivative::template apply<SpatialDerivativeAxis::Y>(u_, dUdy.u());
  SpatialDerivative::template apply<SpatialDerivativeAxis::Y>(v_, dUdy.v());
}

template<typename T,
	 typename Mesh>
T
SaintVenantState<T,Mesh>::
max_control_number(const double& timestep)
{
  ValueType max_cn = 0.0;
  sycl::buffer<ValueType> max_cn_buf(&max_cn, 1);
    
  h_.mesh()->queue_ptr()->submit([&] (sycl::handler& cgh) {
    using CellFieldAccessor = typename CellField<ValueType,MeshType>::
      template Accessor<sycl::access::mode::read>;
    CellFieldAccessor h_acc(h_, cgh);
    CellFieldAccessor u_acc(u_, cgh);
    CellFieldAccessor v_acc(v_, cgh);

    auto max_cn_reduction = sycl::reduction(max_cn_buf.get_access(cgh),
					    sycl::maximum<T>());

    size_t ncells = h_.mesh()->template object_count<MeshComponent::Cell>();
    cgh.parallel_for(sycl::range<1>(ncells), max_cn_reduction,
		     [=](sycl::item<1> item, auto& max) {
		       size_t i = item.get_linear_id();
		       ValueType h = sycl::fmax(h_acc.data()[i],
						ValueType(0.0));
		       ValueType u = sycl::fabs(u_acc.data()[i]);
		       ValueType v = sycl::fabs(v_acc.data()[i]);
		       ValueType c = sycl::sqrt(ValueType(9.81) * h);
		       ValueType dx = h_acc.mesh().dx();
		       ValueType dy = h_acc.mesh().dy();
		       ValueType cn = timestep * (((u+c)/dx) + ((v+c)/dy));
		       max.combine(cn);
		     });
  });
  return max_cn_buf.get_host_access()[0];
}
