/***********************************************************************
 * mfcm Geometry/GeometryParser.hpp
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

#ifndef mfcm_Geometry_GeometryParser_hpp
#define mfcm_Geometry_GeometryParser_hpp

#include <iostream>
#include <utility>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/phoenix.hpp>
#include <boost/phoenix/object/dynamic_cast.hpp>
#include <boost/phoenix/operator/self.hpp>
#include <boost/optional.hpp>

namespace {
  template <typename T>
  struct make_shared_f
  {
    template <typename... A> struct result 
    { typedef std::shared_ptr<T> type; };
    
    template <typename... A>
    typename result<A...>::type operator()(A&&... a) const {
      return std::make_shared<T>(std::forward<A>(a)...);
    }
  };
  
  template <typename T>
  using make_shared_ = boost::phoenix::function<make_shared_f<T> >;
}

typedef std::shared_ptr<Point> PointPtr;

namespace wkt_parser {
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;
  namespace phx = boost::phoenix;

  template<typename It>
  struct WKT : qi::grammar<It, GeometryCollection()>
  {
    qi::rule<It, Point()> point;
    qi::rule<It, Point()> point_text;
    qi::rule<It, Point()> point_tagged_text;
    qi::rule<It, std::shared_ptr<Point>()> point_ptr;

    qi::rule<It, MultiPoint()> multipoint;
    qi::rule<It, MultiPoint()> multipoint_text;
    qi::rule<It, MultiPoint()> multipoint_tagged_text;
    qi::rule<It, std::shared_ptr<MultiPoint>()> multipoint_ptr;
    
    qi::rule<It, LineString()> linestring;
    qi::rule<It, LineString()> linestring_text;
    qi::rule<It, LineString()> linestring_tagged_text;
    qi::rule<It, std::shared_ptr<LineString>()> linestring_ptr;
    
    qi::rule<It, MultiLineString()> multilinestring;
    qi::rule<It, MultiLineString()> multilinestring_text;
    qi::rule<It, MultiLineString()> multilinestring_tagged_text;
    qi::rule<It, std::shared_ptr<MultiLineString>()> multilinestring_ptr;

    qi::rule<It, Polygon()> polygon;
    qi::rule<It, Polygon()> polygon_text;
    qi::rule<It, Polygon()> polygon_tagged_text;
    qi::rule<It, std::shared_ptr<Polygon>()> polygon_ptr;
    
    qi::rule<It, MultiPolygon()> multipolygon;
    qi::rule<It, MultiPolygon()> multipolygon_text;
    qi::rule<It, MultiPolygon()> multipolygon_tagged_text;
    qi::rule<It, std::shared_ptr<MultiPolygon>()> multipolygon_ptr;

    qi::rule<It, std::shared_ptr<Geometry>> geometry_tagged_text;
    qi::rule<It, GeometryCollection()> start;

    WKT(void)
      : WKT::base_type(start)
    {
      point = (qi::double_ % qi::omit[+(qi::space)])[qi::_val = qi::_1];
      point_text %= (qi::lit('(') >>
		    qi::omit[*qi::space] >>
		    point >>
		    qi::omit[*qi::space] >>
		    qi::lit(')'));
      point_tagged_text = (ascii::no_case[qi::lit("point")] >>
			   -(qi::omit[*qi::space] >>
			     ascii::no_case[ascii::char_("z")]) >>
			   -(qi::omit[*qi::space] >>
			     ascii::no_case[ascii::char_("m")]) >>
			   qi::omit[*qi::space] >>
			   point_text)[qi::_val = phx::construct<Point>(qi::_3, qi::_1, qi::_2)];
      point_ptr = (point_tagged_text)[qi::_val = make_shared_<Point>()(qi::_1)];

      multipoint = (point_text % (*qi::space >>
				  ',' >>
				  *qi::space))[qi::_val = qi::_1];
      multipoint_text %= (qi::lit('(') >>
			  qi::omit[*qi::space] >>
			  multipoint >>
			  qi::omit[*qi::space] >>
			  qi::lit(')'));
      multipoint_tagged_text = (ascii::no_case[qi::lit("multipoint")] >>
				-(qi::omit[*qi::space] >>
				  ascii::no_case[ascii::char_("z")]) >>
				-(qi::omit[*qi::space] >>
				  ascii::no_case[ascii::char_("m")]) >>
				qi::omit[*qi::space] >>
				multipoint_text)
	[qi::_val = phx::construct<MultiPoint>(qi::_3, qi::_1, qi::_2)];
      multipoint_ptr = (multipoint_tagged_text)
	[qi::_val = make_shared_<MultiPoint>()(qi::_1)];
      
      linestring = (point % (*qi::space >>
			     ',' >>
			     *qi::space))[qi::_val = qi::_1];
      linestring_text %= (qi::lit('(') >>
			  qi::omit[*qi::space] >>
			  linestring >>
			  qi::omit[*qi::space] >>
			  qi::lit(')'));
      // linestring_tagged_text = (ascii::no_case[qi::lit("linestring")] >>
      // 				qi::omit[*qi::space] >>
      // 				linestring_text);
      linestring_tagged_text = (ascii::no_case[qi::lit("linestring")] >>
				-(qi::omit[*qi::space] >>
				  ascii::no_case[ascii::char_("z")]) >>
				-(qi::omit[*qi::space] >>
				  ascii::no_case[ascii::char_("m")]) >>
				qi::omit[*qi::space] >>
				linestring_text)
	[qi::_val = phx::construct<LineString>(qi::_3, qi::_1, qi::_2)];
      linestring_ptr = (linestring_tagged_text)
	[qi::_val = make_shared_<LineString>()(qi::_1)];

      multilinestring = (linestring_text % (*qi::space >>
					    ',' >>
					    *qi::space))[qi::_val = qi::_1];
      multilinestring_text %= (qi::lit('(') >>
			       qi::omit[*qi::space] >>
			       multilinestring >>
			       qi::omit[*qi::space] >>
			       qi::lit(')'));
      // multilinestring_tagged_text =
      // 	(ascii::no_case[qi::lit("multilinestring")] >>
      // 	 qi::omit[*qi::space] >>
      // 	 multilinestring_text);
      multilinestring_tagged_text =
	(ascii::no_case[qi::lit("multilinestring")] >>
	 -(qi::omit[*qi::space] >>
	   ascii::no_case[ascii::char_("z")]) >>
	 -(qi::omit[*qi::space] >>
	   ascii::no_case[ascii::char_("m")]) >>
	 qi::omit[*qi::space] >>
	 multilinestring_text)
	[qi::_val = phx::construct<MultiLineString>(qi::_3, qi::_1, qi::_2)];
      multilinestring_ptr = (multilinestring_tagged_text)
	[qi::_val = make_shared_<MultiLineString>()(qi::_1)];

      polygon = multilinestring;
      polygon_text = multilinestring_text;
      // polygon_tagged_text =
      // 	(ascii::no_case[qi::lit("polygon")] >>
      // 	 qi::omit[*qi::space] >>
      // 	 multilinestring_text);
      polygon_tagged_text =
	(ascii::no_case[qi::lit("polygon")] >>
	 -(qi::omit[*qi::space] >>
	   ascii::no_case[ascii::char_("z")]) >>
	 -(qi::omit[*qi::space] >>
	   ascii::no_case[ascii::char_("m")]) >>
	 qi::omit[*qi::space] >>
	 polygon_text)
	[qi::_val = phx::construct<Polygon>(qi::_3, qi::_1, qi::_2)];
      polygon_ptr = (polygon_tagged_text)
	[qi::_val = make_shared_<Polygon>()(qi::_1)];
      
      multipolygon = (polygon_text % (*qi::space >>
				      ',' >>
				      *qi::space))[qi::_val = qi::_1];
      multipolygon_text %= (qi::lit('(') >>
			    qi::omit[*qi::space] >>
			    multipolygon >>
			    qi::omit[*qi::space] >>
			    qi::lit(')'));
      // multipolygon_tagged_text =
      // 	(ascii::no_case[qi::lit("multipolygon")] >>
      // 	 qi::omit[*qi::space] >>
      // 	 multipolygon_text);
      multipolygon_tagged_text =
	(ascii::no_case[qi::lit("multipolygon")] >>
	 -(qi::omit[*qi::space] >>
	   ascii::no_case[ascii::char_("z")]) >>
	 -(qi::omit[*qi::space] >>
	   ascii::no_case[ascii::char_("m")]) >>
	 qi::omit[*qi::space] >>
	 multipolygon_text)
	[qi::_val = phx::construct<MultiPolygon>(qi::_3, qi::_1, qi::_2)];
      multipolygon_ptr = (multipolygon_tagged_text)
	[qi::_val = make_shared_<MultiPolygon>()(qi::_1)];

      geometry_tagged_text =
	point_ptr | multipoint_ptr |
	linestring_ptr | multilinestring_ptr |
	polygon_ptr | multipolygon_ptr;
      
      start %= geometry_tagged_text % (*qi::space >> ',' >> *qi::space);
    }
    
  };

};

#endif
