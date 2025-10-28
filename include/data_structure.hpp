/*-------------------------------------------------------------------------------\
| data_structure.hpp                                                             |
+--------------------------------------------------------------------------------+
| CS302_Analysis_and_Design_of_Algorithms                                        |
+--------------------------------------------------------------------------------+
| This file contains some custom data structures used in the entire project.     |
\-------------------------------------------------------------------------------*/

#pragma once

#include <vector>
#include <cstdio>

struct Point {
    double x, y;
    Point(double x = 0, double y = 0);
    bool operator<(const Point& p) const;
    bool operator>(const Point& p) const;
    bool operator==(const Point& p) const;
    Point operator+(const Point& p) const;
    Point operator/(const int k) const;
    bool equals(const Point& other, double epsilon = 1e-9) const {
        return fabs(x - other.x) < epsilon && fabs(y - other.y) < epsilon;
    }
};

struct Polygon {
    std::vector<Point> vertices;
    void addVertex(double x, double y);
    Polygon reflectAboutOrigin();
};

struct Edge {
    Point p1, p2;
    Edge(Point a, Point b);
};

struct Segment {
    Point p1, p2;
    int polygonIndex;
    Segment() : p1(0), p2(0), polygonIndex(-1) {
        normalize();
    }
    Segment(const Point& p1, const Point& p2) 
        : p1(p1), p2(p2), polygonIndex(-1) {
            normalize();
        }
    void normalize() {
        if (p2 < p1) {
            std::swap(p1, p2);
        }
    }
    bool operator==(const Segment& s) const;
    Point getLeftEndpoint() const;
    Point getRightEndpoint() const;
    bool isAbove(const Point& p) const;
    double getY(double x) const;
    double yAt(double x) const;
};

struct Trapezoid {
    Point leftp;
    Point rightp;
    Segment* top;
    Segment* bottom;
    
    Trapezoid* upperLeft;
    Trapezoid* lowerLeft;
    Trapezoid* upperRight;
    Trapezoid* lowerRight;
    
    struct Node* node;
    
    Trapezoid() : top(NULL), bottom(NULL), 
                  upperLeft(NULL), lowerLeft(NULL),
                  upperRight(NULL), lowerRight(NULL),
                  node(NULL) {}
};

enum NodeType {
    X_NODE,
    Y_NODE,
    LEAF_NODE
};

struct Node {
    NodeType type;
    
    Point point;
    Segment* segment;
    Trapezoid* trapezoid;
    
    Node* left;
    Node* right;
    Node* above;
    Node* below;
    Node* parent;
    
    Node() : segment(NULL), trapezoid(NULL),
             left(NULL), right(NULL), 
             above(NULL), below(NULL), parent(NULL) {}
};
