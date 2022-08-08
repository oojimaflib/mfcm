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

template<typename Mesh, MeshComponent FieldMapping>
MeshSelection<Mesh, FieldMapping>::
MeshSelection(const std::shared_ptr<MeshType>& mesh_p,
	      const Config& conf)
  : mesh_p_(mesh_p),
    list_(mesh_p->queue_ptr(), 0)
{
  std::vector<size_t>& id_list = list_.host_vector();
  size_t idmax = mesh_p_->template object_count<FieldMappingType>();

  // Parse the configuration to get a selection. Empty config must
  // equal global selection
  std::string sel_type_str = conf.get_value<std::string>("global");
  if (sel_type_str == "global" or sel_type_str == "") {
    // A global selection is indicated by list_ containing two
    // elements that are the mesh's object count.
    id_list.push_back(idmax);
    id_list.push_back(idmax);
    list_.move_to_device();
    return;
  } else if (sel_type_str == "id list") {
    // A selection via the cell/face/vertex IDs
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
  } else {
    std::cerr << "Unknown mesh selection method: "
	      << sel_type_str << std::endl;
    throw std::runtime_error("Unknown mesh selection method.");
  }

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
