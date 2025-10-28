#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <set>

#include "data_structure.hpp"
#include "trapezoidal_map.hpp"

using namespace std;

void TrapezoidalMap::addTrapezoid(Trapezoid* t) {
    trapezoids.push_back(t);
}

void TrapezoidalMap::removeTrapezoid(Trapezoid* t) {
    for (size_t i = 0; i < trapezoids.size(); i++) {
        if (trapezoids[i] == t) {
            trapezoids.erase(trapezoids.begin() + i);
            return;
        }
    }
}

void TrapezoidalMap::cleanup() {
    deleteSearchStructure(root);
    for (size_t i = 0; i < trapezoids.size(); i++) {
        delete trapezoids[i];
    }
    for (size_t i = 0; i < segments.size(); i++) {
        delete segments[i];
    }
    trapezoids.clear();
    segments.clear();
    root = NULL;
}

Node* queryTrapezoidMap(Node* n, const Point& p) {
    if (n == NULL) return NULL;
    
    if (n->type == LEAF_NODE) {
        return n;
    }
    if (n->type == X_NODE) {
        if (p.x < n->point.x - 1e-9) {
            return queryTrapezoidMap(n->left, p);
        } else {
            return queryTrapezoidMap(n->right, p);
        }
    }
    if (n->type == Y_NODE) {
        if (n->segment->isAbove(p)) {
            return queryTrapezoidMap(n->above, p);
        } else {
            return queryTrapezoidMap(n->below, p);
        }
    }
    return NULL;
}

void findIntersectedTrapezoids(Node* root, const Segment& seg, 
                               vector<Trapezoid*>& result) {
    Point left = seg.getLeftEndpoint();
    Point right = seg.getRightEndpoint();

    Node* startNode = queryTrapezoidMap(root, left);
    if (startNode == NULL || startNode->trapezoid == NULL) {
        cout << "ERROR: No trapezoid found for left endpoint" << endl;
        return;
    }

    Trapezoid* current = startNode->trapezoid;
    result.push_back(current);

    cout << "Starting trapezoid: ";
    printTrapezoid(current);

    while (current != nullptr) {
        if (current->rightp.x >= right.x - 1e-9) {
            cout << "Reached trapezoid containing right endpoint" << endl;
            break;
        }

        Trapezoid* next = nullptr;
        Point rightPoint = current->rightp;
        
        bool segmentAboveRightPoint = seg.isAbove(rightPoint);
        
        cout << "At right boundary x=" << rightPoint.x << ", y=" << rightPoint.y;
        cout << " - segment is " << (segmentAboveRightPoint ? "above" : "below") << endl;
        
        if (segmentAboveRightPoint) {
            next = current->lowerRight;
            cout << "Following lowerRight neighbor" << endl;
        } else {
            next = current->upperRight;
            cout << "Following upperRight neighbor" << endl;
        }

        if (next == nullptr) {
            cout << "ERROR: No next trapezoid found at x=" << current->rightp.x << endl;
            break;
        }
        
        if (!result.empty() && next == result.back()) {
            cout << "WARNING: Circular reference detected" << endl;
            break;
        }
        
        if (result.size() > 100) {
            cout << "ERROR: Too many trapezoids in intersection path" << endl;
            break;
        }
        
        result.push_back(next);
        current = next;
        cout << "Next trapezoid: ";
        printTrapezoid(current);
    }
}

void insertInSingleTrapezoid(TrapezoidalMap& map, Trapezoid* oldTrap, Segment* seg) {
    Point left = seg->getLeftEndpoint();
    Point right = seg->getRightEndpoint();
    
    cout << "=== Single trapezoid insertion ===" << endl;
    cout << "Old trapezoid: ";
    printTrapezoid(oldTrap);
    
    // Check if we need left and right traps
    bool needLeftTrap = (fabs(left.x - oldTrap->leftp.x) > 1e-9);
    bool needRightTrap = (fabs(right.x - oldTrap->rightp.x) > 1e-9);
    
    cout << "Need left trap: " << needLeftTrap << ", Need right trap: " << needRightTrap << endl;
    
    Trapezoid *A = NULL, *B = NULL, *C = NULL, *D = NULL;
    
    // Create left trap only if needed
    if (needLeftTrap) {
        A = new Trapezoid();
        A->leftp = oldTrap->leftp;
        A->rightp = left;
        A->top = oldTrap->top;
        A->bottom = oldTrap->bottom;
    }
    
    // Create B and C (upper and lower trapezoids split by segment)
    B = new Trapezoid();
    B->leftp = left;
    B->rightp = right;
    B->top = oldTrap->top;
    B->bottom = seg;
    
    C = new Trapezoid();
    C->leftp = left;
    C->rightp = right;
    C->top = seg;
    C->bottom = oldTrap->bottom;
    
    // Create right trap only if needed
    if (needRightTrap) {
        D = new Trapezoid();
        D->leftp = right;
        D->rightp = oldTrap->rightp;
        D->top = oldTrap->top;
        D->bottom = oldTrap->bottom;
    }
    
    // Add neighbours
    if (A) {
        A->upperRight = B;
        A->lowerRight = C;
        B->upperLeft = A;
        B->lowerLeft = A;
        C->upperLeft = A;
        C->lowerLeft = A;
        A->upperLeft = oldTrap->upperLeft;
        A->lowerLeft = oldTrap->lowerLeft;
    } else {
        B->upperLeft = oldTrap->upperLeft;
        B->lowerLeft = oldTrap->upperLeft;
        C->upperLeft = oldTrap->lowerLeft;
        C->lowerLeft = oldTrap->lowerLeft;
    }
    
    if (D) {
        B->upperRight = D;
        B->lowerRight = D;
        C->upperRight = D;
        C->lowerRight = D;
        D->upperLeft = B;
        D->lowerLeft = C;
        D->upperRight = oldTrap->upperRight;
        D->lowerRight = oldTrap->lowerRight;
    } else {
        B->upperRight = oldTrap->upperRight;
        B->lowerRight = oldTrap->upperRight;
        C->upperRight = oldTrap->lowerRight;
        C->lowerRight = oldTrap->lowerRight;
    }
    
    // Update external neighbors
    if (oldTrap->upperLeft) {
        Trapezoid* target = A ? A : B;
        if (oldTrap->upperLeft->upperRight == oldTrap) 
            oldTrap->upperLeft->upperRight = target;
        if (oldTrap->upperLeft->lowerRight == oldTrap) 
            oldTrap->upperLeft->lowerRight = target;
    }
    if (oldTrap->lowerLeft) {
        Trapezoid* target = A ? A : C;
        if (oldTrap->lowerLeft->upperRight == oldTrap) 
            oldTrap->lowerLeft->upperRight = target;
        if (oldTrap->lowerLeft->lowerRight == oldTrap) 
            oldTrap->lowerLeft->lowerRight = target;
    }
    if (oldTrap->upperRight) {
        Trapezoid* target = D ? D : B;
        if (oldTrap->upperRight->upperLeft == oldTrap) 
            oldTrap->upperRight->upperLeft = target;
        if (oldTrap->upperRight->lowerLeft == oldTrap) 
            oldTrap->upperRight->lowerLeft = target;
    }
    if (oldTrap->lowerRight) {
        Trapezoid* target = D ? D : C;
        if (oldTrap->lowerRight->upperLeft == oldTrap) 
            oldTrap->lowerRight->upperLeft = target;
        if (oldTrap->lowerRight->lowerLeft == oldTrap) 
            oldTrap->lowerRight->lowerLeft = target;
    }
    
    // Create leaf nodes for all trapezoids
    Node* aNode = NULL;
    Node* bNode = NULL;
    Node* cNode = NULL;
    Node* dNode = NULL;
    
    if (A) {
        aNode = new Node();
        aNode->type = LEAF_NODE;
        aNode->trapezoid = A;
        A->node = aNode;
    }
    
    bNode = new Node();
    bNode->type = LEAF_NODE;
    bNode->trapezoid = B;
    B->node = bNode;
    
    cNode = new Node();
    cNode->type = LEAF_NODE;
    cNode->trapezoid = C;
    C->node = cNode;
    
    if (D) {
        dNode = new Node();
        dNode->type = LEAF_NODE;
        dNode->trapezoid = D;
        D->node = dNode;
    }
    
    // Build the search structure based on which trapezoids exist
    Node* oldNode = oldTrap->node;
    
    if (oldNode) {
        // Y-node to split B and C
        Node* sNode = new Node();
        sNode->type = Y_NODE;
        sNode->segment = seg;
        sNode->above = bNode;
        sNode->below = cNode;
        
        if (needLeftTrap && needRightTrap) {
            // Case 1: A, B, C, D all exist (4 trapezoids)
            // Structure: X_NODE(left) -> [A | X_NODE(right) -> [Y_NODE(seg) -> [B | C] | D]]
            Node* qNode = new Node();
            qNode->type = X_NODE;
            qNode->point = right;
            qNode->left = sNode;
            qNode->right = dNode;
            
            oldNode->type = X_NODE;
            oldNode->point = left;
            oldNode->left = aNode;
            oldNode->right = qNode;
            
        } else if (needLeftTrap && !needRightTrap) {
            // Case 2: A, B, C exist (3 trapezoids, no right cap)
            // Structure: X_NODE(left) -> [A | Y_NODE(seg) -> [B | C]]
            oldNode->type = X_NODE;
            oldNode->point = left;
            oldNode->left = aNode;
            oldNode->right = sNode;
            
        } else if (!needLeftTrap && needRightTrap) {
            // Case 3: B, C, D exist (3 trapezoids, no left cap)
            // Structure: X_NODE(right) -> [Y_NODE(seg) -> [B | C] | D]
            oldNode->type = X_NODE;
            oldNode->point = right;
            oldNode->left = sNode;
            oldNode->right = dNode;
            
        } else {
            // Case 4: Only B, C exist (2 trapezoids)
            // Structure: Y_NODE(seg) -> [B | C]
            oldNode->type = Y_NODE;
            oldNode->segment = seg;
            oldNode->above = bNode;
            oldNode->below = cNode;
        }
        
        // Clear old leaf node data
        oldNode->trapezoid = NULL;
        if (oldNode->type != Y_NODE || needLeftTrap || needRightTrap) {
            oldNode->segment = NULL;
            oldNode->above = NULL;
            oldNode->below = NULL;
        }
        if (oldNode->type != X_NODE) {
            oldNode->left = NULL;
            oldNode->right = NULL;
        }
    }
    
    // Update trapezoid list
    map.removeTrapezoid(oldTrap);
    if (A) map.addTrapezoid(A);
    map.addTrapezoid(B);
    map.addTrapezoid(C);
    if (D) map.addTrapezoid(D);
    
    int numTraps = (A ? 1 : 0) + 2 + (D ? 1 : 0);
    cout << "Created " << numTraps << " trapezoid(s): ";
    if (A) cout << "A ";
    cout << "B C ";
    if (D) cout << "D";
    cout << endl;
    
    delete oldTrap;
}

void insertAcrossMultipleTrapezoids(TrapezoidalMap& map,
                                    const vector<Trapezoid*>& intersected,
                                    Segment* seg) {
    if (intersected.empty()) return;
    
    cout << "=== Multiple trapezoid insertion ===" << endl;
    cout << "Intersecting " << intersected.size() << " trapezoids" << endl;
    
    Point left = seg->getLeftEndpoint();
    Point right = seg->getRightEndpoint();
    
    vector<Trapezoid*> upper;
    vector<Trapezoid*> lower;
    Trapezoid* leftTrap = NULL;
    Trapezoid* rightTrap = NULL;
    
    bool needLeftTrap = (intersected[0]->leftp.x < left.x - 1e-9);
    bool needRightTrap = (intersected.back()->rightp.x > right.x + 1e-9);
    
    if (needLeftTrap) {
        leftTrap = new Trapezoid();
        leftTrap->leftp = intersected[0]->leftp;
        leftTrap->rightp = left;
        leftTrap->top = intersected[0]->top;
        leftTrap->bottom = intersected[0]->bottom;
        leftTrap->upperLeft = intersected[0]->upperLeft;
        leftTrap->lowerLeft = intersected[0]->lowerLeft;
        
        cout << "Created left trap" << endl;
    }
    
    for (size_t i = 0; i < intersected.size(); i++) {
        Trapezoid* t = intersected[i];
        
        Point lp = t->leftp;
        Point rp = t->rightp;
        
        if (i == 0 && lp.x < left.x - 1e-9) {
            lp = left;
        }
        if (i == intersected.size() - 1 && rp.x > right.x + 1e-9) {
            rp = right;
        }
        
        Trapezoid* upperTrap = new Trapezoid();
        upperTrap->leftp = lp;
        upperTrap->rightp = rp;
        upperTrap->top = t->top;
        upperTrap->bottom = seg;
        
        Trapezoid* lowerTrap = new Trapezoid();
        lowerTrap->leftp = lp;
        lowerTrap->rightp = rp;
        lowerTrap->top = seg;
        lowerTrap->bottom = t->bottom;
        
        upper.push_back(upperTrap);
        lower.push_back(lowerTrap);
        
        cout << "Created upper/lower pair " << i << endl;
    }
    
    if (needRightTrap) {
        rightTrap = new Trapezoid();
        rightTrap->leftp = right;
        rightTrap->rightp = intersected.back()->rightp;
        rightTrap->top = intersected.back()->top;
        rightTrap->bottom = intersected.back()->bottom;
        rightTrap->upperRight = intersected.back()->upperRight;
        rightTrap->lowerRight = intersected.back()->lowerRight;
        
        cout << "Created right trap" << endl;
    }
    
    for (size_t i = 0; i < upper.size(); i++) {
        if (i > 0) {
            upper[i]->upperLeft = upper[i-1];
            upper[i]->lowerLeft = upper[i-1];
            upper[i-1]->upperRight = upper[i];
            upper[i-1]->lowerRight = upper[i];
            
            lower[i]->upperLeft = lower[i-1];
            lower[i]->lowerLeft = lower[i-1];
            lower[i-1]->upperRight = lower[i];
            lower[i-1]->lowerRight = lower[i];
        }
    }
    
    // Update neighbours
    if (leftTrap) {
        leftTrap->upperRight = upper[0];
        leftTrap->lowerRight = lower[0];
        upper[0]->upperLeft = leftTrap;
        upper[0]->lowerLeft = leftTrap;
        lower[0]->upperLeft = leftTrap;
        lower[0]->lowerLeft = leftTrap;
        
        if (intersected[0]->upperLeft) {
            if (intersected[0]->upperLeft->upperRight == intersected[0])
                intersected[0]->upperLeft->upperRight = leftTrap;
            if (intersected[0]->upperLeft->lowerRight == intersected[0])
                intersected[0]->upperLeft->lowerRight = leftTrap;
        }
        if (intersected[0]->lowerLeft) {
            if (intersected[0]->lowerLeft->upperRight == intersected[0])
                intersected[0]->lowerLeft->upperRight = leftTrap;
            if (intersected[0]->lowerLeft->lowerRight == intersected[0])
                intersected[0]->lowerLeft->lowerRight = leftTrap;
        }
    } else {
        upper[0]->upperLeft = intersected[0]->upperLeft;
        upper[0]->lowerLeft = intersected[0]->upperLeft;
        lower[0]->upperLeft = intersected[0]->lowerLeft;
        lower[0]->lowerLeft = intersected[0]->lowerLeft;
        
        if (intersected[0]->upperLeft) {
            if (intersected[0]->upperLeft->upperRight == intersected[0])
                intersected[0]->upperLeft->upperRight = upper[0];
            if (intersected[0]->upperLeft->lowerRight == intersected[0])
                intersected[0]->upperLeft->lowerRight = upper[0];
        }
        if (intersected[0]->lowerLeft) {
            if (intersected[0]->lowerLeft->upperRight == intersected[0])
                intersected[0]->lowerLeft->upperRight = lower[0];
            if (intersected[0]->lowerLeft->lowerRight == intersected[0])
                intersected[0]->lowerLeft->lowerRight = lower[0];
        }
    }
    
    if (rightTrap) {
        rightTrap->upperLeft = upper.back();
        rightTrap->lowerLeft = lower.back();
        upper.back()->upperRight = rightTrap;
        upper.back()->lowerRight = rightTrap;
        lower.back()->upperRight = rightTrap;
        lower.back()->lowerRight = rightTrap;
        
        if (intersected.back()->upperRight) {
            if (intersected.back()->upperRight->upperLeft == intersected.back())
                intersected.back()->upperRight->upperLeft = rightTrap;
            if (intersected.back()->upperRight->lowerLeft == intersected.back())
                intersected.back()->upperRight->lowerLeft = rightTrap;
        }
        if (intersected.back()->lowerRight) {
            if (intersected.back()->lowerRight->upperLeft == intersected.back())
                intersected.back()->lowerRight->upperLeft = rightTrap;
            if (intersected.back()->lowerRight->lowerLeft == intersected.back())
                intersected.back()->lowerRight->lowerLeft = rightTrap;
        }
    } else {
        upper.back()->upperRight = intersected.back()->upperRight;
        upper.back()->lowerRight = intersected.back()->upperRight;
        lower.back()->upperRight = intersected.back()->lowerRight;
        lower.back()->lowerRight = intersected.back()->lowerRight;
        
        if (intersected.back()->upperRight) {
            if (intersected.back()->upperRight->upperLeft == intersected.back())
                intersected.back()->upperRight->upperLeft = upper.back();
            if (intersected.back()->upperRight->lowerLeft == intersected.back())
                intersected.back()->upperRight->lowerLeft = upper.back();
        }
        if (intersected.back()->lowerRight) {
            if (intersected.back()->lowerRight->upperLeft == intersected.back())
                intersected.back()->lowerRight->upperLeft = lower.back();
            if (intersected.back()->lowerRight->lowerLeft == intersected.back())
                intersected.back()->lowerRight->lowerLeft = lower.back();
        }
    }
    
    // Update DAG
    for (size_t i = 0; i < intersected.size(); i++) {
        Node* oldNode = intersected[i]->node;
        if (!oldNode) continue;
        
        Node* upperNode = new Node();
        upperNode->type = LEAF_NODE;
        upperNode->trapezoid = upper[i];
        upper[i]->node = upperNode;
        
        Node* lowerNode = new Node();
        lowerNode->type = LEAF_NODE;
        lowerNode->trapezoid = lower[i];
        lower[i]->node = lowerNode;
        
        if (i == 0 && leftTrap) {
            Node* leftNode = new Node();
            leftNode->type = LEAF_NODE;
            leftNode->trapezoid = leftTrap;
            leftTrap->node = leftNode;
            
            Node* yNode = new Node();
            yNode->type = Y_NODE;
            yNode->segment = seg;
            yNode->above = upperNode;
            yNode->below = lowerNode;
            
            oldNode->type = X_NODE;
            oldNode->point = left;
            oldNode->left = leftNode;
            oldNode->right = yNode;
            oldNode->trapezoid = NULL;
            oldNode->segment = NULL;
            oldNode->above = NULL;
            oldNode->below = NULL;
            
            map.addTrapezoid(leftTrap);
            
        } else if (i == intersected.size() - 1 && rightTrap) {
            Node* yNode = new Node();
            yNode->type = Y_NODE;
            yNode->segment = seg;
            yNode->above = upperNode;
            yNode->below = lowerNode;
            
            Node* rightNode = new Node();
            rightNode->type = LEAF_NODE;
            rightNode->trapezoid = rightTrap;
            rightTrap->node = rightNode;
            
            oldNode->type = X_NODE;
            oldNode->point = right;
            oldNode->left = yNode;
            oldNode->right = rightNode;
            oldNode->trapezoid = NULL;
            oldNode->segment = NULL;
            oldNode->above = NULL;
            oldNode->below = NULL;
            
            map.addTrapezoid(rightTrap);
            
        } else {
            oldNode->type = Y_NODE;
            oldNode->segment = seg;
            oldNode->above = upperNode;
            oldNode->below = lowerNode;
            oldNode->trapezoid = NULL;
            oldNode->point = Point(0, 0);
            oldNode->left = NULL;
            oldNode->right = NULL;
        }
        
        map.removeTrapezoid(intersected[i]);
        map.addTrapezoid(upper[i]);
        map.addTrapezoid(lower[i]);
        
        delete intersected[i];
    }
    
    cout << "Finished multiple trapezoid insertion" << endl;
}

TrapezoidalMap BuildTrapezoidalMap(vector<Segment>& S) {
    TrapezoidalMap map;
    
    if (S.empty()) return map;
    
    double minX = 1e9, maxX = -1e9;
    double minY = 1e9, maxY = -1e9;
    
    for (size_t i = 0; i < S.size(); i++) {
        minX = min(minX, min(S[i].p1.x, S[i].p2.x));
        maxX = max(maxX, max(S[i].p1.x, S[i].p2.x));
        minY = min(minY, min(S[i].p1.y, S[i].p2.y));
        maxY = max(maxY, max(S[i].p1.y, S[i].p2.y));
    }
    
    double margin = max(maxX - minX, maxY - minY) * 0.1;
    minX -= margin; maxX += margin;
    minY -= margin; maxY += margin;
    
    Segment* topBound = new Segment(Point(minX, maxY), Point(maxX, maxY));
    Segment* bottomBound = new Segment(Point(minX, minY), Point(maxX, minY));
    
    Trapezoid* initialTrap = new Trapezoid();
    initialTrap->leftp = Point(minX, (minY + maxY) / 2.0);
    initialTrap->rightp = Point(maxX, (minY + maxY) / 2.0);
    initialTrap->top = topBound;
    initialTrap->bottom = bottomBound;
    
    Node* root = new Node();
    root->type = LEAF_NODE;
    root->trapezoid = initialTrap;
    initialTrap->node = root;
    
    map.root = root;
    map.addTrapezoid(initialTrap);
    map.segments.push_back(topBound);
    map.segments.push_back(bottomBound);
    
    srand(static_cast<unsigned>(time(0)));
    // for (size_t i = 0; i < S.size(); i++) {
    //     size_t j = i + rand() % (S.size() - i);
    //     swap(S[i], S[j]);
    // }
    
    for (size_t i = 0; i < S.size(); i++) {
        Segment* seg = new Segment(S[i]);
        map.segments.push_back(seg);
        
        cout << "\n========================================" << endl;
        cout << "Inserting segment " << i << ": (" << seg->p1.x << "," << seg->p1.y 
             << ") -> (" << seg->p2.x << "," << seg->p2.y << ")" << endl;
        
        vector<Trapezoid*> intersected;
        findIntersectedTrapezoids(map.root, *seg, intersected);
        
        cout << "Segment " << i << " intersects " << intersected.size() << " trapezoids" << endl;
        
        if (intersected.empty()) {
            cout << "WARNING: Segment doesn't intersect any trapezoids" << endl;
            continue;
        }
        
        if (intersected.size() == 1) {
            insertInSingleTrapezoid(map, intersected[0], seg);
        } else {
            insertAcrossMultipleTrapezoids(map, intersected, seg);
        }
        
        cout << "========================================\n" << endl;
    }
    
    return map;
}

void deleteSearchStructure(Node* node) {
    if (node == NULL) return;
    
    set<Node*> visited;
    vector<Node*> toDelete;
    vector<Node*> stack;
    stack.push_back(node);
    
    while (!stack.empty()) {
        Node* current = stack.back();
        stack.pop_back();
        
        if (current == NULL || visited.count(current) > 0) {
            continue;
        }
        
        visited.insert(current);
        toDelete.push_back(current);
        
        if (current->type == X_NODE) {
            if (current->left) stack.push_back(current->left);
            if (current->right) stack.push_back(current->right);
        } else if (current->type == Y_NODE) {
            if (current->above) stack.push_back(current->above);
            if (current->below) stack.push_back(current->below);
        }
    }
    
    for (size_t i = 0; i < toDelete.size(); i++) {
        delete toDelete[i];
    }
}

void validateTrapezoid(Trapezoid* t) {
    if (t == NULL) {
        cout << "ERROR: NULL trapezoid" << endl;
        return;
    }
    
    if (t->leftp.x > t->rightp.x + 1e-9) {
        cout << "ERROR: Trapezoid has invalid x-range: left=" << t->leftp.x 
             << " right=" << t->rightp.x << endl;
    }
    
    if (t->top == NULL) {
        cout << "ERROR: Trapezoid has NULL top segment" << endl;
    }
    
    if (t->bottom == NULL) {
        cout << "ERROR: Trapezoid has NULL bottom segment" << endl;
    }
    
    if (t->node == NULL) {
        cout << "WARNING: Trapezoid has NULL node" << endl;
    }
}

void validateSearchStructure(Node* node) {
    if (node == NULL) {
        cout << "ERROR: NULL node in search structure" << endl;
        return;
    }
    
    set<Node*> visited;
    vector<Node*> stack;
    stack.push_back(node);
    
    while (!stack.empty()) {
        Node* current = stack.back();
        stack.pop_back();
        
        if (current == NULL || visited.count(current) > 0) {
            continue;
        }
        
        visited.insert(current);
        
        if (current->type == LEAF_NODE) {
            if (current->trapezoid == NULL) {
                cout << "ERROR: Leaf node has NULL trapezoid" << endl;
            } else {
                validateTrapezoid(current->trapezoid);
            }
        } else if (current->type == X_NODE) {
            if (current->left == NULL || current->right == NULL) {
                cout << "ERROR: X_NODE has NULL children" << endl;
            }
            if (current->left) stack.push_back(current->left);
            if (current->right) stack.push_back(current->right);
        } else if (current->type == Y_NODE) {
            if (current->segment == NULL) {
                cout << "ERROR: Y_NODE has NULL segment" << endl;
            }
            if (current->above == NULL || current->below == NULL) {
                cout << "ERROR: Y_NODE has NULL children" << endl;
            }
            if (current->above) stack.push_back(current->above);
            if (current->below) stack.push_back(current->below);
        }
    }
}

void printTrapezoid(Trapezoid* t) {
    if (t == NULL) {
        cout << "NULL trapezoid" << endl;
        return;
    }
    
    cout << "Trap " << t << ": left=(" << t->leftp.x << "," << t->leftp.y 
         << ") right=(" << t->rightp.x << "," << t->rightp.y << ")" 
         << " top=" << t->top << " bottom=" << t->bottom << endl;
}

void debugIntersection(const vector<Trapezoid*>& traps, const Segment& seg) {
    cout << "=== DEBUG: Segment intersection ===" << endl;
    cout << "Segment from (" << seg.p1.x << "," << seg.p1.y 
         << ") to (" << seg.p2.x << "," << seg.p2.y << ")" << endl;
    cout << "Intersects " << traps.size() << " trapezoids:" << endl;
    for (size_t i = 0; i < traps.size(); i++) {
        cout << "  " << i << ": ";
        printTrapezoid(traps[i]);
    }
    cout << "===================================" << endl;
}
