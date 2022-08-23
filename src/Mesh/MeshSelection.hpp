/***********************************************************************
 * mfcm Mesh/MeshSelection.hpp
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

#ifndef mfcm_Mesh_MeshSelection_hpp
#define mfcm_Mesh_MeshSelection_hpp

#include "Mesh.hpp"
#include "DataArray.hpp"
#include "Config.hpp"
#include "Geometry.hpp"

template<typename Mesh, MeshComponent FieldMapping>
class MeshSelectionAccessor;

template<typename Mesh, MeshComponent FieldMapping>
class MeshSelection
{
public:

  using MeshType = Mesh;
  static const MeshComponent FieldMappingType = FieldMapping;

  using Accessor = MeshSelectionAccessor<MeshType, FieldMappingType>;
  
private:

  void initialize_global_(void);
  
  void initialize_mask_(void);
  void update_mask_from_point_(const std::shared_ptr<Point>& pt);
  void update_mask_from_geometry_(const std::shared_ptr<Geometry>& geom_ptr);
  
  void finalize_id_list_(void);
  
  std::shared_ptr<MeshType> mesh_p_;

  DataArray<size_t> list_;

protected:

  friend class MeshSelectionAccessor<MeshType, FieldMappingType>;

  const DataArray<size_t>& list(void) const { return list_; }
  
public:
  
  MeshSelection(const std::shared_ptr<MeshType>& mesh_p,
		const Config& conf = Config());

  const std::shared_ptr<MeshType>& mesh(void) const
  {
    return mesh_p_;
  }
  
  /**
     Returns true if all of the objects in the mesh should be selected.
   */
  bool is_global(void) const;

  /**
     Returns the number of mesh objects selected.
   */
  size_t size(void) const;

  const std::vector<size_t>& get_id_list(void) const
  {
    // list_.move_to_host();
    const std::vector<size_t>& list_cref = list_.host_vector();
    // list_.move_to_device();
    return list_cref;
  }

  ///**
  //   Returns a pointer to the array of selected objects.
  // */
  //  std::shared_ptr<DataArray<size_t>> list_ptr(void) const { return list_; }

};

template<typename Mesh, MeshComponent FieldMapping>
class MeshSelectionAccessor
{
private:

  using ListAccessor = typename DataArray<size_t>::
    template Accessor<sycl::access::mode::read,
		      sycl::access::target::global_buffer,
		      sycl::access::placeholder::true_t>;

  ListAccessor list_ro_;
  
public:

  MeshSelectionAccessor(const MeshSelection<Mesh,FieldMapping>& ms)
    : list_ro_(ms.list().template get_placeholder_accessor<sycl::access::mode::read,sycl::access::target::global_buffer>())
  {}

  void bind(sycl::handler& cgh)
  {
    cgh.require(list_ro_);
  }

  size_t operator()(const size_t& id) const
  {
    if (list_ro_.size() == 2 and list_ro_[0] == list_ro_[1]) {
      // Global selection
      return id;
    } else if (list_ro_.size() > 2 and
	       list_ro_[list_ro_.size() - 1] == list_ro_[list_ro_.size() - 2]) {
      // The list is actually a mask
      if (list_ro_[id] > 0) {
	return id;
      } else {
	return list_ro_[list_ro_.size() - 1];
      }
    } else {
      // List selection
      return list_ro_[id];
    }
  }
  
};


#endif
