#include "compute_free_space.hpp"
#include <iostream>
#include <cmath>
#include <set>

using namespace std;

TrapezoidalMap FreeSpaceComputer::COMPUTEFREESPACE(const vector<Polygon>& S) {
    cout << "=== COMPUTEFREESPACE Algorithm ===" << endl;
    cout << "Input: " << S.size() << " polygons (obstacles)" << endl;
    
    // Step 1: Extract all edges from obstacles
    vector<Segment> E = extractEdges(S);
    cout << "Extracted " << E.size() << " edges from polygons" << endl;
    
    // Step 2: Build trapezoidal decomposition
    cout << "Building trapezoidal map..." << endl;
    TrapezoidalMap T = BuildTrapezoidalMap(E);
    cout << "Trapezoidal map built with " << T.trapezoids.size() << " trapezoids" << endl;
    
    // Step 3: Remove trapezoids that are inside obstacles
    cout << "Identifying and removing interior trapezoids..." << endl;
    removeInteriorTrapezoids(T, S);
    cout << "Free space computed. Remaining trapezoids: " << T.trapezoids.size() << endl;
    
    return T;
}

vector<Segment> FreeSpaceComputer::extractEdges(const vector<Polygon>& polygons) {
    vector<Segment> edges;
    
    for (size_t polyIdx = 0; polyIdx < polygons.size(); polyIdx++) {
        const Polygon& poly = polygons[polyIdx];
        
        if (poly.vertices.size() < 3) {
            cerr << "Warning: Polygon with less than 3 vertices skipped" << endl;
            continue;
        }

        for (size_t i = 0; i < poly.vertices.size(); i++) {
            size_t next = (i + 1) % poly.vertices.size();
            Segment edge(poly.vertices[i], poly.vertices[next]);
            edge.polygonIndex = polyIdx;
            edges.push_back(edge);
        }
    }
    
    return edges;
}

bool FreeSpaceComputer::isTrapezoidInsideObstacle(Trapezoid* trap) {
    if (!trap) {
        return false;
    }
    // A trapezoid is inside an obstacle if BOTH its top and bottom edges belong to the SAME obstacle
    int topPolyIndex = (trap->top && trap->top->polygonIndex != -1) 
                       ? trap->top->polygonIndex : -1;
    int bottomPolyIndex = (trap->bottom && trap->bottom->polygonIndex != -1) 
                          ? trap->bottom->polygonIndex : -1;
    if (topPolyIndex != -1 && bottomPolyIndex != -1 && topPolyIndex == bottomPolyIndex) {
        return true;
    }
    return false;
}

void FreeSpaceComputer::removeInteriorTrapezoids(TrapezoidalMap& map, const vector<Polygon>& polygons) {
    vector<Trapezoid*> toRemove;
    int trapIndex = 0;
    for (Trapezoid* trap : map.trapezoids) {
        if (isTrapezoidInsideObstacle(trap)) {
            toRemove.push_back(trap);
        }
    }
    for (Trapezoid* trap : toRemove) {
        map.removeTrapezoid(trap);
    }
}
