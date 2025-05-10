#include "context-engine.hpp"
#ifdef __EMSCRIPTEN__
#include <SDL.h>
#include <emscripten.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#endif
#include <iostream>

namespace ContextEngine {

// Engine implementation
Engine::Engine(const char* title, int width, int height)
    : window(nullptr)
    , renderer(nullptr)
    , ctx(nullptr)
    , currentSceneIndex(-1)
    , running(false)
{
    std::cout << "Initializing Engine..." << std::endl;
    
    // Initialize input state
    input.mouseX = 0;
    input.mouseY = 0;
    input.mouseDown = false;
    input.mouseReleased = false;
    input.keys.resize(SDL_NUM_SCANCODES, false);
    
    // Initialize SDL
    std::cout << "Initializing SDL..." << std::endl;
    int sdlInitResult = SDL_Init(SDL_INIT_VIDEO);
    std::cout << "SDL_Init result: " << sdlInitResult << std::endl;
    
    if (sdlInitResult != 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return;
    }
    std::cout << "SDL initialized successfully." << std::endl;
    
    // Create window
    std::cout << "Creating window..." << std::endl;
#ifdef __EMSCRIPTEN__
    // For WebAssembly, create a window with the canvas size
    window = SDL_CreateWindow(title, 
                             SDL_WINDOWPOS_CENTERED, 
                             SDL_WINDOWPOS_CENTERED, 
                             width, height, 
                             SDL_WINDOW_SHOWN);
#else
    // For native builds, create a resizable window
    window = SDL_CreateWindow(title, 
                             SDL_WINDOWPOS_CENTERED, 
                             SDL_WINDOWPOS_CENTERED, 
                             width, height, 
                             SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
#endif
    if (!window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return;
    }
    std::cout << "Window created successfully." << std::endl;
    
    // Create renderer
    std::cout << "Creating renderer..." << std::endl;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return;
    }
    std::cout << "Renderer created successfully." << std::endl;
    
    // Create rendering context
    std::cout << "Creating rendering context..." << std::endl;
    ctx = std::make_unique<OtherCtx>(renderer);
    std::cout << "Engine initialization complete." << std::endl;
}

Engine::~Engine() {
    // Release context (will not destroy renderer since we set ownsRenderer to false)
    ctx.reset();
    
    // Destroy renderer
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    
    // Destroy window
    if (window) {
        SDL_DestroyWindow(window);
    }
    
    // Quit SDL
    SDL_Quit();
}

bool Engine::init() {
    // Check if SDL components were successfully initialized
    if (!window || !renderer || !ctx) {
        std::cerr << "Engine not properly initialized!" << std::endl;
        return false;
    }
    
    running = true;
    return true;
}

void Engine::handleEvents() {
    // Reset mouse released state at the beginning of frame
    input.mouseReleased = false;
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Handle engine-level events
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
                
            case SDL_KEYDOWN:
                input.keys[event.key.keysym.scancode] = true;
                break;
                
            case SDL_KEYUP:
                input.keys[event.key.keysym.scancode] = false;
                break;
                
            case SDL_MOUSEMOTION:
                input.mouseX = event.motion.x;
                input.mouseY = event.motion.y;
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    input.mouseDown = true;
                }
                break;
                
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    input.mouseDown = false;
                    input.mouseReleased = true;
                }
                break;
        }
        
        // Pass events to the current scene if one exists
        if (currentSceneIndex >= 0 && currentSceneIndex < static_cast<int>(scenes.size())) {
            scenes[currentSceneIndex]->handleEvent(event);
        }
    }
}

void Engine::update(float deltaTime) {
    // Update the current scene if one exists
    if (currentSceneIndex >= 0 && currentSceneIndex < static_cast<int>(scenes.size())) {
        scenes[currentSceneIndex]->update(deltaTime, this);
    }
}

void Engine::render() {
    // Clear screen
    ctx->clear(Color(0, 0, 0));
    
    // Render the current scene if one exists
    if (currentSceneIndex >= 0 && currentSceneIndex < static_cast<int>(scenes.size())) {
        scenes[currentSceneIndex]->render(ctx.get());
    }
    
    // Present the rendered content
    ctx->present();
}

void Engine::run() {
    if (!running) {
        init();
    }
    
    Uint32 previousTime = SDL_GetTicks();
    
    // Main game loop
    while (running) {
        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;
        
        // Process events
        handleEvents();
        
        // Update game state
        update(deltaTime);
        
        // Render
        render();
        
        // Cap the frame rate at ~60 FPS
        SDL_Delay(16);
    }
}

void Engine::addScene(std::unique_ptr<Scene> scene) {
    scenes.push_back(std::move(scene));
    
    // If this is the first scene, make it the current scene
    if (scenes.size() == 1) {
        switchScene(0);
    }
}

void Engine::switchScene(int index) {
    if (index < 0 || index >= static_cast<int>(scenes.size())) {
        return;
    }
    
    // Exit current scene if needed
    if (currentSceneIndex >= 0 && currentSceneIndex < static_cast<int>(scenes.size())) {
        scenes[currentSceneIndex]->onExit();
    }
    
    // Set and load the new scene
    currentSceneIndex = index;
    scenes[currentSceneIndex]->onLoad();
}

int Engine::getCurrentSceneIndex() const {
    return currentSceneIndex;
}

bool Engine::isKeyPressed(int keyCode) const {
    if (keyCode < 0 || keyCode >= static_cast<int>(input.keys.size())) {
        return false;
    }
    return input.keys[keyCode];
}

int Engine::getMouseX() const {
    return input.mouseX;
}

int Engine::getMouseY() const {
    return input.mouseY;
}

bool Engine::isMouseDown() const {
    return input.mouseDown;
}

bool Engine::isMouseReleased() const {
    return input.mouseReleased;
}

OtherCtx* Engine::getContext() const {
    return ctx.get();
}

void Engine::quit() {
    running = false;
}

Vector2 Engine::getWindowSize() const {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    return Vector2(static_cast<float>(width), static_cast<float>(height));
}

} // namespace ContextEngine 