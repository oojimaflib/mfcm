/***********************************************************************
 * mfcm Mesh/Cartesian2DMesh.cpp
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
#include "Cartesian2DMesh.hpp"
#include "Config.hpp"
#include <cmath>

Cartesian2DMesh::
Cartesian2DMesh(const std::shared_ptr<sycl::queue>& queue,
		bool on_device)
  : ncells_(queue, 2),
    geotrans_(queue, 12)
{
  const Config& conf = GlobalConfig::instance().mesh_configuration();
  
  std::array<size_t,2> user_ncells =
    split_string<size_t,2>(conf.get<std::string>("cell count"));
  ncells_.host_vector() = { user_ncells[0], user_ncells[1] };
  
  std::array<double,2> user_origin =
    split_string<double,2>(conf.get<std::string>("origin", "0.0, 0.0"));
  std::array<double,2> user_cellsize =
    split_string<double,2>(conf.get<std::string>("cell size"));
  double rotation = conf.get<double>("angle", 0.0);
  
  std::vector<double>& gtvec = geotrans_.host_vector();
  gtvec[0] = user_origin[0];                        // a
  gtvec[1] = user_cellsize[0] * std::cos(rotation); // b
  gtvec[2] = user_cellsize[0] * std::sin(rotation); // c
  gtvec[3] = user_origin[1];                        // d
  gtvec[4] = user_cellsize[1] * std::sin(rotation); // e
  gtvec[5] = user_cellsize[1] * std::cos(rotation);  // f

  gtvec[6] = 1.0 / gtvec.at(1); // 6: 1/b
  gtvec[7] = 1.0 / gtvec.at(5); // 7: 1/f
  gtvec[8] = 1.0 / (gtvec.at(1) * gtvec.at(5)); // 8: 1/(fb)
  gtvec[9] = 1.0 - (gtvec.at(2) * gtvec.at(4)) * gtvec.at(8); // 9: 1 - (ce)/(fb)
  gtvec[10] = std::sqrt(gtvec[1] * gtvec[1] + gtvec[2] * gtvec[2]);
  gtvec[11] = std::sqrt(gtvec[4] * gtvec[4] + gtvec[5] * gtvec[5]);

  if (on_device) {
    ncells_.move_to_device();
    geotrans_.move_to_device();
  }
}

Cartesian2DMesh::
Cartesian2DMesh(const std::shared_ptr<sycl::queue>& queue,
		const std::array<size_t,2>& ncells,
		const std::array<double,6>& geo_transform,
		bool on_device)
  : ncells_(queue, 2),
    geotrans_(queue, 12)
{
  std::vector<size_t> nc = ncells_.host_vector();
  nc.at(0) = ncells.at(0);
  nc.at(1) = ncells.at(1);

  std::vector<double>& gtvec = geotrans_.host_vector();
  gtvec[0] = geo_transform.at(0);
  gtvec[1] = geo_transform.at(1);
  gtvec[2] = geo_transform.at(2);
  gtvec[3] = geo_transform.at(3);
  gtvec[4] = geo_transform.at(4);
  gtvec[5] = geo_transform.at(5);
  gtvec[6] = 1.0 / gtvec.at(1); // 6: 1/b
  gtvec[7] = 1.0 / gtvec.at(5); // 7: 1/f
  gtvec[8] = 1.0 / (gtvec.at(1) * gtvec.at(5)); // 8: 1/(fb)
  gtvec[9] = 1.0 - (gtvec.at(2) * gtvec.at(4)) * gtvec.at(8); // 9: 1 - (ce)/(fb)
  gtvec[10] = std::sqrt(gtvec[1] * gtvec[1] + gtvec[2] * gtvec[2]);
  gtvec[11] = std::sqrt(gtvec[4] * gtvec[4] + gtvec[5] * gtvec[5]);

  if (on_device) {
    ncells_.move_to_device();
    geotrans_.move_to_device();
  }
}

void Cartesian2DMesh::move_to_device(void)
{
  ncells_.move_to_device();
  geotrans_.move_to_device();
}

void Cartesian2DMesh::move_to_host(void)
{
  ncells_.move_to_host();
  geotrans_.move_to_host();
}

Cartesian2DMeshAccessor::
Cartesian2DMeshAccessor(const Cartesian2DMesh& c2m)
  : ncells_ro_(c2m.ncells_data().template get_placeholder_accessor<sycl::access::mode::read,sycl::access::target::global_buffer>()),
    geotrans_ro_(c2m.geotrans_data().template get_placeholder_accessor<sycl::access::mode::read,sycl::access::target::global_buffer>())
{}

void Cartesian2DMeshAccessor::bind(sycl::handler& cgh)
{
  cgh.require(ncells_ro_);
  cgh.require(geotrans_ro_);
}
