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

template<typename TT, typename T, typename Mesh>
class EnergyLossModel
{
public:

  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType, MeshType>;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;
  using MeshSelectionType = MeshSelection<MeshType,MeshComponent::Cell>;
  
protected:

  std::shared_ptr<MeshType> mesh_;
  
  MeshSelectionType sel_;

public:

  EnergyLossModel(const std::shared_ptr<MeshType>& mesh,
		  const Config& conf)
    : mesh_(mesh), sel_(mesh_, conf.get_child("cells"))
  {}

  virtual ~EnergyLossModel(void) {}

  virtual void apply(const State& U, const Constants& constants,
		     const State& dUdx, const State& dUdy, const TimeType& time_now,
		     FieldType& fdx, FieldType& fdy) = 0;

  static std::shared_ptr<EnergyLossModel<TimeType,ValueType,MeshType>>
  create(const std::shared_ptr<MeshType>& mesh,
	 const Config& conf);
  
};

template<typename TT,
	 typename T,
	 typename Mesh>
class ConstantEnergyLossModel : public EnergyLossModel<TT,T,Mesh>
{
public:
  
  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType, MeshType>;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;
  using MeshSelectionType = MeshSelection<MeshType,MeshComponent::Cell>;
  
private:

  ValueType fdx_val_;
  ValueType fdy_val_;

public:

  ConstantEnergyLossModel(const std::shared_ptr<MeshType>& mesh,
			  const Config& conf)
    : EnergyLossModel<TimeType,ValueType,MeshType>(mesh, conf),
      fdx_val_(conf.get<ValueType>("fx", 0.0)),
      fdy_val_(conf.get<ValueType>("fy", 0.0))
  {}

  virtual ~ConstantEnergyLossModel(void) {}

  virtual void apply(const State& U, const Constants& constants,
		     const State& dUdx, const State& dUdy, const TimeType& time_now,
		     FieldType& fdx, FieldType& fdy)
  {
    using Op = BinaryFieldCompoundAssignmentOperator
      <ValueType,MeshType,MeshComponent::Cell,std::plus<ValueType>>;
    Op::apply(fdx, fdx_val_, this->sel_);
    Op::apply(fdy, fdy_val_, this->sel_);
  }
  
};

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

  using Model = EnergyLossModel<TimeType,ValueType,MeshType>;
  std::vector<std::shared_ptr<Model>> models_;
  
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

  EnergyLossSourceTerm(const std::shared_ptr<MeshType>& mesh,
		       const std::shared_ptr<FieldType>& fdx,
		       const std::shared_ptr<FieldType>& fdy,
		       bool on_device = true)
    : SaintVenantSourceTerm<TimeType,ValueType,MeshType>(),
      mesh_(mesh),
      fdx_(fdx), fdy_(fdy)
  {}

  EnergyLossSourceTerm(const std::shared_ptr<MeshType>& mesh,
		       const Config& conf,
		       bool on_device = true)
    : SaintVenantSourceTerm<TimeType,ValueType,MeshType>(),
      mesh_(mesh),
      fdx_(std::make_shared<FieldType>
	   (mesh_->queue_ptr(), "fdx", mesh_, 0.0, on_device)),
      fdy_(std::make_shared<FieldType>
	   (mesh_->queue_ptr(), "fdy", mesh_, 0.0, on_device))
  {
    auto md_crange = conf.equal_range("model");
    for (auto it = md_crange.first; it != md_crange.second; ++it) {
      models_.push_back(EnergyLossModel<TimeType,ValueType,MeshType>::create(mesh, it->second));
    }
  }

  virtual ~EnergyLossSourceTerm(void)
  {}

  virtual FieldType* get_output_cell_field_ptr(const std::string& name)
  {
    if (name == "fx") {
      return fdx_.get();
    } else if (name == "fy") {
      return fdy_.get();
    }
    return nullptr;
  }
  
  virtual void apply(State& U, Constants& constants,
		     State& dUdx, State& dUdy, State& dUdt,
		     const TimeType& timestep, const TimeType& time_now,
		     const std::shared_ptr<TimeParameters<TimeType>>& tp_ptr)
  {
    if (models_.size() > 0) {
      (*fdx_) = 0.0;
      (*fdy_) = 0.0;

      for (auto&& model : models_) {
	model->apply(U, constants, dUdx, dUdy, time_now, *fdx_, *fdy_);
      }
    }
    
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
