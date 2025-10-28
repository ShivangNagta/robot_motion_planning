#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include "data_structure.hpp"
#include "trapezoidal_map.hpp"
#include "compute_free_space.hpp"
#include "compute_path.hpp"

#include "sdl_layer.hpp"

void compute_path_demo() {
    // Application start
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    if (!sdl_start(window, renderer, font)) return;

    // Obstacle polygons
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
    
    std::cout << "Building roadmap..." << std::endl;
    RoadMap roadMap = PathComputer::buildRoadMap(freeSpaceMap);

    Point start(20, 50);
    Point goal(85, 60);

    std::cout << "Computing path from (" << start.x << ", " << start.y 
              << ") to (" << goal.x << ", " << goal.y << ")" << std::endl;
    
    std::vector<Point> path = PathComputer::COMPUTEPATH(freeSpaceMap, roadMap, start, goal);

    bool running = true;
    SDL_Event event;
    bool showRoadmap = true;
    bool selectingStart = true;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_r) {
                    showRoadmap = !showRoadmap;
                    std::cout << (showRoadmap ? "Showing roadmap" : "Hiding roadmap") << std::endl;
                }
                if (event.key.keysym.sym == SDLK_c) {
                    path = PathComputer::COMPUTEPATH(freeSpaceMap, roadMap, start, goal);
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;
                    
                    if (mouseX < MAP_WIDTH) {
                        SDL_Point mousePos = {mouseX, mouseY};
                        Point worldPos = screenToWorld(mousePos);
                        
                        if (selectingStart) {
                            start = worldPos;
                            std::cout << "New start: (" << start.x << ", " << start.y << ")" << std::endl;
                        } else {
                            goal = worldPos;
                            std::cout << "New goal: (" << goal.x << ", " << goal.y << ")" << std::endl;
                        }
                        selectingStart = !selectingStart;
                        
                        path = PathComputer::COMPUTEPATH(freeSpaceMap, roadMap, start, goal);
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawLine(renderer, MAP_WIDTH, 0, MAP_WIDTH, SCREEN_HEIGHT);

        // ===== LEFT SIDE =====
        
        for (Trapezoid* trap : freeSpaceMap.trapezoids) {
            drawTrapezoid(renderer, trap, 200, 255, 200, 100, false);
        }

        // Draw obstacles
        for (const Polygon& poly : polygons) {
            drawPolygon(renderer, poly, 255, 100, 100, 150, false);
        }

        if (showRoadmap) {
            drawRoadMap(renderer, roadMap, false);
        }

        if (!path.empty()) {
            drawPath(renderer, path, 0, 0, 255, false);
        }

        SDL_FPoint startScreen = worldToScreen(start, false);
        SDL_FPoint goalScreen = worldToScreen(goal, false);
        drawCircle(renderer, static_cast<int>(startScreen.x), static_cast<int>(startScreen.y), 10, 0, 255, 0, 255);
        drawCircle(renderer, static_cast<int>(goalScreen.x), static_cast<int>(goalScreen.y), 10, 255, 0, 0, 255);

        for (const Polygon& poly : polygons) {
            for (size_t i = 0; i < poly.vertices.size(); i++) {
                size_t next = (i + 1) % poly.vertices.size();
                Segment seg(poly.vertices[i], poly.vertices[next]);
                drawSegment(renderer, seg, 0, 0, 0, 255, false);
            }
        }

        // ===== RIGHT SIDE =====
        
        for (Trapezoid* trap : freeSpaceMap.trapezoids) {
            drawTrapezoid(renderer, trap, 220, 220, 255, 150, true);
        }
    
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 200);
        for (Trapezoid* trap : freeSpaceMap.trapezoids) {
            SDL_FPoint tl = worldToScreen({trap->leftp.x, trap->top->getY(trap->leftp.x)}, true);
            SDL_FPoint tr = worldToScreen({trap->rightp.x, trap->top->getY(trap->rightp.x)}, true);
            SDL_FPoint bl = worldToScreen({trap->leftp.x, trap->bottom->getY(trap->leftp.x)}, true);
            SDL_FPoint br = worldToScreen({trap->rightp.x, trap->bottom->getY(trap->rightp.x)}, true);
            
            SDL_RenderDrawLineF(renderer, tl.x, tl.y, bl.x, bl.y);
            SDL_RenderDrawLineF(renderer, tr.x, tr.y, br.x, br.y);
        }

        for (const Polygon& poly : polygons) {
            for (size_t i = 0; i < poly.vertices.size(); i++) {
                size_t next = (i + 1) % poly.vertices.size();
                Segment seg(poly.vertices[i], poly.vertices[next]);
                drawSegment(renderer, seg, 0, 0, 0, 255, true);
                SDL_FPoint p1 = worldToScreen(seg.p1, true);
                SDL_FPoint p2 = worldToScreen(seg.p2, true);
                SDL_RenderDrawLineF(renderer, p1.x-0.5f, p1.y, p2.x-0.5f, p2.y);
                SDL_RenderDrawLineF(renderer, p1.x+0.5f, p1.y, p2.x+0.5f, p2.y);
            }
        }
        
        if (font) {
            SDL_Color textColor = {0, 0, 0, 255};
            drawText(renderer, font, "Path Planning with Roadmap", MAP_WIDTH/2, 20, textColor);
            drawText(renderer, font, "Free Space", MAP_WIDTH + TRAP_VIZ_WIDTH/2, 20, textColor);
            
            char stats[100];
            snprintf(stats, sizeof(stats), "Free Trapezoids: %zu | Roadmap Nodes: %zu", 
                    freeSpaceMap.trapezoids.size(), roadMap.nodes.size());
            drawText(renderer, font, stats, MAP_WIDTH/2, 40, textColor);
            
            if (path.empty()) {
                drawText(renderer, font, "NO PATH FOUND", MAP_WIDTH/2, SCREEN_HEIGHT - 40, {255, 0, 0, 255});
            } else {
                char pathInfo[100];
                snprintf(pathInfo, sizeof(pathInfo), "Path found: %zu points", path.size());
                drawText(renderer, font, pathInfo, MAP_WIDTH/2, SCREEN_HEIGHT - 40, {0, 100, 0, 255});
            }
            
            drawText(renderer, font, "Click to set start/goal | R: Toggle roadmap | C: Compute path", 
                    MAP_WIDTH/2, SCREEN_HEIGHT - 20, textColor);
            
            const char* mode = selectingStart ? "Click to set START (green)" : "Click to set GOAL (red)";
            drawText(renderer, font, mode, MAP_WIDTH/2, 60, textColor);
        }

        SDL_RenderPresent(renderer);
    }

    std::cout << "Cleaning up..." << std::endl;
    if (font) TTF_CloseFont(font);
    freeSpaceMap.cleanup();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
