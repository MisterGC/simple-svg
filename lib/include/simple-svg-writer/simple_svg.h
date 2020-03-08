/*******************************************************************************
*  The "New BSD License" : http://www.opensource.org/licenses/bsd-license.php  *
********************************************************************************

Copyright (c) 2010, Mark Turney
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#ifndef SIMPLE_SVG_HPP
#define SIMPLE_SVG_HPP

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include <iostream>

namespace simple_svg
{
    // Utility XML/String Functions.
    template <typename T>
    inline std::string attribute(std::string const & attribute_name,
        T const & value, std::string const & unit = "")
    {
        std::stringstream ss;
        ss << " " << attribute_name << "=\"" << value << unit << "\"";
        return ss.str();
    }
    inline std::string openStartTag(std::string const & element_name)
    {
        return "<" + element_name;
    }
    inline std::string closeEmptyTag()
    {
        return "/>";
    }
    inline std::string closeTag()
    {
        return ">";
    }
    inline std::string endTag(std::string const & element_name)
    {
        return "</" + element_name + ">\n";
    }

    // Quick optional return type.  This allows functions to return an invalid
    //  value if no good return is possible.  The user checks for validity
    //  before using the returned value.
    template <typename T>
    class optional
    {
    public:
        optional<T>(T const & type)
            : valid(true), type(type) { }
        optional<T>() : valid(false), type(T()) { }
        T * operator->()
        {
            // If we try to access an invalid value, an exception is thrown.
            if (!valid)
                throw std::exception();

            return &type;
        }
        // Test for validity.
        bool operator!() const { return !valid; }
    private:
        bool valid;
        T type;
    };

    struct Dimensions
    {
        Dimensions(double width, double height) : width(width), height(height) { }
        Dimensions(double combined = 0) : width(combined), height(combined) { }
        double width;
        double height;
    };

    struct Point
    {
        Point(double x = 0, double y = 0) : x(x), y(y) { }
        double x;
        double y;
    };
    inline optional<Point> getMinPoint(std::vector<Point> const & points)
    {
        if (points.empty())
            return optional<Point>();

        Point min = points[0];
        for (unsigned i = 0; i < points.size(); ++i) {
            if (points[i].x < min.x)
                min.x = points[i].x;
            if (points[i].y < min.y)
                min.y = points[i].y;
        }
        return optional<Point>(min);
    }
    inline optional<Point> getMaxPoint(std::vector<Point> const & points)
    {
        if (points.empty())
            return optional<Point>();

        Point max = points[0];
        for (unsigned i = 0; i < points.size(); ++i) {
            if (points[i].x > max.x)
                max.x = points[i].x;
            if (points[i].y > max.y)
                max.y = points[i].y;
        }
        return optional<Point>(max);
    }

    // Defines the dimensions, scale, origin, and origin offset of the document.
    struct Layout
    {
        enum Origin { TopLeft, BottomLeft, TopRight, BottomRight };
        enum Unit {px, mm};

        Layout(Dimensions const & dimensions = Dimensions(400, 300),
               Unit unit = mm,
               Origin origin = BottomLeft,
               double scale = 1,
               Point const & origin_offset = Point(0, 0)):
            dimensions(dimensions),
            unit(unit),
            scale(scale),
            origin(origin),
            origin_offset(origin_offset)
        { }

        Dimensions dimensions;
        Unit unit;
        double scale;
        Origin origin;
        Point origin_offset;
    };

    // Convert coordinates in user space to SVG native space.
    inline double translateX(double x, Layout const & layout)
    {
        if (layout.origin == Layout::BottomRight || layout.origin == Layout::TopRight)
            return layout.dimensions.width - ((x + layout.origin_offset.x) * layout.scale);
        else
            return (layout.origin_offset.x + x) * layout.scale;
    }

    inline double translateY(double y, Layout const & layout)
    {
        if (layout.origin == Layout::BottomLeft || layout.origin == Layout::BottomRight)
            return layout.dimensions.height - ((y + layout.origin_offset.y) * layout.scale);
        else
            return (layout.origin_offset.y + y) * layout.scale;
    }
    inline double translateScale(double dimension, Layout const & layout)
    {
        return dimension * layout.scale;
    }

    class Serializeable
    {
    public:
        Serializeable() { }
        virtual ~Serializeable() { };
        virtual std::string toString(Layout const & layout) const = 0;
    };

    class Color : public Serializeable
    {
    public:
        enum Defaults { Transparent = -1, Aqua, Black, Blue, Brown, Cyan, Fuchsia,
            Green, Lime, Magenta, Orange, Purple, Red, Silver, White, Yellow };

        Color(int r, int g, int b) : transparent(false), red(r), green(g), blue(b) { }
        Color(Defaults color)
            : transparent(false), red(0), green(0), blue(0)
        {
            switch (color)
            {
                case Aqua: assign(0, 255, 255); break;
                case Black: assign(0, 0, 0); break;
                case Blue: assign(0, 0, 255); break;
                case Brown: assign(165, 42, 42); break;
                case Cyan: assign(0, 255, 255); break;
                case Fuchsia: assign(255, 0, 255); break;
                case Green: assign(0, 128, 0); break;
                case Lime: assign(0, 255, 0); break;
                case Magenta: assign(255, 0, 255); break;
                case Orange: assign(255, 165, 0); break;
                case Purple: assign(128, 0, 128); break;
                case Red: assign(255, 0, 0); break;
                case Silver: assign(192, 192, 192); break;
                case White: assign(255, 255, 255); break;
                case Yellow: assign(255, 255, 0); break;
                default: transparent = true; break;
            }
        }
        virtual ~Color() { }
        std::string toString(Layout const &) const
        {
            std::stringstream ss;
            if (transparent)
                ss << "none";
            else
                ss << "rgb(" << red << "," << green << "," << blue << ")";
            return ss.str();
        }
    private:
            bool transparent;
            int red;
            int green;
            int blue;

            void assign(int r, int g, int b)
            {
                red = r;
                green = g;
                blue = b;
            }
    };

    class Fill : public Serializeable
    {
    public:
        Fill(Color::Defaults color) : color(color) { }
        Fill(Color color = Color::Transparent)
            : color(color) { }
        std::string toString(Layout const & layout) const
        {
            std::stringstream ss;
            ss << attribute("fill", color.toString(layout));
            return ss.str();
        }
    private:
        Color color;
    };

    class Stroke : public Serializeable
    {
    public:
        Stroke(double width = -1, Color color = Color::Transparent, bool nonScalingStroke = false)
            : width(width), color(color), nonScaling(nonScalingStroke) { }
        std::string toString(Layout const & layout) const
        {
            // If stroke width is invalid.
            if (width < 0)
                return std::string();

            std::stringstream ss;
            ss << attribute("stroke-width", translateScale(width, layout)) << attribute("stroke", color.toString(layout));
            if (nonScaling)
               ss << attribute("vector-effect", "non-scaling-stroke");
            return ss.str();
        }
    private:
        double width;
        Color color;
        bool nonScaling;
    };

    class Font : public Serializeable
    {
    public:
        Font(double size = 12, std::string const & family = "Verdana") : size(size), family(family) { }
        std::string toString(Layout const & layout) const
        {
            std::stringstream ss;
            ss << attribute("font-size", translateScale(size, layout)) << attribute("font-family", family);
            return ss.str();
        }
    private:
        double size;
        std::string family;
    };

    class Shape : public Serializeable
    {
    public:
        Shape(Fill const & fill = Fill(), Stroke const & stroke = Stroke())
            : fill(fill), stroke(stroke) { }
        virtual ~Shape() = default;
        virtual void offset(Point const & offset) = 0;

        std::string description() const { return descr; }
        void setDescription(const std::string &desc) { descr = desc; }
        std::string toString(const Layout &layout) const override
        {
            std::stringstream ss;
            ss << openStartTag(shapeName())
               << shapeAttributes(layout)
               << fill.toString(layout)
               << stroke.toString(layout)
               << closeTag()
               << "\n"
               << openStartTag("desc")
               << closeTag()
               << descr
               << endTag("desc")
               << endTag(shapeName());
            return ss.str();
        }

        virtual std::string shapeName() const = 0;
        virtual std::string shapeAttributes(const Layout &layout) const = 0;

    protected:
        Fill fill;
        Stroke stroke;
        std::string descr = "";
    };

    template <typename T>
    inline std::string vectorToString(std::vector<T> collection, Layout const & layout)
    {
        std::string combination_str;
        for (unsigned i = 0; i < collection.size(); ++i)
            combination_str += collection[i].toString(layout);

        return combination_str;
    }

    class Circle : public Shape
    {
    public:
        Circle(Point const & center, double radius, Fill const & fill,
            Stroke const & stroke = Stroke())
            : Shape(fill, stroke), center(center), radius(radius) { }

        std::string shapeName() const override
        {
            return "circle";
        }

        std::string shapeAttributes(const Layout &layout) const override
        {
            std::stringstream ss;
            ss << attribute("cx", translateX(center.x, layout))
               << attribute("cy", translateY(center.y, layout))
               << attribute("r", translateScale(radius, layout));
            return ss.str();
        }

        void offset(Point const & offset)
        {
            center.x += offset.x;
            center.y += offset.y;
        }
    private:
        Point center;
        double radius;
    };

    class Elipse : public Shape
    {
    public:
        Elipse(Point const & center, double width, double height,
            Fill const & fill = Fill(), Stroke const & stroke = Stroke())
            : Shape(fill, stroke), center(center), radius_width(width / 2),
            radius_height(height / 2) { }

        std::string shapeName() const override
        {
            return "ellipse";
        }

        std::string shapeAttributes(const Layout &layout) const override
        {
            std::stringstream ss;
            ss <<  attribute("cx", translateX(center.x, layout))
                << attribute("cy", translateY(center.y, layout))
                << attribute("rx", translateScale(radius_width, layout))
                << attribute("ry", translateScale(radius_height, layout));
            return ss.str();
        }

        void offset(Point const & offset)
        {
            center.x += offset.x;
            center.y += offset.y;
        }
    private:
        Point center;
        double radius_width;
        double radius_height;
    };

    class Rectangle : public Shape
    {
    public:
        Rectangle(Point const & edge, double width, double height,
            Fill const & fill = Fill(), Stroke const & stroke = Stroke())
            : Shape(fill, stroke), edge(edge), width(width),
            height(height) { }

        std::string shapeName() const override
        {
            return "rect";
        }

        std::string shapeAttributes(const Layout &layout) const override
        {
            std::stringstream ss;
            ss <<  attribute("x", translateX(edge.x, layout))
                << attribute("y", translateY(edge.y, layout))
                << attribute("width", translateScale(width, layout))
                << attribute("height", translateScale(height, layout));
            return ss.str();
        }

        void offset(Point const & offset)
        {
            edge.x += offset.x;
            edge.y += offset.y;
        }
    private:
        Point edge;
        double width;
        double height;
    };

    class Line : public Shape
    {
    public:
        Line(Point const & start_point, Point const & end_point,
            Stroke const & stroke = Stroke())
            : Shape(Fill(), stroke), start_point(start_point),
            end_point(end_point) { }

        std::string shapeName() const override
        {
            return "line";
        }

        std::string shapeAttributes(const Layout &layout) const override
        {
            std::stringstream ss;
            ss  << attribute("x1", translateX(start_point.x, layout))
                << attribute("y1", translateY(start_point.y, layout))
                << attribute("x2", translateX(end_point.x, layout))
                << attribute("y2", translateY(end_point.y, layout));
            return ss.str();
        }

        void offset(Point const & offset)
        {
            start_point.x += offset.x;
            start_point.y += offset.y;

            end_point.x += offset.x;
            end_point.y += offset.y;
        }
    private:
        Point start_point;
        Point end_point;
    };

    class Polygon : public Shape
    {
    public:
        Polygon(Fill const & fill = Fill(), Stroke const & stroke = Stroke())
            : Shape(fill, stroke) { }
        Polygon(Stroke const & stroke = Stroke()) : Shape(Color::Transparent, stroke) { }
        Polygon & operator<<(Point const & point)
        {
            points.push_back(point);
            return *this;
        }

        std::string shapeName() const override
        {
            return "polygon";
        }

        std::string shapeAttributes(const Layout &layout) const override
        {
            std::stringstream ss;
            ss << "points=\"";
            for (unsigned i = 0; i < points.size(); ++i)
                ss << translateX(points[i].x, layout) << "," << translateY(points[i].y, layout) << " ";
            ss << "\" ";
            return ss.str();
        }

        void offset(Point const & offset)
        {
            for (unsigned i = 0; i < points.size(); ++i) {
                points[i].x += offset.x;
                points[i].y += offset.y;
            }
        }
    private:
        std::vector<Point> points;
    };

    class Path : public Shape
    {
    public:
       Path(Fill const & fill = Fill(), Stroke const & stroke = Stroke())
          : Shape(fill, stroke)
       {  startNewSubPath(); }
       Path(Stroke const & stroke = Stroke()) : Shape(Color::Transparent, stroke)
       {  startNewSubPath(); }
       Path & operator<<(Point const & point)
       {
          paths.back().push_back(point);
          return *this;
       }

       void startNewSubPath()
       {
          if (paths.empty() || 0 < paths.back().size())
            paths.emplace_back();
       }

       std::string shapeName() const override
       {
           return "path";
       }

       std::string shapeAttributes(const Layout &layout) const override
       {
           std::stringstream ss;
           ss << "d=\"";
           for (auto const& subpath: paths)
           {
               if (subpath.empty())
                   continue;

               ss << "M";
               for (auto const& point: subpath)
                   ss << translateX(point.x, layout) << "," << translateY(point.y, layout) << " ";
               ss << "z ";
           }
           ss << "\" ";
           ss << "fill-rule=\"evenodd\" ";
           return ss.str();
       }


       void offset(Point const & offset)
       {
          for (auto& subpath : paths)
             for (auto& point : subpath)
             {
                point.x += offset.x;
                point.y += offset.y;
             }
       }
    private:
       std::vector<std::vector<Point>> paths;
    };

    class Polyline : public Shape
    {
    public:
        Polyline(Fill const & fill = Fill(), Stroke const & stroke = Stroke())
            : Shape(fill, stroke) { }
        Polyline(Stroke const & stroke = Stroke()) : Shape(Color::Transparent, stroke) { }
        Polyline(std::vector<Point> const & points,
            Fill const & fill = Fill(), Stroke const & stroke = Stroke())
            : Shape(fill, stroke), points(points) { }
        Polyline & operator<<(Point const & point)
        {
            points.push_back(point);
            return *this;
        }

        std::string shapeName() const override
        {
            return "polyline";
        }

        std::string shapeAttributes(const Layout &layout) const override
        {
            std::stringstream ss;
            ss << "points=\"";
            for (unsigned i = 0; i < points.size(); ++i)
                ss << translateX(points[i].x, layout) << "," << translateY(points[i].y, layout) << " ";
            ss << "\" ";
            return ss.str();
        }

        void offset(Point const & offset)
        {
            for (unsigned i = 0; i < points.size(); ++i) {
                points[i].x += offset.x;
                points[i].y += offset.y;
            }
        }
        std::vector<Point> points;
    };

    class Text : public Shape
    {
    public:
        Text(Point const & origin, std::string const & content, Fill const & fill = Fill(),
             Font const & font = Font(), Stroke const & stroke = Stroke())
            : Shape(fill, stroke), origin(origin), content(content), font(font) { }

        std::string shapeName() const override
        {
            return "text";
        }

        std::string shapeAttributes(const Layout &layout) const override
        {
            std::stringstream ss;
            ss  << attribute("x", translateX(origin.x, layout))
                << attribute("y", translateY(origin.y, layout));
            return ss.str();
        }

        void offset(Point const & offset)
        {
            origin.x += offset.x;
            origin.y += offset.y;
        }
    private:
        Point origin;
        std::string content;
        Font font;
    };

    class Document
    {
    public:
        Document(std::string const & file_name, Layout layout = Layout())
            : file_name(file_name), layout(layout) { }

        Document & operator<<(Shape const & shape)
        {
            bodyNodeStrs.push_back(shape.toString(layout));
            return *this;
        }
        std::string toString() const
        {
            std::stringstream ss;
            writeToStream(ss);
            return ss.str();
        }
        bool save() const
        {
            std::ofstream ofs(file_name.c_str());
            if (!ofs.good())
                return false;

            writeToStream(ofs);
            ofs.close();
            return true;
        }
    private:
        void writeToStream(std::ostream& str) const
        {
            auto unit = layout.unit == Layout::mm ? "mm" : "px";
            str << "<?xml " << attribute("version", "1.0") << attribute("standalone", "no")
                << "?>\n<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
                << "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n<svg "
                << attribute("width", layout.dimensions.width, unit)
                << attribute("height", layout.dimensions.height, unit)
                << attribute("xmlns", "http://www.w3.org/2000/svg")
                << attribute("version", "1.1") << ">\n";
            for (const auto& body_node_str : bodyNodeStrs) {
                str << body_node_str;
            }
            str << endTag("svg");
        }

    private:
        std::string file_name;
        Layout layout;

        std::vector<std::string> bodyNodeStrs;
    };

}

#endif
