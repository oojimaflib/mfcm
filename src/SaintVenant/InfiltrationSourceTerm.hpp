/***********************************************************************
 * mfcm SaintVenant/InfiltrationSourceTerm.hpp
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

#ifndef mfcm_SaintVenant_InfiltrationSourceTerm_hpp
#define mfcm_SaintVenant_InfiltrationSourceTerm_hpp

#include "SourceTerm.hpp"
#include "FieldGenerator.hpp"
#include "../Output/CheckFile.hpp"

template<typename TT,
	 typename T,
	 typename Mesh>
class InfiltrationSourceKernel
{
public:

  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;

  using State = SaintVenantState<ValueType, MeshType>;
  using Constants = SaintVenantConstants<ValueType, MeshType>;

  using CellReadAccessor = typename FieldType::
    template Accessor<sycl::access::mode::read,
    sycl::access::target::global_buffer>;
  
  using CellReadWriteAccessor = typename FieldType::
    template Accessor<sycl::access::mode::read_write,
		      sycl::access::target::global_buffer>;

private:

  CellReadAccessor h_;

  CellReadAccessor i_rate_;
  CellReadWriteAccessor i_cap_;

  CellReadWriteAccessor dhdt_;

  double timestep_;

public:

  InfiltrationSourceKernel(sycl::handler& cgh,
			   const State& U,
			   const Constants& K,
			   const FieldType& i_rate,
			   FieldType& i_cap,
			   State& dUdt,
			   const TimeType& timestep);

  void operator()(sycl::item<1> item) const;
  
};

template<typename TT,
	 typename T,
	 typename Mesh>
class InfiltrationSourceTerm : public SaintVenantSourceTerm<TT,T,Mesh>
{
public:

  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;

  using Kernel = InfiltrationSourceKernel<TimeType,ValueType,MeshType>;

private:

  std::shared_ptr<MeshType> mesh_;

  FieldType infiltration_rate_;
  FieldType infiltration_capacity_;

public:

  InfiltrationSourceTerm(const std::shared_ptr<MeshType>& mesh,
			 bool on_device = true)
    : SaintVenantSourceTerm<TimeType,ValueType,MeshType>(),
      mesh_(mesh),
      infiltration_rate_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
			 (mesh_->queue_ptr(), "IR", mesh_, 1e-6, on_device)()),
      infiltration_capacity_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
			     (mesh_->queue_ptr(), "IC", mesh_, 0.1, on_device)())
  {
    FieldCheckFile<FieldType> cf("infiltration");
    cf.output({&infiltration_rate_, &infiltration_capacity_});
  }

  virtual ~InfiltrationSourceTerm(void)
  {}

  /*
  virtual FieldType* get_output_cell_field_ptr(const std::string& name)
  {
    return nullptr;
  }
  */

  virtual void apply(State& U, Constants& constants, State& dUdt,
		     const TimeType& timestep, const TimeType& time_now,
		     const std::shared_ptr<TimeParameters<TimeType>>& tp_ptr)
  {
    size_t ncells = mesh_->template object_count<MeshComponent::Cell>();
    mesh_->queue_ptr()->submit([&] (sycl::handler& cgh) {
      auto kernel = Kernel(cgh, U, constants,
			   infiltration_rate_, infiltration_capacity_,
			   dUdt, timestep);
      cgh.parallel_for(sycl::range<1>(ncells), kernel);
    });
  }
  
};

#endif
