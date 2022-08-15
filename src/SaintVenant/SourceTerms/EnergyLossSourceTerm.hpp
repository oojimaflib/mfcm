/***********************************************************************
 * mfcm SaintVenant/EnergyLossSourceTerm.hpp
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

#ifndef mfcm_SaintVenant_EnergyLossSourceTerm_hpp
#define mfcm_SaintVenant_EnergyLossSourceTerm_hpp

#include "SourceTerm.hpp"
#include "FieldGenerator.hpp"
#include "../Output/CheckFile.hpp"

template<typename TT,
	 typename T,
	 typename Mesh>
class EnergyLossSourceKernel
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

  CellReadAccessor fdx_;
  CellReadAccessor fdy_;
  
  ReadWriteAccessor dudt_;
  ReadWriteAccessor dvdt_;

  double timestep_;
  
public:

  EnergyLossSourceKernel(sycl::handler& cgh,
			 const State& U,
			 const Constants& K,
			 const FieldType& fdx,
			 const FieldType& fdy,
			 State& dUdt,
			 const TimeType& timestep);

  void operator()(sycl::item<1> item) const;
  
};

template<typename TT,
	 typename T,
	 typename Mesh>
class EnergyLossSourceTerm : public SaintVenantSourceTerm<TT,T,Mesh>
{
public:

  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;

  using Kernel = EnergyLossSourceKernel<TimeType,ValueType,MeshType>;

private:

  std::shared_ptr<MeshType> mesh_;

  std::shared_ptr<FieldType> fdx_;
  std::shared_ptr<FieldType> fdy_;

public:
  
  EnergyLossSourceTerm(const std::shared_ptr<MeshType>& mesh,
		       const ValueType& fdx_val = 0.0,
		       const ValueType& fdy_val = 0.0,
		       bool on_device = true)
    : SaintVenantSourceTerm<TimeType,ValueType,MeshType>(),
      mesh_(mesh),
      fdx_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
	   (mesh_->queue_ptr(), "fdx", mesh_, fdx_val, on_device).make_shared()),
      fdy_(FieldGenerator<ValueType,MeshType,MeshComponent::Cell>
	   (mesh_->queue_ptr(), "fdy", mesh_, fdy_val, on_device).make_shared())
  {
    FieldCheckFile<FieldType> cf("energy-loss-coefficients");
    cf.output({fdx_.get(), fdy_.get()});
  }

  virtual ~EnergyLossSourceTerm(void)
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
			   *fdx_, *fdy_,
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
