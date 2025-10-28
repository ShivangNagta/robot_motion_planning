#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include "data_structure.hpp"
#include "minkowski_sum.hpp"
#include "sdl_layer.hpp"

void minkowski_sum_demo() {

    // Application start
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    if (!sdl_start(window, renderer, font)) return;

    Polygon P, R;

    P.addVertex(70, 30);
    P.addVertex(50, 80);
    P.addVertex(30, 40);

    R.addVertex(-10, -10);
    R.addVertex(10, -10);
    R.addVertex(10, 10);
    R.addVertex(-10, 10);

    // R.addVertex(0, 0);
    // R.addVertex(20, 0);
    // R.addVertex(20, 20);
    // R.addVertex(0, 20);

    Polygon reflected_R = R.reflectAboutOrigin();

    MinkowskiSum::normalizePolygon(P);
    MinkowskiSum::normalizePolygon(reflected_R);

    std::cout << "Computing Minkowski sum..." << std::endl;
    Polygon minkowskiSum = MinkowskiSum::MINKOWSKISUM(P, reflected_R);
    
    bool running = true;
    SDL_Event event;
    int visualizationStep = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    visualizationStep = (visualizationStep + 1) % 2;
                    std::cout << "Visualization step: " << visualizationStep << std::endl;
                }
            }
        }

        // Clear
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Draw coordinate grid across full viewport
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 100);
        for (int x = (int)std::floor(WORLD_MIN_X); x <= (int)std::ceil(WORLD_MAX_X); x += 10) {
            SDL_FPoint p1 = worldToScreen(Point(x, (int)WORLD_MIN_Y));
            SDL_FPoint p2 = worldToScreen(Point(x, (int)WORLD_MAX_Y));
            SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
        }
        for (int y = (int)std::floor(WORLD_MIN_Y); y <= (int)std::ceil(WORLD_MAX_Y); y += 10) {
            SDL_FPoint p1 = worldToScreen(Point((int)WORLD_MIN_X, y));
            SDL_FPoint p2 = worldToScreen(Point((int)WORLD_MAX_X, y));
            SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
        }

        if (visualizationStep == 0) {
            drawPolygon(renderer, P, 255, 100, 100, 150);
            drawPolygon(renderer, R, 100, 255, 100, 150);
        }
        else {
            drawPolygon(renderer, P, 255, 100, 100, 100, false);
            drawPolygon(renderer, R, 100, 255, 100, 100, false);
            drawPolygon(renderer, minkowskiSum, 100, 100, 255, 150);

        }     
        
        if (font) {
            SDL_Color textColor = {0, 0, 0, 255};
            drawText(renderer, font, "Minkowski Sum - MINKOWSKISUM Algorithm", SCREEN_WIDTH / 2, 20, textColor);
            
            char stats[200];
            snprintf(stats, sizeof(stats), 
                    "P: %zu vertices | R: %zu vertices | P+R: %zu vertices", 
                    P.vertices.size(), R.vertices.size(), minkowskiSum.vertices.size());
            drawText(renderer, font, stats, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 40, textColor);
            
            const char* stepDesc[] = {
                "Original Polygons",
                "Minkowski Sum Result",
            };
            char stepbuf[128];
            snprintf(stepbuf, sizeof(stepbuf), "Visualization: %s", stepDesc[visualizationStep]);
            drawText(renderer, font, stepbuf, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 20, textColor);
        }

        SDL_RenderPresent(renderer);
    }

    std::cout << "Cleaning up..." << std::endl;
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
