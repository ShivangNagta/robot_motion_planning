#include "minkowski_sum.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
using namespace std;

Polygon MinkowskiSum::MINKOWSKISUM(const Polygon& P, const Polygon& Q) {
    cout << "=== MINKOWSKI SUM ===" << endl;

    if (P.vertices.size() < 3 || Q.vertices.size() < 3) {
        cerr << "ERROR: Input polygons must have at least 3 vertices" << endl;
        return Polygon();
    }

    Polygon A = P;
    Polygon B = Q;

    auto lowestVertexIndex = [](const Polygon& poly) {
        int idx = 0;
        for (int i = 1; i < (int)poly.vertices.size(); i++) {
            const Point& p = poly.vertices[i];
            const Point& best = poly.vertices[idx];
            if (p.y < best.y || (fabs(p.y - best.y) < 1e-9 && p.x < best.x))
                idx = i;
        }
        return idx;
    };

    int ia = lowestVertexIndex(A);
    int ib = lowestVertexIndex(B);

    int n = A.vertices.size();
    int m = B.vertices.size();

    Polygon R;
    R.vertices.reserve(n + m);

    int i = ia, j = ib;

    R.vertices.push_back(Point(A.vertices[i].x + B.vertices[j].x,
                               A.vertices[i].y + B.vertices[j].y));

    do {
        int inext = (i + 1) % n;
        int jnext = (j + 1) % m;

        Point eA(A.vertices[inext].x - A.vertices[i].x,
                 A.vertices[inext].y - A.vertices[i].y);
        Point eB(B.vertices[jnext].x - B.vertices[j].x,
                 B.vertices[jnext].y - B.vertices[j].y);

        double cross = eA.x * eB.y - eA.y * eB.x;

        if (cross >= 0) i = inext;
        if (cross <= 0) j = jnext;

        Point sum(A.vertices[i].x + B.vertices[j].x,
                  A.vertices[i].y + B.vertices[j].y);
        R.vertices.push_back(sum);

    } while (i != ia || j != ib);

    cout << "Resulting polygon has " << R.vertices.size() << " vertices" << endl;
    return R;
}

void MinkowskiSum::normalizePolygon(Polygon& poly) {
    if (poly.vertices.empty()) return;
    
    int start_index = 0;
    for (size_t i = 1; i < poly.vertices.size(); i++) {
        if (poly.vertices[i].y < poly.vertices[start_index].y ||
            (poly.vertices[i].y == poly.vertices[start_index].y && 
             poly.vertices[i].x < poly.vertices[start_index].x)) {
            start_index = i;
        }
    }
    
    vector<Point> normalized;
    for (size_t i = 0; i < poly.vertices.size(); i++) {
        normalized.push_back(poly.vertices[(start_index + i) % poly.vertices.size()]);
    }
    
    poly.vertices = normalized;
    
    double area = 0;
    int n = poly.vertices.size();
    for (int i = 0; i < n; i++) {
        const Point& p1 = poly.vertices[i];
        const Point& p2 = poly.vertices[(i + 1) % n];
        area += (p2.x - p1.x) * (p2.y + p1.y);
    }
    
    if (area > 0) {
        vector<Point> reversed;
        reversed.push_back(poly.vertices[0]);
        for (int i = n - 1; i > 0; i--) {
            reversed.push_back(poly.vertices[i]);
        }
        poly.vertices = reversed;
    }
}

