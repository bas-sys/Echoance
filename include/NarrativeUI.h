#pragma once
/**
 * NarrativeUI.h - Polished UI Components for Visual Novel
 *
 * Features:
 * - DynamicTextBox: Auto-sizing with word wrap, max-height + scroll
 * - ChoiceCard: Rounded backgrounds with Default/Hover/Selected states
 * - ChoicePanel: Stacked choices with keyboard navigation
 * - Relative layout system (no hardcoded positions)
 */

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

// =============================================================================
// CONFIGURATION CONSTANTS
// =============================================================================

namespace UIConfig {
// Layout
constexpr float SCREEN_PADDING = 40.0f;
constexpr float BOX_PADDING = 28.0f;
constexpr float LINE_SPACING = 1.5f;
constexpr float CHOICE_SPACING = 14.0f;

// Text Box Sizing
constexpr float MIN_BOX_HEIGHT = 120.0f;
constexpr float MAX_BOX_HEIGHT = 280.0f;
constexpr float DEFAULT_BOX_WIDTH_RATIO = 0.88f;

// Choice Cards
constexpr float CHOICE_MIN_HEIGHT = 55.0f;
constexpr float CHOICE_PADDING_H = 24.0f;
constexpr float CHOICE_PADDING_V = 14.0f;
constexpr float CORNER_RADIUS = 12.0f;

// Animation
constexpr float TYPEWRITER_SPEED = 0.028f;
constexpr float HOVER_TRANSITION_SPEED = 10.0f;
constexpr float GLOW_PULSE_SPEED = 2.8f;

// Enhanced Color Palette - Premium Visual Novel Style
const sf::Color BG_DARK(18, 22, 42, 235);         // Deep navy
const sf::Color BG_GRADIENT_TOP(25, 30, 55, 240); // Gradient top
const sf::Color BG_GRADIENT_BOT(15, 18, 38, 240); // Gradient bottom
const sf::Color BG_HOVER(45, 65, 105, 245);       // Hover state
const sf::Color BG_SELECTED(55, 95, 145, 250);    // Selected state

// Borders - Glowing cyan theme
const sf::Color BORDER_DEFAULT(70, 140, 190, 160);
const sf::Color BORDER_HOVER(90, 200, 255, 200);
const sf::Color BORDER_SELECTED(100, 230, 255, 255);
const sf::Color BORDER_GLOW(80, 180, 255, 100);

// Text Colors
const sf::Color TEXT_PRIMARY(248, 250, 255);   // Crisp white
const sf::Color TEXT_SECONDARY(180, 195, 220); // Soft blue-gray
const sf::Color TEXT_HIGHLIGHT(160, 230, 255); // Bright cyan
const sf::Color TEXT_SPEAKER(120, 220, 255);   // Speaker name

// Accent Colors
const sf::Color ACCENT_CYAN(100, 220, 255);
const sf::Color ACCENT_PINK(255, 140, 200);
const sf::Color ACCENT_PURPLE(180, 140, 255);
const sf::Color ACCENT_GLOW(90, 200, 255, 50);

// Special Effects
const sf::Color GLOW_INNER(100, 200, 255, 40);
const sf::Color GLOW_OUTER(80, 180, 255, 20);
const sf::Color SHIMMER(255, 255, 255, 30);
} // namespace UIConfig

// =============================================================================
// WORD WRAPPING UTILITY
// =============================================================================

/**
 * Wraps text to fit within maxWidth pixels.
 * Returns vector of lines that fit within the constraint.
 */
inline std::vector<std::string> wrapText(const std::string &text,
                                         const sf::Font &font,
                                         unsigned int fontSize,
                                         float maxWidth) {
  std::vector<std::string> lines;
  std::string currentLine;
  std::string word;

  for (size_t i = 0; i <= text.length(); i++) {
    char c = (i < text.length()) ? text[i] : ' ';

    if (c == ' ' || c == '\n' || i == text.length()) {
      if (!word.empty()) {
        // Test if word fits on current line
        std::string testLine =
            currentLine.empty() ? word : currentLine + " " + word;

        sf::Text measure;
        measure.setFont(font);
        measure.setCharacterSize(fontSize);
        measure.setString(testLine);

        if (measure.getLocalBounds().width <= maxWidth) {
          currentLine = testLine;
        } else {
          // Word doesn't fit - push current line and start new
          if (!currentLine.empty()) {
            lines.push_back(currentLine);
          }
          currentLine = word;
        }
        word.clear();
      }

      if (c == '\n') {
        lines.push_back(currentLine);
        currentLine.clear();
      }
    } else {
      word += c;
    }
  }

  if (!currentLine.empty()) {
    lines.push_back(currentLine);
  }
  if (lines.empty()) {
    lines.push_back("");
  }

  return lines;
}

// =============================================================================
// ROUNDED RECTANGLE HELPER
// =============================================================================

/**
 * Creates a rounded rectangle as a ConvexShape.
 */
inline sf::ConvexShape createRoundedRect(float width, float height,
                                         float radius, int cornerPoints = 8) {
  sf::ConvexShape shape;
  int totalPoints = cornerPoints * 4;
  shape.setPointCount(totalPoints);

  float pi = 3.14159265f;
  int idx = 0;

  // Four corners: top-left, top-right, bottom-right, bottom-left
  float corners[4][2] = {{radius, radius},
                         {width - radius, radius},
                         {width - radius, height - radius},
                         {radius, height - radius}};

  float startAngles[4] = {pi, pi * 1.5f, 0, pi * 0.5f};

  for (int corner = 0; corner < 4; corner++) {
    float cx = corners[corner][0];
    float cy = corners[corner][1];
    float startAngle = startAngles[corner];

    for (int i = 0; i < cornerPoints; i++) {
      float angle = startAngle + (pi * 0.5f) * i / (cornerPoints - 1);
      float x = cx + std::cos(angle) * radius;
      float y = cy + std::sin(angle) * radius;
      shape.setPoint(idx++, sf::Vector2f(x, y));
    }
  }

  return shape;
}

// =============================================================================
// DYNAMIC TEXT BOX
// =============================================================================

/**
 * DynamicTextBox - Auto-sizing narration/dialogue container
 *
 * Features:
 * - Word wrapping
 * - Auto-height based on content
 * - Max-height with scroll support
 * - Typewriter animation
 * - Semi-transparent rounded background
 */
class DynamicTextBox {
private:
  // Visual elements
  sf::ConvexShape background;
  sf::RectangleShape topAccent;
  sf::RectangleShape bottomAccent;
  sf::Text speakerText;
  sf::Text contentText;

  // Content
  const sf::Font *fontPtr = nullptr;
  std::string fullText;
  std::string displayedText;
  std::vector<std::string> wrappedLines;

  // Typewriter
  float charTimer = 0;
  float charDelay = UIConfig::TYPEWRITER_SPEED;
  size_t currentCharIndex = 0;
  bool animationComplete = false;

  // Dimensions
  float boxX = 0, boxY = 0;
  float boxWidth = 0, boxHeight = 0;
  float contentHeight = 0;
  float maxWidth = 800;
  unsigned int fontSize = 20;

  // Scroll
  float scrollOffset = 0;
  float maxScroll = 0;
  bool scrollEnabled = false;

  // Animation
  float glowPulse = 0;

public:
  /**
   * Initialize with font and screen dimensions.
   */
  void setup(const sf::Font &font, float screenWidth, float screenHeight,
             unsigned int textSize = 20) {
    fontPtr = &font;
    fontSize = textSize;
    maxWidth = screenWidth * UIConfig::DEFAULT_BOX_WIDTH_RATIO;

    // Position at bottom of screen
    boxX = (screenWidth - maxWidth) / 2.0f;
    boxY = screenHeight - UIConfig::MAX_BOX_HEIGHT - UIConfig::SCREEN_PADDING;

    // Setup speaker text
    speakerText.setFont(font);
    speakerText.setCharacterSize(fontSize - 2);
    speakerText.setFillColor(UIConfig::ACCENT_CYAN);
    speakerText.setStyle(sf::Text::Bold);

    // Setup content text
    contentText.setFont(font);
    contentText.setCharacterSize(fontSize);
    contentText.setFillColor(UIConfig::TEXT_PRIMARY);

    // Setup accent lines
    topAccent.setFillColor(sf::Color(100, 200, 255, 150));
    bottomAccent.setFillColor(sf::Color(100, 200, 255, 80));
  }

  /**
   * Set speaker name (e.g., "NARRATION", "NPC Name").
   */
  void setSpeaker(const std::string &name) { speakerText.setString(name); }

  /**
   * Set the narration text - triggers word wrap and size calculation.
   */
  void setText(const std::string &text) {
    fullText = text;
    displayedText.clear();
    currentCharIndex = 0;
    animationComplete = false;
    charTimer = 0;
    scrollOffset = 0;

    calculateLayout();
  }

  /**
   * Calculate box height based on wrapped text.
   */
  void calculateLayout() {
    if (!fontPtr)
      return;

    float contentMaxWidth = maxWidth - (UIConfig::BOX_PADDING * 2);

    // Wrap the text
    wrappedLines = wrapText(fullText, *fontPtr, fontSize, contentMaxWidth);

    // Calculate content height
    float lineHeight = fontSize * UIConfig::LINE_SPACING;
    float speakerHeight = fontSize + 10; // Speaker name + margin
    contentHeight = speakerHeight + (wrappedLines.size() * lineHeight);

    // Calculate box height (clamped)
    float paddedHeight = contentHeight + (UIConfig::BOX_PADDING * 2);
    boxHeight = std::max(UIConfig::MIN_BOX_HEIGHT,
                         std::min(paddedHeight, UIConfig::MAX_BOX_HEIGHT));

    // Enable scroll if content exceeds max height
    if (paddedHeight > UIConfig::MAX_BOX_HEIGHT) {
      scrollEnabled = true;
      maxScroll = paddedHeight - UIConfig::MAX_BOX_HEIGHT;
    } else {
      scrollEnabled = false;
      maxScroll = 0;
    }

    // Update background shape
    background =
        createRoundedRect(maxWidth, boxHeight, UIConfig::CORNER_RADIUS);
    background.setPosition(boxX, boxY);
    background.setFillColor(UIConfig::BG_DARK);
    background.setOutlineColor(UIConfig::BORDER_DEFAULT);
    background.setOutlineThickness(2);

    // Position accent lines
    float accentWidth = maxWidth - 50;
    topAccent.setSize(sf::Vector2f(accentWidth, 2));
    topAccent.setPosition(boxX + 25, boxY + 10);
    bottomAccent.setSize(sf::Vector2f(accentWidth, 1));
    bottomAccent.setPosition(boxX + 25, boxY + boxHeight - 10);

    // Position speaker
    speakerText.setPosition(boxX + UIConfig::BOX_PADDING, boxY + 15);
  }

  /**
   * Update typewriter animation.
   */
  void update(float deltaTime) {
    glowPulse += deltaTime * 2.5f;

    if (animationComplete)
      return;

    charTimer += deltaTime;
    while (charTimer >= charDelay && currentCharIndex < fullText.length()) {
      charTimer -= charDelay;
      displayedText += fullText[currentCharIndex];
      currentCharIndex++;
    }

    if (currentCharIndex >= fullText.length()) {
      animationComplete = true;
    }

    // Rebuild displayed text with wrapping
    updateDisplayedText();
  }

  /**
   * Build the currently displayed wrapped text.
   */
  void updateDisplayedText() {
    if (wrappedLines.empty())
      return;

    std::string displayStr;
    size_t charsRemaining = currentCharIndex;

    for (size_t i = 0; i < wrappedLines.size() && charsRemaining > 0; i++) {
      size_t lineLen = wrappedLines[i].length();
      if (charsRemaining >= lineLen) {
        displayStr += wrappedLines[i];
        charsRemaining -= lineLen;
        if (i < wrappedLines.size() - 1) {
          displayStr += "\n";
          if (charsRemaining > 0)
            charsRemaining--; // Account for space between words
        }
      } else {
        displayStr += wrappedLines[i].substr(0, charsRemaining);
        charsRemaining = 0;
      }
    }

    contentText.setString(displayStr);
  }

  /**
   * Skip to end of animation.
   */
  void skipAnimation() {
    displayedText = fullText;
    currentCharIndex = fullText.length();
    animationComplete = true;

    // Build full wrapped text
    std::string displayStr;
    for (size_t i = 0; i < wrappedLines.size(); i++) {
      displayStr += wrappedLines[i];
      if (i < wrappedLines.size() - 1) {
        displayStr += "\n";
      }
    }
    contentText.setString(displayStr);
  }

  /**
   * Scroll content (for long text).
   */
  void scroll(float delta) {
    if (!scrollEnabled)
      return;
    scrollOffset = std::max(0.0f, std::min(scrollOffset + delta, maxScroll));
  }

  /**
   * Draw the text box with premium visual effects.
   */
  void draw(sf::RenderWindow &window) {
    // Outer glow (soft, pulsing)
    sf::ConvexShape outerGlow = createRoundedRect(maxWidth + 16, boxHeight + 16,
                                                  UIConfig::CORNER_RADIUS + 4);
    outerGlow.setPosition(boxX - 8, boxY - 8);
    float outerAlpha = 25.0f + std::sin(glowPulse * 0.8f) * 15.0f;
    outerGlow.setFillColor(
        sf::Color(80, 180, 255, static_cast<sf::Uint8>(outerAlpha)));
    window.draw(outerGlow);

    // Inner glow (brighter, tighter)
    sf::ConvexShape innerGlow = createRoundedRect(maxWidth + 6, boxHeight + 6,
                                                  UIConfig::CORNER_RADIUS + 2);
    innerGlow.setPosition(boxX - 3, boxY - 3);
    float innerAlpha = 45.0f + std::sin(glowPulse) * 25.0f;
    innerGlow.setFillColor(
        sf::Color(100, 200, 255, static_cast<sf::Uint8>(innerAlpha)));
    window.draw(innerGlow);

    // Main background with gradient-like effect
    window.draw(background);

    // Top accent line (animated shimmer)
    float shimmerPos = std::fmod(glowPulse * 50.0f, maxWidth - 50);
    topAccent.setSize(sf::Vector2f(maxWidth - 50, 2));
    topAccent.setPosition(boxX + 25, boxY + 12);
    topAccent.setFillColor(sf::Color(100, 210, 255, 180));
    window.draw(topAccent);

    // Shimmer highlight moving across top accent
    sf::RectangleShape shimmer(sf::Vector2f(60, 2));
    shimmer.setPosition(boxX + 25 + shimmerPos, boxY + 12);
    shimmer.setFillColor(sf::Color(255, 255, 255, 80));
    window.draw(shimmer);

    // Bottom accent
    bottomAccent.setSize(sf::Vector2f(maxWidth - 50, 1));
    bottomAccent.setPosition(boxX + 25, boxY + boxHeight - 12);
    bottomAccent.setFillColor(sf::Color(80, 160, 220, 120));
    window.draw(bottomAccent);

    // Speaker name with subtle glow
    window.draw(speakerText);

    // Content text
    float textY = boxY + 48 - scrollOffset;
    contentText.setPosition(boxX + UIConfig::BOX_PADDING, textY);
    window.draw(contentText);

    // Scroll indicator with gradient
    if (scrollEnabled && maxScroll > 0) {
      float trackHeight = boxHeight - 40;
      float indicatorHeight = std::max(20.0f, trackHeight * 0.3f);
      float indicatorY =
          boxY + 20 +
          (scrollOffset / maxScroll) * (trackHeight - indicatorHeight);

      // Track
      sf::RectangleShape track(sf::Vector2f(3, trackHeight));
      track.setPosition(boxX + maxWidth - 18, boxY + 20);
      track.setFillColor(sf::Color(60, 80, 120, 100));
      window.draw(track);

      // Indicator
      sf::RectangleShape scrollBar(sf::Vector2f(5, indicatorHeight));
      scrollBar.setPosition(boxX + maxWidth - 19, indicatorY);
      scrollBar.setFillColor(sf::Color(100, 200, 255, 200));
      window.draw(scrollBar);
    }

    // Corner decorations (small triangular accents)
    sf::ConvexShape cornerTL;
    cornerTL.setPointCount(3);
    cornerTL.setPoint(0, sf::Vector2f(boxX + 8, boxY + 4));
    cornerTL.setPoint(1, sf::Vector2f(boxX + 28, boxY + 4));
    cornerTL.setPoint(2, sf::Vector2f(boxX + 8, boxY + 24));
    cornerTL.setFillColor(sf::Color(100, 200, 255, 60));
    window.draw(cornerTL);

    sf::ConvexShape cornerBR;
    cornerBR.setPointCount(3);
    cornerBR.setPoint(0,
                      sf::Vector2f(boxX + maxWidth - 8, boxY + boxHeight - 4));
    cornerBR.setPoint(1,
                      sf::Vector2f(boxX + maxWidth - 28, boxY + boxHeight - 4));
    cornerBR.setPoint(2,
                      sf::Vector2f(boxX + maxWidth - 8, boxY + boxHeight - 24));
    cornerBR.setFillColor(sf::Color(100, 200, 255, 60));
    window.draw(cornerBR);
  }

  // Getters
  bool isComplete() const { return animationComplete; }
  float getHeight() const { return boxHeight; }
  float getY() const { return boxY; }
  float getX() const { return boxX; }
  float getWidth() const { return maxWidth; }
  bool canScroll() const { return scrollEnabled; }

  /**
   * Reposition (for responsive layout).
   */
  void setPosition(float x, float y) {
    boxX = x;
    boxY = y;
    background.setPosition(boxX, boxY);
    topAccent.setPosition(boxX + 25, boxY + 10);
    bottomAccent.setPosition(boxX + 25, boxY + boxHeight - 10);
    speakerText.setPosition(boxX + UIConfig::BOX_PADDING, boxY + 15);
  }
};

// =============================================================================
// CHOICE CARD
// =============================================================================

/**
 * ChoiceCard - Single selectable choice with visual states
 */
class ChoiceCard {
public:
  enum State { DEFAULT, HOVER, SELECTED };

private:
  sf::ConvexShape background;
  sf::Text text;
  State currentState = DEFAULT;
  float hoverProgress = 0; // 0-1 for smooth transitions

  float cardX = 0, cardY = 0;
  float cardWidth = 0, cardHeight = 0;

public:
  std::string choiceId;
  std::string choiceText;

  void setup(const sf::Font &font, const std::string &label, float x, float y,
             float width, unsigned int fontSize = 18) {
    choiceText = label;
    cardX = x;
    cardY = y;
    cardWidth = width;

    // Setup text
    text.setFont(font);
    text.setString(label);
    text.setCharacterSize(fontSize);
    text.setFillColor(UIConfig::TEXT_PRIMARY);

    // Calculate height based on text
    sf::FloatRect textBounds = text.getLocalBounds();
    cardHeight = std::max(UIConfig::CHOICE_MIN_HEIGHT,
                          textBounds.height + UIConfig::CHOICE_PADDING_V * 2);

    // Create rounded background
    background =
        createRoundedRect(cardWidth, cardHeight, UIConfig::CORNER_RADIUS);
    background.setPosition(cardX, cardY);
    updateVisuals();

    // Center text
    text.setPosition(cardX + UIConfig::CHOICE_PADDING_H,
                     cardY + (cardHeight - textBounds.height) / 2 - 3);
  }

  void setState(State newState) { currentState = newState; }

  void update(float deltaTime) {
    // Smooth hover transition
    float targetProgress =
        (currentState == HOVER || currentState == SELECTED) ? 1.0f : 0.0f;
    float speed = UIConfig::HOVER_TRANSITION_SPEED * deltaTime;
    if (hoverProgress < targetProgress) {
      hoverProgress = std::min(hoverProgress + speed, targetProgress);
    } else {
      hoverProgress = std::max(hoverProgress - speed, targetProgress);
    }

    updateVisuals();
  }

  void updateVisuals() {
    sf::Color bgColor, borderColor;
    float borderThickness;

    switch (currentState) {
    case SELECTED:
      bgColor = UIConfig::BG_SELECTED;
      borderColor = UIConfig::BORDER_SELECTED;
      borderThickness = 3;
      text.setFillColor(UIConfig::ACCENT_CYAN);
      break;
    case HOVER:
      bgColor = UIConfig::BG_HOVER;
      borderColor = UIConfig::BORDER_HOVER;
      borderThickness = 2.5f;
      text.setFillColor(UIConfig::TEXT_HIGHLIGHT);
      break;
    default:
      bgColor = UIConfig::BG_DARK;
      borderColor = UIConfig::BORDER_DEFAULT;
      borderThickness = 2;
      text.setFillColor(UIConfig::TEXT_PRIMARY);
    }

    background.setFillColor(bgColor);
    background.setOutlineColor(borderColor);
    background.setOutlineThickness(borderThickness);
  }

  void draw(sf::RenderWindow &window) {
    // UI POLISH START - Scale animation and enhanced glow
    float scale = 1.0f;
    if (currentState == SELECTED) {
      scale = 1.02f; // Slight scale up when selected
    } else if (currentState == HOVER) {
      scale = 1.01f + hoverProgress * 0.01f; // Subtle scale on hover
    }

    // Calculate scaled dimensions
    float scaledWidth = cardWidth * scale;
    float scaledHeight = cardHeight * scale;
    float offsetX = (scaledWidth - cardWidth) / 2.0f;
    float offsetY = (scaledHeight - cardHeight) / 2.0f;

    // Draw outer glow for hover/selected
    if (currentState == HOVER || currentState == SELECTED) {
      float glowSize = (currentState == SELECTED) ? 12.0f : 8.0f;
      sf::Uint8 glowAlpha = (currentState == SELECTED) ? 70 : 40;
      sf::ConvexShape outerGlow =
          createRoundedRect(scaledWidth + glowSize, scaledHeight + glowSize,
                            UIConfig::CORNER_RADIUS + 2);
      outerGlow.setPosition(cardX - offsetX - glowSize / 2,
                            cardY - offsetY - glowSize / 2);
      outerGlow.setFillColor(sf::Color(100, 200, 255, glowAlpha));
      window.draw(outerGlow);
    }

    // Draw inner glow for selected
    if (currentState == SELECTED) {
      sf::ConvexShape innerGlow = createRoundedRect(
          scaledWidth + 4, scaledHeight + 4, UIConfig::CORNER_RADIUS);
      innerGlow.setPosition(cardX - offsetX - 2, cardY - offsetY - 2);
      innerGlow.setFillColor(sf::Color(100, 220, 255, 90));
      window.draw(innerGlow);
    }

    // Draw scaled background
    sf::ConvexShape scaledBg =
        createRoundedRect(scaledWidth, scaledHeight, UIConfig::CORNER_RADIUS);
    scaledBg.setPosition(cardX - offsetX, cardY - offsetY);
    scaledBg.setFillColor(background.getFillColor());
    scaledBg.setOutlineColor(background.getOutlineColor());
    scaledBg.setOutlineThickness(background.getOutlineThickness());
    window.draw(scaledBg);

    // Draw text (slightly offset for scale)
    sf::Text scaledText = text;
    sf::FloatRect textBounds = text.getLocalBounds();
    scaledText.setPosition(cardX - offsetX + UIConfig::CHOICE_PADDING_H,
                           cardY - offsetY +
                               (scaledHeight - textBounds.height) / 2 - 3);
    window.draw(scaledText);
    // UI POLISH END
  }

  bool contains(sf::Vector2f point) const {
    return point.x >= cardX && point.x <= cardX + cardWidth &&
           point.y >= cardY && point.y <= cardY + cardHeight;
  }

  float getHeight() const { return cardHeight; }
  float getY() const { return cardY; }

  void setY(float y) {
    cardY = y;
    background.setPosition(cardX, cardY);
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setPosition(cardX + UIConfig::CHOICE_PADDING_H,
                     cardY + (cardHeight - textBounds.height) / 2 - 3);
  }
};

// =============================================================================
// CHOICE PANEL
// =============================================================================

/**
 * ChoicePanel - Container for stacked choice cards with navigation
 */
class ChoicePanel {
private:
  std::vector<ChoiceCard> cards;
  const sf::Font *fontPtr = nullptr;
  int selectedIndex = 0;
  int hoveredIndex = -1;
  bool isVisible = false;

  float panelX = 0, panelY = 0;
  float panelWidth = 600;
  float screenWidth = 1280;

public:
  /**
   * Setup with font and choices.
   */
  template <typename ChoiceType>
  void setup(const sf::Font &font, const std::vector<ChoiceType> &choices,
             float startY, float scrWidth) {
    fontPtr = &font;
    screenWidth = scrWidth;
    panelWidth = std::min(600.0f, scrWidth * 0.5f);
    panelX = (scrWidth - panelWidth) / 2.0f;
    panelY = startY;

    cards.clear();
    float currentY = panelY;

    for (size_t i = 0; i < choices.size(); i++) {
      ChoiceCard card;
      std::string label = std::to_string(i + 1) + ". " + choices[i].text;
      card.setup(font, label, panelX, currentY, panelWidth, 18);
      card.choiceId = choices[i].id;
      cards.push_back(card);
      currentY += card.getHeight() + UIConfig::CHOICE_SPACING;
    }

    selectedIndex = 0;
    hoveredIndex = -1;
    isVisible = true;
    updateStates();
  }

  void moveUp() {
    if (selectedIndex > 0) {
      selectedIndex--;
      updateStates();
    }
  }

  void moveDown() {
    if (selectedIndex < static_cast<int>(cards.size()) - 1) {
      selectedIndex++;
      updateStates();
    }
  }

  void handleMouseMove(sf::Vector2f pos) {
    hoveredIndex = -1;
    for (size_t i = 0; i < cards.size(); i++) {
      if (cards[i].contains(pos)) {
        hoveredIndex = static_cast<int>(i);
        break;
      }
    }
    updateStates();
  }

  bool handleMouseClick(sf::Vector2f pos) {
    for (size_t i = 0; i < cards.size(); i++) {
      if (cards[i].contains(pos)) {
        selectedIndex = static_cast<int>(i);
        updateStates();
        return true;
      }
    }
    return false;
  }

  void updateStates() {
    for (size_t i = 0; i < cards.size(); i++) {
      if (static_cast<int>(i) == selectedIndex) {
        cards[i].setState(ChoiceCard::SELECTED);
      } else if (static_cast<int>(i) == hoveredIndex) {
        cards[i].setState(ChoiceCard::HOVER);
      } else {
        cards[i].setState(ChoiceCard::DEFAULT);
      }
    }
  }

  void update(float deltaTime) {
    for (auto &card : cards) {
      card.update(deltaTime);
    }
  }

  void draw(sf::RenderWindow &window) {
    if (!isVisible)
      return;
    for (auto &card : cards) {
      card.draw(window);
    }
  }

  int getSelectedIndex() const { return selectedIndex; }
  size_t getChoiceCount() const { return cards.size(); }
  bool visible() const { return isVisible; }
  void hide() { isVisible = false; }
  void show() { isVisible = true; }

  void reset() {
    cards.clear();
    selectedIndex = 0;
    hoveredIndex = -1;
    isVisible = false;
  }

  /**
   * Get total panel height for layout calculations.
   */
  float getTotalHeight() const {
    if (cards.empty())
      return 0;
    float lastCardBottom = cards.back().getY() + cards.back().getHeight();
    return lastCardBottom - panelY;
  }
};
