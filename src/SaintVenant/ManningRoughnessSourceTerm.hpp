/***********************************************************************
 * mfcm SaintVenant/ManningRoughnessSourceTerm.hpp
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

#ifndef mfcm_SaintVenant_ManningRoughnessSourceTerm_hpp
#define mfcm_SaintVenant_ManningRoughnessSourceTerm_hpp

#include "SourceTerm.hpp"

template<typename TT,
	 typename T,
	 typename Mesh>
class ManningRoughnessSourceKernel
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

  CellReadAccessor n_shallow_;
  CellReadAccessor n_deep_;
  CellReadAccessor d_shallow_;
  CellReadAccessor d_deep_;

  CellWriteAccessor nh_;
  CellWriteAccessor Sf_;
  
  ReadWriteAccessor dudt_;
  ReadWriteAccessor dvdt_;

  double timestep_;
  
public:

  ManningRoughnessSourceKernel(sycl::handler& cgh,
			       const State& U,
			       const Constants& K,
			       const FieldType& n_shallow,
			       const FieldType& n_deep,
			       const FieldType& d_shallow,
			       const FieldType& d_deep,
			       FieldType& nh,
			       FieldType& Sf,
			       State& dUdt,
			       const TimeType& timestep);

  void operator()(sycl::item<1> item) const;
  
};

template<typename TT,
	 typename T,
	 typename Mesh>
class ManningRoughnessSourceTerm : public SaintVenantSourceTerm<TT,T,Mesh>
{
public:

  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;

  using Kernel = ManningRoughnessSourceKernel<TimeType,ValueType,MeshType>;

private:

  std::shared_ptr<MeshType> mesh_;

  FieldType n_shallow_;
  FieldType n_deep_;
  FieldType d_shallow_;
  FieldType d_deep_;

  FieldType nh_;
  FieldType Sf_;

public:
  
  ManningRoughnessSourceTerm(const std::shared_ptr<MeshType>& mesh,
			     bool on_device = true)
    : SaintVenantSourceTerm<TimeType,ValueType,MeshType>(),
      mesh_(mesh),
      n_shallow_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
		 (mesh_->queue_ptr(), "n_shallow", mesh_, 0.3f, on_device)()),
      n_deep_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
	      (mesh_->queue_ptr(), "n_deep", mesh_, 0.03f, on_device)()),
      d_shallow_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
		 (mesh_->queue_ptr(), "d_shallow", mesh_, 0.1f, on_device)()),
      d_deep_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
	      (mesh_->queue_ptr(), "d_deep", mesh_, 0.3f, on_device)()),
      nh_(mesh_->queue_ptr(), "mannings_n", mesh_, 0.0f, on_device),
      Sf_(mesh_->queue_ptr(), "friction_slope", mesh_, 0.0f, on_device)
  {
    FieldCheckFile<FieldType> cf("manning");
    cf.output({&n_shallow_, &n_deep_, &d_shallow_, &d_deep_});
  }

  virtual ~ManningRoughnessSourceTerm(void)
  {}

  virtual FieldType* get_output_cell_field_ptr(const std::string& name)
  {
    if (name == "mannings_n") {
      return &nh_;
    } else if (name == "friction_slope") {
      return &Sf_;
    }
    return nullptr;
  }
  
  virtual void apply(State& U, Constants& constants, State& dUdt,
		     const TimeType& timestep, const TimeType& time_now,
		     const std::shared_ptr<TimeParameters<TimeType>>& tp_ptr)
  {
    size_t ncells = mesh_->template object_count<MeshComponent::Cell>();
    mesh_->queue_ptr()->submit([&] (sycl::handler& cgh) {
      auto kernel = Kernel(cgh, U, constants,
			   n_shallow_, n_deep_,
			   d_shallow_, d_deep_,
			   nh_, Sf_,
			   dUdt, timestep);
      cgh.parallel_for(sycl::range<1>(ncells), kernel);
    });
  }

};

#endif
