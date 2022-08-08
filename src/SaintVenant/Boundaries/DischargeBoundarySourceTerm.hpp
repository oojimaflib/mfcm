/***********************************************************************
 * mfcm SaintVenant/Boundaries/DischargeBoundarySourceTerm.hpp
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

#ifndef mfcm_SaintVenant_Boundaries_DischargeBoundarySourceTerm_hpp
#define mfcm_SaintVenant_Boundaries_DischargeBoundarySourceTerm_hpp

#include "BoundarySourceTerm.hpp"
#include "TimeSeries.hpp"

template<typename T,
	 typename Mesh>
class DischargeBoundarySourceKernel
{
public:

  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;

  using CellReadAccessor = typename FieldType::
    template Accessor<sycl::access::mode::read,
		      sycl::access::target::global_buffer>;

  using ReadWriteAccessor = typename FieldType::
    template Accessor<sycl::access::mode::read_write,
		      sycl::access::target::global_buffer>;

private:

  CellReadAccessor h_;
  CellReadAccessor u_;
  CellReadAccessor v_;

  CellReadAccessor qbdy0_;
  CellReadAccessor qbdy1_;

  ReadWriteAccessor dhdt_;
  ReadWriteAccessor dudt_;
  ReadWriteAccessor dvdt_;

  double timestep_;
  double time_now_;

  double step_length_;

public:

  DischargeBoundarySourceKernel(sycl::handler& cgh,
				const State& U,
				const FieldType& qbdy0,
				const FieldType& qbdy1,
				State& dUdt,
				const double& timestep,
				const double& time_now,
				const double& step_length);

  void operator()(sycl::item<1> item) const;
  
};

template<typename TT,
	 typename T,
	 typename Mesh>
class DischargeBoundarySourceTerm
  : public BoundarySourceTerm<TT, T, Mesh,
			      DischargeBoundarySourceKernel<T,Mesh>>
{
public:

  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;
  using SelectionType = MeshSelection<MeshType,MeshComponent::Cell>;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;

  using KernelType = DischargeBoundarySourceKernel<ValueType,MeshType>;

public:

  DischargeBoundarySourceTerm(const std::shared_ptr<MeshType>& mesh,
			      bool on_device = true)
    : BoundarySourceTerm<TimeType,ValueType,MeshType,KernelType>
    (mesh,
     FieldType(mesh->queue_ptr(), "qbdy0", mesh, ValueType(0.0), on_device),
     FieldType(mesh->queue_ptr(), "qbdy1", mesh, ValueType(0.0), on_device))
  {
  }

  virtual ~DischargeBoundarySourceTerm(void)
  {}

  virtual void start_new_step(Constants& constants,
			      const TimeType& time_now,
			      const std::shared_ptr<TimeParameters<TimeType>>& tp_ptr)
  {
    const TimeType& step_duration = tp_ptr->step_duration();
    const Config& conf =
      GlobalConfig::instance().boundary_configuration("discharge");
    this->clear_values();
    for (auto&& kv : conf) {
      std::string key = kv.first;
      std::string name = kv.second.get_value<std::string>();
      std::cout << "Updating discharge boundary: " << name << std::endl;
      MeshSelection<MeshType,MeshComponent::Cell> sel(this->mesh(),
						      kv.second.get_child("cells"));
      
      if (key == "constant") {
	ValueType value = kv.second.get<ValueType>("value");
	MeshSelectionCheckFile<MeshSelection<MeshType,MeshComponent::Cell>> cf("qbdy_sel");
	cf.output(sel);
	this->add_values(value, value, sel);
      } else if (key == "time series") {
	const std::shared_ptr<TimeSeries<TimeType,ValueType>>& ts =
	  TimeSeriesDatabase<TimeType,ValueType>::instance()
	  .get_time_series_ptr(this->mesh()->queue_ptr(), tp_ptr->parser(),
			       kv.second.get<std::string>("values"));
	this->add_values(ts->at(time_now),
			 ts->at(time_now + step_duration), sel);
      }
    }

    FieldCheckFile<Field<ValueType,MeshType,MeshComponent::Cell>> cf("qbdy");
    cf.output({ &(this->xbdy0()), &(this->xbdy1()) });
  }
  
};

#endif
