#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "data_structure.hpp"

using namespace std;

struct TrapezoidalMap {
    vector<Trapezoid*> trapezoids;
    Node* root;
    vector<Segment*> segments;
    
    TrapezoidalMap() : root(NULL) {};
    void addTrapezoid(Trapezoid* t);
    void removeTrapezoid(Trapezoid* t);
    void cleanup();
};

// Query trapezoid containing point
Node* queryTrapezoidMap(Node* n, const Point& p);

// Find all trapezoids intersected by a segment
void findIntersectedTrapezoids(Node* root, const Segment& seg, 
                               vector<Trapezoid*>& result);

// Insert segment in single trapezoid
void insertInSingleTrapezoid(TrapezoidalMap& map, Trapezoid* oldTrap,
                             Segment* seg);

// Insert segment across multiple trapezoids
void insertAcrossMultipleTrapezoids(TrapezoidalMap& map,
                                    const vector<Trapezoid*>& intersected,
                                    Segment* seg);

TrapezoidalMap BuildTrapezoidalMap(vector<Segment>& S);

void deleteSearchStructure(Node* node);
void validateTrapezoid(Trapezoid* t);
void validateSearchStructure(Node* node);
void validateDAGStructure(Node* node, int depth = 0);
void printTrapezoid(Trapezoid* t);
void printSearchStructure(Node* node, const std::string& prefix = "", bool isLeft = true);
void debugIntersection(const std::vector<Trapezoid*>& traps, const Segment& seg);
