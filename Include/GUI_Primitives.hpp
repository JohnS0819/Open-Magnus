#ifndef GUI_PRIMITIVES_HPP
#define GUI_PRIMITIVES_HPP
#include "Grid.hpp"
#include <SFML/Window/Event.hpp>
//#include "Event.hpp"
#include <vector>
namespace GUI{


    class Primitive {
    public:
        virtual bool containsPoint(const Geometry::coordinate&) = 0;
        virtual void scale(const float&) = 0;
        virtual void translate(const Geometry::coordinate&) = 0;
    };

    class BoundingBox : public Primitive {
        
    protected:
        Geometry::coordinate minPoint;
        Geometry::coordinate maxPoint;

    public:
        BoundingBox(float x1, float y1, float x2, float y2) : minPoint(x1, y1), maxPoint(x2, y2) {}

        BoundingBox(const Geometry::coordinate& upperLeft, const Geometry::coordinate& lowerRight)
            : minPoint(upperLeft), maxPoint(lowerRight) {}

        bool containsPoint(const Geometry::coordinate& point) override {
            return point.x >= minPoint.x && point.x <= maxPoint.x && point.y >= minPoint.y && point.y <= maxPoint.y;
        }

        bool intersects(const BoundingBox& other) {
            return !(maxPoint.x < other.minPoint.x || minPoint.x > other.maxPoint.x ||
                maxPoint.y < other.minPoint.y || minPoint.y > other.maxPoint.y);
        }

        void resize(float x1, float y1, float x2, float y2) {
            minPoint.x = x1;
            minPoint.y = y1;
            maxPoint.x = x2;
            maxPoint.y = y2;
        }

        void scale(const float &scaleFactor) override {
            float centerX = (minPoint.x + maxPoint.x) / 2.0f;
            float centerY = (minPoint.y + maxPoint.y) / 2.0f;

            float halfWidth = (maxPoint.x - minPoint.x) / 2.0f;
            float halfHeight = (maxPoint.y - minPoint.y) / 2.0f;

            float newHalfWidth = halfWidth * scaleFactor;
            float newHalfHeight = halfHeight * scaleFactor;

            minPoint.x = centerX - newHalfWidth;
            minPoint.y = centerY - newHalfHeight;
            maxPoint.x = centerX + newHalfWidth;
            maxPoint.y = centerY + newHalfHeight;
        }

        void translate(const Geometry::coordinate& translation) override {
            minPoint.move_to(translation);
            maxPoint.move_to(translation);
        }

        void operator=(const BoundingBox& other) {
            if (this != &other) {
                minPoint = other.minPoint;
                maxPoint = other.maxPoint;
            }
        }
    };

    class BoundingCircle : public Primitive {
    protected:
        float radius_;
        Geometry::coordinate center_;

    public:
        BoundingCircle(const Geometry::coordinate &center, const float& radius) : radius_(radius), center_(center){}

        bool containsPoint(const Geometry::coordinate& point) override {
            point.distance_to_sq(center_) < radius_* radius_;
        }

        void scale(const float &factor) override {
            radius_ *= factor;

        }
        void translate(const Geometry::coordinate& offset) override {
            center_.move_to(offset);

        }
    };

    /*class Widget : public Communication::Consumer {
    protected:
        std::vector<Communication::Connection&> References;

    public:
        virtual void close() {

        }




    };*/



};




#endif
