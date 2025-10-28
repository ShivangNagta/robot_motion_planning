#pragma once

#include "data_structure.hpp"
#include "trapezoidal_map.hpp"
#include "compute_free_space.hpp"
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>

struct RoadMapNode {
    Point position;
    std::vector<RoadMapNode*> neighbors;
    Trapezoid* trapezoid;
    
    RoadMapNode(const Point& p, Trapezoid* trap = nullptr) 
        : position(p), trapezoid(trap) {}
};

class RoadMap {
public:
    std::vector<RoadMapNode*> nodes;
    std::unordered_map<Trapezoid*, RoadMapNode*> trapToNode;
    
    ~RoadMap() {
        for (RoadMapNode* node : nodes) {
            delete node;
        }
    }
    
    void addNode(RoadMapNode* node) {
        nodes.push_back(node);
        if (node->trapezoid) {
            trapToNode[node->trapezoid] = node;
        }
    }
    
    RoadMapNode* getNodeForTrapezoid(Trapezoid* trap) {
        auto it = trapToNode.find(trap);
        return (it != trapToNode.end()) ? it->second : nullptr;
    }
};

class PathComputer {
public:
    static std::vector<Point> COMPUTEPATH(TrapezoidalMap& freeSpaceMap, 
                                         RoadMap& roadMap,
                                         const Point& pstart, 
                                         const Point& pgoal);
    
    static RoadMap buildRoadMap(TrapezoidalMap& freeSpaceMap);
    static std::vector<Point> breadthFirstSearch(RoadMapNode* start, RoadMapNode* goal);
    static Trapezoid* findTrapezoidContainingPoint(TrapezoidalMap& map, const Point& p);
    static Point getTrapezoidCenter(Trapezoid* trap);
    static bool isValidPath(const std::vector<Point>& path, 
                           const std::vector<Polygon>& obstacles);
    static int countTotalEdges(RoadMap& roadMap);
};
