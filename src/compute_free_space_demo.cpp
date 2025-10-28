#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>

#include "data_structure.hpp"
#include "trapezoidal_map.hpp"
#include "compute_free_space.hpp"
#include "sdl_layer.hpp"


void compute_free_space_demo() {
    // Application start
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    if (!sdl_start(window, renderer, font)) return;

    std::vector<Polygon> polygons;
    Polygon triangle;
    triangle.addVertex(25, 30);
    triangle.addVertex(30, 75);
    triangle.addVertex(55, 85);
    triangle.addVertex(65, 45);
    polygons.push_back(triangle);
    
    Polygon triangle2;
    triangle2.addVertex(62, 80);
    triangle2.addVertex(95, 70);
    triangle2.addVertex(90, 40);
    polygons.push_back(triangle2);

    Polygon triangle3;
    triangle3.addVertex(68, 20);
    triangle3.addVertex(75, 50);
    triangle3.addVertex(92, 25);
    polygons.push_back(triangle3);

    std::cout << "Computing free space..." << std::endl;
    TrapezoidalMap freeSpaceMap = FreeSpaceComputer::COMPUTEFREESPACE(polygons);
    std::cout << "Free space computed. Found " << freeSpaceMap.trapezoids.size() << " free trapezoids." << std::endl;

    std::vector<Segment> allEdges = FreeSpaceComputer::extractEdges(polygons);
    TrapezoidalMap originalMap = BuildTrapezoidalMap(allEdges);
    std::cout << "Original map has " << originalMap.trapezoids.size() << " trapezoids." << std::endl;

    bool running = true;
    SDL_Event event;
    Trapezoid* highlightedTrap = NULL;
    Node* highlightedNode = NULL;
    bool showOriginalMap = false;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    showOriginalMap = !showOriginalMap;
                    std::cout << (showOriginalMap ? "Showing original map" : "Showing free space") << std::endl;
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;
                    SDL_Point mousePos = {mouseX, mouseY};
                    Point worldPos = screenToWorld(mousePos);
                    
                    std::cout << "Querying map at (" << worldPos.x << ", " << worldPos.y << ")" << std::endl;

                    TrapezoidalMap& currentMap = showOriginalMap ? originalMap : freeSpaceMap;
                    Node* leaf = queryTrapezoidMap(currentMap.root, worldPos);
                    if (leaf && leaf->type == LEAF_NODE) {
                        highlightedTrap = leaf->trapezoid;
                        highlightedNode = leaf;
                        std::cout << "Found trapezoid at leaf node." << std::endl;
                    } else {
                        highlightedTrap = NULL;
                        highlightedNode = NULL;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawLine(renderer, MAP_WIDTH, 0, MAP_WIDTH, SCREEN_HEIGHT);

        // Draw obstacles
        for (const Polygon& poly : polygons) {
            drawPolygon(renderer, poly, 255, 100, 100, 150);
        }

        // Draw trapezoids
        TrapezoidalMap& currentMap = showOriginalMap ? originalMap : freeSpaceMap;
        for (Trapezoid* trap : currentMap.trapezoids) {
            Uint8 r, g, b;
            if (showOriginalMap) {
                r = g = b = 220;
            } else {
                r = 100; g = 255; b = 100;
            }
            drawTrapezoid(renderer, trap, r, g, b, 150);
        }

        // Highlight selected trapezoid
        if (highlightedTrap) {
            drawTrapezoid(renderer, highlightedTrap, 255, 0, 0, 200);
        }
        
        // Draw trapezoid boundaries
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 100);
        for (Trapezoid* trap : currentMap.trapezoids) {
            SDL_FPoint tl = worldToScreen({trap->leftp.x, trap->top->getY(trap->leftp.x)});
            SDL_FPoint tr = worldToScreen({trap->rightp.x, trap->top->getY(trap->rightp.x)});
            SDL_FPoint bl = worldToScreen({trap->leftp.x, trap->bottom->getY(trap->leftp.x)});
            SDL_FPoint br = worldToScreen({trap->rightp.x, trap->bottom->getY(trap->rightp.x)});
            
            SDL_RenderDrawLineF(renderer, tl.x, tl.y, bl.x, bl.y);
            SDL_RenderDrawLineF(renderer, tr.x, tr.y, br.x, br.y);
        }

        // Draw polygon edges
        for (const Polygon& poly : polygons) {
            for (size_t i = 0; i < poly.vertices.size(); i++) {
                size_t next = (i + 1) % poly.vertices.size();
                Segment seg(poly.vertices[i], poly.vertices[next]);
                drawSegment(renderer, seg, 0, 0, 0, 255);
            }
        }
        
        SDL_RenderPresent(renderer);
    }

    std::cout << "Cleaning up..." << std::endl;
    if (font) TTF_CloseFont(font);
    originalMap.cleanup();
    freeSpaceMap.cleanup();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
