#pragma once
#include "MemoryManager.h"
#include "NarrativeUI.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <vector>

/**
 * GameState - Current phase of the game
 */
enum class GameState {
  PROLOGUE_NARRATION,
  PROLOGUE_THOUGHTS,
  PROLOGUE_SYSTEM,
  PROLOGUE_CHOICES,
  MEMORY_NARRATION,
  MEMORY_NPC_DIALOGUE,
  MEMORY_ECHO_DIALOGUE,
  MEMORY_CHOICES,
  SUB_CHOICES,
  SHOW_OUTCOME,
  ENDING,
  TRANSITION,
  GAME_OVER
};

/**
 * TypewriterText - Animates text character by character
 */
class TypewriterText {
public:
  sf::Text displayText;
  std::string fullText;
  std::string currentText;
  float charTimer = 0;
  float charDelay = 0.03f;
  size_t currentIndex = 0;
  bool isComplete = false;

  void setup(const sf::Font &font, float x, float y,
             unsigned int charSize = 22);
  void setText(const std::string &text);
  void update(float deltaTime);
  void skipToEnd();
  void draw(sf::RenderWindow &window);
  void reset();
};

/**
 * DialogueBox - Semi-transparent panel for displaying text
 */
class DialogueBox {
public:
  sf::RectangleShape background;
  sf::RectangleShape topAccent;
  sf::Text speakerName;
  std::vector<TypewriterText> lines;
  size_t currentLineIndex = 0;
  bool allLinesComplete = false;

  void setup(const sf::Font &font, float x, float y, float width, float height);
  void setSpeaker(const std::string &name);
  void setLines(const sf::Font &font,
                const std::vector<std::string> &textLines);
  void update(float deltaTime);
  void skipCurrentLine();
  void draw(sf::RenderWindow &window);
  bool isComplete();
  void reset();
};

/**
 * ChoiceSelector - Keyboard and mouse navigated choice menu
 */
class ChoiceSelector {
public:
  std::vector<sf::RectangleShape> backgrounds;
  std::vector<sf::Text> texts;
  std::vector<Choice> choices;
  Choice lastSelectedChoice; // Store selection before reset
  int selectedIndex = 0;
  float hoverProgress = 0;
  bool isVisible = false;

  void setup(const sf::Font &font, const std::vector<Choice> &choiceList,
             float startY);
  void moveUp();
  void moveDown();
  Choice *getSelectedChoice();
  void handleMouseMove(sf::Vector2f mousePos); // Mouse hover
  bool handleMouseClick(
      sf::Vector2f mousePos); // Mouse click - returns true if clicked
  void update(float deltaTime);
  void draw(sf::RenderWindow &window);
  void reset();
};

/**
 * GameEngine - Main SFML game engine for Echoance
 */
class GameEngine {
private:
  // SFML Components
  sf::RenderWindow window;
  sf::Font font;
  sf::Clock clock;
  float totalTime = 0;

  // Story Data
  MemoryManager storyManager;
  Memory *currentMemory = nullptr;

  // =========================================================================
  // UI INTEGRATION START - New polished UI components
  // =========================================================================
  DynamicTextBox narrativeBox; // Replaces old DialogueBox rendering
  ChoicePanel polishedChoices; // Replaces old ChoiceSelector rendering
  bool usePolishedUI = true;   // Toggle for new UI (set false to revert)
  // =========================================================================
  // UI INTEGRATION END
  // =========================================================================

  // Legacy UI Components (kept for compatibility, logic still uses these)
  DialogueBox dialogueBox;
  ChoiceSelector choiceSelector;
  sf::Text titleText;
  sf::Text subtitleText;
  sf::Text continueHint;

  // Game State
  GameState state = GameState::PROLOGUE_NARRATION;
  size_t prologueSection = 0; // 0=narration, 1=thoughts, 2=system

  // Transition
  float fadeAlpha = 0;
  bool isFadingOut = false;
  bool isFadingIn = true;
  std::function<void()> onFadeComplete;

  // Background particles
  struct Particle {
    sf::CircleShape shape;
    float speedY, speedX, alpha;
  };
  std::vector<Particle> particles;

  // Methods
  void handleEvents();
  void update(float deltaTime);
  void render();
  void drawBackground();
  void updateParticles();

  void startPrologue();
  void advancePrologue();
  void startMemory(Memory *memory);
  void advanceMemoryNarration();
  void showMemoryChoices();
  void selectChoice();
  void showOutcome(const std::string &outcome);
  void advanceAfterOutcome(); // Handle state after outcome is shown
  void startEnding(Ending *ending);
  void transitionTo(std::function<void()> nextStateAction);

public:
  GameEngine();
  bool init(const std::string &storyFile);
  void run();
};
