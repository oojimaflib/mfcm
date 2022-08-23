/***********************************************************************
 * mfcm Geometry/Geometry.hpp
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

#ifndef mfcm_Geometry_Geometry_hpp
#define mfcm_Geometry_Geometry_hpp

#include <string>

#include "Config.hpp"

/**
   \defgroup Geometry Geometric Primitives
   @{
 */

/**
   Base class for geometric objects. Geometric objects can be up to
   four-dimensional with a required X and Y coordinate and optional Z
   and M coordinates.
 */
class Geometry
{
private:

  bool has_z_;
  bool has_m_;

protected:

  /**
     Returns a string representation of the geometry data.
   */
  virtual std::string inner_text(void) const = 0;
  
public:

  /**
     Enumeration of valid geometric object types.
   */
  enum class Type {
    point,
    multipoint,
    linestring,
    multilinestring,
    polygon,
    multipolygon,
    collection
  };

  /**
     Constructor.

     @param has_z True if the object has elevation coordinates, false
     otherwise.
     @param has_m True if the object has "measure" coordinates, false
     otherwise.
   */
  Geometry(bool has_z = false, bool has_m = false)
    : has_z_(has_z), has_m_(has_m)
  {}

  /**
     Destructor.
   */
  virtual ~Geometry(void) {}

  /**
     String representation of the type of geometry.
   */
  virtual std::string type_str(void) const = 0;
  /**
     The type of geometry represented by this object.
  */
  virtual Type type(void) const = 0;

  /**
     Returns a string representation of the geometry in the
     Well-Known-Text (WKT) format.

     This function returns type_str() + "(" + inner_text() + ")"
   */
  std::string wkt(void) const
  {
    std::string s = this->type_str() + " ( ";
    s += this->inner_text();
    s += " )";
    return s;
  }

  /**
     Returns true if the geometry has elevation coordinates.
  */
  bool has_z(void) const { return has_z_; }
  /**
     Returns true if the geometry has measure coordinates.
  */
  bool has_m(void) const { return has_m_; }

  /**
     Assert that this geometry has (not) elevation coordinates.

     @param has_z True if this geometry has elevation coordinates,
     false otherwise.
   */
  virtual void set_has_z(bool has_z)
  {
    has_z_ = has_z;
  }
  /**
     Assert that this geometry has (not) elevation coordinates.

     @param has_m True if this geometry has measure coordinates,
     false otherwise.
   */
  virtual void set_has_m(bool has_m)
  {
    has_m_ = has_m;
  }
};

/**
   Geometry object representing a point.
 */
class Point : public Geometry,
	      public std::vector<double>
{
public:

  /**
     Default constructor. Makes a 2D point at (0.0, 0.0).
   */
  Point(void)
    : Geometry(false, false),
      std::vector<double>({0.0,0.0})
  {
    // std::cout << "Calling default" << std::endl;
  }

  /**
     Construct from an X and Y coordinate.

     @param x X coordinate of point.
     @param y Y coordinate of point.
   */
  Point(const double& x,
	const double& y)
    : Geometry(false, false),
      std::vector<double>({x, y})
  {}

  /**
     Construct a 3D point.

     @param x X coordinate of point.
     @param y Y coordinate of point.
     @param z_or_m Z or M coordinate of point.
     @param xyz If true, #z_or_m is interpreted as a Z coordinate. If
     false #z_or_m is interpreted as an M coordinate.
   */
  Point(const double& x,
	const double& y,
	const double& z_or_m,
	bool xyz = true)
    : Geometry(xyz, !xyz),
      std::vector<double>({x, y, z_or_m})
  {}

  /**
     Construct a 4D point.

     @param x X coordinate of point.
     @param y Y coordinate of point.
     @param z Z coordinate of point.
     @param m M coordinate of point.
   */
  Point(const double& x,
	const double& y,
	const double& z,
	const double& m)
    : Geometry(true, true),
      std::vector<double>({x, y, z, m})
  {}

  /**
     Construct a point from a vector of coordinates.

     @param vec Vector of coordinates.
   */
  Point(const std::vector<double>& vec)
    : Geometry(false, false),
      std::vector<double>(vec)
  {
    assert(std::vector<double>::size() >= 2);
  }

  /**
     Construct a point from a vector of coordinates with optional
     setting of the has_z and has_m flags.
   */
  Point(const std::vector<double>& vec,
	boost::optional<char> zflag,
	boost::optional<char> mflag)
    : Geometry(bool(zflag), bool(mflag)), std::vector<double>(vec)
  {
    // std::cout << "Calling optional" << std::endl;
    // if (Geometry::has_z()) std::cout << "Has Z" << std::endl;
    // if (Geometry::has_m()) std::cout << "Has M" << std::endl;
      
    //     assert_correct_size();
  }

  /*
  Point(const double& x, const double& y)
    : Geometry(), std::vector<double>({x,y})
  {}
  */

  /**
     Destructor.
   */
  virtual ~Point(void) {}

  /**
     Return a string representation of the geometry type.
   */
  virtual std::string type_str(void) const
  {
    if (has_z() && has_m()) {
      return "Point ZM";
    } else if (has_z()) {
      return "Point Z";
    } else if (has_m()) {
      return "Point M";
    } else {
      return "Point";
    }
  }
  /**
     Return Geometry::Type::point.
   */
  virtual Geometry::Type type(void) const { return Geometry::Type::point; }

  /**
     Return the X coordinate.
   */
  double x(void) const { return this->operator[](0); }
  /**
     Return the Y coordinate.
   */
  double y(void) const { return this->operator[](1); }
  /**
     Return the Z coordinate or NaN if there is none.
   */
  double z(void) const
  {
    if (has_z()) return this->operator[](2);
    return std::numeric_limits<double>::quiet_NaN();
  }
  /**
     Return the M coordinate or NaN if there is none.
   */
  double m(void) const
  {
    if (has_m()) {
      if (has_z()) return this->operator[](3);
      return this->operator[](2);
    }
    return std::numeric_limits<double>::quiet_NaN();
  }

  /**
     Assert that the coordinate vector has the right number of
     coordinates based on the has_z and has_m flags.
   */
  void assert_correct_size(void) const
  {
    assert(std::vector<double>::size() == (2 + (has_z()?1:0) + (has_m()?1:0)));
  }

  /**
     Return the coordinates of the point as a string, space separated.
   */
  virtual std::string inner_text(void) const
  {
    std::string s = std::to_string(this->x())
      + " " + std::to_string(this->y());
    if (has_z()) s += " " + std::to_string(this->z());
    if (has_m()) s += " " + std::to_string(this->m());
    return s;
  }

  /**
     Return the X and Y coordinates of the point as a std::array.
   */
  std::array<double, 2> as_2d_array(void) const
  {
    return std::array<double, 2>({ x(), y() });
  }
  
};

/**
   Geometry object representing multiple disconnected points.
 */
class MultiPoint : public Geometry,
		   public std::vector<Point>
{
public:

  /**
     Default constructor. Creates a MultiPoint object with no points.
   */
  MultiPoint(void)
    : Geometry(), std::vector<Point>()
  {}
  /**
     Construct from a std::vector of Point objects.
   */
  MultiPoint(const std::vector<Point>& vec)
    : Geometry(), std::vector<Point>(vec)
  {}
  /**
     Construct from a std::vector of Point objects with optional
     setting of the has_z and has_m flags.
  */
  MultiPoint(const std::vector<Point>& vec,
	     boost::optional<char> zflag,
	     boost::optional<char> mflag)
    : Geometry(bool(zflag), bool(mflag)), std::vector<Point>(vec)
  {
    //std::cout << "Calling MP optional" << std::endl;
    //if (Geometry::has_z()) std::cout << "Has Z" << std::endl;
    //if (Geometry::has_m()) std::cout << "Has M" << std::endl;
    
    for (auto&& p : *this) {
      p.set_has_z(Geometry::has_z());
      p.set_has_m(Geometry::has_m());
      // p.assert_correct_size();
    }
  }

  /**
     Destructor.
  */
  virtual ~MultiPoint(void) {}

  /**
     Return the type of geometry as a string.
   */
  virtual std::string type_str(void) const { return "MultiPoint"; }
  /**
     Return Geometry::Type::multipoint.
   */
  virtual Geometry::Type type(void) const { return Geometry::Type::multipoint; }

  /**
     Returns a string with a comma-separated list of bracketed point
     data
   */
  virtual std::string inner_text(void) const
  {
    std::string s = "";
    bool first = true;
    for (auto&& p : *this) {
      if (first) {
	first = false;
      } else {
	s += ", ";
      }
      s += "( " + p.inner_text() + " )";
    }
    return s;
  }
  
};

/**
   Geometry object representing a series of connected points.
 */
class LineString : public MultiPoint
{
public:

  /**
     Default constructor. Creates no points.
   */
  LineString(void)
    : MultiPoint()
  {}
  /**
     Construct from a MultiPoint object by connecting the points in
     order.
   */
  LineString(const MultiPoint& mp)
    : MultiPoint(mp)
  {}
  /**
     Construct from a vector of Point objects.
   */
  LineString(const std::vector<Point>& vec)
    : MultiPoint(vec)
  {}
  /**
     Construct from a vector of Point objects with optional setting of
     the has_z and has_m flags.
  */
  LineString(const std::vector<Point>& vec,
	     boost::optional<char> zflag,
	     boost::optional<char> mflag)
    : MultiPoint(vec, zflag, mflag)
  {}

  /**
     Destructor.
  */
  virtual ~LineString(void) {}

  /**
     Return the type of geometry as a string.
   */
  virtual std::string type_str(void) const { return "LineString"; }
  /**
     Return Geometry::Type::linestring.
   */
  virtual Geometry::Type type(void) const { return Geometry::Type::linestring; }

  /**
     Returns a string with a comma-separated list of point data
   */
  virtual std::string inner_text(void) const
  {
    std::string s = "";
    bool first = true;
    for (auto&& p : *this) {
      if (first) {
	first = false;
      } else {
	s += ", ";
      }
      s += p.inner_text();
    }
    return s;
  }  

};

/**
   Geometry object representing a series of disconnected linestrings.
 */
class MultiLineString : public Geometry,
			public std::vector<LineString>
{
public:

  /**
     Default constructor. Creates a MultiLineString with no LineString
     objects.
   */
  MultiLineString(void)
    : Geometry(), std::vector<LineString>()
  {}
  /**
     Construct from a vector of LineString objects.
  */
  MultiLineString(const std::vector<LineString>& vec)
    : Geometry(), std::vector<LineString>(vec)
  {}
  /**
     Construct from a vector of LineString objects with optional
     setting of the has_z and has_m flags.
  */
  MultiLineString(const std::vector<LineString>& vec,
		  boost::optional<char> zflag,
		  boost::optional<char> mflag)
    : Geometry(bool(zflag), bool(mflag)), std::vector<LineString>(vec)
  {
    for (auto&& ls : *this) {
      ls.set_has_z(Geometry::has_z());
      ls.set_has_m(Geometry::has_m());
      //ls.assert_correct_size();
    }
  }

  /**
     Destructor
  */
  virtual ~MultiLineString(void) {}

  /**
     Return the type of geometry as a string.
  */
  virtual std::string type_str(void) const { return "MultiLineString"; }
  /**
     Return Geometry::Type::multilinestring.
  */
  virtual Geometry::Type type(void) const { return Geometry::Type::multilinestring; }

  /**
     Returns a string with a comma-separated list of bracketed
     linestring data
   */
  virtual std::string inner_text(void) const
  {
    std::string s = "";
    bool first = true;
    for (auto&& ls : *this) {
      if (first) {
	first = false;
      } else {
	s += ", ";
      }
      s += "( " + ls.inner_text() + " )";
    }
    return s;
  }
  
};

/**
   Geometry object representing a polygon with zero or more holes.
 */
class Polygon : public MultiLineString
{
public:

  /**
     Default constructor. Creates a Polygon with no perimeter object.
   */
  Polygon(void)
    : MultiLineString()
  {}
  /**
     Construct from a MultiLineString object.
  */
  Polygon(const MultiLineString& mls)
    : MultiLineString(mls)
  {}
  /**
     Construct from a vector of LineString objects.
  */
  Polygon(const std::vector<LineString>& vec)
    : MultiLineString(vec)
  {}
  /**
     Construct from a vector of LineString objects with optional
     setting of the has_z and has_m flags.
  */
  Polygon(const std::vector<LineString>& vec,
	  boost::optional<char> zflag,
	  boost::optional<char> mflag)
    : MultiLineString(vec, bool(zflag), bool(mflag))
  {
    for (auto&& ls : *this) {
      ls.set_has_z(Geometry::has_z());
      ls.set_has_m(Geometry::has_m());
      //ls.assert_correct_size();
    }
  }

  /** 
      Destructor.
  */
  virtual ~Polygon(void) {}

  /**
     Return the type of geometry as a string.
  */
  virtual std::string type_str(void) const { return "Polygon"; }
  /**
     Return Geometry::Type::polygon.
  */
  virtual Geometry::Type type(void) const { return Geometry::Type::polygon; }
  
};
  
/**
   Geometry object representing multiple polygons, each with zero or
   more holes.
 */
class MultiPolygon : public Geometry,
		     public std::vector<Polygon>
{
public:
  
  /**
     Default constructor. Creates a MultiPolygon with no polygons.
   */
  MultiPolygon(void)
    : Geometry(), std::vector<Polygon>()
  {}
  /**
     Construct from a vector of Polygon objects.
  */
  MultiPolygon(const std::vector<Polygon>& vec)
    : Geometry(), std::vector<Polygon>(vec)
  {}
  /**
     Construct from a vector of Polygon objects with optional setting
     of the has_z and has_m flags.
  */
  MultiPolygon(const std::vector<Polygon>& vec,
	       boost::optional<char> zflag,
	       boost::optional<char> mflag)
    : Geometry(bool(zflag), bool(mflag)), std::vector<Polygon>(vec)
  {
    for (auto&& pg : *this) {
      pg.set_has_z(Geometry::has_z());
      pg.set_has_m(Geometry::has_m());
    }
  }

  /** 
      Destructor.
  */
  virtual ~MultiPolygon(void) {}

  /**
     Return the type of geometry as a string.
  */
  virtual std::string type_str(void) const { return "MultiPolygon"; }
  /**
     Return Geometry::Type::multipolygon.
  */
  virtual Geometry::Type type(void) const { return Geometry::Type::multipolygon; }

  /**
     Not implemented.
   */
  virtual std::string inner_text(void) const { return "Not implemented"; }  

};

class GeometryCollection;

void read_wkt_geometry(const std::string& str, GeometryCollection& gc);
void read_gdal_geometry(const Config& config, GeometryCollection& gc);

class GeometryCollection : public Geometry,
			   public std::vector<std::shared_ptr<Geometry>>
{
public:

  GeometryCollection(void)
    : Geometry(), std::vector<std::shared_ptr<Geometry>>()
  {}
  GeometryCollection(const std::vector<std::shared_ptr<Geometry>>& vec)
    : Geometry(), std::vector<std::shared_ptr<Geometry>>(vec)
  {}

  GeometryCollection(const Config& config);
  
  virtual ~GeometryCollection(void) {}
  
  virtual std::string type_str(void) const { return "GeometryCollection"; }
  virtual Geometry::Type type(void) const { return Geometry::Type::collection; }

  virtual std::string inner_text(void) const { return "Not implemented"; }  

};

/*
void read_wkt_geometry(const std::string& str, GeometryCollection& gc);

void read_gdal_geometry(const Config& config, GeometryCollection& gc);
*/

/**
   @}
*/

#endif
