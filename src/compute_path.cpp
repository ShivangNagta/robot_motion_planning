#include "compute_path.hpp"
#include <iostream>
#include <queue>
#include <unordered_set>
#include <set>
#include <cmath>
#include <algorithm>
#include <map>

using namespace std;


Point PathComputer::getTrapezoidCenter(Trapezoid* trap) {
    if (!trap) return Point(0, 0);
    
    double centerX = (trap->leftp.x + trap->rightp.x) / 2.0;
    
    double topY = trap->top->yAt(centerX);
    double bottomY = trap->bottom->yAt(centerX);
    double centerY = (topY + bottomY) / 2.0;
    
    return Point(centerX, centerY);
}

Trapezoid* PathComputer::findTrapezoidContainingPoint(TrapezoidalMap& map, const Point& p) {
    Node* leaf = queryTrapezoidMap(map.root, p);
    return leaf->trapezoid;
}

vector<Point> PathComputer::COMPUTEPATH(TrapezoidalMap& freeSpaceMap, 
                                       RoadMap& roadMap,
                                       const Point& pstart, 
                                       const Point& pgoal) {
    Trapezoid* delta_start = findTrapezoidContainingPoint(freeSpaceMap, pstart);
    Trapezoid* delta_goal = findTrapezoidContainingPoint(freeSpaceMap, pgoal);
    
    if (!delta_start || !delta_goal) {
        string message = !delta_start ? "Start position is in forbidden space" : 
                         "Goal position is in forbidden space";
        cout << "ERROR: " << message << endl;
        return {};
    }
    
    cout << "Found start trapezoid and goal trapezoid" << endl;
    
    RoadMapNode* nu_start = roadMap.getNodeForTrapezoid(delta_start);
    RoadMapNode* nu_goal = roadMap.getNodeForTrapezoid(delta_goal);
    
    if (!nu_start || !nu_goal) {
        cout << "ERROR: Could not find roadmap nodes for trapezoids" << endl;
        return {};
    }
    
    cout << "Found roadmap nodes: start at (" << nu_start->position.x << ", " 
         << nu_start->position.y << "), goal at (" << nu_goal->position.x << ", " 
         << nu_goal->position.y << ")" << endl;
    
    vector<Point> roadmapPath = breadthFirstSearch(nu_start, nu_goal);
    
    if (roadmapPath.empty()) {
        cout << "No path found in roadmap" << endl;
        return {};
    }
    
    vector<Point> finalPath;
    
    finalPath.push_back(pstart);
    
    for (size_t i = 0; i < roadmapPath.size(); i++) {
        if (finalPath.empty() || !finalPath.back().equals(roadmapPath[i])) {
            finalPath.push_back(roadmapPath[i]);
        }
    }
    
    if (finalPath.empty() || !finalPath.back().equals(pgoal)) {
        finalPath.push_back(pgoal);
    }
    
    cout << "Path found with " << finalPath.size() << " points" << endl;
    return finalPath;
}

RoadMap PathComputer::buildRoadMap(TrapezoidalMap& freeSpaceMap) {
    RoadMap roadMap;

    unordered_map<Trapezoid*, RoadMapNode*> trapCenter;
    map<tuple<double, double, double>, RoadMapNode*> verticalEdgeNodes; 

    // 1. Create center nodes for each trapezoid
    for (Trapezoid* trap : freeSpaceMap.trapezoids) {
        Point c = getTrapezoidCenter(trap);
        auto* node = new RoadMapNode(c, trap);
        roadMap.addNode(node);
        trapCenter[trap] = node;
    }

    // 2. Create vertical edge nodes
    for (Trapezoid* trap : freeSpaceMap.trapezoids) {
        for (bool rightSide : {false, true}) {
            double x = rightSide ? trap->rightp.x : trap->leftp.x;
            if (isinf(x)) continue;
            Trapezoid* upper = rightSide ? trap->upperRight : trap->upperLeft;
            Trapezoid* lower = rightSide ? trap->lowerRight : trap->lowerLeft;
            double topY = trap->top->yAt(x);
            double bottomY = trap->bottom->yAt(x);

            // upper part
            if (upper) {
                double uy1 = max(bottomY, upper->bottom->yAt(x));
                double uy2 = min(topY, upper->top->yAt(x));
                if (uy1 < uy2) {
                    auto key = make_tuple(x, uy1, uy2);
                    if (!verticalEdgeNodes[key]) {
                        Point mid(x, 0.5 * (uy1 + uy2));
                        auto* vNode = new RoadMapNode(mid, nullptr);
                        verticalEdgeNodes[key] = vNode;
                        roadMap.addNode(vNode);
                    }
                }
            }

            // lower part
            if (lower) {
                double ly1 = max(lower->bottom->yAt(x), bottomY);
                double ly2 = min(lower->top->yAt(x), topY);
                if (ly1 < ly2) {
                    auto key = make_tuple(x, ly1, ly2);
                    if (!verticalEdgeNodes[key]) {
                        Point mid(x, 0.5 * (ly1 + ly2));
                        auto* vNode = new RoadMapNode(mid, nullptr);
                        verticalEdgeNodes[key] = vNode;
                        roadMap.addNode(vNode);
                    }
                }
            }
        }
    }

    // 3. Connect trapezoid centers to corresponding vertical boundary nodes
    for (Trapezoid* trap : freeSpaceMap.trapezoids) {
        RoadMapNode* center = trapCenter[trap];
        for (bool rightSide : {false, true}) {
            double x = rightSide ? trap->rightp.x : trap->leftp.x;
            if (isinf(x)) continue;
            double y1 = trap->bottom->yAt(x);
            double y2 = trap->top->yAt(x);

            for (auto& [key, node] : verticalEdgeNodes) {
                auto [vx, vy1, vy2] = key;
                if (fabs(vx - x) < 1e-9 &&
                    vy1 >= y1 - 1e-9 && vy2 <= y2 + 1e-9) {
                    center->neighbors.push_back(node);
                    node->neighbors.push_back(center);
                }
            }
        }
    }

    cout << "Built roadmap with " << roadMap.nodes.size() << " nodes and "
         << countTotalEdges(roadMap) << " edges" << endl;

    return roadMap;
}

int PathComputer::countTotalEdges(RoadMap& roadMap) {
    int totalEdges = 0;
    for (RoadMapNode* node : roadMap.nodes) {
        totalEdges += node->neighbors.size();
    }
    return totalEdges / 2;
}

vector<Point> PathComputer::breadthFirstSearch(RoadMapNode* start, RoadMapNode* goal) {
    if (!start || !goal) return {};
    if (start == goal) return {start->position};
    
    unordered_map<RoadMapNode*, RoadMapNode*> cameFrom;
    queue<RoadMapNode*> q;
    unordered_set<RoadMapNode*> visited;
    
    q.push(start);
    visited.insert(start);
    cameFrom[start] = nullptr;
    
    while (!q.empty()) {
        RoadMapNode* current = q.front();
        q.pop();
        
        if (current == goal) {
            vector<Point> path;
            RoadMapNode* node = goal;
            while (node) {
                path.push_back(node->position);
                node = cameFrom[node];
            }
            reverse(path.begin(), path.end());
            return path;
        }
        
        for (RoadMapNode* neighbor : current->neighbors) {
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                cameFrom[neighbor] = current;
                q.push(neighbor);
            }
        }
    }
    
    return {};
}
