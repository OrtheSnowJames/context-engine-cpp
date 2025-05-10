#include "context-engine.hpp"
#include <iostream>
#include <cmath>

// Include Emscripten headers when compiling for WebAssembly
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#else
#include <SDL2/SDL.h>
#endif
#include <SDL2/SDL_stdinc.h>

using namespace ContextEngine;

// Forward declaration for our main game class
class GameScene;

// Global variables for Emscripten main loop
#ifdef __EMSCRIPTEN__
Engine* g_engine = nullptr;
#endif

// Example game scene to demonstrate the Context Engine features
class GameScene : public Scene {
private:
    // Game state
    Vector2 windowSize;
    float playerX = 400;
    float playerY = 300;
    float playerSpeed = 200.0f;
    Color playerColor = Color(0, 128, 255);
    
    // Objects to draw
    Rect playerRect = Rect(playerX, playerY, 50, 50);
    std::vector<Rect> blocks;
    
    // Random block colors
    std::vector<Color> blockColors;

    // Since the render method doesn't have access to the engine, we'll store mouse position in our class
    int lastMouseX = 0;
    int lastMouseY = 0;

    // Camera zoom control
    float cameraZoom = 1.0f;
    bool zoomingIn = false;
    float targetZoom = 1.0f;
    const float ZOOM_SPEED = 5.0f; // Increased zoom speed

public:
    GameScene() {
        // Create some blocks with random colors
        for (int i = 0; i < 5; i++) {
            float x = 100.0f + i * 120.0f;
            blocks.push_back(Rect(x, 200, 80, 80));
            
            // Generate a random color
            Uint8 r = rand() % 256;
            Uint8 g = rand() % 256;
            Uint8 b = rand() % 256;
            blockColors.push_back(Color(r, g, b));
        }
    }
    
    void onLoad() override {
        std::cout << "Game scene loaded!" << std::endl;
    }
    
    void onExit() override {
        std::cout << "Game scene exited!" << std::endl;
    }
    
    void handleEvent(const SDL_Event& event) override {
        // Handle additional scene-specific events here
    }
    
    void update(float deltaTime, Engine* engine) override {
        // Store mouse position for rendering
        lastMouseX = engine->getMouseX();
        lastMouseY = engine->getMouseY();
        
        // Player movement based on arrow keys
        if (engine->isKeyPressed(SDL_SCANCODE_LEFT)) {
            playerX -= playerSpeed * deltaTime;
        }
        if (engine->isKeyPressed(SDL_SCANCODE_RIGHT)) {
            playerX += playerSpeed * deltaTime;
        }
        if (engine->isKeyPressed(SDL_SCANCODE_UP)) {
            playerY -= playerSpeed * deltaTime;
        }
        if (engine->isKeyPressed(SDL_SCANCODE_DOWN)) {
            playerY += playerSpeed * deltaTime;
        }
        
        // Update player rectangle position
        playerRect.x = playerX;
        playerRect.y = playerY;
        
        // Keep player within the screen bounds
        if (playerRect.x < 0) playerRect.x = 0;
        if (playerRect.y < 0) playerRect.y = 0;
        if (playerRect.x + playerRect.w > 800) playerRect.x = 800 - playerRect.w;
        if (playerRect.y + playerRect.h > 600) playerRect.y = 600 - playerRect.h;
        
        // Animate blocks (move up and down)
        for (size_t i = 0; i < blocks.size(); i++) {
            blocks[i].y = 200 + sinf(engine->getMouseX() * 0.01f + i) * 50;
        }

        windowSize = engine->getWindowSize();

        // Toggle zoom with Z key
        if (engine->isKeyPressed(SDL_SCANCODE_Z)) {
            zoomingIn = true;
            targetZoom = zoomingIn ? 2.0f : 1.0f;
        } else {
            zoomingIn = false;
            targetZoom = 1.0f;
        }
        
        // Smoothly interpolate camera zoom
        if (cameraZoom < targetZoom) {
            cameraZoom = std::min(cameraZoom + ZOOM_SPEED * deltaTime, targetZoom);
        } else if (cameraZoom > targetZoom) {
            cameraZoom = std::max(cameraZoom - ZOOM_SPEED * deltaTime, targetZoom);
        }
        
        // Exit on ESC key
        if (engine->isKeyPressed(SDL_SCANCODE_ESCAPE)) {
            engine->quit();
        }
    }
    
    void render(OtherCtx* ctx) override {
        // Set camera to follow player
        ctx->setCameraPosition(Vector2(playerX - 400, playerY - 300));
        ctx->setCameraZoom(cameraZoom);
        
        // Draw the background
        ctx->clear(Color(40, 40, 60));
        
        // Draw a large world boundary
        ctx->drawRectLines(0, 0, 2000, 2000, Color(100, 100, 100));
        
        // Draw blocks with rounded corners
        for (size_t i = 0; i < blocks.size(); i++) {
            // Draw filled rounded rectangle
            ctx->drawRoundedRect(blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h, 10, blockColors[i]);
            // Draw outline with slightly larger radius to prevent gaps
            ctx->drawRoundedRectLines(blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h, 10, Color(255, 255, 255));
        }
        
        // Draw player as a triangle
        float centerX = playerRect.x + playerRect.w/2;
        float centerY = playerRect.y + playerRect.h/2;
        float size = 25.0f; // Fixed size for the triangle
        
        ctx->drawTriangle(
            centerX, centerY - size,
            centerX - size, centerY + size,
            centerX + size, centerY + size,
            playerColor
        );
        
        // Draw a circle at the player's center
        ctx->drawCircle(
            centerX,
            centerY,
            10,
            Color(255, 255, 0)
        );
        
        // Draw a line from the player to the mouse position
        ctx->drawLine(
            centerX,
            centerY,
            static_cast<float>(lastMouseX),
            static_cast<float>(lastMouseY),
            Color(255, 255, 0)
        );
        
        // Draw GUI elements (disable camera for these)
        ctx->enableCamera(false);
        
        // Draw a rounded rectangle for the score panel
        ctx->drawRoundedRect(10, 10, 200, 100, 15, Color(0, 0, 0, 200));
        ctx->drawText("Score: 100", 20, 20, Color(255, 255, 255));
        ctx->drawText("Press Z to zoom", 20, 50, Color(255, 255, 255));
        ctx->drawText(std::to_string(windowSize.x) + "x" + std::to_string(windowSize.y), 20, 80, Color(255, 255, 255));
        
        // Draw a triangle indicator for zoom level
        float zoomIndicatorX = 20;
        float zoomIndicatorY = 80;
        float indicatorSize = 10.0f;
        ctx->drawTriangle(
            zoomIndicatorX, zoomIndicatorY,
            zoomIndicatorX + indicatorSize * 2, zoomIndicatorY,
            zoomIndicatorX + indicatorSize, zoomIndicatorY - indicatorSize * cameraZoom,
            Color(255, 255, 255)
        );
        
        // Re-enable camera for next frame
        ctx->enableCamera(true);
    }
};

#ifdef __EMSCRIPTEN__
// Emscripten main loop function
void main_loop() {
    if (g_engine) {
        g_engine->handleEvents();
        g_engine->update(1.0f / 60.0f); // Fixed time step for WebAssembly
        g_engine->render();
    }
}
#endif

int main(int argc, char* argv[]) {
    // Seed the random number generator
    srand(static_cast<unsigned int>(time(nullptr)));
    
    // Create the engine
    Engine engine("Context Engine Demo", 800, 600);
    
    // Check if SDL was properly initialized
    if (!engine.init()) {
        std::cerr << "Failed to initialize the engine!" << std::endl;
        return 1;
    }
    
    // Add a scene
    std::unique_ptr<Scene> gameScene = std::make_unique<GameScene>();
    engine.addScene(std::move(gameScene));
    
#ifdef __EMSCRIPTEN__
    // Store engine pointer for Emscripten main loop
    g_engine = &engine;
    
    // Set main loop for Emscripten - use 0 for the frame rate to use requestAnimationFrame
    emscripten_set_main_loop(main_loop, 0, 1);
    
    // The main loop will run until the program is terminated
#else
    // Start the game loop (native platforms)
    engine.run();
#endif
    
    return 0;
} 