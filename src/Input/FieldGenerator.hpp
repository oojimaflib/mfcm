/***********************************************************************
 * mfcm sycl.hpp
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

#ifndef mfcm_Input_FieldGenerator_hpp
#define mfcm_Input_FieldGenerator_hpp

#include "Config.hpp"
#include "Field.hpp"
#include "FieldOperators.hpp"

template<typename T,
	 typename Mesh,
	 MeshComponent FieldMapping>
class FieldGenerator
{
private:

  std::shared_ptr<Field<T,Mesh,FieldMapping>> field_;
  
  void load_constant_input_field(Field<T,Mesh,FieldMapping>& input_field,
				 const MeshSelection<Mesh,FieldMapping>& selection,
				 const T& value);
  
  void load_sloped_input_field(Field<T,Mesh,FieldMapping>& input_field,
			       const MeshSelection<Mesh,FieldMapping>& selection,
			       const std::array<double,3>& origin,
			       const std::array<double,2>& slope);
  
  void load_raster_input_field(Field<T,Mesh,FieldMapping>& input_field,
			       const MeshSelection<Mesh,FieldMapping>& selection,
			       const std::string& name);
  
  void load_input_field(Field<T,Mesh,FieldMapping>& input_field,
			const Config& conf);
  
public:

  FieldGenerator(const std::shared_ptr<sycl::queue>& queue,
		 const std::string& name,
		 std::shared_ptr<Mesh>& mesh_p,
		 const T& init_value,
		 bool on_device = true);

  const Field<T,Mesh,FieldMapping>& operator()(void)
  {
    return *field_;
  }
  
  const std::shared_ptr<Field<T,Mesh,FieldMapping>>& make_shared(void)
  {
    return field_;
  }
  
};

#endif
