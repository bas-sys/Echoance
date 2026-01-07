/**
 * Echoance - The Emotional Stream
 * Main entry point for the narrative game engine
 *
 * All story content is loaded from story.json - no hardcoded text.
 */

#include "GameEngine.h"

int main() {
  GameEngine engine;

  // Initialize and run the game
  // Story file path is relative to executable location
  if (engine.init("src/story.json")) {
    engine.run();
  }

  return 0;
}
 