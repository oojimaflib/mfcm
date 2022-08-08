/***********************************************************************
 * mfcm SaintVenant/Boundaries/BoundarySourceTerm.hpp
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

#ifndef mfcm_SaintVenant_Boundaries_BoundarySourceTerm_hpp
#define mfcm_SaintVenant_Boundaries_BoundarySourceTerm_hpp

#include "../SourceTerm.hpp"

template<typename TT,
	 typename T,
	 typename Mesh,
	 typename Kernel>
class BoundarySourceTerm : public SaintVenantSourceTerm<TT,T,Mesh>
{
public:

  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;
  using FieldType = CellField<ValueType,MeshType>;
  using SelectionType = MeshSelection<MeshType,MeshComponent::Cell>;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;

private:

  std::shared_ptr<MeshType> mesh_;

  FieldType xbdy0_;
  FieldType xbdy1_;

protected:

  const std::shared_ptr<MeshType>& mesh(void) const { return mesh_; }

  FieldType& xbdy0(void) { return xbdy0_; }
  FieldType& xbdy1(void) { return xbdy1_; }

  void clear_values(void)
  {
    xbdy0_ = 0.0;
    xbdy1_ = 0.0;
  }
  
  void add_values(const ValueType& v0, const ValueType& v1,
		  const SelectionType& sel)
  {
    std::cout << "Adding " << v0 << " to xbdy0." << std::endl;
    BinaryFieldCompoundAssignmentOperator<ValueType,MeshType,MeshComponent::Cell,std::plus<ValueType>>::apply(xbdy0_, v0, sel);
    BinaryFieldCompoundAssignmentOperator<ValueType,MeshType,MeshComponent::Cell,std::plus<ValueType>>::apply(xbdy1_, v1, sel);
  }
  
  void add_values(const FieldType& v0, const FieldType& v1,
		  const SelectionType& sel)
  {
    BinaryFieldCompoundAssignmentOperator<ValueType,MeshType,MeshComponent::Cell,std::plus<ValueType>>::apply(xbdy0_, v0, sel);
    BinaryFieldCompoundAssignmentOperator<ValueType,MeshType,MeshComponent::Cell,std::plus<ValueType>>::apply(xbdy1_, v1, sel);
  }

  void set_values(const ValueType& v0, const ValueType& v1,
		  const SelectionType& sel)
  {
    CastFieldOperator<ValueType,ValueType,MeshType,MeshComponent::Cell>::apply(xbdy0_, v0, sel);
    CastFieldOperator<ValueType,ValueType,MeshType,MeshComponent::Cell>::apply(xbdy1_, v1, sel);
  }
  
  void set_values(const FieldType& v0, const FieldType& v1,
		  const SelectionType& sel)
  {
    CastFieldOperator<ValueType,ValueType,MeshType,MeshComponent::Cell>::apply(xbdy0_, v0, sel);
    CastFieldOperator<ValueType,ValueType,MeshType,MeshComponent::Cell>::apply(xbdy1_, v1, sel);
  }
  
public:

  BoundarySourceTerm(const std::shared_ptr<MeshType>& mesh,
		     const FieldType&& xbdy0,
		     const FieldType&& xbdy1)
    : SaintVenantSourceTerm<TimeType,ValueType,MeshType>(),
      mesh_(mesh), xbdy0_(xbdy0), xbdy1_(xbdy1)
  {
  }

  virtual ~BoundarySourceTerm(void)
  {}

  virtual void apply(State& U, Constants& constants, State& dUdt,
		     const TimeType& timestep, const TimeType& time_now,
		     const std::shared_ptr<TimeParameters<TimeType>>& tp_ptr)
  {
    size_t ncells = mesh_->template object_count<MeshComponent::Cell>();
    mesh_->queue_ptr()->submit([&] (sycl::handler& cgh) {
      auto kernel = Kernel(cgh, U,
			   xbdy0_, xbdy1_,
			   dUdt, timestep, time_now,
			   tp_ptr->step_duration());
      cgh.parallel_for(sycl::range<1>(ncells), kernel);
    });
  }
  
};

#endif
