#include "sdl_layer.hpp"

bool sdl_start(SDL_Window*& window, SDL_Renderer*& renderer, TTF_Font*& font) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }
    
    if (TTF_Init() < 0) {
        std::cerr << "Could not initialize SDL_ttf: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    window = SDL_CreateWindow("Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    font = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 14);
    if (!font) {
        std::cerr << "Could not load font; text will not be shown." << std::endl;
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return true;
}

SDL_FPoint worldToScreen(const Point& p, bool rightSide) {
    double offsetX = rightSide ? TRAP_OFFSET_X : OFFSET_X;
    float x = static_cast<float>(offsetX + (p.x - WORLD_MIN_X) * SCALE);
    float y = static_cast<float>(OFFSET_Y + SCREEN_H_GFX - (p.y - WORLD_MIN_Y) * SCALE);
    return {x, y};
}

Point screenToWorld(const SDL_Point& p) {
    double x = WORLD_MIN_X + (p.x - OFFSET_X) / SCALE;
    double y = WORLD_MIN_Y + (SCREEN_H_GFX - (p.y - OFFSET_Y)) / SCALE;
    return {x, y};
}

void drawSegment(SDL_Renderer* renderer, const Segment& seg, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool rightSide) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_FPoint p1 = worldToScreen(seg.p1, rightSide);
    SDL_FPoint p2 = worldToScreen(seg.p2, rightSide);
    SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
}

void drawPolygon(SDL_Renderer* renderer, const Polygon& poly, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool rightSide) {
    if (poly.vertices.size() < 3) return;
    
    std::vector<SDL_Vertex> vertices;
    for (const Point& p : poly.vertices) {
        SDL_FPoint screenPos = worldToScreen(p, rightSide);
        SDL_Vertex vertex = { screenPos, {r, g, b, a}, {0, 0} };
        vertices.push_back(vertex);
    }
    
    std::vector<int> indices;
    for (size_t i = 1; i < poly.vertices.size() - 1; i++) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    
    if (!indices.empty()) {
        SDL_RenderGeometry(renderer, NULL, vertices.data(), vertices.size(), 
                          indices.data(), indices.size());
    }
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (size_t i = 0; i < poly.vertices.size(); i++) {
        size_t next = (i + 1) % poly.vertices.size();
        SDL_FPoint p1 = worldToScreen(poly.vertices[i], rightSide);
        SDL_FPoint p2 = worldToScreen(poly.vertices[next], rightSide);
        SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
    }
}

void drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius, Uint8 r=255, Uint8 g=255, Uint8 b=255, Uint8 a=255) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}

void drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, 
              int x, int y, SDL_Color color, bool centered) {
    if (!font) return;
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect destRect;
    if (centered) {
        destRect = {x - surface->w/2, y - surface->h/2, surface->w, surface->h};
    } else {
        destRect = {x, y, surface->w, surface->h};
    }
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void drawTrapezoid(SDL_Renderer* renderer, Trapezoid* trap, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool rightSide) {
    if (!trap->top || !trap->bottom) return;

    Point tl = {trap->leftp.x, trap->top->getY(trap->leftp.x)};
    Point tr = {trap->rightp.x, trap->top->getY(trap->rightp.x)};
    Point bl = {trap->leftp.x, trap->bottom->getY(trap->leftp.x)};
    Point br = {trap->rightp.x, trap->bottom->getY(trap->rightp.x)};

    SDL_FPoint v_tl = worldToScreen(tl, rightSide);
    SDL_FPoint v_tr = worldToScreen(tr, rightSide);
    SDL_FPoint v_bl = worldToScreen(bl, rightSide);
    SDL_FPoint v_br = worldToScreen(br, rightSide);

    SDL_Vertex vertices[4] = {
        { v_tl, {r, g, b, a}, {0, 0} },
        { v_tr, {r, g, b, a}, {0, 0} },
        { v_bl, {r, g, b, a}, {0, 0} },
        { v_br, {r, g, b, a}, {0, 0} }
    };

    int indices[6] = { 0, 1, 2,  2, 1, 3 };
    SDL_RenderGeometry(renderer, NULL, vertices, 4, indices, 6);
}

void calculateDAGLayout(Node* node, std::map<Node*, NodePosition>& positions, 
                       int depth, int& nodeCounter, std::map<int, int>& levelCounters) {
    if (!node || positions.find(node) != positions.end()) return;
    
    int positionInLevel = levelCounters[depth]++;
    
    NodePosition pos;
    pos.node = node;
    pos.x = DAG_START_X + positionInLevel * DAG_NODE_SPACING;
    pos.y = DAG_START_Y + depth * DAG_LEVEL_HEIGHT;
    positions[node] = pos;
    
    if (node->type == X_NODE) {
        calculateDAGLayout(node->left, positions, depth + 1, nodeCounter, levelCounters);
        calculateDAGLayout(node->right, positions, depth + 1, nodeCounter, levelCounters);
    } else if (node->type == Y_NODE) {
        calculateDAGLayout(node->above, positions, depth + 1, nodeCounter, levelCounters);
        calculateDAGLayout(node->below, positions, depth + 1, nodeCounter, levelCounters);
    }
}

void drawThickLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int thickness) {
    for (int i = -thickness/2; i <= thickness/2; i++) {
        SDL_RenderDrawLine(renderer, x1 + i, y1, x2 + i, y2);
        SDL_RenderDrawLine(renderer, x1, y1 + i, x2, y2 + i);
    }
}

void drawDAGNode(SDL_Renderer* renderer, TTF_Font* font, Node* node, 
                 int x, int y, bool highlighted) {
    Uint8 r, g, b;
    const char* label;
    char buffer[32];
    
    if (node->type == X_NODE) {
        r = 100; g = 150; b = 255;
        snprintf(buffer, sizeof(buffer), "X(%.1f)", node->point.x);
        label = buffer;
    } else if (node->type == Y_NODE) {
        r = 100; g = 255; b = 150;
        label = "Y";
    } else {
        r = 255; g = 200; b = 100;
        label = "T";
    }
    
    if (highlighted) {
        r = 255; g = 50; b = 50;
    }
    
    drawCircle(renderer, x, y, DAG_NODE_RADIUS, r, g, b, 255);
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int angle = 0; angle < 360; angle += 10) {
        int x1 = x + DAG_NODE_RADIUS * cos(angle * M_PI / 180);
        int y1 = y + DAG_NODE_RADIUS * sin(angle * M_PI / 180);
        int x2 = x + DAG_NODE_RADIUS * cos((angle + 10) * M_PI / 180);
        int y2 = y + DAG_NODE_RADIUS * sin((angle + 10) * M_PI / 180);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
    
    SDL_Color textColor = {0, 0, 0, 255};
    drawText(renderer, font, label, x, y, textColor);
}

void drawDAGEdge(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, 
                 bool isLeft, bool highlighted) {
    double angle = atan2(y2 - y1, x2 - x1);
    int startX = x1 + DAG_NODE_RADIUS * cos(angle);
    int startY = y1 + DAG_NODE_RADIUS * sin(angle);
    int endX = x2 - DAG_NODE_RADIUS * cos(angle);
    int endY = y2 - DAG_NODE_RADIUS * sin(angle);
    
    if (highlighted) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        drawThickLine(renderer, startX, startY, endX, endY, 3);
    } else {
        if (isLeft) {
            SDL_SetRenderDrawColor(renderer, 50, 50, 200, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
        }
        SDL_RenderDrawLine(renderer, startX, startY, endX, endY);
    }
}

void drawDAG(SDL_Renderer* renderer, TTF_Font* font, Node* root, 
             std::map<Node*, NodePosition>& positions, Node* highlightNode) {
    for (auto& pair : positions) {
        Node* node = pair.first;
        NodePosition& pos = pair.second;
        
        if (node->type == X_NODE) {
            if (node->left && positions.find(node->left) != positions.end()) {
                NodePosition& childPos = positions[node->left];
                bool highlight = (highlightNode == node || highlightNode == node->left);
                drawDAGEdge(renderer, pos.x, pos.y, childPos.x, childPos.y, true, highlight);
            }
            if (node->right && positions.find(node->right) != positions.end()) {
                NodePosition& childPos = positions[node->right];
                bool highlight = (highlightNode == node || highlightNode == node->right);
                drawDAGEdge(renderer, pos.x, pos.y, childPos.x, childPos.y, false, highlight);
            }
        } else if (node->type == Y_NODE) {
            if (node->above && positions.find(node->above) != positions.end()) {
                NodePosition& childPos = positions[node->above];
                bool highlight = (highlightNode == node || highlightNode == node->above);
                drawDAGEdge(renderer, pos.x, pos.y, childPos.x, childPos.y, true, highlight);
            }
            if (node->below && positions.find(node->below) != positions.end()) {
                NodePosition& childPos = positions[node->below];
                bool highlight = (highlightNode == node || highlightNode == node->below);
                drawDAGEdge(renderer, pos.x, pos.y, childPos.x, childPos.y, false, highlight);
            }
        }
    }
    
    for (auto& pair : positions) {
        Node* node = pair.first;
        NodePosition& pos = pair.second;
        bool highlight = (node == highlightNode);
        drawDAGNode(renderer, font, node, pos.x, pos.y, highlight);
    }
}

void drawLegend(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Color textColor = {0, 0, 0, 255};
    
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_Rect legendRect = {2*MAP_WIDTH - 20 - 300 , 150, 300, 130};
    SDL_RenderFillRect(renderer, &legendRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &legendRect);
    
    int legendX = 2 * MAP_WIDTH - 280;
    int legendY = 140 + 30;
    drawCircle(renderer, legendX, legendY, 15, 100, 150, 255, 255);
    drawText(renderer, font, "X-Node (Point Split)", legendX + 100, legendY, textColor);
    
    legendY += 40;
    drawCircle(renderer, legendX, legendY, 15, 100, 255, 150, 255);
    drawText(renderer, font, "Y-Node (Segment Split)", legendX + 110, legendY, textColor);
    
    legendY += 40;
    drawCircle(renderer, legendX, legendY, 15, 255, 200, 100, 255);
    drawText(renderer, font, "Leaf (Trapezoid)", legendX + 95, legendY, textColor);
}

void drawPath(SDL_Renderer* renderer, const std::vector<Point>& path, Uint8 r, Uint8 g, Uint8 b, bool rightSide) {
    if (path.size() < 2) return;
    
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    
    for (size_t i = 0; i < path.size() - 1; i++) {
        SDL_FPoint p1 = worldToScreen(path[i], rightSide);
        SDL_FPoint p2 = worldToScreen(path[i+1], rightSide);
        SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
    }
    for (size_t i = 0; i < path.size(); i++) {
        SDL_FPoint p = worldToScreen(path[i], rightSide);
        int radius = (i == 0 || i == path.size() - 1) ? 8 : 5;
        Uint8 pointR, pointG, pointB;
        
        if (i == 0) {
            pointR = 0; pointG = 255; pointB = 0;
        } else if (i == path.size() - 1) {
            pointR = 255; pointG = 0; pointB = 0;
        } else {
            pointR = r; pointG = g; pointB = b;
        }
        
        drawCircle(renderer, static_cast<int>(p.x), static_cast<int>(p.y), radius, pointR, pointG, pointB, 255);
    }
}


void drawRoadMap(SDL_Renderer* renderer, RoadMap& roadMap, bool rightSide) {
    // Draw edges
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 100);
    for (RoadMapNode* node : roadMap.nodes) {
        for (RoadMapNode* neighbor : node->neighbors) {
            SDL_FPoint p1 = worldToScreen(node->position, rightSide);
            SDL_FPoint p2 = worldToScreen(neighbor->position, rightSide);
            SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
        }
    }
    // Draw nodes
    for (RoadMapNode* node : roadMap.nodes) {
        SDL_FPoint p = worldToScreen(node->position, rightSide);
        drawCircle(renderer, static_cast<int>(p.x), static_cast<int>(p.y), 4, 100, 100, 255, 255);
    }
}
