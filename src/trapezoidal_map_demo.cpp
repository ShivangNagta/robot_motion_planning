#include "demo/trapezoidal_map_demo.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>

#include "data_structure.hpp"
#include "trapezoidal_map.hpp"
#include "sdl_layer.hpp"

void trapezoidal_map_demo() {
    // Application start
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    if (!sdl_start(window, renderer, font)) return;

    // std::vector<Segment> segments = {
    //     Segment(Point(20, 30), Point(60, 80)),
    //     Segment(Point(60, 80), Point(50, 30)),
    //     Segment(Point(50, 30), Point(40, 20)),
    //     Segment(Point(40, 20), Point(20, 30)),

    //     Segment(Point(70, 30), Point(90, 80)),
    //     Segment(Point(90, 80), Point(80, 30)),
    //     Segment(Point(80, 30), Point(75, 20)),
    //     Segment(Point(75, 20), Point(70, 30)),
    // };

    // Book demo
    //     std::vector<Segment> segments = {

    //     Segment(Point(25, 30), Point(30, 75)),
    //     Segment(Point(30, 75), Point(55, 85)),
    //     Segment(Point(55, 85), Point(65, 45)),
    //     Segment(Point(65, 45), Point(25, 30)),

    //     Segment(Point(62, 80), Point(95, 70)),
    //     Segment(Point(95, 70), Point(90, 40)),
    //     Segment(Point(90, 40), Point(62, 80)),

    //     Segment(Point(60, 20), Point(75, 50)),
    //     Segment(Point(75, 50), Point(90, 25)),
    //     Segment(Point(90, 25), Point(60, 20))
    // };

    std::vector<Segment> segments = {
    Segment(Point(25, 30), Point(30, 75)),
    Segment(Point(30, 75), Point(55, 85)),
    Segment(Point(55, 85), Point(65, 45)),
    Segment(Point(65, 45), Point(25, 30)),
    
    Segment(Point(62, 80), Point(95, 70)),
    Segment(Point(95, 70), Point(90, 40)),
    Segment(Point(90, 40), Point(62, 80)),

    Segment(Point(68, 20), Point(75, 50)),
    Segment(Point(75, 50), Point(92, 25)),
    Segment(Point(92, 25), Point(68, 20))
};

    std::cout << "Building trapezoidal map..." << std::endl;
    TrapezoidalMap map = BuildTrapezoidalMap(segments);
    std::cout << "Map built. Found " << map.trapezoids.size() << " trapezoids." << std::endl;

    std::map<Node*, NodePosition> dagPositions;
    std::map<int, int> levelCounters;
    int nodeCounter = 0;
    calculateDAGLayout(map.root, dagPositions, 0, nodeCounter, levelCounters);
    std::cout << "DAG has " << dagPositions.size() << " nodes." << std::endl;

    bool running = true;
    SDL_Event event;
    Trapezoid* highlightedTrap = NULL;
    Node* highlightedNode = NULL;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;
                    
                    if (mouseX < MAP_WIDTH) {
                        SDL_Point mousePos = {mouseX, mouseY};
                        Point worldPos = screenToWorld(mousePos);
                        
                        std::cout << "Querying map at (" << worldPos.x << ", " << worldPos.y << ")" << std::endl;

                        Node* leaf = queryTrapezoidMap(map.root, worldPos);
                        if (leaf && leaf->type == LEAF_NODE) {
                            highlightedTrap = leaf->trapezoid;
                            highlightedNode = leaf;
                            std::cout << "Found trapezoid at leaf node." << std::endl;
                        } else {
                            highlightedTrap = NULL;
                            highlightedNode = NULL;
                        }
                    } else {
                        highlightedNode = NULL;
                        highlightedTrap = NULL;
                        
                        for (auto& pair : dagPositions) {
                            Node* node = pair.first;
                            NodePosition& pos = pair.second;
                            
                            int dx = mouseX - pos.x;
                            int dy = mouseY - pos.y;
                            if (dx*dx + dy*dy <= DAG_NODE_RADIUS*DAG_NODE_RADIUS) {
                                highlightedNode = node;
                                if (node->type == LEAF_NODE) {
                                    highlightedTrap = node->trapezoid;
                                }
                                std::cout << "Clicked on ";
                                if (node->type == X_NODE) std::cout << "X-Node";
                                else if (node->type == Y_NODE) std::cout << "Y-Node";
                                else std::cout << "Leaf Node";
                                std::cout << std::endl;
                                break;
                            }
                        }
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawLine(renderer, MAP_WIDTH, 0, MAP_WIDTH, SCREEN_HEIGHT);

        for (Trapezoid* trap : map.trapezoids) {
            drawTrapezoid(renderer, trap, 220, 220, 220, 150);
        }

        if (highlightedTrap) {
            drawTrapezoid(renderer, highlightedTrap, 255, 0, 0, 200);
        }
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 100);
        for (Trapezoid* trap : map.trapezoids) {
            SDL_FPoint tl = worldToScreen({trap->leftp.x, trap->top->getY(trap->leftp.x)});
            SDL_FPoint tr = worldToScreen({trap->rightp.x, trap->top->getY(trap->rightp.x)});
            SDL_FPoint bl = worldToScreen({trap->leftp.x, trap->bottom->getY(trap->leftp.x)});
            SDL_FPoint br = worldToScreen({trap->rightp.x, trap->bottom->getY(trap->rightp.x)});
            
            SDL_RenderDrawLineF(renderer, tl.x, tl.y, bl.x, bl.y);
            SDL_RenderDrawLineF(renderer, tr.x, tr.y, br.x, br.y);
        }

        for (const auto& seg : segments) {
            drawSegment(renderer, seg, 0, 0, 0, 255);
            SDL_FPoint p1 = worldToScreen(seg.p1);
            SDL_FPoint p2 = worldToScreen(seg.p2);
            SDL_RenderDrawLineF(renderer, p1.x-0.5f, p1.y, p2.x-0.5f, p2.y);
            SDL_RenderDrawLineF(renderer, p1.x+0.5f, p1.y, p2.x+0.5f, p2.y);
        }
        
        if (font) {
            drawDAG(renderer, font, map.root, dagPositions, highlightedNode);
            drawLegend(renderer, font);
            
            SDL_Color titleColor = {0, 0, 0, 255};
            drawText(renderer, font, "Search Structure (DAG)", MAP_WIDTH + DAG_WIDTH/2, 20, titleColor);
        }

        SDL_RenderPresent(renderer);
    }

    std::cout << "Cleaning up..." << std::endl;
    if (font) TTF_CloseFont(font);
    map.cleanup();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
