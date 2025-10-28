#pragma once

#include "data_structure.hpp"
#include "trapezoidal_map.hpp"
#include <vector>

class FreeSpaceComputer {
public:
    static TrapezoidalMap COMPUTEFREESPACE(const std::vector<Polygon>& S);
    static std::vector<Segment> extractEdges(const std::vector<Polygon>& polygons);
    static bool isTrapezoidInsideObstacle(Trapezoid* trap);
    static void removeInteriorTrapezoids(TrapezoidalMap& map, 
                                        const std::vector<Polygon>& polygons);
};

