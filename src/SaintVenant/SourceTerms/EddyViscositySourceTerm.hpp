/***********************************************************************
 * mfcm SaintVenant/EddyViscositySourceTerm.hpp
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

#ifndef mfcm_SaintVenant_EddyViscositySourceTerm_hpp
#define mfcm_SaintVenant_EddyViscositySourceTerm_hpp

#include "SourceTerm.hpp"
#include "FieldGenerator.hpp"
#include "../Output/CheckFile.hpp"

template<typename TT,
	 typename T,
	 typename Mesh>
class EddyViscositySourceKernel
{
public:

  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;
  
  using CellReadAccessor = typename FieldType::
    template Accessor<sycl::access::mode::read,
		      sycl::access::target::global_buffer>;

  using CellWriteAccessor = typename FieldType::
    template Accessor<sycl::access::mode::write,
		      sycl::access::target::global_buffer>;

  using ReadWriteAccessor = typename FieldType::
    template Accessor<sycl::access::mode::read_write,
		      sycl::access::target::global_buffer>;

private:

  CellReadAccessor h_;
  CellReadAccessor u_;
  CellReadAccessor v_;

  CellReadAccessor mu_;
  CellReadAccessor d2udx2_;
  CellReadAccessor d2udy2_;
  CellReadAccessor d2vdx2_;
  CellReadAccessor d2vdy2_;
  
  ReadWriteAccessor dudt_;
  ReadWriteAccessor dvdt_;

  double timestep_;
  
public:

  EddyViscositySourceKernel(sycl::handler& cgh,
			    const State& U,
			    const Constants& K,
			    const FieldType& mu,
			    const FieldType& d2udx2,
			    const FieldType& d2udy2,
			    const FieldType& d2vdx2,
			    const FieldType& d2vdy2,
			    State& dUdt,
			    const TimeType& timestep);

  void operator()(sycl::item<1> item) const;
  
};

template<typename T,
	 typename Mesh>
class SmagorinskyCoefficientKernel
{
public:
  
  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;

  using CellReadAccessor = typename FieldType::
    template Accessor<sycl::access::mode::read,
		      sycl::access::target::global_buffer>;

  using CellWriteAccessor = typename FieldType::
    template Accessor<sycl::access::mode::write,
		      sycl::access::target::global_buffer>;

private:

  ValueType mu_const_;
  ValueType mu_smag_;

  CellReadAccessor dudx_;
  CellReadAccessor dudy_;
  CellReadAccessor dvdx_;
  CellReadAccessor dvdy_;

  CellWriteAccessor mu_;
  
public:

  SmagorinskyCoefficientKernel(sycl::handler& cgh,
			       const ValueType& mu_const,
			       const ValueType& mu_smag,
			       const FieldType& dudx,
			       const FieldType& dudy,
			       const FieldType& dvdx,
			       const FieldType& dvdy,
			       FieldType& mu);

  void operator()(sycl::item<1> item) const;
  
};

template<typename TT,
	 typename T,
	 typename Mesh>
class EddyViscositySourceTerm : public SaintVenantSourceTerm<TT,T,Mesh>
{
public:

  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;

  using Kernel = EddyViscositySourceKernel<TimeType,ValueType,MeshType>;

private:

  std::shared_ptr<MeshType> mesh_;

  std::shared_ptr<FieldType> mu_;

  std::shared_ptr<FieldType> d2udx2_;
  std::shared_ptr<FieldType> d2udy2_;
  std::shared_ptr<FieldType> d2vdx2_;
  std::shared_ptr<FieldType> d2vdy2_;

  ValueType mu_const_;
  ValueType mu_smag_;
  
public:
  
  EddyViscositySourceTerm(const std::shared_ptr<MeshType>& mesh,
			  const ValueType& mu_val = 1.0,
			  bool on_device = true)
    : SaintVenantSourceTerm<TimeType,ValueType,MeshType>(),
      mesh_(mesh),
      mu_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
	  (mesh_->queue_ptr(), "mu", mesh_, mu_val, on_device).make_shared()),
      d2udx2_(std::make_shared<FieldType>
	      (mesh_->queue_ptr(), "d2udx2", mesh_, 0.0, on_device)),
      d2udy2_(std::make_shared<FieldType>
	      (mesh_->queue_ptr(), "d2udy2", mesh_, 0.0, on_device)),
      d2vdx2_(std::make_shared<FieldType>
	      (mesh_->queue_ptr(), "d2vdx2", mesh_, 0.0, on_device)),
      d2vdy2_(std::make_shared<FieldType>
	      (mesh_->queue_ptr(), "d2vdy2", mesh_, 0.0, on_device)),
      mu_const_(std::numeric_limits<ValueType>::quiet_NaN()),
      mu_smag_(std::numeric_limits<ValueType>::quiet_NaN())
  {
  }

  EddyViscositySourceTerm(const std::shared_ptr<MeshType>& mesh,
			  const ValueType& mu_const,
			  const ValueType& mu_smag,
			  bool on_device = true)
    : SaintVenantSourceTerm<TimeType,ValueType,MeshType>(),
      mesh_(mesh),
      mu_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
	  (mesh_->queue_ptr(), "mu", mesh_, 0.0, on_device).make_shared()),
      d2udx2_(std::make_shared<FieldType>
	      (mesh_->queue_ptr(), "d2udx2", mesh_, 0.0, on_device)),
      d2udy2_(std::make_shared<FieldType>
	      (mesh_->queue_ptr(), "d2udy2", mesh_, 0.0, on_device)),
      d2vdx2_(std::make_shared<FieldType>
	      (mesh_->queue_ptr(), "d2vdx2", mesh_, 0.0, on_device)),
      d2vdy2_(std::make_shared<FieldType>
	      (mesh_->queue_ptr(), "d2vdy2", mesh_, 0.0, on_device)),
      mu_const_(mu_const),
      mu_smag_(mu_smag)
  {
  }

  virtual ~EddyViscositySourceTerm(void)
  {}

  /*
  virtual FieldType* get_output_cell_field_ptr(const std::string& name)
  {
    return nullptr;
  }
  */
  
  virtual void apply(State& U, Constants& constants,
		     State& dUdx, State& dUdy, State& dUdt,
		     const TimeType& timestep, const TimeType& time_now,
		     const std::shared_ptr<TimeParameters<TimeType>>& tp_ptr)
  {
    using SpatialDerivative = SpatialDerivativeOperator<ValueType,
							MeshType,
							MeshComponent::Cell,
							Minmod3<ValueType>>;
    SpatialDerivative::template apply<SpatialDerivativeAxis::X>(dUdx.u(), *d2udx2_);
    SpatialDerivative::template apply<SpatialDerivativeAxis::Y>(dUdy.u(), *d2udy2_);
    SpatialDerivative::template apply<SpatialDerivativeAxis::X>(dUdx.v(), *d2vdx2_);
    SpatialDerivative::template apply<SpatialDerivativeAxis::Y>(dUdy.v(), *d2vdy2_);

    size_t ncells = mesh_->template object_count<MeshComponent::Cell>();
    if (not (mu_const_ != mu_const_) and
	not (mu_smag_ != mu_smag_)) {
      mesh_->queue_ptr()->submit([&] (sycl::handler& cgh) {
	auto kernel = SmagorinskyCoefficientKernel<ValueType,MeshType>(cgh,
								       mu_const_,
								       mu_smag_,
								       dUdx.u(),
								       dUdy.u(),
								       dUdx.v(),
								       dUdy.v(),
								       *mu_);
	cgh.parallel_for(sycl::range<1>(ncells), kernel);
      });
    }
    
    mesh_->queue_ptr()->submit([&] (sycl::handler& cgh) {
      auto kernel = Kernel(cgh, U, constants,
			   *mu_,
			   *d2udx2_, *d2udy2_,
			   *d2vdx2_, *d2vdy2_,
			   dUdt, timestep);
      cgh.parallel_for(sycl::range<1>(ncells), kernel);
    });
  }

  static std::shared_ptr<SaintVenantSourceTerm<TT,T,Mesh>>
  create_source_term(const Config& conf,
		     const std::shared_ptr<MeshType>& mesh,
		     bool on_device = true);
  
};

#endif
