#pragma once

#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <map>

#include "data_structure.hpp"
#include "compute_path.hpp"

static const int SCREEN_WIDTH = 1600;
static const int SCREEN_HEIGHT = 900;

static const int MAP_WIDTH = 800;
static const int DAG_WIDTH = 800;
static const int TRAP_VIZ_WIDTH = 800;

static const double WORLD_MIN_X = 0;
static const double WORLD_MAX_X = 100;
static const double WORLD_MIN_Y = 0;
static const double WORLD_MAX_Y = 100;
static const double WORLD_W = WORLD_MAX_X - WORLD_MIN_X;
static const double WORLD_H = WORLD_MAX_Y - WORLD_MIN_Y;
static const double SCREEN_PAD = 50.0;
static const double SCREEN_W_GFX = MAP_WIDTH - 2.0 * SCREEN_PAD;
static const double SCREEN_H_GFX = SCREEN_HEIGHT - 2.0 * SCREEN_PAD;
static const double SCALE_X = SCREEN_W_GFX / WORLD_W;
static const double SCALE_Y = SCREEN_H_GFX / WORLD_H;
static const double SCALE = std::min(SCALE_X, SCALE_Y);
static const double OFFSET_X = SCREEN_PAD + (SCREEN_W_GFX - WORLD_W * SCALE) / 2.0;
static const double OFFSET_Y = SCREEN_PAD + (SCREEN_H_GFX - WORLD_H * SCALE) / 2.0;
static const int DAG_NODE_RADIUS = 15;
static const int DAG_LEVEL_HEIGHT = 40;
static const int DAG_NODE_SPACING = 60;
static const int DAG_START_X = MAP_WIDTH + 50;
static const int DAG_START_Y = 50;
static const double TRAP_OFFSET_X = MAP_WIDTH + SCREEN_PAD + (SCREEN_W_GFX - WORLD_W * SCALE) / 2.0;
static const double TRAP_OFFSET_Y = SCREEN_PAD + (SCREEN_H_GFX - WORLD_H * SCALE) / 2.0;

struct NodePosition {
    int x, y;
    Node* node;
};

bool sdl_start(SDL_Window*& window, SDL_Renderer*& renderer, TTF_Font*& font);

SDL_FPoint worldToScreen(const Point& p, bool rightSide = false);
Point screenToWorld(const SDL_Point& p);
void drawSegment(SDL_Renderer* renderer, const Segment& seg, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool rightSide = false);
void drawPolygon(SDL_Renderer* renderer, const Polygon& poly, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool rightSide = false);
void drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, 
              int x, int y, SDL_Color color, bool centered = true);
void drawTrapezoid(SDL_Renderer* renderer, Trapezoid* trap, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool rightSide = false);
void calculateDAGLayout(Node* node, std::map<Node*, NodePosition>& positions, 
                       int depth, int& nodeCounter, std::map<int, int>& levelCounters);
void drawThickLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int thickness);
void drawDAGNode(SDL_Renderer* renderer, TTF_Font* font, Node* node, 
                 int x, int y, bool highlighted);
void drawDAGEdge(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, 
                 bool isLeft, bool highlighted);
void drawDAG(SDL_Renderer* renderer, TTF_Font* font, Node* root, 
             std::map<Node*, NodePosition>& positions, Node* highlightNode);
void drawLegend(SDL_Renderer* renderer, TTF_Font* font);
void drawPath(SDL_Renderer* renderer, const std::vector<Point>& path, Uint8 r, Uint8 g, Uint8 b, bool rightSide = false);

void drawRoadMap(SDL_Renderer* renderer, RoadMap& roadMap, bool rightSide = false);
