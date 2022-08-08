/***********************************************************************
 * mflow.cpp
 *
 * Copyright (C) Gerald C J Morgan 2022
 ***********************************************************************/

#include "Config/Config.hpp"
  
#include "Mesh/Cartesian2DMesh.hpp"
#include "SaintVenant/Solver.hpp"
#include "TemporalScheme/RungeKutta.hpp"

using ValueType = float;
using TimeType = ValueType;
using MeshType = Cartesian2DMesh;
using SolverType = SaintVenantSolver<TimeType,ValueType,MeshType>;
  

template<typename Solver>
std::shared_ptr<TemporalScheme> create_scheme(const std::shared_ptr<sycl::queue>& queue)
{
  const Config& conf = GlobalConfig::instance().scheme_configuration();

  std::string scheme_type_str = conf.get_value<std::string>("runge-kutta");
  if (scheme_type_str == "runge-kutta") {
    auto tparams = std::make_shared<RungeKuttaTimeParameters<typename Solver::TimeType>>(conf);
    std::string method_type_str = conf.get<std::string>("method", "classic");
    return std::make_shared<RungeKuttaSolver<Solver>>(tparams,
						      method_type_str,
						      queue);
  }

  std::cerr << "Unknown scheme type: "
	    << std::quoted(scheme_type_str) << std::endl;
  throw std::runtime_error("Unknown scheme type.");
}

int main(int argc, char* argv[])
{
  std::locale loc;
  GlobalConfig::init(argc, argv);

  std::cout << "Initialised global configuration" << std::endl;

  auto scheme_ptr = create_scheme<SolverType>(get_sycl_queue());
  scheme_ptr->solve();
  
  return 0;
}
