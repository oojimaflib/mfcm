/***********************************************************************
 * mfcm SaintVenant/Boundaries/HeadBoundarySourceTerm.hpp
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

#ifndef mfcm_SaintVenant_HeadBoundarySourceTerm_hpp
#define mfcm_SaintVenant_HeadBoundarySourceTerm_hpp

#include "BoundarySourceTerm.hpp"

template<typename T,
	 typename Mesh>
class HeadBoundarySourceKernel
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

  CellReadAccessor hbdy0_;
  CellReadAccessor hbdy1_;

  ReadWriteAccessor dhdt_;
  ReadWriteAccessor dudt_;
  ReadWriteAccessor dvdt_;

  double timestep_;
  double time_now_;

  double step_length_;

public:

  HeadBoundarySourceKernel(sycl::handler& cgh,
			   const State& U,
			   const Constants& K,
			   const FieldType& hbdy0,
			   const FieldType& hbdy1,
			   State& dUdt,
			   const double& timestep,
			   const double& time_now,
			   const double& step_length);
  
  void operator()(sycl::item<1> item) const;
  
};

template<typename TT,
	 typename T,
	 typename Mesh>
class HeadBoundarySourceTerm
  : public KernelBoundarySourceTerm<TT, T, Mesh,
				    HeadBoundarySourceKernel<T,Mesh>>
{
public:

  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;

  using KernelType = HeadBoundarySourceKernel<ValueType,MeshType>;

public:

  HeadBoundarySourceTerm(const std::shared_ptr<MeshType>& mesh,
			 bool on_device = true)
    : KernelBoundarySourceTerm<TimeType,ValueType,MeshType,KernelType>
    (mesh,
     FieldType(mesh->queue_ptr(), "hbdy0", mesh,
	       std::numeric_limits<ValueType>::quiet_NaN(), on_device),
     FieldType(mesh->queue_ptr(), "hbdy1", mesh,
	       std::numeric_limits<ValueType>::quiet_NaN(), on_device))
  {
  }

  virtual ~HeadBoundarySourceTerm(void)
  {}

  virtual void start_new_step(Constants& constants,
			      const TimeType& time_now,
			      const std::shared_ptr<TimeParameters<TimeType>>& tp_ptr)
  {
    const TimeType& step_duration = tp_ptr->step_duration();
    const Config& conf =
      GlobalConfig::instance().boundary_configuration("head");
    this->clear_values();
    for (auto&& kv : conf) {
      std::string key = kv.first;
      std::string name = kv.second.get_value<std::string>();
      std::cout << "Updating head boundary: " << name << std::endl;
      MeshSelection<MeshType,MeshComponent::Cell> sel(this->mesh(),
						      kv.second.get_child("cells"));
      
      if (key == "constant") {
	ValueType value = kv.second.get<ValueType>("value");
	MeshSelectionCheckFile<MeshSelection<MeshType,MeshComponent::Cell>> cf("hbdy_sel");
	cf.output(sel);
	this->set_values(value, value, sel);
      } else if (key == "time series") {
	const std::shared_ptr<TimeSeries<TimeType,ValueType>>& ts =
	  TimeSeriesDatabase<TimeType,ValueType>::instance()
	  .get_time_series_ptr(this->mesh()->queue_ptr(), tp_ptr->parser(),
			       kv.second.get<std::string>("values"));
	this->set_values(ts->at(time_now),
			 ts->at(time_now + step_duration), sel);
      }
    }

    FieldCheckFile<Field<ValueType,MeshType,MeshComponent::Cell>> cf("hbdy");
    cf.output({ &(this->xbdy0()), &(this->xbdy1()) });
  }
  
  static std::shared_ptr<SaintVenantSourceTerm<TT,T,Mesh>>
  create_boundary(const Config& conf,
		  const std::shared_ptr<MeshType>& mesh,
		  bool on_device);
};

#endif
