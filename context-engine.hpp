#pragma once

// For WebAssembly compatibility
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#endif

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>

namespace ContextEngine {

// Forward declarations
class Scene;
class OtherCtx;

// Color structure for easier color handling
struct Color {
    Uint8 r, g, b, a;
    
    Color() : r(255), g(255), b(255), a(255) {}
    Color(Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha = 255) 
        : r(red), g(green), b(blue), a(alpha) {}
        
    SDL_Color toSDLColor() const {
        return {r, g, b, a};
    }
};

// Rectangle structure
struct Rect {
    float x, y, w, h;
    
    Rect() : x(0), y(0), w(0), h(0) {}
    Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
    
    SDL_FRect toSDLFRect() const {
        return {x, y, w, h};
    }
    
    bool contains(float px, float py) const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
};

// Vector2 structure for 2D positions and movements
struct Vector2 {
    float x, y;
    
    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}
    
    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }
    
    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }
    
    Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }
    
    Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    
    Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
};

// OtherCtx class for rendering
class OtherCtx {
private:
    SDL_Renderer* renderer;
    bool ownsRenderer;
    TTF_Font* defaultFont;
    std::unordered_map<std::string, TTF_Font*> fonts;
    std::unordered_map<std::string, std::string> fontPaths; // Store font paths
    
    // Camera properties
    Vector2 cameraPos;
    float cameraZoom;
    bool useCamera;

public:
    // Constructor with SDL_Renderer
    OtherCtx(SDL_Renderer* renderer, bool takeOwnership = false) 
        : renderer(renderer), ownsRenderer(takeOwnership), defaultFont(nullptr),
          cameraPos(0, 0), cameraZoom(1.0f), useCamera(true) {
        // Initialize TTF
        if (TTF_Init() == -1) {
            SDL_Log("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        } else {
            // Try to load a default font
            loadFont("default", "assets/font.ttf", 16);
        }
    }
        
    // Destructor
    ~OtherCtx() {
        // Clean up fonts
        for (auto& [name, font] : fonts) {
            if (font) {
                TTF_CloseFont(font);
            }
        }
        fonts.clear();
        
        // Quit TTF
        TTF_Quit();
        
        if (ownsRenderer && renderer) {
            SDL_DestroyRenderer(renderer);
        }
    }
    
    // Prevent copying
    OtherCtx(const OtherCtx&) = delete;
    OtherCtx& operator=(const OtherCtx&) = delete;
    
    // Allow moving
    OtherCtx(OtherCtx&& other) noexcept 
        : renderer(other.renderer), ownsRenderer(other.ownsRenderer), defaultFont(other.defaultFont) {
        other.renderer = nullptr;
        other.ownsRenderer = false;
        other.defaultFont = nullptr;
    }
    
    OtherCtx& operator=(OtherCtx&& other) noexcept {
        if (this != &other) {
            if (ownsRenderer && renderer) {
                SDL_DestroyRenderer(renderer);
            }
            renderer = other.renderer;
            ownsRenderer = other.ownsRenderer;
            defaultFont = other.defaultFont;
            other.renderer = nullptr;
            other.ownsRenderer = false;
            other.defaultFont = nullptr;
        }
        return *this;
    }
    
    // Get the underlying SDL_Renderer
    SDL_Renderer* getRenderer() const {
        return renderer;
    }
    
    // Clear the screen with a color
    void clear(const Color& color = Color(0, 0, 0, 255)) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderClear(renderer);
    }
    
    // Present the rendered content
    void present() {
        SDL_RenderPresent(renderer);
    }
    
    // Set the drawing color
    void setDrawColor(const Color& color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    }
    
    // Draw a filled rectangle
    void drawRect(float x, float y, float width, float height, const Color& color) {
        setDrawColor(color);
        SDL_FRect rect = {x, y, width, height};
        SDL_RenderFillRectF(renderer, &rect);
    }
    
    // Draw a filled rectangle using Rect structure
    void drawRect(const Rect& rect, const Color& color) {
        drawRect(rect.x, rect.y, rect.w, rect.h, color);
    }
    
    // Draw an outline rectangle
    void drawRectOutline(float x, float y, float width, float height, const Color& color) {
        setDrawColor(color);
        SDL_FRect rect = {x, y, width, height};
        SDL_RenderDrawRectF(renderer, &rect);
    }
    
    // Draw an outline rectangle using Rect structure
    void drawRectOutline(const Rect& rect, const Color& color) {
        drawRectOutline(rect.x, rect.y, rect.w, rect.h, color);
    }
    
    // Draw a line
    void drawLine(float x1, float y1, float x2, float y2, const Color& color) {
        setDrawColor(color);
        SDL_RenderDrawLineF(renderer, x1, y1, x2, y2);
    }
    
    // Draw a point
    void drawPoint(float x, float y, const Color& color) {
        setDrawColor(color);
        SDL_RenderDrawPointF(renderer, x, y);
    }
    
    // Load a font with a name for later use
    bool loadFont(const std::string& name, const std::string& path, int size) {
        TTF_Font* font = TTF_OpenFont(path.c_str(), size);
        if (!font) {
            SDL_Log("Failed to load font %s! SDL_ttf Error: %s\n", path.c_str(), TTF_GetError());
            return false;
        }
        
        // Store the font and its path
        fonts[name] = font;
        fontPaths[name] = path;
        
        // Set as default if we don't have one
        if (!defaultFont) {
            defaultFont = font;
        }
        
        return true;
    }
    
    // Draw text using the default font
    void drawText(const std::string& text, float x, float y, const Color& color, float textSize = 1.0f) {
        if (!defaultFont) {
            SDL_Log("No default font loaded for text rendering!");
            return;
        }
        
        drawText(text, x, y, color, defaultFont, textSize);
    }
    
    // Draw text using a specific font by name
    void drawText(const std::string& text, float x, float y, const Color& color, const std::string& fontName, float textSize = 1.0f) {
        auto it = fonts.find(fontName);
        if (it == fonts.end()) {
            SDL_Log("Font '%s' not found!", fontName.c_str());
            return;
        }
        
        drawText(text, x, y, color, it->second, textSize);
    }
    
    // Draw text using a specific TTF_Font
    void drawText(const std::string& text, float x, float y, const Color& color, TTF_Font* font, float textSize = 1.0f) {
        if (!font) {
            SDL_Log("Invalid font for text rendering!");
            return;
        }
        
        // Get the original font size
        int originalSize = TTF_FontHeight(font);
        
        // Find the font name and path
        std::string fontPath;
        for (const auto& [name, f] : fonts) {
            if (f == font) {
                fontPath = fontPaths[name];
                break;
            }
        }
        
        if (fontPath.empty()) {
            SDL_Log("Could not find font path for resizing!");
            return;
        }
        
        // Create a new font with the desired size
        TTF_Font* sizedFont = TTF_OpenFont(fontPath.c_str(), static_cast<int>(originalSize * textSize));
        if (!sizedFont) {
            SDL_Log("Failed to create sized font! SDL_ttf Error: %s\n", TTF_GetError());
            return;
        }
        
        // Render the text to a surface
        SDL_Surface* surface = TTF_RenderText_Blended(sizedFont, text.c_str(), color.toSDLColor());
        TTF_CloseFont(sizedFont); // Clean up the temporary font
        
        if (!surface) {
            SDL_Log("Failed to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
            return;
        }
        
        // Create a texture from the surface
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        
        if (!texture) {
            SDL_Log("Failed to create texture from text surface! SDL Error: %s\n", SDL_GetError());
            return;
        }
        
        // Set up the destination rectangle
        int textWidth, textHeight;
        SDL_QueryTexture(texture, nullptr, nullptr, &textWidth, &textHeight);
        SDL_FRect destRect = {x, y, static_cast<float>(textWidth), static_cast<float>(textHeight)};
        
        // Draw the texture
        SDL_RenderCopyF(renderer, texture, nullptr, &destRect);
        
        // Clean up
        SDL_DestroyTexture(texture);
    }

    // Camera control methods
    void setCameraPosition(const Vector2& pos) { cameraPos = pos; }
    void setCameraZoom(float zoom) { cameraZoom = zoom; }
    void enableCamera(bool enable) { useCamera = enable; }
    Vector2 getCameraPosition() const { return cameraPos; }
    float getCameraZoom() const { return cameraZoom; }
    bool isCameraEnabled() const { return useCamera; }

    // Transform coordinates based on camera
    Vector2 transformPoint(float x, float y) const {
        if (!useCamera) return Vector2(x, y);
        return Vector2(
            (x - cameraPos.x) * cameraZoom,
            (y - cameraPos.y) * cameraZoom
        );
    }

    // Draw a triangle
    void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, const Color& color, bool fill = true) {
        setDrawColor(color);
        
        Vector2 p1 = transformPoint(x1, y1);
        Vector2 p2 = transformPoint(x2, y2);
        Vector2 p3 = transformPoint(x3, y3);
        
        if (fill) {
            // Draw filled triangle using SDL_RenderGeometry
            SDL_Vertex vertices[3] = {
                {SDL_FPoint{p1.x, p1.y}, color.toSDLColor(), SDL_FPoint{0, 0}},
                {SDL_FPoint{p2.x, p2.y}, color.toSDLColor(), SDL_FPoint{0, 0}},
                {SDL_FPoint{p3.x, p3.y}, color.toSDLColor(), SDL_FPoint{0, 0}}
            };
            SDL_RenderGeometry(renderer, nullptr, vertices, 3, nullptr, 0);
        } else {
            // Draw triangle outline
            SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
            SDL_RenderDrawLineF(renderer, p2.x, p2.y, p3.x, p3.y);
            SDL_RenderDrawLineF(renderer, p3.x, p3.y, p1.x, p1.y);
        }
    }

    // Draw a rounded rectangle
    void drawRoundedRect(float x, float y, float width, float height, float radius, const Color& color, bool fill = true) {
        setDrawColor(color);
        
        Vector2 pos = transformPoint(x, y);
        float scaledWidth = width * cameraZoom;
        float scaledHeight = height * cameraZoom;
        float scaledRadius = radius * cameraZoom;
        
        if (fill) {
            // Draw filled rounded rectangle using multiple rectangles and circles
            // Center rectangle
            SDL_FRect centerRect = {
                pos.x + scaledRadius,
                pos.y,
                scaledWidth - 2 * scaledRadius,
                scaledHeight
            };
            SDL_RenderFillRectF(renderer, &centerRect);
            
            // Left rectangle
            SDL_FRect leftRect = {
                pos.x,
                pos.y + scaledRadius,
                scaledRadius,
                scaledHeight - 2 * scaledRadius
            };
            SDL_RenderFillRectF(renderer, &leftRect);
            
            // Right rectangle
            SDL_FRect rightRect = {
                pos.x + scaledWidth - scaledRadius,
                pos.y + scaledRadius,
                scaledRadius,
                scaledHeight - 2 * scaledRadius
            };
            SDL_RenderFillRectF(renderer, &rightRect);
            
            // Draw corner circles
            drawCircle(pos.x + scaledRadius, pos.y + scaledRadius, scaledRadius, color, true);
            drawCircle(pos.x + scaledWidth - scaledRadius, pos.y + scaledRadius, scaledRadius, color, true);
            drawCircle(pos.x + scaledRadius, pos.y + scaledHeight - scaledRadius, scaledRadius, color, true);
            drawCircle(pos.x + scaledWidth - scaledRadius, pos.y + scaledHeight - scaledRadius, scaledRadius, color, true);
        } else {
            // Draw rounded rectangle outline
            // Top line
            SDL_RenderDrawLineF(renderer, pos.x + scaledRadius, pos.y, pos.x + scaledWidth - scaledRadius, pos.y);
            // Bottom line
            SDL_RenderDrawLineF(renderer, pos.x + scaledRadius, pos.y + scaledHeight, pos.x + scaledWidth - scaledRadius, pos.y + scaledHeight);
            // Left line
            SDL_RenderDrawLineF(renderer, pos.x, pos.y + scaledRadius, pos.x, pos.y + scaledHeight - scaledRadius);
            // Right line
            SDL_RenderDrawLineF(renderer, pos.x + scaledWidth, pos.y + scaledRadius, pos.x + scaledWidth, pos.y + scaledHeight - scaledRadius);
            
            // Draw corner arcs
            drawArc(pos.x + scaledRadius, pos.y + scaledRadius, scaledRadius, 180, 270, color);
            drawArc(pos.x + scaledWidth - scaledRadius, pos.y + scaledRadius, scaledRadius, 270, 360, color);
            drawArc(pos.x + scaledRadius, pos.y + scaledHeight - scaledRadius, scaledRadius, 90, 180, color);
            drawArc(pos.x + scaledWidth - scaledRadius, pos.y + scaledHeight - scaledRadius, scaledRadius, 0, 90, color);
        }
    }

    // Draw a circle
    void drawCircle(float x, float y, float radius, const Color& color, bool fill = true) {
        setDrawColor(color);
        
        Vector2 center = transformPoint(x, y);
        float scaledRadius = radius * cameraZoom;
        
        if (fill) {
            // Draw filled circle using multiple rectangles
            for (float dy = -scaledRadius; dy <= scaledRadius; dy++) {
                float dx = sqrtf(scaledRadius * scaledRadius - dy * dy);
                SDL_RenderDrawLineF(renderer,
                    center.x - dx, center.y + dy,
                    center.x + dx, center.y + dy);
            }
        } else {
            // Draw circle outline using Bresenham's algorithm
            float dx = scaledRadius;
            float dy = 0;
            float err = 0;
            
            while (dx >= dy) {
                SDL_RenderDrawPointF(renderer, center.x + dx, center.y + dy);
                SDL_RenderDrawPointF(renderer, center.x + dy, center.y + dx);
                SDL_RenderDrawPointF(renderer, center.x - dy, center.y + dx);
                SDL_RenderDrawPointF(renderer, center.x - dx, center.y + dy);
                SDL_RenderDrawPointF(renderer, center.x - dx, center.y - dy);
                SDL_RenderDrawPointF(renderer, center.x - dy, center.y - dx);
                SDL_RenderDrawPointF(renderer, center.x + dy, center.y - dx);
                SDL_RenderDrawPointF(renderer, center.x + dx, center.y - dy);
                
                if (err <= 0) {
                    dy += 1;
                    err += 2 * dy + 1;
                }
                if (err > 0) {
                    dx -= 1;
                    err -= 2 * dx + 1;
                }
            }
        }
    }

    // Draw an arc (for rounded rectangle corners)
    void drawArc(float x, float y, float radius, float startAngle, float endAngle, const Color& color) {
        setDrawColor(color);
        
        Vector2 center = transformPoint(x, y);
        float scaledRadius = radius * cameraZoom;
        
        const int segments = 8; // Number of segments for each corner
        float angleStep = (endAngle - startAngle) / segments;
        
        for (int i = 0; i < segments; i++) {
            float angle1 = (startAngle + i * angleStep) * M_PI / 180.0f;
            float angle2 = (startAngle + (i + 1) * angleStep) * M_PI / 180.0f;
            
            float x1 = center.x + cosf(angle1) * scaledRadius;
            float y1 = center.y + sinf(angle1) * scaledRadius;
            float x2 = center.x + cosf(angle2) * scaledRadius;
            float y2 = center.y + sinf(angle2) * scaledRadius;
            
            SDL_RenderDrawLineF(renderer, x1, y1, x2, y2);
        }
    }

    // Draw rectangle lines (multiple lines forming a rectangle)
    void drawRectLines(float x, float y, float width, float height, const Color& color) {
        setDrawColor(color);
        
        Vector2 pos = transformPoint(x, y);
        float scaledWidth = width * cameraZoom;
        float scaledHeight = height * cameraZoom;
        
        SDL_RenderDrawLineF(renderer, pos.x, pos.y, pos.x + scaledWidth, pos.y);
        SDL_RenderDrawLineF(renderer, pos.x + scaledWidth, pos.y, pos.x + scaledWidth, pos.y + scaledHeight);
        SDL_RenderDrawLineF(renderer, pos.x + scaledWidth, pos.y + scaledHeight, pos.x, pos.y + scaledHeight);
        SDL_RenderDrawLineF(renderer, pos.x, pos.y + scaledHeight, pos.x, pos.y);
    }

    // Draw rounded rectangle lines (multiple lines forming a rounded rectangle)
    void drawRoundedRectLines(float x, float y, float width, float height, float radius, const Color& color) {
        drawRoundedRect(x, y, width, height, radius, color, false);
    }
};

// Engine class to manage the game window, renderer, and scenes
class Engine {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::unique_ptr<OtherCtx> ctx;
    std::vector<std::unique_ptr<Scene>> scenes;
    int currentSceneIndex;
    bool running;
    
    // Input state
    struct {
        int mouseX, mouseY;
        bool mouseDown;
        bool mouseReleased;
        std::vector<bool> keys;
    } input;

public:
    Engine(const char* title, int width, int height);
    ~Engine();
    
    // Prevent copying
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    
    // Initialization
    bool init();
    
    // Game loop methods
    void handleEvents();
    void update(float deltaTime);
    void render();
    void run();
    
    // Scene management
    void addScene(std::unique_ptr<Scene> scene);
    void switchScene(int index);
    int getCurrentSceneIndex() const;
    
    // Input methods
    bool isKeyPressed(int keyCode) const;
    int getMouseX() const;
    int getMouseY() const;
    bool isMouseDown() const;
    bool isMouseReleased() const;
    
    // Access to context
    OtherCtx* getContext() const;
    
    // Get current window size
    Vector2 getWindowSize() const;
    
    // Stop the engine
    void quit();
};

// Scene interface for game scenes
class Scene {
public:
    virtual ~Scene() = default;
    
    // Scene lifecycle methods
    virtual void onLoad() {}
    virtual void onExit() {}
    virtual void handleEvent(const SDL_Event& event) {}
    virtual void update(float deltaTime, Engine* engine) {}
    virtual void render(OtherCtx* ctx) {}
};

} // namespace ContextEngine
