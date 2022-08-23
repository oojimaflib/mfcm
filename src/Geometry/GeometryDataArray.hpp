/***********************************************************************
 * mfcm Geometry/GeometryDataArray.hpp
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

#ifndef mfcm_Geometry_GeometryDataArray_hpp
#define mfcm_Geometry_GeometryDataArray_hpp

#include "DataArray.hpp"
#include "../Geometry/Geometry.hpp"

class GeometryDataArray : public DataArray<double>
{
protected:

  bool has_z_;
  bool has_m_;
  
public:

  GeometryDataArray(const std::shared_ptr<sycl::queue>& queue,
		    const std::shared_ptr<std::vector<double>>& values,
		    bool has_z = false,
		    bool has_m = false,
		    bool on_device = true)
    : DataArray(queue, *values, on_device)
  {}

  GeometryDataArray(const std::shared_ptr<sycl::queue>& queue,
		    const std::vector<double>& values,
		    bool has_z = false,
		    bool has_m = false,
		    bool on_device = true)
    : DataArray(queue, values, on_device)
  {}

  const bool has_z(void) const { return has_z_; }
  const bool has_m(void) const { return has_m_; }
  
};

template<sycl::access::mode Mode,
	 sycl::access::target Target = sycl::access::target::global_buffer>
class PointDataArrayAccessor;

class PointDataArray : public GeometryDataArray
{
public:

  PointDataArray(const std::shared_ptr<sycl::queue>& queue,
		 const std::shared_ptr<Point>& pt,
		 bool on_device = true)
    : GeometryDataArray(queue, pt, pt->has_z(), pt->has_m(), on_device)
  {}
  
};

template<sycl::access::mode Mode,
	 sycl::access::target Target>
class PointDataArrayAccessor
{
private:

  bool has_z_;
  bool has_m_;
  
  using DataAccessor = typename DataArray<double>::
    template Accessor<Mode, Target, sycl::access::placeholder::true_t>;

  DataAccessor data_acc_;

public:

  PointDataArrayAccessor(const PointDataArray& pda);

  PointDataArrayAccessor(const PointDataArray& pda,
			 sycl::handler& cgh);

  void bind(sycl::handler& cgh);

  double x(void) const
  {
    return data_acc_[0];
  }

  double y(void) const
  {
    return data_acc_[1];
  }

  double z(void) const
  {
    if (has_z_) return data_acc_[2];
    return std::numeric_limits<double>::quiet_NaN();
  }

  double m(void) const
  {
    if (has_m_) {
      if (has_z_) return data_acc_[3];
      return data_acc_[2];
    }
    return std::numeric_limits<double>::quiet_NaN();
  }
  
  const std::array<double, 2> operator()(void) const
  {
    return { data_acc_[0], data_acc_[1] };
  }
  
};

template<sycl::access::mode Mode,
	 sycl::access::target Target = sycl::access::target::global_buffer>
class MultiPointDataArrayAccessor;

class MultiPointDataArray : public GeometryDataArray
{
private:

  static std::vector<double> get_data(const std::shared_ptr<MultiPoint>& mpt)
  {
    std::vector<double> result;
    for (auto&& pt : *mpt) {
      result.push_back(pt.x());
      result.push_back(pt.y());
      if (mpt->has_z()) result.push_back(pt.z());
      if (mpt->has_m()) result.push_back(pt.m());
    }
    return result;
  }
  
public:

  MultiPointDataArray(const std::shared_ptr<sycl::queue>& queue,
		      const std::shared_ptr<MultiPoint>& mpt,
		      bool on_device = true)
    : GeometryDataArray(queue, MultiPointDataArray::get_data(mpt),
			mpt->has_z(), mpt->has_m(), on_device)
  {}
  
};

template<sycl::access::mode Mode,
	 sycl::access::target Target>
class MultiPointDataArrayAccessor
{
private:

  bool has_z_;
  bool has_m_;
  
  using DataAccessor = typename DataArray<double>::
    template Accessor<Mode, Target, sycl::access::placeholder::true_t>;

  DataAccessor data_acc_;

  inline size_t stride(void) const
  {
    return 2 + (has_z_ ? 1 : 0) + (has_m_ ? 1 : 0);
  }
  
public:

  MultiPointDataArrayAccessor(const MultiPointDataArray& pda);

  MultiPointDataArrayAccessor(const MultiPointDataArray& pda,
			      sycl::handler& cgh);

  void bind(sycl::handler& cgh);
  
  double x(const size_t& i) const
  {
    return data_acc_[i * stride()];
  }

  double y(const size_t& i) const
  {
    return data_acc_[i * stride() + 1];
  }

  double z(const size_t& i) const
  {
    if (has_z_) return data_acc_[i * stride() + 2];
    return std::numeric_limits<double>::quiet_NaN();
  }

  double m(const size_t& i) const
  {
    if (has_m_) {
      if (has_z_) return data_acc_[i * stride() + 3];
      return data_acc_[i * stride() + 2];
    }
    return std::numeric_limits<double>::quiet_NaN();
  }
  
  const std::array<double, 2> operator()(const size_t& i) const
  {
    return { data_acc_[0], data_acc_[1] };
  }
    
};

template<sycl::access::mode Mode,
	 sycl::access::target Target = sycl::access::target::global_buffer>
class LineStringDataArrayAccessor;

class LineStringDataArray : public GeometryDataArray
{
private:

  static std::vector<double> get_data(const std::shared_ptr<LineString>& ls)
  {
    std::vector<double> result;
    for (auto&& pt : *ls) {
      result.push_back(pt.x());
      result.push_back(pt.y());
      if (ls->has_z()) result.push_back(pt.z());
      if (ls->has_m()) result.push_back(pt.m());
    }
    return result;
  }
  
public:

  LineStringDataArray(const std::shared_ptr<sycl::queue>& queue,
		      const std::shared_ptr<LineString>& ls,
		      bool on_device = true)
    : GeometryDataArray(queue, LineStringDataArray::get_data(ls),
			ls->has_z(), ls->has_m(), on_device)
  {}
  
};

template<sycl::access::mode Mode,
	 sycl::access::target Target>
class LineStringDataArrayAccessor
{
private:

  bool has_z_;
  bool has_m_;
  
  using DataAccessor = typename DataArray<double>::
    template Accessor<Mode, Target, sycl::access::placeholder::true_t>;

  DataAccessor data_acc_;

  inline size_t stride(void) const
  {
    return 2 + (has_z_ ? 1 : 0) + (has_m_ ? 1 : 0);
  }
  
public:

  LineStringDataArrayAccessor(const LineStringDataArray& pda);

  LineStringDataArrayAccessor(const LineStringDataArray& pda,
			      sycl::handler& cgh);

  void bind(sycl::handler& cgh);
  
  double x(const size_t& i) const
  {
    return data_acc_[i * stride()];
  }

  double y(const size_t& i) const
  {
    return data_acc_[i * stride() + 1];
  }

  double z(const size_t& i) const
  {
    if (has_z_) return data_acc_[i * stride() + 2];
    return std::numeric_limits<double>::quiet_NaN();
  }

  double m(const size_t& i) const
  {
    if (has_m_) {
      if (has_z_) return data_acc_[i * stride() + 3];
      return data_acc_[i * stride() + 2];
    }
    return std::numeric_limits<double>::quiet_NaN();
  }
  
  const std::array<double, 2> operator()(const size_t& i) const
  {
    return { data_acc_[0], data_acc_[1] };
  }
    
};

template<sycl::access::mode Mode,
	 sycl::access::target Target = sycl::access::target::global_buffer>
class MultiLineStringDataArrayAccessor;

class MultiLineStringDataArray : public GeometryDataArray
{
private:

  static std::vector<double> get_data(const std::shared_ptr<MultiLineString>& mls)
  {
    std::vector<double> result;
    for (auto&& ls : *mls) {
      for (auto&& pt : ls) {
	result.push_back(pt.x());
	result.push_back(pt.y());
	if (mls->has_z()) result.push_back(pt.z());
	if (mls->has_m()) result.push_back(pt.m());
      }
      if (ls.at(0) != ls.at(ls.size()-1)) {
	result.push_back(ls.at(0).x());
	result.push_back(ls.at(0).y());
	if (mls->has_z()) result.push_back(ls.at(0).z());
	if (mls->has_m()) result.push_back(ls.at(0).m());
      }
    }
    return result;
  }
  
public:

  MultiLineStringDataArray(const std::shared_ptr<sycl::queue>& queue,
			   const std::shared_ptr<MultiLineString>& mls,
			   bool on_device = true)
    : GeometryDataArray(queue, MultiLineStringDataArray::get_data(mls),
			mls->has_z(), mls->has_m(), on_device)
  {}
  
};

template<sycl::access::mode Mode,
	 sycl::access::target Target>
class MultiLineStringDataArrayAccessor
{
private:

  bool has_z_;
  bool has_m_;
  
  using DataAccessor = typename DataArray<double>::
    template Accessor<Mode, Target, sycl::access::placeholder::true_t>;

  DataAccessor data_acc_;

  inline size_t stride(void) const
  {
    return 2 + (has_z_ ? 1 : 0) + (has_m_ ? 1 : 0);
  }
  
public:

  MultiLineStringDataArrayAccessor(const MultiLineStringDataArray& pda);

  MultiLineStringDataArrayAccessor(const MultiLineStringDataArray& pda,
				   sycl::handler& cgh);

  void bind(sycl::handler& cgh);
  
  double x(const size_t& i) const
  {
    return data_acc_[i * stride()];
  }

  double y(const size_t& i) const
  {
    return data_acc_[i * stride() + 1];
  }

  double z(const size_t& i) const
  {
    if (has_z_) return data_acc_[i * stride() + 2];
    return std::numeric_limits<double>::quiet_NaN();
  }

  double m(const size_t& i) const
  {
    if (has_m_) {
      if (has_z_) return data_acc_[i * stride() + 3];
      return data_acc_[i * stride() + 2];
    }
    return std::numeric_limits<double>::quiet_NaN();
  }
  
  const std::array<double, 2> operator()(const size_t& i) const
  {
    return { data_acc_[0], data_acc_[1] };
  }
    
};

template<sycl::access::mode Mode,
	 sycl::access::target Target = sycl::access::target::global_buffer>
class PolygonDataArrayAccessor;

class PolygonDataArray : public GeometryDataArray
{
private:

  static std::vector<double> get_data(const std::shared_ptr<Polygon>& pgn)
  {
    std::vector<double> result;
    for (auto&& ls : *pgn) {
      for (auto&& pt : ls) {
	result.push_back(pt.x());
	result.push_back(pt.y());
	if (pgn->has_z()) result.push_back(pt.z());
	if (pgn->has_m()) result.push_back(pt.m());
      }
      if (ls.at(0) != ls.at(ls.size()-1)) {
	result.push_back(ls.at(0).x());
	result.push_back(ls.at(0).y());
	if (pgn->has_z()) result.push_back(ls.at(0).z());
	if (pgn->has_m()) result.push_back(ls.at(0).m());
      }
    }
    return result;
  }
  
public:

  PolygonDataArray(const std::shared_ptr<sycl::queue>& queue,
		   const std::shared_ptr<Polygon>& pgn,
		   bool on_device = true)
    : GeometryDataArray(queue, PolygonDataArray::get_data(pgn),
			pgn->has_z(), pgn->has_m(), on_device)
  {}
  
};

template<sycl::access::mode Mode,
	 sycl::access::target Target>
class PolygonDataArrayAccessor
{
private:

  bool has_z_;
  bool has_m_;
  
  using DataAccessor = typename DataArray<double>::
    template Accessor<Mode, Target, sycl::access::placeholder::true_t>;

  DataAccessor data_acc_;

  inline size_t stride(void) const
  {
    return 2 + (has_z_ ? 1 : 0) + (has_m_ ? 1 : 0);
  }
  
public:

  PolygonDataArrayAccessor(const PolygonDataArray& pda);

  PolygonDataArrayAccessor(const PolygonDataArray& pda,
			   sycl::handler& cgh);

  void bind(sycl::handler& cgh);
  
  double x(const size_t& i) const
  {
    return data_acc_[i * stride()];
  }

  double y(const size_t& i) const
  {
    return data_acc_[i * stride() + 1];
  }

  double z(const size_t& i) const
  {
    if (has_z_) return data_acc_[i * stride() + 2];
    return std::numeric_limits<double>::quiet_NaN();
  }

  double m(const size_t& i) const
  {
    if (has_m_) {
      if (has_z_) return data_acc_[i * stride() + 3];
      return data_acc_[i * stride() + 2];
    }
    return std::numeric_limits<double>::quiet_NaN();
  }
  
  const std::array<double, 2> operator()(const size_t& i) const
  {
    return { data_acc_[0], data_acc_[1] };
  }
    
};

template<sycl::access::mode Mode,
	 sycl::access::target Target = sycl::access::target::global_buffer>
class MultiPolygonDataArrayAccessor;

class MultiPolygonDataArray : public GeometryDataArray
{
private:

  static std::vector<double> get_data(const std::shared_ptr<MultiPolygon>& mpgn)
  {
    std::vector<double> result;
    for (auto&& pgn : *mpgn) {
      for (auto&& ls : pgn) {
	for (auto&& pt : ls) {
	  result.push_back(pt.x());
	  result.push_back(pt.y());
	  if (mpgn->has_z()) result.push_back(pt.z());
	  if (mpgn->has_m()) result.push_back(pt.m());
	}
	if (ls.at(0) != ls.at(ls.size()-1)) {
	  result.push_back(ls.at(0).x());
	  result.push_back(ls.at(0).y());
	  if (mpgn->has_z()) result.push_back(ls.at(0).z());
	  if (mpgn->has_m()) result.push_back(ls.at(0).m());
	}
      }
      result.push_back(std::numeric_limits<double>::quiet_NaN());
    }
    return result;
  }
  
public:

  MultiPolygonDataArray(const std::shared_ptr<sycl::queue>& queue,
			const std::shared_ptr<MultiPolygon>& mpgn,
			bool on_device = true)
    : GeometryDataArray(queue, MultiPolygonDataArray::get_data(mpgn),
			mpgn->has_z(), mpgn->has_m(), on_device)
  {}
  
};

template<sycl::access::mode Mode,
	 sycl::access::target Target>
class MultiPolygonDataArrayAccessor
{
private:

  bool has_z_;
  bool has_m_;
  
  using DataAccessor = typename DataArray<double>::
    template Accessor<Mode, Target, sycl::access::placeholder::true_t>;

  DataAccessor data_acc_;

  inline size_t stride(void) const
  {
    return 2 + (has_z_ ? 1 : 0) + (has_m_ ? 1 : 0);
  }
  
public:

  MultiPolygonDataArrayAccessor(const MultiPolygonDataArray& mpda);

  MultiPolygonDataArrayAccessor(const MultiPolygonDataArray& mpda,
				sycl::handler& cgh);

  void bind(sycl::handler& cgh);
  
  double x(const size_t& i) const
  {
    return data_acc_[i * stride()];
  }

  double y(const size_t& i) const
  {
    return data_acc_[i * stride() + 1];
  }

  double z(const size_t& i) const
  {
    if (has_z_) return data_acc_[i * stride() + 2];
    return std::numeric_limits<double>::quiet_NaN();
  }

  double m(const size_t& i) const
  {
    if (has_m_) {
      if (has_z_) return data_acc_[i * stride() + 3];
      return data_acc_[i * stride() + 2];
    }
    return std::numeric_limits<double>::quiet_NaN();
  }
  
  const std::array<double, 2> operator()(const size_t& i) const
  {
    return { data_acc_[0], data_acc_[1] };
  }
    
};

#endif
