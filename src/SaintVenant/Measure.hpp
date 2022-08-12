/***********************************************************************
 * mfcm SaintVenant/Measure.hpp
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

#ifndef mfcm_SaintVenant_Measure_hpp
#define mfcm_SaintVenant_Measure_hpp

#include "TimeSeries.hpp"
#include "../Output/Measure.hpp"

template<typename TT,
	 typename T,
	 typename Mesh>
class SaintVenantMeasure : public Measure<TT, SaintVenantState<T,Mesh>>
{
public:
  
  using TimeType = TT;

  using StateType = SaintVenantState<T,Mesh>;
  using FieldType = typename StateType::FieldType;
  using ValueType = typename FieldType::ValueType;
  using MeshType = typename FieldType::MeshType;
  static const MeshComponent FieldMappingType = FieldType::FieldMappingType;
  
public:

  SaintVenantMeasure(const std::shared_ptr<sycl::queue>& queue,
		     const std::shared_ptr<TimeParameters<TimeType>>& tparams,
		     const Config& config)
    : Measure<TimeType, StateType>(queue, tparams, config)
  {}

  virtual ~SaintVenantMeasure(void) {}

  virtual void update(const TimeType& time_now,
		      const StateType& state) = 0;

};

template<typename TT,
	 typename T,
	 typename Mesh>
class SaintVenantPointMeasure : public SaintVenantMeasure<TT,T,Mesh>
{
public:

  using TimeType = TT;

  using StateType = SaintVenantState<T,Mesh>;
  using FieldType = typename StateType::FieldType;
  using ValueType = typename FieldType::ValueType;
  using MeshType = typename FieldType::MeshType;
  static const MeshComponent FieldMappingType = FieldType::FieldMappingType;

private:

  size_t mesh_object_index_;

protected:

  T get_point_value(const FieldType& field)
  {
    std::vector<size_t> pt_vec({ mesh_object_index_, });
    sycl::buffer<size_t,1> pt_buf(pt_vec.data(),
				  sycl::range<1>(pt_vec.size()));
    ValueType val;
    sycl::buffer<ValueType> val_buf(&val, 1);

    field.mesh()->queue_ptr()->submit([&] (sycl::handler& cgh) {
      using ValAcc = typename FieldType::template Accessor<sycl::access::mode::read>;
      ValAcc val_acc(field, cgh);
      auto pt_acc = pt_buf.get_access<sycl::access::mode::read>(cgh);

      auto val_reduction = sycl::reduction(val_buf.get_access(cgh),
					   sycl::plus<ValueType>());
      
      cgh.parallel_for(sycl::range<1>(pt_vec.size()), val_reduction,
		       [=](sycl::item<1> item, auto& val) {
			 val.combine(val_acc.data()[pt_acc[item]]);
		       });
    });
    return val_buf.get_host_access()[0] / pt_vec.size();
  }
  
public:

  SaintVenantPointMeasure(const std::shared_ptr<sycl::queue>& queue,
			  const std::shared_ptr<TimeParameters<TimeType>>& tparams,
			  const std::shared_ptr<MeshType>& mesh,
			  const Config& config)
    : SaintVenantMeasure<TimeType,ValueType,MeshType>(queue, tparams, config)
  {
    std::array<double,2> location = split_string<double,2>(config.get<std::string>("location"));
    mesh_object_index_ = mesh->template get_nearest_object_index<FieldMappingType>(location);
  }

  virtual ~SaintVenantPointMeasure(void)
  {}

  virtual void update(const TimeType& time_now,
		      const StateType& state) = 0;

};

template<typename TT,
	 typename T,
	 typename Mesh>
class SaintVenantVolumeMeasure : public SaintVenantMeasure<TT,T,Mesh>
{
public:

  using TimeType = TT;

  using StateType = SaintVenantState<T,Mesh>;
  using FieldType = typename StateType::FieldType;
  using ValueType = typename FieldType::ValueType;
  using MeshType = typename FieldType::MeshType;
  static const MeshComponent FieldMappingType = FieldType::FieldMappingType;

protected:

  T get_volume(StateType& state)
  {
    FieldType& field = state.h();
    ValueType val;
    sycl::buffer<ValueType> val_buf(&val, 1);

    field.mesh()->queue_ptr()->submit([&] (sycl::handler& cgh) {
      using ValAcc = typename FieldType::template Accessor<sycl::access::mode::read>;
      ValAcc val_acc(field, cgh);

      auto val_reduction = sycl::reduction(val_buf.get_access(cgh),
					   sycl::plus<ValueType>());
      
      cgh.parallel_for(val_acc.data().get_buffer().get_range(), val_reduction,
		       [=](sycl::item<1> item, auto& val) {
			 val.combine(val_acc.data()[item] * val_acc.mesh().cell_area());
		       });
    });
    return val_buf.get_host_access()[0];
  }
  
public:

  SaintVenantVolumeMeasure(const std::shared_ptr<sycl::queue>& queue,
			   const std::shared_ptr<TimeParameters<TimeType>>& tparams,
			   const std::shared_ptr<MeshType>& mesh,
			   const Config& config)
    : SaintVenantMeasure<TimeType,ValueType,MeshType>(queue, tparams, config)
  {
  }

  virtual ~SaintVenantVolumeMeasure(void)
  {}

  virtual void update(const TimeType& time_now,
		      const StateType& state)
  {
    this->set_point(time_now, this->get_volume(state));
  }

};

template<typename TT,
	 typename T,
	 typename Mesh>
class SaintVenantHPointMeasure : public SaintVenantPointMeasure<TT,T,Mesh>
{
public:

  using TimeType = TT;

  using StateType = SaintVenantState<T,Mesh>;
  using FieldType = typename StateType::FieldType;
  using ValueType = typename FieldType::ValueType;
  using MeshType = typename FieldType::MeshType;
  static const MeshComponent FieldMappingType = FieldType::FieldMappingType;

  SaintVenantHPointMeasure(const std::shared_ptr<sycl::queue>& queue,
			   const std::shared_ptr<TimeParameters<TimeType>>& tparams,
			   const std::shared_ptr<MeshType>& mesh,
			   const Config& config)
    : SaintVenantPointMeasure<TimeType,ValueType,MeshType>(queue,
							   tparams,
							   mesh,
							   config)
							   
  {}

  virtual ~SaintVenantHPointMeasure(void)
  {}

  virtual void update(const TimeType& time_now,
		      const StateType& state)
  {
    this->set_point(time_now, this->get_point_value(state.h()));
  }

  static void create_measures(const std::shared_ptr<sycl::queue>& queue,
			      const std::shared_ptr<TimeParameters<TimeType>>& tparams,
			      const std::shared_ptr<MeshType>& mesh,
			      std::vector<std::shared_ptr<SaintVenantMeasure<TimeType,ValueType,MeshType>>>& measures)
  {
    const Config& mconf = GlobalConfig::instance().measure_configuration();
    auto m_crange = mconf.equal_range("h-point");
    for (auto it = m_crange.first; it != m_crange.second; ++it) {
      measures.push_back(std::make_shared<SaintVenantHPointMeasure<TT,T,Mesh>>(queue, tparams,
									       mesh, it->second));
    }
  }
  
};

#endif
