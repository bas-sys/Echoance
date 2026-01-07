
/**
 * GameEngine.cpp - Echoance Narrative Game Engine
 *
 * SFML-based visual novel engine with:
 * - Typewriter text animation
 * - Keyboard-navigated choices
 * - Fade transitions between scenes
 * - Particle effects for atmosphere
 */

#include "GameEngine.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

// =============================================================================
// CONSTANTS
// =============================================================================
// UI POLISH: Increased window size for more vertical space
const int WINDOW_WIDTH = 1400;
const int WINDOW_HEIGHT = 850;

// Color palette - Sci-fi/mysterious aesthetic
const sf::Color BG_PRIMARY(15, 12, 30);
const sf::Color BG_SECONDARY(25, 20, 50);
const sf::Color ACCENT_CYAN(100, 220, 255);
const sf::Color ACCENT_PINK(255, 120, 180);
const sf::Color TEXTBOX_BG(20, 25, 45, 230);
const sf::Color TEXTBOX_BORDER(80, 150, 200, 200);
const sf::Color BUTTON_NORMAL(40, 50, 80, 220);
const sf::Color BUTTON_HOVER(60, 100, 140, 240);
const sf::Color TEXT_WHITE(255, 255, 255);
const sf::Color TEXT_ACCENT(180, 230, 255);
const sf::Color SYSTEM_COLOR(100, 255, 180);
const sf::Color THOUGHT_COLOR(200, 180, 255);

// =============================================================================
// TypewriterText Implementation
// =============================================================================

void TypewriterText::setup(const sf::Font &font, float x, float y,
                           unsigned int charSize) {
  displayText.setFont(font);
  displayText.setCharacterSize(charSize);
  displayText.setFillColor(TEXT_WHITE);
  displayText.setPosition(x, y);
}

void TypewriterText::setText(const std::string &text) {
  fullText = text;
  currentText = "";
  currentIndex = 0;
  isComplete = false;
  charTimer = 0;
}

void TypewriterText::update(float deltaTime) {
  if (isComplete)
    return;

  charTimer += deltaTime;
  while (charTimer >= charDelay && currentIndex < fullText.length()) {
    charTimer -= charDelay;
    currentText += fullText[currentIndex];
    currentIndex++;
  }

  if (currentIndex >= fullText.length()) {
    isComplete = true;
  }

  displayText.setString(currentText);
}

void TypewriterText::skipToEnd() {
  currentText = fullText;
  currentIndex = fullText.length();
  isComplete = true;
  displayText.setString(currentText);
}

void TypewriterText::draw(sf::RenderWindow &window) {
  window.draw(displayText);
}

void TypewriterText::reset() {
  fullText = "";
  currentText = "";
  currentIndex = 0;
  isComplete = false;
  charTimer = 0;
}

// =============================================================================
// DialogueBox Implementation
// =============================================================================

void DialogueBox::setup(const sf::Font &font, float x, float y, float width,
                        float height) {
  background.setSize(sf::Vector2f(width, height));
  background.setPosition(x, y);
  background.setFillColor(TEXTBOX_BG);
  background.setOutlineColor(TEXTBOX_BORDER);
  background.setOutlineThickness(2);

  topAccent.setSize(sf::Vector2f(width - 40, 3));
  topAccent.setPosition(x + 20, y + 8);
  topAccent.setFillColor(sf::Color(100, 200, 255, 180));

  speakerName.setFont(font);
  speakerName.setCharacterSize(20);
  speakerName.setFillColor(ACCENT_CYAN);
  speakerName.setStyle(sf::Text::Bold);
  speakerName.setPosition(x + 25, y + 15);
}

void DialogueBox::setSpeaker(const std::string &name) {
  speakerName.setString(name);
}

void DialogueBox::setLines(const sf::Font &font,
                           const std::vector<std::string> &textLines) {
  reset();
  float startY = background.getPosition().y + 50;
  float x = background.getPosition().x + 25;

  for (size_t i = 0; i < textLines.size(); i++) {
    TypewriterText line;
    line.setup(font, x, startY + i * 35, 20);
    line.setText(textLines[i]);
    lines.push_back(line);
  }
}

void DialogueBox::update(float deltaTime) {
  if (lines.empty()) {
    allLinesComplete = true;
    return;
  }

  // Animate current line
  if (currentLineIndex < lines.size()) {
    lines[currentLineIndex].update(deltaTime);
    if (lines[currentLineIndex].isComplete &&
        currentLineIndex < lines.size() - 1) {
      currentLineIndex++;
    }
  }

  // Check if all lines complete
  allLinesComplete = true;
  for (auto &line : lines) {
    if (!line.isComplete) {
      allLinesComplete = false;
      break;
    }
  }
}

void DialogueBox::skipCurrentLine() {
  if (currentLineIndex < lines.size()) {
    if (!lines[currentLineIndex].isComplete) {
      lines[currentLineIndex].skipToEnd();
    } else if (currentLineIndex < lines.size() - 1) {
      currentLineIndex++;
    }
  }
}

void DialogueBox::draw(sf::RenderWindow &window) {
  window.draw(background);
  window.draw(topAccent);
  window.draw(speakerName);

  for (size_t i = 0; i <= currentLineIndex && i < lines.size(); i++) {
    lines[i].draw(window);
  }
}

bool DialogueBox::isComplete() { return allLinesComplete; }

void DialogueBox::reset() {
  lines.clear();
  currentLineIndex = 0;
  allLinesComplete = false;
}

// =============================================================================
// ChoiceSelector Implementation
// =============================================================================

void ChoiceSelector::setup(const sf::Font &font,
                           const std::vector<Choice> &choiceList,
                           float startY) {
  reset();
  choices = choiceList;

  float buttonWidth = 600;
  float buttonHeight = 50;
  float spacing = 15;
  float startX = (WINDOW_WIDTH - buttonWidth) / 2;

  for (size_t i = 0; i < choices.size(); i++) {
    float y = startY + i * (buttonHeight + spacing);

    sf::RectangleShape bg(sf::Vector2f(buttonWidth, buttonHeight));
    bg.setPosition(startX, y);
    bg.setFillColor(BUTTON_NORMAL);
    bg.setOutlineColor(sf::Color(100, 180, 220, 150));
    bg.setOutlineThickness(2);
    backgrounds.push_back(bg);

    sf::Text text;
    text.setFont(font);
    text.setString(std::to_string(i + 1) + ". " + choices[i].text);
    text.setCharacterSize(18);
    text.setFillColor(TEXT_WHITE);

    sf::FloatRect bounds = text.getLocalBounds();
    text.setPosition(startX + 20, y + (buttonHeight - bounds.height) / 2 - 5);
    texts.push_back(text);
  }

  selectedIndex = 0;
  isVisible = true;
}

void ChoiceSelector::moveUp() {
  if (selectedIndex > 0) {
    selectedIndex--;
  }
}

void ChoiceSelector::moveDown() {
  if (selectedIndex < static_cast<int>(choices.size()) - 1) {
    selectedIndex++;
  }
}

Choice *ChoiceSelector::getSelectedChoice() {
  if (selectedIndex >= 0 && selectedIndex < static_cast<int>(choices.size())) {
    return &choices[selectedIndex];
  }
  return nullptr;
}

void ChoiceSelector::update(float deltaTime) {
  for (size_t i = 0; i < backgrounds.size(); i++) {
    if (static_cast<int>(i) == selectedIndex) {
      backgrounds[i].setFillColor(BUTTON_HOVER);
      backgrounds[i].setOutlineColor(sf::Color(100, 220, 255, 220));
      backgrounds[i].setOutlineThickness(3);
      texts[i].setFillColor(ACCENT_CYAN);
    } else {
      backgrounds[i].setFillColor(BUTTON_NORMAL);
      backgrounds[i].setOutlineColor(sf::Color(100, 180, 220, 150));
      backgrounds[i].setOutlineThickness(2);
      texts[i].setFillColor(TEXT_WHITE);
    }
  }
}

void ChoiceSelector::draw(sf::RenderWindow &window) {
  if (!isVisible)
    return;

  for (size_t i = 0; i < backgrounds.size(); i++) {
    window.draw(backgrounds[i]);
    window.draw(texts[i]);
  }
}

// Handle mouse hover - updates selectedIndex based on mouse position
void ChoiceSelector::handleMouseMove(sf::Vector2f mousePos) {
  if (!isVisible)
    return;

  for (size_t i = 0; i < backgrounds.size(); i++) {
    if (backgrounds[i].getGlobalBounds().contains(mousePos)) {
      selectedIndex = static_cast<int>(i);
      return;
    }
  }
}

// Handle mouse click - returns true if a choice was clicked
bool ChoiceSelector::handleMouseClick(sf::Vector2f mousePos) {
  if (!isVisible)
    return false;

  for (size_t i = 0; i < backgrounds.size(); i++) {
    if (backgrounds[i].getGlobalBounds().contains(mousePos)) {
      selectedIndex = static_cast<int>(i);
      return true;
    }
  }
  return false;
}

void ChoiceSelector::reset() {
  backgrounds.clear();
  texts.clear();
  choices.clear();
  selectedIndex = 0;
  isVisible = false;
}

// =============================================================================
// GameEngine Implementation
// =============================================================================

GameEngine::GameEngine() { srand(static_cast<unsigned>(time(nullptr))); }

bool GameEngine::init(const std::string &storyFile) {
  // Create window
  window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
                "Echoance - The Emotional Stream",
                sf::Style::Titlebar | sf::Style::Close);
  window.setFramerateLimit(60);

  // Load font
  if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
    std::cerr << "Failed to load font!\n";
    return false;
  }

  // Load story
  storyManager.loadFromJSON(storyFile);

  // Setup title
  titleText.setFont(font);
  titleText.setString(storyManager.gameTitle);
  titleText.setCharacterSize(48);
  titleText.setFillColor(sf::Color(100, 200, 255, 200));
  titleText.setStyle(sf::Text::Bold);
  sf::FloatRect titleBounds = titleText.getLocalBounds();
  titleText.setPosition((WINDOW_WIDTH - titleBounds.width) / 2, 20);

  // Setup subtitle
  subtitleText.setFont(font);
  subtitleText.setString(storyManager.subtitle);
  subtitleText.setCharacterSize(20);
  subtitleText.setFillColor(sf::Color(180, 160, 200, 180));
  sf::FloatRect subBounds = subtitleText.getLocalBounds();
  subtitleText.setPosition((WINDOW_WIDTH - subBounds.width) / 2, 80);

  // Setup continue hint
  continueHint.setFont(font);
  continueHint.setString("Press SPACE to continue...");
  continueHint.setCharacterSize(14);
  continueHint.setFillColor(sf::Color(150, 200, 255, 180));
  continueHint.setPosition(WINDOW_WIDTH - 220, WINDOW_HEIGHT - 35);

  // Setup dialogue box
  float boxWidth = WINDOW_WIDTH - 100;
  float boxHeight = 180;
  dialogueBox.setup(font, 50, WINDOW_HEIGHT - boxHeight - 40, boxWidth,
                    boxHeight);

  // =========================================================================
  // UI INTEGRATION START - Initialize new polished UI components
  // =========================================================================
  narrativeBox.setup(font, WINDOW_WIDTH, WINDOW_HEIGHT, 20);
  // Note: polishedChoices is setup dynamically when choices appear
  // =========================================================================
  // UI INTEGRATION END
  // =========================================================================

  // Initialize particles
  for (int i = 0; i < 50; i++) {
    Particle p;
    float size = 1.0f + static_cast<float>(rand() % 30) / 10.0f;
    p.shape.setRadius(size);
    p.shape.setPosition(static_cast<float>(rand() % WINDOW_WIDTH),
                        static_cast<float>(rand() % WINDOW_HEIGHT));
    p.speedY = -0.2f - static_cast<float>(rand() % 50) / 100.0f;
    p.speedX = (static_cast<float>(rand() % 100) - 50.0f) / 200.0f;
    p.alpha = static_cast<float>(rand() % 150 + 50);

    if (rand() % 2 == 0) {
      p.shape.setFillColor(
          sf::Color(100, 200, 255, static_cast<sf::Uint8>(p.alpha)));
    } else {
      p.shape.setFillColor(
          sf::Color(255, 150, 200, static_cast<sf::Uint8>(p.alpha)));
    }
    particles.push_back(p);
  }

  // Start with prologue
  startPrologue();

  return true;
}

void GameEngine::run() {
  while (window.isOpen()) {
    float deltaTime = clock.restart().asSeconds();
    totalTime += deltaTime;

    handleEvents();
    update(deltaTime);
    render();
  }
}

void GameEngine::handleEvents() {
  // Get mouse position for hover effects
  sf::Vector2f mousePos =
      static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

  // Update choice hover based on mouse position
  // DUAL INPUT: Mouse hover updates selectedIndex for both UI systems
  if (state == GameState::PROLOGUE_CHOICES ||
      state == GameState::MEMORY_CHOICES || state == GameState::SUB_CHOICES) {
    choiceSelector.handleMouseMove(mousePos);

    // DUAL INPUT SYNC: Sync polishedChoices with choiceSelector's selection
    if (usePolishedUI) {
      polishedChoices.handleMouseMove(mousePos);
    }
  }

  sf::Event event;
  while (window.pollEvent(event)) {
    if (event.type == sf::Event::Closed) {
      window.close();
    }

    // Handle mouse clicks
    if (event.type == sf::Event::MouseButtonPressed) {
      if (event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f clickPos(static_cast<float>(event.mouseButton.x),
                              static_cast<float>(event.mouseButton.y));

        if (state == GameState::TRANSITION) {
          // Do nothing during transition
        } else if (state == GameState::PROLOGUE_CHOICES ||
                   state == GameState::MEMORY_CHOICES ||
                   state == GameState::SUB_CHOICES) {
          // DUAL INPUT: Check click on either UI system
          bool clicked = choiceSelector.handleMouseClick(clickPos);
          if (usePolishedUI) {
            polishedChoices.handleMouseClick(clickPos);
          }
          if (clicked) {
            selectChoice();
          }
        } else if (state == GameState::PROLOGUE_NARRATION ||
                   state == GameState::PROLOGUE_THOUGHTS ||
                   state == GameState::PROLOGUE_SYSTEM) {
          // Click to advance prologue
          bool complete = usePolishedUI ? narrativeBox.isComplete()
                                        : dialogueBox.isComplete();
          if (complete) {
            advancePrologue();
          } else {
            dialogueBox.skipCurrentLine();
            if (usePolishedUI)
              narrativeBox.skipAnimation();
          }
        } else if (state == GameState::MEMORY_NARRATION ||
                   state == GameState::MEMORY_NPC_DIALOGUE ||
                   state == GameState::MEMORY_ECHO_DIALOGUE) {
          // Click to advance memory
          bool complete = usePolishedUI ? narrativeBox.isComplete()
                                        : dialogueBox.isComplete();
          if (complete) {
            advanceMemoryNarration();
          } else {
            dialogueBox.skipCurrentLine();
            if (usePolishedUI)
              narrativeBox.skipAnimation();
          }
        } else if (state == GameState::SHOW_OUTCOME) {
          bool complete = usePolishedUI ? narrativeBox.isComplete()
                                        : dialogueBox.isComplete();
          if (complete) {
            advanceAfterOutcome();
          } else {
            dialogueBox.skipCurrentLine();
            if (usePolishedUI)
              narrativeBox.skipAnimation();
          }
        } else if (state == GameState::ENDING) {
          bool complete = usePolishedUI ? narrativeBox.isComplete()
                                        : dialogueBox.isComplete();
          if (complete) {
            state = GameState::GAME_OVER;
          } else {
            dialogueBox.skipCurrentLine();
            if (usePolishedUI)
              narrativeBox.skipAnimation();
          }
        } else if (state == GameState::GAME_OVER) {
          window.close();
        }
      }
    }

    if (event.type == sf::Event::KeyPressed) {
      switch (event.key.code) {
      case sf::Keyboard::Space:
      case sf::Keyboard::Enter:
        if (state == GameState::TRANSITION)
          break;

        // SPACE SKIP: Instantly finish typewriter or advance to next node
        if (state == GameState::PROLOGUE_NARRATION ||
            state == GameState::PROLOGUE_THOUGHTS ||
            state == GameState::PROLOGUE_SYSTEM) {
          bool complete = usePolishedUI ? narrativeBox.isComplete()
                                        : dialogueBox.isComplete();
          if (complete) {
            advancePrologue();
          } else {
            dialogueBox.skipCurrentLine();
            if (usePolishedUI)
              narrativeBox.skipAnimation();
          }
        } else if (state == GameState::MEMORY_NARRATION ||
                   state == GameState::MEMORY_NPC_DIALOGUE ||
                   state == GameState::MEMORY_ECHO_DIALOGUE) {
          bool complete = usePolishedUI ? narrativeBox.isComplete()
                                        : dialogueBox.isComplete();
          if (complete) {
            advanceMemoryNarration();
          } else {
            dialogueBox.skipCurrentLine();
            if (usePolishedUI)
              narrativeBox.skipAnimation();
          }
        } else if (state == GameState::SHOW_OUTCOME) {
          bool complete = usePolishedUI ? narrativeBox.isComplete()
                                        : dialogueBox.isComplete();
          if (complete) {
            advanceAfterOutcome();
          } else {
            dialogueBox.skipCurrentLine();
            if (usePolishedUI)
              narrativeBox.skipAnimation();
          }
        } else if (state == GameState::PROLOGUE_CHOICES ||
                   state == GameState::MEMORY_CHOICES ||
                   state == GameState::SUB_CHOICES) {
          selectChoice();
        } else if (state == GameState::ENDING) {
          bool complete = usePolishedUI ? narrativeBox.isComplete()
                                        : dialogueBox.isComplete();
          if (complete) {
            state = GameState::GAME_OVER;
          } else {
            dialogueBox.skipCurrentLine();
            if (usePolishedUI)
              narrativeBox.skipAnimation();
          }
        } else if (state == GameState::GAME_OVER) {
          window.close();
        }
        break;

      case sf::Keyboard::Up:
        if (state == GameState::PROLOGUE_CHOICES ||
            state == GameState::MEMORY_CHOICES ||
            state == GameState::SUB_CHOICES) {
          choiceSelector.moveUp();
          // UI INTEGRATION - Sync polished choices
          if (usePolishedUI)
            polishedChoices.moveUp();
        }
        break;

      case sf::Keyboard::Down:
        if (state == GameState::PROLOGUE_CHOICES ||
            state == GameState::MEMORY_CHOICES ||
            state == GameState::SUB_CHOICES) {
          choiceSelector.moveDown();
          // UI INTEGRATION - Sync polished choices
          if (usePolishedUI)
            polishedChoices.moveDown();
        }
        break;

      case sf::Keyboard::Escape:
        window.close();
        break;

      default:
        break;
      }
    }
  }
}

void GameEngine::update(float deltaTime) {
  // Update particles
  updateParticles();

  // Update dialogue
  dialogueBox.update(deltaTime);

  // =========================================================================
  // UI INTEGRATION START - Update new polished UI
  // =========================================================================
  if (usePolishedUI) {
    narrativeBox.update(deltaTime);

    // Sync narrative box with dialogue box state when choices are visible
    bool inChoiceState =
        (state == GameState::PROLOGUE_CHOICES ||
         state == GameState::MEMORY_CHOICES || state == GameState::SUB_CHOICES);
    if (inChoiceState) {
      polishedChoices.update(deltaTime);
    }
  }
  // =========================================================================
  // UI INTEGRATION END
  // =========================================================================

  // Update choices (legacy - still needed for logic)
  choiceSelector.update(deltaTime);

  // Update transition fade
  if (isFadingIn) {
    fadeAlpha -= deltaTime * 255;
    if (fadeAlpha <= 0) {
      fadeAlpha = 0;
      isFadingIn = false;
    }
  } else if (isFadingOut) {
    fadeAlpha += deltaTime * 255;
    if (fadeAlpha >= 255) {
      fadeAlpha = 255;
      isFadingOut = false;
      if (onFadeComplete) {
        onFadeComplete();
      }
      isFadingIn = true;
    }
  }

  // Update continue hint visibility (use new UI state if enabled)
  bool textComplete =
      usePolishedUI ? narrativeBox.isComplete() : dialogueBox.isComplete();
  float hintAlpha = textComplete ? (100 + std::sin(totalTime * 3.0f) * 80) : 0;
  continueHint.setFillColor(
      sf::Color(150, 200, 255, static_cast<sf::Uint8>(hintAlpha)));
}

void GameEngine::render() {
  window.clear(BG_PRIMARY);

  drawBackground();

  // Draw particles
  for (auto &p : particles) {
    window.draw(p.shape);
  }

  // Draw title
  window.draw(titleText);
  window.draw(subtitleText);

  // =========================================================================
  // UI INTEGRATION START - Render new polished UI instead of legacy
  // =========================================================================
  if (usePolishedUI) {
    // Draw polished narrative box
    narrativeBox.draw(window);

    // Draw polished choices if in choice state
    bool inChoiceState =
        (state == GameState::PROLOGUE_CHOICES ||
         state == GameState::MEMORY_CHOICES || state == GameState::SUB_CHOICES);
    if (inChoiceState) {
      polishedChoices.draw(window);
    }
  } else {
    // Legacy UI fallback
    dialogueBox.draw(window);
    choiceSelector.draw(window);
  }
  // =========================================================================
  // UI INTEGRATION END
  // =========================================================================

  // Draw continue hint
  if (state != GameState::PROLOGUE_CHOICES &&
      state != GameState::MEMORY_CHOICES && state != GameState::SUB_CHOICES &&
      state != GameState::GAME_OVER) {
    window.draw(continueHint);
  }

  // Draw fade overlay
  if (fadeAlpha > 0) {
    sf::RectangleShape fadeOverlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    fadeOverlay.setFillColor(
        sf::Color(0, 0, 0, static_cast<sf::Uint8>(fadeAlpha)));
    window.draw(fadeOverlay);
  }

  window.display();
}

void GameEngine::drawBackground() {
  // Gradient background
  sf::VertexArray gradient(sf::Quads, 4);
  float pulse = std::sin(totalTime * 0.5f) * 0.1f + 1.0f;

  sf::Uint8 topR = static_cast<sf::Uint8>(15 * pulse);
  sf::Uint8 topG = static_cast<sf::Uint8>(12 * pulse);
  sf::Uint8 topB = static_cast<sf::Uint8>(35 * pulse);

  sf::Uint8 botR = static_cast<sf::Uint8>(30 * pulse);
  sf::Uint8 botG = static_cast<sf::Uint8>(25 * pulse);
  sf::Uint8 botB = static_cast<sf::Uint8>(60 * pulse);

  gradient[0].position = sf::Vector2f(0, 0);
  gradient[0].color = sf::Color(topR, topG, topB);
  gradient[1].position = sf::Vector2f(WINDOW_WIDTH, 0);
  gradient[1].color = sf::Color(topR, topG, topB);
  gradient[2].position = sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT);
  gradient[2].color = sf::Color(botR, botG, botB);
  gradient[3].position = sf::Vector2f(0, WINDOW_HEIGHT);
  gradient[3].color = sf::Color(botR, botG, botB);

  window.draw(gradient);

  // Central glow
  sf::CircleShape glow(300);
  glow.setPosition(WINDOW_WIDTH / 2 - 300, WINDOW_HEIGHT / 2 - 300);
  glow.setFillColor(sf::Color(60, 40, 80, 30));
  window.draw(glow);
}

void GameEngine::updateParticles() {
  for (auto &p : particles) {
    sf::Vector2f pos = p.shape.getPosition();
    pos.y += p.speedY;
    pos.x += p.speedX;

    if (pos.y < -10) {
      pos.x = static_cast<float>(rand() % WINDOW_WIDTH);
      pos.y = static_cast<float>(WINDOW_HEIGHT + 10);
    }

    p.shape.setPosition(pos);
  }
}

// =============================================================================
// Story Flow Methods
// =============================================================================

void GameEngine::startPrologue() {
  prologueSection = 0;
  dialogueBox.setSpeaker("NARRATION");
  dialogueBox.setLines(font, storyManager.prologue.narration);
  state = GameState::PROLOGUE_NARRATION;

  // =========================================================================
  // UI INTEGRATION - Sync narrativeBox with dialogueBox content
  // =========================================================================
  if (usePolishedUI) {
    narrativeBox.setSpeaker("NARRATION");
    std::string combinedText;
    for (size_t i = 0; i < storyManager.prologue.narration.size(); i++) {
      combinedText += storyManager.prologue.narration[i];
      if (i < storyManager.prologue.narration.size() - 1)
        combinedText += "\n";
    }
    narrativeBox.setText(combinedText);
  }
}

void GameEngine::advancePrologue() {
  prologueSection++;

  if (prologueSection == 1) {
    // Player thoughts
    dialogueBox.setSpeaker("YOUR THOUGHTS");
    dialogueBox.setLines(font, storyManager.prologue.player_thoughts);
    for (auto &line : dialogueBox.lines) {
      line.displayText.setFillColor(THOUGHT_COLOR);
    }
    state = GameState::PROLOGUE_THOUGHTS;
  } else if (prologueSection == 2) {
    // System dialogue
    dialogueBox.setSpeaker("SYSTEM");
    dialogueBox.setLines(font, storyManager.prologue.system_dialogue);
    for (auto &line : dialogueBox.lines) {
      line.displayText.setFillColor(SYSTEM_COLOR);
    }
    state = GameState::PROLOGUE_SYSTEM;
  } else if (prologueSection == 3) {
    // Show choices
    choiceSelector.setup(font, storyManager.prologue.choices, 250);
    state = GameState::PROLOGUE_CHOICES;

    // =========================================================================
    // UI INTEGRATION - Sync polishedChoices with choiceSelector
    // =========================================================================
    if (usePolishedUI) {
      polishedChoices.setup(font, storyManager.prologue.choices, 200.0f,
                            WINDOW_WIDTH);
    }
  } else {
    // After prologue choice, go to first memory
    transitionTo([this]() {
      Memory *firstMem = storyManager.getFirstMemory();
      if (firstMem) {
        startMemory(firstMem);
      }
    });
  }
}

void GameEngine::startMemory(Memory *memory) {
  currentMemory = memory;

  // Update subtitle with memory title
  subtitleText.setString(memory->title);
  sf::FloatRect bounds = subtitleText.getLocalBounds();
  subtitleText.setPosition((WINDOW_WIDTH - bounds.width) / 2, 80);

  // Show narration
  dialogueBox.setSpeaker("NARRATION");
  dialogueBox.setLines(font, memory->narration);
  state = GameState::MEMORY_NARRATION;

  // =========================================================================
  // UI INTEGRATION - Sync narrativeBox with memory narration
  // =========================================================================
  if (usePolishedUI) {
    narrativeBox.setSpeaker("NARRATION");
    std::string combinedText;
    for (size_t i = 0; i < memory->narration.size(); i++) {
      combinedText += memory->narration[i];
      if (i < memory->narration.size() - 1)
        combinedText += "\n";
    }
    narrativeBox.setText(combinedText);
  }
}

void GameEngine::advanceMemoryNarration() {
  if (!currentMemory)
    return;

  if (state == GameState::MEMORY_NARRATION) {
    // Move to NPC dialogue
    if (!currentMemory->npc.dialogues.empty()) {
      dialogueBox.setSpeaker(currentMemory->npc.name);
      dialogueBox.setLines(font, currentMemory->npc.dialogues);
      state = GameState::MEMORY_NPC_DIALOGUE;
    } else {
      showMemoryChoices();
    }
  } else if (state == GameState::MEMORY_NPC_DIALOGUE) {
    // Move to Echo dialogue if present
    if (!currentMemory->echo.dialogues.empty()) {
      dialogueBox.setSpeaker("THE ECHO");
      dialogueBox.setLines(font, currentMemory->echo.dialogues);
      for (auto &line : dialogueBox.lines) {
        line.displayText.setFillColor(ACCENT_PINK);
      }
      state = GameState::MEMORY_ECHO_DIALOGUE;
    } else {
      showMemoryChoices();
    }
  } else if (state == GameState::MEMORY_ECHO_DIALOGUE) {
    showMemoryChoices();
  } else if (state == GameState::SHOW_OUTCOME) {
    // After showing outcome, move to next memory or ending
    Choice *selected = choiceSelector.getSelectedChoice();
    if (selected) {
      // Check for ending
      Ending *ending = storyManager.getEndingById(selected->id);
      if (ending) {
        transitionTo([this, ending]() { startEnding(ending); });
        return;
      }

      // Check for specific next memory
      if (!selected->next_memory_id.empty()) {
        Memory *nextMem = storyManager.getMemoryById(selected->next_memory_id);
        if (nextMem) {
          transitionTo([this, nextMem]() { startMemory(nextMem); });
          return;
        }
      }
    }

    // Default: go to next memory in sequence
    if (currentMemory) {
      bool foundCurrent = false;
      Memory *nextMem = nullptr;
      for (auto &mem : storyManager.memories) {
        if (foundCurrent) {
          nextMem = &mem;
          break;
        }
        if (mem.id == currentMemory->id) {
          foundCurrent = true;
        }
      }

      if (nextMem) {
        transitionTo([this, nextMem]() { startMemory(nextMem); });
      } else {
        // No more memories - check if this was an ending choice
        if (!storyManager.endings.empty()) {
          transitionTo([this]() { startEnding(&storyManager.endings[0]); });
        }
      }
    }
  }
}

void GameEngine::showMemoryChoices() {
  if (!currentMemory)
    return;
  choiceSelector.setup(font, currentMemory->choices, 200);
  state = GameState::MEMORY_CHOICES;

  // =========================================================================
  // UI INTEGRATION - Sync polishedChoices with memory choices
  // =========================================================================
  if (usePolishedUI) {
    polishedChoices.setup(font, currentMemory->choices, 200.0f, WINDOW_WIDTH);
  }
}

void GameEngine::selectChoice() {
  Choice *selected = choiceSelector.getSelectedChoice();
  if (!selected)
    return;

  // Check for sub-choices
  if (!selected->sub_choices.empty()) {
    choiceSelector.setup(font, selected->sub_choices, 200);
    state = GameState::SUB_CHOICES;
    return;
  }

  // Store the choice before showing outcome
  choiceSelector.lastSelectedChoice = *selected;

  // Show outcome
  showOutcome(selected->outcome);
}

void GameEngine::showOutcome(const std::string &outcome) {
  if (outcome.empty()) {
    // No outcome text, advance directly
    advanceAfterOutcome();
    return;
  }

  dialogueBox.setSpeaker("OUTCOME");
  std::vector<std::string> outcomeLines = {outcome};
  dialogueBox.setLines(font, outcomeLines);
  for (auto &line : dialogueBox.lines) {
    line.displayText.setFillColor(ACCENT_CYAN);
  }
  state = GameState::SHOW_OUTCOME;

  // =========================================================================
  // FINAL CHAPTER FIX START - Sync polished UI with outcome
  // =========================================================================
  if (usePolishedUI) {
    narrativeBox.setSpeaker("OUTCOME");
    narrativeBox.setText(outcome);
  }
  // =========================================================================
  // FINAL CHAPTER FIX END
  // =========================================================================
}

void GameEngine::advanceAfterOutcome() {
  // Use the stored choice from lastSelectedChoice
  Choice &selected = choiceSelector.lastSelectedChoice;

  // Check for ending by choice ID
  Ending *ending = storyManager.getEndingById(selected.id);
  if (ending) {
    transitionTo([this, ending]() { startEnding(ending); });
    return;
  }

  // Check for specific next memory from the choice
  if (!selected.next_memory_id.empty()) {
    Memory *nextMem = storyManager.getMemoryById(selected.next_memory_id);
    if (nextMem) {
      transitionTo([this, nextMem]() { startMemory(nextMem); });
      return;
    }
  }

  // Default: go to next memory in sequence
  if (currentMemory) {
    bool foundCurrent = false;
    Memory *nextMem = nullptr;
    for (auto &mem : storyManager.memories) {
      if (foundCurrent) {
        nextMem = &mem;
        break;
      }
      if (mem.id == currentMemory->id) {
        foundCurrent = true;
      }
    }

    if (nextMem) {
      transitionTo([this, nextMem]() { startMemory(nextMem); });
    } else {
      // No more memories - show first ending or game over
      if (!storyManager.endings.empty()) {
        transitionTo([this]() { startEnding(&storyManager.endings[0]); });
      } else {
        state = GameState::GAME_OVER;
      }
    }
  }
}

void GameEngine::startEnding(Ending *ending) {
  subtitleText.setString(ending->title);
  sf::FloatRect bounds = subtitleText.getLocalBounds();
  subtitleText.setPosition((WINDOW_WIDTH - bounds.width) / 2, 80);

  dialogueBox.setSpeaker("ENDING");
  dialogueBox.setLines(font, ending->narration);
  choiceSelector.reset();
  state = GameState::ENDING;

  // =========================================================================
  // FINAL CHAPTER FIX START - Sync polished UI with ending
  // =========================================================================
  if (usePolishedUI) {
    narrativeBox.setSpeaker("ENDING");
    std::string combinedText;
    for (size_t i = 0; i < ending->narration.size(); i++) {
      combinedText += ending->narration[i];
      if (i < ending->narration.size() - 1)
        combinedText += "\n";
    }
    narrativeBox.setText(combinedText);
    polishedChoices.reset(); // Hide choices during ending
  }
  // =========================================================================
  // FINAL CHAPTER FIX END
  // =========================================================================
}

void GameEngine::transitionTo(std::function<void()> nextStateAction) {
  onFadeComplete = nextStateAction;
  isFadingOut = true;
  state = GameState::TRANSITION;
  choiceSelector.reset();
}
