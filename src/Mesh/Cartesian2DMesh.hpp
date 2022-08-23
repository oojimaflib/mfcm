/***********************************************************************
 * mfcm Mesh/Cartesian2DMesh.hpp
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

#ifndef mfcm_Mesh_Cartesian2DMesh_hpp
#define mfcm_Mesh_Cartesian2DMesh_hpp

#include "Mesh.hpp"
#include "DataArray.hpp"
#include "../Geometry/Geometry.hpp"

class Cartesian2DMeshAccessor;

class Cartesian2DMesh
{
public:

  using Accessor = Cartesian2DMeshAccessor;
  
private:

  DataArray<size_t> ncells_;

  DataArray<double> geotrans_;

  inline const size_t& nxcells(void) const
  {
    return ncells_.host_vector().at(0);
  }
  inline const size_t& nycells(void) const
  {
    return ncells_.host_vector().at(1);
  }

  inline const double& origin_x(void) const { return geotrans_.host_vector()[0]; }
  inline const double& cell_width(void) const { return geotrans_.host_vector()[1]; }
  inline const double& row_rotation(void) const { return geotrans_.host_vector()[2]; }
  inline const double& origin_y(void) const { return geotrans_.host_vector()[3]; }
  inline const double& col_rotation(void) const { return geotrans_.host_vector()[4]; }
  inline const double& cell_height(void) const { return geotrans_.host_vector()[5]; }
  inline const double& inv_cell_width(void) const { return geotrans_.host_vector()[6]; }
  inline const double& inv_cell_height(void) const { return geotrans_.host_vector()[7]; }
  inline const double& inv_cell_size(void) const { return geotrans_.host_vector()[8]; }
  inline const double& inv_denom(void) const { return geotrans_.host_vector()[9]; }
  
public:

  Cartesian2DMesh(const std::shared_ptr<sycl::queue>& queue,
		  bool on_device = true);

  Cartesian2DMesh(const std::shared_ptr<sycl::queue>& queue,
		  const std::array<size_t,2>& ncells,
		  const std::array<double,6>& geo_transform,
		  bool on_device = true);

  ~Cartesian2DMesh(void)
  {
    std::cout << "Freeing memory for mesh." << std::endl;
  }

  const std::shared_ptr<sycl::queue>& queue_ptr(void)
  {
    return ncells_.queue_ptr();
  }

  bool is_on_device(void)
  {
    return ncells_.is_on_device();
  }
  
  void move_to_device(void);

  void move_to_host(void);

  template<MeshComponent C>
  inline size_t object_count(void) const;

  template<>
  inline size_t object_count<MeshComponent::Cell>(void) const
  {
    return nxcells() * nycells();
  }
  
  template<>
  inline size_t object_count<MeshComponent::Face>(void) const
  {
    return (nxcells() + 1) * nycells() +
      (nycells() + 1) * nxcells();
  }
  
  template<>
  inline size_t object_count<MeshComponent::Vertex>(void) const
  {
    return (nxcells() + 1) * (nycells() + 1);
  }

  /**
     Return the physical location corresponding to a sub-cell
     coordinate in the mesh.
  */
  inline std::array<double,2> get_location(const std::array<double,2>& ic) const
  {
    return { origin_x() + ic[0] * cell_width() + ic[1] * row_rotation(),
      origin_y() + ic[0] * col_rotation() + ic[1] * cell_height() };
  }

  /**
     Return the sub-cell coordinate in the mesh corresponding to a
     physical location.
  */
  inline std::array<double,2> get_coordinate(const std::array<double,2>& c) const
  {
    return
      {
	((c[0] - origin_x()) * inv_cell_width() -
	 (c[1] - origin_y()) * row_rotation() * inv_cell_size()) * inv_denom(),
	((c[1] - origin_y()) * inv_cell_height() -
	 (c[0] - origin_x()) * col_rotation() * inv_cell_size()) * inv_denom()
      };
  }

  template<MeshComponent C>
  inline std::array<double,2> get_object_location(const size_t& i) const;

  template<>
  inline std::array<double,2>
  get_object_location<MeshComponent::Cell>(const size_t& i) const
  {
    double xi = (double)(i % nxcells()) + 0.5;
    double yi = (double)(i / nxcells()) + 0.5;
    return get_location({xi, yi});
  }

  template<>
  inline std::array<double,2>
  get_object_location<MeshComponent::Face>(const size_t& i) const
  {
    if (i < (nxcells() + 1) * nycells()) {
      // Face is vertical and has cells to left and right
      double xi = (double)(i % (nxcells() + 1));
      double yi = (double)(i / (nxcells() + 1)) + 0.5;
      return get_location({xi, yi});
    } else {
      // Face is horizontal
      size_t i2 = i - ((nxcells() + 1) * nycells());
      double xi = (double)(i2 % nxcells()) + 0.5;
      double yi = (double)(i2 / nxcells());
      return get_location({xi, yi});
    }
  }

  template<>
  inline std::array<double,2>
  get_object_location<MeshComponent::Vertex>(const size_t& i) const
  {
    double xi = (double)(i % (nxcells() + 1));
    double yi = (double)(i / (nxcells() + 1));    
    return get_location({xi, yi});
  }

  template<MeshComponent C>
  size_t get_nearest_object_index(const std::array<double,2>& loc) const;

  template<>
  size_t get_nearest_object_index<MeshComponent::Cell>(const std::array<double,2>& loc) const
  {
    std::array<double,2> coord = get_coordinate(loc);
    if (coord[0] >= 0.0 and coord[0] < nxcells() and
	coord[1] >= 0.0 and coord[1] < nycells()) {
      return ((size_t)coord[1]) * nxcells() + (size_t)coord[0];
    } else {
      return nxcells() * nycells();
    }
  }

protected:

  friend class Cartesian2DMeshAccessor;

  const DataArray<size_t>& ncells_data(void) const
  {
    return ncells_;
  }

  const DataArray<double>& geotrans_data(void) const
  {
    return geotrans_;
  }
  
};

class Cartesian2DMeshAccessor
{
private:

  using NCAccessor =
    typename DataArray<size_t>::
    template Accessor<sycl::access::mode::read,
		      sycl::access::target::global_buffer,
		      sycl::access::placeholder::true_t>;

  using GTAccessor =
    typename DataArray<double>::
    template Accessor<sycl::access::mode::read,
		      sycl::access::target::global_buffer,
		      sycl::access::placeholder::true_t>;

  NCAccessor ncells_ro_;

  GTAccessor geotrans_ro_;

  inline const size_t& nxcells(void) const { return ncells_ro_[0]; }
  inline const size_t& nycells(void) const { return ncells_ro_[1]; }
  
  inline const double& origin_x(void) const { return geotrans_ro_[0]; }
  inline const double& cell_width(void) const { return geotrans_ro_[1]; }
  inline const double& row_rotation(void) const { return geotrans_ro_[2]; }
  inline const double& origin_y(void) const { return geotrans_ro_[3]; }
  inline const double& col_rotation(void) const { return geotrans_ro_[4]; }
  inline const double& cell_height(void) const { return geotrans_ro_[5]; }
  inline const double& inv_cell_width(void) const { return geotrans_ro_[6]; }
  inline const double& inv_cell_height(void) const { return geotrans_ro_[7]; }
  inline const double& inv_cell_size(void) const { return geotrans_ro_[8]; }
  inline const double& inv_denom(void) const { return geotrans_ro_[9]; }

public:

  inline const double& dx(void) const { return geotrans_ro_[10]; }
  inline const double& dy(void) const { return geotrans_ro_[11]; }

  Cartesian2DMeshAccessor(const Cartesian2DMesh& c2m);

  void bind(sycl::handler& cgh);

  template<MeshComponent C>
  inline size_t object_count(void) const;

  template<>
  inline size_t object_count<MeshComponent::Cell>(void) const
  {
    return nxcells() * nycells();
  }
  
  template<>
  inline size_t object_count<MeshComponent::Face>(void) const
  {
    return (nxcells() + 1) * nycells() +
      (nycells() + 1) * nxcells();
  }
  
  template<>
  inline size_t object_count<MeshComponent::Vertex>(void) const
  {
    return (nxcells() + 1) * (nycells() + 1);
  }

  /**
     Return the physical location corresponding to a sub-cell
     coordinate in the mesh.
  */
  inline std::array<double,2> get_location(const std::array<double,2>& ic) const
  {
    return { origin_x() + ic[0] * cell_width() + ic[1] * row_rotation(),
      origin_y() + ic[0] * col_rotation() + ic[1] * cell_height() };
  }

  /**
     Return the sub-cell coordinate in the mesh corresponding to a
     physical location.
  */
  inline std::array<double,2> get_coordinate(const std::array<double,2>& c) const
  {
    return
      {
	((c[0] - origin_x()) * inv_cell_width() -
	 (c[1] - origin_y()) * row_rotation() * inv_cell_size()) * inv_denom(),
	((c[1] - origin_y()) * inv_cell_height() -
	 (c[0] - origin_x()) * col_rotation() * inv_cell_size()) * inv_denom()
      };
  }

  inline double cell_area(const size_t& i) const
  {
    return dx() * dy();
  }
  
  template<MeshComponent C>
  inline std::array<double,2> get_object_location(const size_t& i) const;

  template<>
  inline std::array<double,2>
  get_object_location<MeshComponent::Cell>(const size_t& i) const
  {
    double xi = (double)(i % nxcells()) + 0.5;
    double yi = (double)(i / nxcells()) + 0.5;
    return get_location({xi, yi});
  }

  template<>
  inline std::array<double,2>
  get_object_location<MeshComponent::Face>(const size_t& i) const
  {
    if (i < (nxcells() + 1) * nycells()) {
      // Face is vertical and has cells to left and right
      double xi = (double)(i % (nxcells() + 1));
      double yi = (double)(i / (nxcells() + 1)) + 0.5;
      return get_location({xi, yi});
    } else {
      // Face is horizontal
      size_t i2 = i - ((nxcells() + 1) * nycells());
      double xi = (double)(i2 % nxcells()) + 0.5;
      double yi = (double)(i2 / nxcells());
      return get_location({xi, yi});
    }
  }

  template<>
  inline std::array<double,2>
  get_object_location<MeshComponent::Vertex>(const size_t& i) const
  {
    double xi = (double)(i % (nxcells() + 1));
    double yi = (double)(i / (nxcells() + 1));    
    return get_location({xi, yi});
  }

  template<MeshComponent C>
  size_t get_nearest_object_index(const std::array<double,2>& loc) const;

  template<>
  size_t get_nearest_object_index<MeshComponent::Cell>(const std::array<double,2>& loc) const
  {
    std::array<double,2> coord = get_coordinate(loc);
    if (coord[0] >= 0.0 and coord[0] < nxcells() and
	coord[1] >= 0.0 and coord[1] < nycells()) {
      return ((size_t)coord[1]) * nxcells() + (size_t)coord[0];
    } else {
      return nxcells() * nycells();
    }
  }

  template<>
  size_t get_nearest_object_index<MeshComponent::Face>(const std::array<double,2>& loc) const
  {
    // TODO
    return nxcells() * nycells();
  }

  template<>
  size_t get_nearest_object_index<MeshComponent::Vertex>(const std::array<double,2>& loc) const
  {
    // TODO
    return nxcells() * nycells();
  }


  struct get_adjacent_cells_result
  {
    size_t lhs_id;
    size_t rhs_id;
    int edge;
    int dir;
    double dx;
  };

  get_adjacent_cells_result get_adjacent_cells(const size_t& face_id) const
  {
    get_adjacent_cells_result result;

    if (face_id < (nxcells() + 1) * nycells()) {
      // Face is vertical and has cells to the left and right.
      result.dir = 0;
      result.dx = dx();
      size_t fxid = face_id % (nxcells() + 1);
      size_t fyid = face_id / (nxcells() + 1);

      if (fxid < nxcells()) {
	result.rhs_id = fyid * nxcells() + fxid;
	if (fxid > 0) {
	  // Mid-row
	  result.edge = 0;
	  result.lhs_id = fyid * nxcells() + (fxid - 1);
	} else {
	  // Left hand edge of row
	  result.edge = -1;
	  result.lhs_id = result.rhs_id;
	}
      } else {
	// Right-hand edge of row
	result.lhs_id = fyid * nxcells() + (fxid - 1);
	result.rhs_id = result.lhs_id;
	result.edge = 1;
      }
    } else {
      // Face is horizontal and has cells above and below
      result.dir = 1;
      result.dx = dy();
      size_t f2id = face_id - (nxcells() + 1) * nycells();
      size_t fxid = f2id % nxcells();
      size_t fyid = f2id / nxcells();

      if (fyid < nycells()) {
	result.rhs_id = fyid * nxcells() + fxid;
	if (fyid > 0) {
	  // Mid-column
	  result.edge = 0;
	  result.lhs_id = (fyid - 1) * nxcells() + fxid;
	} else {
	  // Bottom of column
	  result.edge = -1;
	  result.lhs_id = result.rhs_id;
	}
      } else {
	// Top of column
	result.lhs_id = (fyid - 1) * nxcells() + fxid;
	result.rhs_id = result.lhs_id;
	result.edge = 1;
      }
    }

    return result;
  }

  struct get_adjacent_faces_result
  {
    size_t face_w;
    size_t face_e;
    double dx;
    size_t face_s;
    size_t face_n;
    double dy;
  };

  get_adjacent_faces_result get_adjacent_faces(const size_t& cell_id) const
  {
    size_t cxid = cell_id % nxcells();
    size_t cyid = cell_id / nxcells();

    get_adjacent_faces_result result;
    result.face_w = cyid * (nxcells() + 1) + cxid;
    result.face_e = result.face_w + 1;
    result.dx = dx();
    result.face_s = (nxcells() + 1) * nycells() + cyid * nxcells() + cxid;
    result.face_n = result.face_s + nxcells();
    result.dy = dy();
    return result;
  }
  
  struct offset_type
  {
    size_t i;
    double dx;
  };
  
  template<MeshComponent C>
  offset_type get_object_west(const size_t& i) const;
  template<MeshComponent C>
  offset_type get_object_east(const size_t& i) const;
  template<MeshComponent C>
  offset_type get_object_north(const size_t& i) const;
  template<MeshComponent C>
  offset_type get_object_south(const size_t& i) const;

  template<>
  offset_type get_object_west<MeshComponent::Cell>(const size_t& i) const
  {
    size_t xi = i % nxcells();
    if (xi > 0) {
      return { i-1, dx() };
    } else {
      return { i, 0.0 };
    }
  }
  
  template<>
  offset_type get_object_east<MeshComponent::Cell>(const size_t& i) const
  {
    size_t xi = i % nxcells();
    if (xi < nxcells() - 1) {
      return { i+1, dx() };
    } else {
      return { i, 0.0 };
    }
  }
  
  template<>
  offset_type get_object_north<MeshComponent::Cell>(const size_t& i) const
  {
    size_t yi = i / nxcells();
    if (yi < nycells() - 1) {
      return { i+nxcells(), dy() };
    } else {
      return { i, 0.0 };
    }
  }
  
  template<>
  offset_type get_object_south<MeshComponent::Cell>(const size_t& i) const
  {
    size_t yi = i / nxcells();
    if (yi > 0) {
      return { i-nxcells(), dy() };
    } else {
      return { i, 0.0 };
    }
  }

};

#endif
