/***********************************************************************
 * mfcm Mesh/MeshSelection.cpp
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

#include "MeshSelection.hpp"
#include "../Geometry/GeometryDataArray.hpp"

template<typename Mesh, MeshComponent FieldMapping>
void MeshSelection<Mesh, FieldMapping>::
initialize_global_(void)
{
  // A global selection is indicated by list_ containing two
  // elements that are the mesh's object count.
  std::vector<size_t>& id_list = list_.host_vector();
  size_t idmax = mesh_p_->template object_count<FieldMappingType>();
  id_list.push_back(idmax);
  id_list.push_back(idmax);
  list_.move_to_device();
}

template<typename Mesh, MeshComponent FieldMapping>
void MeshSelection<Mesh, FieldMapping>::
initialize_mask_(void)
{
  // A mask selection is indicated by list_ having size N + 2 where N
  // is the total number of objects in the mesh. The last two elements
  // contain N.
  std::vector<size_t>& mask = list_.host_vector();
  size_t idmax = mesh_p_->template object_count<FieldMappingType>();
  mask.clear();
  mask.resize(idmax + 2);
  mask.at(idmax) = idmax;
  mask.at(idmax + 1) = idmax;
  list_.move_to_device();
}

template<typename Mesh, MeshComponent FieldMapping>
class UpdateMaskFromPointKernel
{
public:

  using MeshType = Mesh;
  using MeshAccessor = typename MeshType::Accessor;
  using DataAccessor = typename DataArray<size_t>::
    template Accessor<sycl::access::mode::read_write,
		      sycl::access::target::global_buffer>;
  using PDAAccessor = PointDataArrayAccessor<sycl::access::mode::read,
					     sycl::access::target::global_buffer>;
  
private:

  std::shared_ptr<sycl::queue> queue_;
  
  MeshAccessor mesh_ro_;

  DataAccessor list_acc_;

  PDAAccessor pda_acc_;

public:
  
  UpdateMaskFromPointKernel(sycl::handler& cgh,
			    std::shared_ptr<MeshType>& mesh_ptr,
			    DataArray<size_t>& list,
			    PointDataArray& pda)
    : queue_(mesh_ptr->queue_ptr()), mesh_ro_(*mesh_ptr),
      list_acc_(list.get_read_write_accessor(cgh)),
      pda_acc_(pda, cgh)
  {
    mesh_ro_.bind(cgh);
  }

  void operator()(sycl::item<1> item) const
  {
    size_t i = item.get_linear_id();
    size_t n_obj = mesh_ro_.template object_count<FieldMapping>();
    if (i < n_obj) {
      std::array<double,2> location = { pda_acc_.x(), pda_acc_.y() };
      size_t j = mesh_ro_.template get_nearest_object_index<FieldMapping>(location);
      if (i == j) {
	list_acc_[item] = 1;
      }
    }
  }
};

template<typename Mesh, MeshComponent FieldMapping>
void MeshSelection<Mesh,FieldMapping>::
update_mask_from_point_(const std::shared_ptr<Point>& pt)
{
  PointDataArray pda(mesh_p_->queue_ptr(), pt);
  mesh_p_->queue_ptr()->submit([&](sycl::handler& cgh)
  {
    auto kernel = UpdateMaskFromPointKernel<Mesh,FieldMapping>
      (cgh, mesh_p_, list_, pda);
    cgh.parallel_for(sycl::range<1>(list_.size()), kernel);
  });
}

template<typename Mesh, MeshComponent FieldMapping>
void MeshSelection<Mesh,FieldMapping>::
update_mask_from_geometry_(const std::shared_ptr<Geometry>& geom_ptr)
{
  switch (geom_ptr->type()) {
  case Geometry::Type::point:
    update_mask_from_point_(std::dynamic_pointer_cast<Point>(geom_ptr));
    return;
  default:
    std::cerr << "Cannot update mask with geometry of type: "
	      << geom_ptr->type_str() << std::endl;
    throw std::runtime_error("Cannot update mask with this type of geometry.");
  };
}

template<typename Mesh, MeshComponent FieldMapping>
void MeshSelection<Mesh, FieldMapping>::
finalize_id_list_(void)
{
  size_t idmax = mesh_p_->template object_count<FieldMappingType>();
  std::vector<size_t>& id_list = list_.host_vector();
  // Sort the selected mesh component ids
  std::sort(id_list.begin(), id_list.end());
  // Remove duplicates
  auto last = std::unique(id_list.begin(), id_list.end());
  id_list.erase(last, id_list.end());
  // Add the end element (the mesh's object count)
  id_list.push_back(idmax);
  // Put the list on the device
  list_.move_to_device();
}

template<typename Mesh, MeshComponent FieldMapping>
MeshSelection<Mesh, FieldMapping>::
MeshSelection(const std::shared_ptr<MeshType>& mesh_p,
	      const Config& conf)
  : mesh_p_(mesh_p),
    list_(mesh_p->queue_ptr(), 0)
{
  // Parse the configuration to get a selection. Empty config must
  // equal global selection
  std::string sel_type_str = conf.get_value<std::string>("global");
  if (sel_type_str == "global" or sel_type_str == "") {
    initialize_global_();
  } else if (sel_type_str == "id list") {
    // A selection via the cell/face/vertex IDs
    std::vector<size_t>& id_list = list_.host_vector();
    auto single_range = conf.equal_range("id");
    for (auto it = single_range.first; it != single_range.second; ++it) {
      id_list.push_back(it->second.get_value<size_t>());
    }
    auto multi_range = conf.equal_range("ids");
    for (auto it = multi_range.first; it != multi_range.second; ++it) {
      auto ids = split_string<size_t>(it->second.get_value<std::string>());
      for (auto&& id : ids) {
	id_list.push_back(id);
      }
    }
    finalize_id_list_();
  } else if (sel_type_str == "at") {
    GeometryCollection gc(conf);
    initialize_mask_();
    for (auto&& geom_ptr : gc) {
      update_mask_from_geometry_(geom_ptr);
    }
    /*
  } else if (sel_type_str == "at point") {
    GeometryCollection gc(conf);
    for (auto&& geom_ptr : gc) {
      if (geom_ptr->type() == Geometry::Type::point) {
	PointDataArray pda(mesh_p->queue_ptr(),
			   std::dynamic_pointer_cast<Point>(geom_ptr));
      }
    }
    */
  } else {
    std::cerr << "Unknown mesh selection method: "
	      << sel_type_str << std::endl;
    throw std::runtime_error("Unknown mesh selection method.");
  }

}

template<typename Mesh, MeshComponent FieldMapping>
bool MeshSelection<Mesh, FieldMapping>::is_global(void) const
{
  if (list_.size() == 2 and
      list_.host_vector().at(0) == list_.host_vector().at(1)) {
    return true;
  }
  return false;
}

template<typename Mesh, MeshComponent FieldMapping>
size_t MeshSelection<Mesh, FieldMapping>::size(void) const
{
  if (is_global()) {
    return mesh_p_->template object_count<FieldMappingType>();
  }
  return list_.size() - 1;
}
