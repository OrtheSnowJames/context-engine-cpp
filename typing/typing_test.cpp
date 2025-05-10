#include "context-engine.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <algorithm>
#include <functional>
#include <sstream>
#include <iomanip>

using namespace ContextEngine;

class TypingTestGame : public Scene {
private:
    // Current game state
    enum class GameState {
        START,
        TYPING,
        FINISHED
    };
    
    GameState state = GameState::START;
    
    // Collection of sentences to type
    std::vector<std::string> sentences = {
        "The quick brown fox jumps over the lazy dog.",
        "Programming is the art of telling another human what one wants the computer to do.",
        "Be the change you wish to see in the world.",
        "To be or not to be, that is the question.",
        "In the end, we only regret the chances we didn't take.",
        "Life is what happens when you're busy making other plans.",
        "The greatest glory in living lies not in never falling, but in rising every time we fall.",
        "The way to get started is to quit talking and begin doing.",
        "It does not matter how slowly you go as long as you do not stop.",
        "Whether you think you can or you think you can't, you're right."
    };
    
    // Game variables
    std::string currentSentence;
    std::string userInput;
    int currentPosition = 0;
    int errors = 0;
    
    // Timing variables for WPM calculation
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
    
    // Calculated statistics
    float wpm = 0.0f;
    float accuracy = 100.0f;
    
    // UI variables
    Color bgColor = Color(40, 44, 52);
    Color textColor = Color(220, 223, 228);
    Color highlightColor = Color(86, 182, 194);
    Color errorColor = Color(224, 108, 117);
    Color correctColor = Color(152, 195, 121);
    
    // Visual elements
    Rect textBackground = Rect(50, 150, 700 * 1.5, 150 * 1.5); // Increased height and width by 1.5x
    Rect inputBackground = Rect(50, 400, 700 * 1.5, 60 * 1.5);  // Increased height and width by 1.5x
    
    // Animation variables
    float shakeDuration = 0.0f;
    float shakeIntensity = 0.0f;
    Vector2 shakeOffset = Vector2(0, 0);
    
    // Text wrapping variables
    int maxCharsPerLine = 78;  // Increased character count for wider box
    float charWidth = 12.0f;   // Approximate width of each character
    
    // Generate a random number
    std::mt19937 rng;
    
    // For initialization
    bool initialized = false;
    std::string fontPath = "/usr/share/fonts/TTF/JetBrainsMono-Regular.ttf";

public:
    TypingTestGame() {
        // Seed the random number generator
        auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        rng.seed(static_cast<unsigned int>(seed));
        
        // Start with a random sentence
        selectRandomSentence();
    }
    
    void selectRandomSentence() {
        std::uniform_int_distribution<int> dist(0, sentences.size() - 1);
        currentSentence = sentences[dist(rng)];
        currentPosition = 0;
        userInput = "";
    }
    
    void startTest() {
        state = GameState::TYPING;
        startTime = std::chrono::high_resolution_clock::now();
        userInput = "";
        currentPosition = 0;
        errors = 0;
        wpm = 0.0f;
        accuracy = 100.0f;
    }
    
    void finishTest() {
        state = GameState::FINISHED;
        endTime = std::chrono::high_resolution_clock::now();
        
        // Calculate WPM and accuracy
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        float minutes = duration / 60000.0f;
        
        // Standard WPM calculation (5 characters = 1 word)
        int wordCount = currentSentence.length() / 5;
        wpm = wordCount / minutes;
        
        // Calculate accuracy
        int totalChars = currentSentence.length();
        accuracy = 100.0f * (1.0f - static_cast<float>(errors) / totalChars);
        if (accuracy < 0.0f) accuracy = 0.0f;
    }
    
    void handleEvent(const SDL_Event& event) override {
        if (event.type == SDL_KEYDOWN) {
            switch (state) {
                case GameState::START:
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        startTest();
                    }
                    break;
                    
                case GameState::TYPING:
                    handleTypingInput(event);
                    break;
                    
                case GameState::FINISHED:
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        selectRandomSentence();
                        startTest();
                    }
                    break;
            }
        }
    }
    
    void handleTypingInput(const SDL_Event& event) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            state = GameState::START;
            return;
        }
        
        // Handle backspace
        if (event.key.keysym.sym == SDLK_BACKSPACE && !userInput.empty()) {
            userInput.pop_back();
            if (currentPosition > 0) currentPosition--;
            return;
        }
        
        // Process all keys (including capital letters)
        char inputChar = 0;
        
        // Handle ASCII and special characters
        if (event.key.keysym.sym >= 32 && event.key.keysym.sym <= 126) {
            // For basic ASCII, we can just convert the key code
            inputChar = static_cast<char>(event.key.keysym.sym);
            
            // Apply shift modifier for capital letters and special characters
            if (event.key.keysym.mod & KMOD_SHIFT) {
                if (inputChar >= 'a' && inputChar <= 'z') {
                    // Convert to uppercase for letters
                    inputChar = inputChar - 32;
                } else {
                    // Handle special characters with shift
                    switch (inputChar) {
                        case '1': inputChar = '!'; break;
                        case '2': inputChar = '@'; break;
                        case '3': inputChar = '#'; break;
                        case '4': inputChar = '$'; break;
                        case '5': inputChar = '%'; break;
                        case '6': inputChar = '^'; break;
                        case '7': inputChar = '&'; break;
                        case '8': inputChar = '*'; break;
                        case '9': inputChar = '('; break;
                        case '0': inputChar = ')'; break;
                        case '-': inputChar = '_'; break;
                        case '=': inputChar = '+'; break;
                        case '[': inputChar = '{'; break;
                        case ']': inputChar = '}'; break;
                        case '\\': inputChar = '|'; break;
                        case ';': inputChar = ':'; break;
                        case '\'': inputChar = '"'; break;
                        case ',': inputChar = '<'; break;
                        case '.': inputChar = '>'; break;
                        case '/': inputChar = '?'; break;
                        case '`': inputChar = '~'; break;
                    }
                }
            }
            
            // Check if user has reached the end of the sentence
            if (currentPosition >= currentSentence.length()) {
                finishTest();
                return;
            }
            
            // Check if the typed character matches the expected character
            if (inputChar == currentSentence[currentPosition]) {
                userInput += inputChar;
                currentPosition++;
                
                // Check if the sentence is complete
                if (currentPosition >= currentSentence.length()) {
                    finishTest();
                }
            } else {
                // Wrong character typed
                userInput += inputChar;
                wpm -= 5.0f * static_cast<float>(errors);
                currentPosition++;
                errors++;
                
                // Add shake effect for wrong input
                shakeDuration = 0.3f;
                shakeIntensity = 5.0f;
            }
        }
    }
    
    void update(float deltaTime, Engine* engine) override {
        // Update shake effect
        if (shakeDuration > 0.0f) {
            shakeDuration -= deltaTime;
            
            if (shakeDuration <= 0.0f) {
                shakeDuration = 0.0f;
                shakeOffset = Vector2(0, 0);
            } else {
                // Generate random shake offset
                std::uniform_real_distribution<float> dist(-shakeIntensity, shakeIntensity);
                shakeOffset.x = dist(rng);
                shakeOffset.y = dist(rng);
            }
        }
        
        // Update WPM while typing
        if (state == GameState::TYPING) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
            float minutes = duration / 60000.0f;
            
            if (minutes > 0) {
                // Calculate current WPM based on characters typed
                int charactersTyped = currentPosition;
                wpm = (charactersTyped / 5.0f) / minutes;
            }
        }
        
        // Handle escape key to quit
        if (engine->isKeyPressed(SDL_SCANCODE_ESCAPE)) {
            engine->quit();
        }
    }
    
    void render(OtherCtx* ctx) override {
        // Check if we need initialization
        if (!initialized) {
            // Load the monospace font
            if (ctx->loadFont("monospace", fontPath, 16)) {
                std::cout << "Monospace font loaded successfully" << std::endl;
            } else {
                std::cout << "Failed to load monospace font, falling back to default" << std::endl;
            }
            initialized = true;
        }
        
        // Clear background
        ctx->clear(bgColor);
        
        // Apply shake effect to the entire rendering if needed
        ctx->setCameraPosition(shakeOffset);
        
        // Draw title
        ctx->drawText("Typing Speed Test", 350, 50, textColor, "monospace", 2.0f);
        
        // Draw instructions based on game state
        switch (state) {
            case GameState::START:
                ctx->drawText("Press ENTER to start typing", 300, 100, textColor, "monospace", 1.2f);
                drawWrappedText(ctx, currentSentence, textBackground.x + 20, textBackground.y + 30, Color(150, 150, 150), "monospace");
                break;
                
            case GameState::TYPING:
                drawTypingUI(ctx);
                break;
                
            case GameState::FINISHED:
                drawFinishedUI(ctx);
                break;
        }
        
        // Reset camera for steady UI elements
        ctx->setCameraPosition(Vector2(0, 0));
    }
    
    // Draw text with word wrapping
    void drawWrappedText(OtherCtx* ctx, const std::string& text, float x, float y, const Color& color, const std::string& fontName) {
        // Create wrapped lines
        std::vector<std::string> lines;
        std::string currentLine;
        int currentLineChars = 0;
        
        std::istringstream stream(text);
        std::string word;
        
        while (stream >> word) {
            // Check if adding this word would exceed the max chars per line
            if (currentLineChars + word.length() + 1 > maxCharsPerLine && !currentLine.empty()) {
                // Start a new line
                lines.push_back(currentLine);
                currentLine = word;
                currentLineChars = word.length();
            } else {
                // Add word to current line
                if (!currentLine.empty()) {
                    currentLine += " " + word;
                    currentLineChars += word.length() + 1;
                } else {
                    currentLine = word;
                    currentLineChars = word.length();
                }
            }
        }
        
        // Add the last line
        if (!currentLine.empty()) {
            lines.push_back(currentLine);
        }
        
        // Draw each line
        float lineHeight = 30.0f; // Increased line height for better readability
        for (size_t i = 0; i < lines.size(); i++) {
            ctx->drawText(lines[i], x, y + i * lineHeight, color, fontName, 1.2f); // Increased text size
        }
    }
    
    void drawTypingUI(OtherCtx* ctx) {
        // Show what to type
        ctx->drawRoundedRect(textBackground.x, textBackground.y, textBackground.w, textBackground.h, 15, Color(30, 34, 42));
        drawWrappedText(ctx, currentSentence, textBackground.x + 20, textBackground.y + 30, textColor, "monospace");
        
        // Draw user's input with character highlighting
        ctx->drawRoundedRect(inputBackground.x, inputBackground.y, inputBackground.w, inputBackground.h, 15, Color(30, 34, 42));
        
        float xOffset = inputBackground.x + 20;
        float yOffset = inputBackground.y + 30;
        float xPos = xOffset;
        
        for (size_t i = 0; i < userInput.length(); i++) {
            Color charColor;
            if (i < currentSentence.length() && userInput[i] == currentSentence[i]) {
                charColor = correctColor;
            } else {
                charColor = errorColor;
            }
            
            std::string charStr(1, userInput[i]);
            ctx->drawText(charStr, xPos, yOffset, charColor, "monospace", 1.2f);
            xPos += charWidth;
            
            // Wrap to next line if needed (simplified wrapping for input field)
            if (xPos > inputBackground.x + inputBackground.w - 30) {
                xPos = xOffset;
                yOffset += 30.0f; // Line height
            }
        }
        
        // Draw current cursor position (blinking cursor at current position)
        int cursorBlinkRate = (SDL_GetTicks() / 500) % 2; // Blink every 0.5 seconds
        if (cursorBlinkRate == 0) {
            float cursorX = xOffset + (userInput.length() % (maxCharsPerLine - 2)) * charWidth;
            float cursorY = yOffset;
            ctx->drawRectOutline(cursorX, cursorY - 2, 2, 24, highlightColor);
        }
        
        // Draw current WPM
        std::stringstream wpmText;
        wpmText << "WPM: " << std::fixed << std::setprecision(1) << wpm;
        ctx->drawText(wpmText.str(), 50, 520, textColor, "monospace", 1.2f);
    }
    
    void drawFinishedUI(OtherCtx* ctx) {
        // Show completed text
        ctx->drawRoundedRect(textBackground.x, textBackground.y, textBackground.w, textBackground.h, 15, Color(30, 34, 42));
        drawWrappedText(ctx, currentSentence, textBackground.x + 20, textBackground.y + 30, textColor, "monospace");
        
        // Show user's input
        ctx->drawRoundedRect(inputBackground.x, inputBackground.y, inputBackground.w, inputBackground.h, 15, Color(30, 34, 42));
        drawWrappedText(ctx, userInput, inputBackground.x + 20, inputBackground.y + 30, highlightColor, "monospace");
        
        // Show results
        ctx->drawText("Typing test completed!", 300, 100, textColor, "monospace", 1.2f);
        
        std::stringstream wpmText;
        wpmText << "WPM: " << std::fixed << std::setprecision(1) << wpm;
        ctx->drawText(wpmText.str(), 380, 520, textColor, "monospace", 1.2f);
        
        std::stringstream accuracyText;
        accuracyText << "Accuracy: " << std::fixed << std::setprecision(1) << accuracy << "%";
        ctx->drawText(accuracyText.str(), 620, 520, textColor, "monospace", 1.2f);
        
        ctx->drawText("Press ENTER to try again", 300, 550, textColor, "monospace", 1.2f);
    }
};

int main(int argc, char* argv[]) {
    // Create engine with a nice window size for the typing test
    Engine engine("Typing Speed Test", 1200, 700); // Increased window size to fit larger boxes
    
    // Initialize the engine
    if (!engine.init()) {
        std::cerr << "Failed to initialize the engine!" << std::endl;
        return 1;
    }
    
    // Create and add our typing test game scene
    std::unique_ptr<Scene> gameScene = std::make_unique<TypingTestGame>();
    engine.addScene(std::move(gameScene));
    
    // Run the game
    engine.run();
    
    return 0;
} 