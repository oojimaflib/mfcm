/***********************************************************************
 * mfcm Field/FieldVector.cpp
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

#include "FieldVector.hpp"

template<typename Field, size_t N>
FieldVector<Field,N>::FieldVector(void)
  : std::vector<FieldType>()
{}

template<typename Field, size_t N>
FieldVector<Field,N>::
FieldVector(const std::shared_ptr<sycl::queue>& queue,
	    const std::array<std::string,N>& names,
	    const std::shared_ptr<MeshType>& mesh_p,
	    const ValueType& init_value,
	    bool on_device)
  : std::vector<FieldType>()
{
  for (size_t i = 0; i < N; ++i) {
    this->template emplace_back(queue, names.at(i),
				mesh_p, init_value, on_device);
  }
}

template<typename Field, size_t N>
FieldVector<Field,N>::
FieldVector(const std::initializer_list<FieldType>& il)
  : std::vector<FieldType>(il)
{
  //    assert(std::vector<CellField<T,MeshDefn>>::size() == N);
}

template<typename Field, size_t N>
FieldVector<Field,N>::
FieldVector(const std::string& prefix,
	    const FieldVector<FieldType,N>& cfv,
	    const std::string& suffix)
  : std::vector<FieldType>()
{
  for (size_t i = 0; i < N; ++i) {
    this->template emplace_back(prefix, cfv.at(i), suffix);
  }
}

template<typename Field, size_t N>
void FieldVector<Field,N>::move_to_device(void)
{
  for (auto&& cf : *this) {
    cf.move_to_device();
  }
}

template<typename Field, size_t N>
void FieldVector<Field,N>::move_to_host(void)
{
  for (auto&& cf : *this) {
    cf.move_to_host();
  }
}

template<typename Field, size_t N,
	 sycl::access::mode Mode, sycl::access::target Target>
struct FieldAccessorBuilder {
  using ValueType = typename Field::ValueType;
  using DataAccessor = typename DataArray<ValueType>::
    template Accessor<Mode,Target,sycl::access::placeholder::true_t>;
  
  std::array<DataAccessor,N> operator()(const FieldVector<Field,N>& fv);
};

template<typename Field,
	 sycl::access::mode Mode, sycl::access::target Target>
struct FieldAccessorBuilder<Field,2,Mode,Target> {
  using ValueType = typename Field::ValueType;
  using DataAccessor = typename DataArray<ValueType>::
    template Accessor<Mode,Target,sycl::access::placeholder::true_t>;
  
  std::array<DataAccessor,2> operator()(const FieldVector<Field,2>& fv)
  {
    return { fv[0].data().template get_placeholder_accessor<Mode,Target>(),
      fv[1].data().template get_placeholder_accessor<Mode,Target>() };
  }
};

template<typename Field,
	 sycl::access::mode Mode, sycl::access::target Target>
struct FieldAccessorBuilder<Field,3,Mode,Target> {
  using ValueType = typename Field::ValueType;
  using DataAccessor = typename DataArray<ValueType>::
    template Accessor<Mode,Target,sycl::access::placeholder::true_t>;
  
  std::array<DataAccessor,3> operator()(const FieldVector<Field,3>& fv)
  {
    return { fv[0].data().template get_placeholder_accessor<Mode,Target>(),
      fv[1].data().template get_placeholder_accessor<Mode,Target>(),
      fv[2].data().template get_placeholder_accessor<Mode,Target>() };
  }
};

template<typename Field,
	 sycl::access::mode Mode, sycl::access::target Target>
struct FieldAccessorBuilder<Field,4,Mode,Target> {
  using ValueType = typename Field::ValueType;
  using DataAccessor = typename DataArray<ValueType>::
    template Accessor<Mode,Target,sycl::access::placeholder::true_t>;
  
  std::array<DataAccessor,4> operator()(const FieldVector<Field,4>& fv)
  {
    return { fv[0].data().template get_placeholder_accessor<Mode,Target>(),
      fv[1].data().template get_placeholder_accessor<Mode,Target>(),
      fv[2].data().template get_placeholder_accessor<Mode,Target>(),
      fv[3].data().template get_placeholder_accessor<Mode,Target>() };
  }
};

template<typename Field,
	 sycl::access::mode Mode, sycl::access::target Target>
struct FieldAccessorBuilder<Field,5,Mode,Target> {
  using ValueType = typename Field::ValueType;
  using DataAccessor = typename DataArray<ValueType>::
    template Accessor<Mode,Target,sycl::access::placeholder::true_t>;
  
  std::array<DataAccessor,5> operator()(const FieldVector<Field,5>& fv)
  {
    return { fv[0].data().template get_placeholder_accessor<Mode,Target>(),
      fv[1].data().template get_placeholder_accessor<Mode,Target>(),
      fv[2].data().template get_placeholder_accessor<Mode,Target>(),
      fv[3].data().template get_placeholder_accessor<Mode,Target>(),
      fv[4].data().template get_placeholder_accessor<Mode,Target>() };
  }
};

template<typename Field,
	 sycl::access::mode Mode, sycl::access::target Target>
struct FieldAccessorBuilder<Field,6,Mode,Target> {
  using ValueType = typename Field::ValueType;
  using DataAccessor = typename DataArray<ValueType>::
    template Accessor<Mode,Target,sycl::access::placeholder::true_t>;
  
  std::array<DataAccessor,6> operator()(const FieldVector<Field,6>& fv)
  {
    return { fv[0].data().template get_placeholder_accessor<Mode,Target>(),
      fv[1].data().template get_placeholder_accessor<Mode,Target>(),
      fv[2].data().template get_placeholder_accessor<Mode,Target>(),
      fv[3].data().template get_placeholder_accessor<Mode,Target>(),
      fv[4].data().template get_placeholder_accessor<Mode,Target>(),
      fv[5].data().template get_placeholder_accessor<Mode,Target>() };
  }
};

template<typename Field,
	 size_t N,
	 sycl::access::mode Mode,
	 sycl::access::target Target>
FieldVectorAccessor<Field,N,Mode,Target>::
FieldVectorAccessor(const FieldVector<Field,N>& fv)
  : mesh_ro_(MeshAccessor(*(fv[0].mesh()))),
    data_acc_(FieldAccessorBuilder<Field,N,Mode,Target>()(fv))
{
}

template<typename Field,
	 size_t N,
	 sycl::access::mode Mode,
	 sycl::access::target Target>
void FieldVectorAccessor<Field,N,Mode,Target>::bind(sycl::handler& cgh)
{
  mesh_ro_.bind(cgh);
  for (auto&& da : data_acc_) {
    cgh.require(da);
  }  
}
