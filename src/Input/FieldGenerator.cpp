/***********************************************************************
 * mfcm Input/FieldGenerator.cpp
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

#include "FieldGenerator.hpp"
#include <charconv>

#include "Raster.hpp"

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
void
FieldGenerator<T,Mesh,FieldMapping>::
load_constant_input_field(Field<T,Mesh,FieldMapping>& input_field,
			  const MeshSelection<Mesh,FieldMapping>& selection,
			  const T& value)
{

  BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,std::multiplies<T>>
    ::apply(input_field, 0.0, selection);
  BinaryFieldCompoundAssignmentOperator<T,Mesh,FieldMapping,std::plus<T>>
    ::apply(input_field, value, selection);
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
void
FieldGenerator<T,Mesh,FieldMapping>::
load_sloped_input_field(Field<T,Mesh,FieldMapping>& input_field,
			const MeshSelection<Mesh,FieldMapping>& selection,
			const std::array<double,3>& origin,
			const std::array<double,2>& slope)
{
  if (not selection.is_global()) {
    throw std::runtime_error("Sloped input field only supported with global selection.");
  }
  input_field.move_to_host();
  const auto& mesh_ptr = input_field.mesh();
  for (size_t i = 0; i < mesh_ptr->template object_count<FieldMapping>(); ++i) {
    auto loc = mesh_ptr->template get_object_location<FieldMapping>(i);
    double dx = loc[0] - origin[0];
    double dy = loc[1] - origin[1];
    T v = origin[2] + slope[0] * dx + slope[1] * dy;
    input_field.set_value(i, v);
  }
  input_field.move_to_device();
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
void
FieldGenerator<T,Mesh,FieldMapping>::
load_raster_input_field(Field<T,Mesh,FieldMapping>& input_field,
			const MeshSelection<Mesh,FieldMapping>& selection,
			const std::string& name)
{
  auto& rdb = RasterDatabase<T>::instance();
  auto rf = rdb.get_raster_field_ptr(input_field.queue_ptr(), name);
  MapFieldOperator<T,Cartesian2DMesh,Mesh,
		   MeshComponent::Cell,FieldMapping>::
    apply(*rf, input_field, selection);
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
void
FieldGenerator<T,Mesh,FieldMapping>::
load_input_field(Field<T,Mesh,FieldMapping>& input_field,
		 const Config& conf)
{
  using boost::algorithm::to_lower_copy;

  MeshSelection<Mesh,FieldMapping> selection(input_field.mesh(),
					     conf.get_child("within", Config()));

  std::string type_str = to_lower_copy(conf.get_value<std::string>());
  if (type_str == "constant") {
    load_constant_input_field(input_field, selection, conf.get<T>("value"));
  } else if (type_str == "slope") {
    std::array<double,3> origin = split_string<double,3>(conf.get<std::string>("origin"));
    std::array<double,2> slope = split_string<double,2>(conf.get<std::string>("slope"));
    load_sloped_input_field(input_field, selection, origin, slope);
  } else if (type_str == "raster") {
    load_raster_input_field(input_field, selection,
			    conf.get<std::string>("name"));
  } else {
    T value;
    auto [ptr, ec] { std::from_chars(type_str.data(),
				     type_str.data() + type_str.size(),
				     value) };
    if (ec == std::errc()) {
      load_constant_input_field(input_field, selection, value);
    } else {
      std::cerr << std::quoted(type_str) << " is not a known input field specifier and cannot be interpreted as a constant value." << std::endl;
      throw std::runtime_error("Unknown input field specifier.");
    }
  }
}

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
FieldGenerator<T,Mesh,FieldMapping>::
FieldGenerator(const std::shared_ptr<sycl::queue>& queue,
	       const std::string& name,
	       std::shared_ptr<Mesh>& mesh_p,
	       const T& init_value,
	       bool on_device)
  : field_(queue, name, mesh_p, init_value, on_device)
{
  if (!on_device) {
    field_.move_to_device();
  }
  const Config& conf = GlobalConfig::instance().field_configuration(name);
  std::cout << "Generating field '" << name << "'" << std::endl;
    
  for (auto&& kv : conf) {
    const std::string& key = kv.first;
    const Config& value_spec = kv.second;

    if (key == "set") {
      load_input_field(field_, value_spec);
    } else if (key == "add") {
      Field<T,Mesh,FieldMapping> input_field(queue, "input", mesh_p, 0.0);
      load_input_field(input_field, value_spec);
      field_ += input_field;
    } else if (key == "multiply") {
      Field<T,Mesh,FieldMapping> input_field(queue, "input", mesh_p, 1.0);
      load_input_field(input_field, value_spec);
      field_ *= input_field;
    } else {
      std::cerr << "Unknown field operation '" << key << "'." << std::endl;
      throw std::runtime_error("Unknown field operation.");
    }
  }
  if (!on_device) {
    field_.move_to_host();
  }
}
