#include "data_structure.hpp"

#include <vector>
#include <cmath>

Point::Point(double x, double y) : x(x), y(y) {};
bool Point::operator<(const Point& p) const {
        if (fabs(x - p.x) > 1e-9) return x < p.x;
        return y < p.y;
    }
bool Point::operator>(const Point& p) const {
        return p < *this;
    }
bool Point::operator==(const Point& p) const {
        return fabs(x - p.x) < 1e-9 && fabs(y - p.y) < 1e-9;
    }
Point Point::operator+(const Point& p) const {
        return Point{x+p.x, y+p.y};
    }
Point Point::operator/(const int k) const {
        return Point{x/k, y/k};
    }

void Polygon::addVertex(double x, double y) {
        vertices.push_back(Point(x, y));
    }
Polygon Polygon::reflectAboutOrigin() {
    Polygon reflectedPoly;
    reflectedPoly.vertices.reserve((*this).vertices.size());
    for (const Point& p : (*this).vertices) {
        Point reflectedPoint(-p.x, -p.y);
        reflectedPoly.vertices.push_back(reflectedPoint);
    }
    return reflectedPoly;
}

Edge::Edge(Point a, Point b) : p1(a), p2(b) {};

bool Segment::operator==(const Segment& s) const {
    return p1 == s.p1;
};
Point Segment::getLeftEndpoint() const{
    return (this->p1 < this->p2) ? this->p1 : this->p2;
}

Point Segment::getRightEndpoint() const {
    return (this->p1 < this->p2) ? this->p2 : this->p1;
}

bool Segment::isAbove(const Point& p) const {
    Point left = getLeftEndpoint();
    Point right = getRightEndpoint();
    double cross = (right.x - left.x) * (p.y - left.y) - 
                    (right.y - left.y) * (p.x - left.x);
    return cross > 1e-9;
}

double Segment::yAt(double x) const {
    if (fabs(p1.x - p2.x) < 1e-9)
        return std::numeric_limits<double>::infinity();
    double t = (x - p1.x) / (p2.x - p1.x);
    return p1.y + t * (p2.y - p1.y);
}


double Segment::getY(double x) const {
    Point left = getLeftEndpoint();
    Point right = getRightEndpoint();
    
    if (std::abs(left.x - right.x) < 1e-9) {
        return (left.y + right.y) / 2.0;
    }
    
    return left.y + (right.y - left.y) * (x - left.x) / (right.x - left.x);
}
