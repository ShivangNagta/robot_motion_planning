#pragma once

#include "data_structure.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

class MinkowskiSum {
public:
    static Polygon MINKOWSKISUM(const Polygon& P, const Polygon& R);
    static void normalizePolygon(Polygon& poly);
};
