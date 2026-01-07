#pragma once
// CLEANUP: Merged Character.h into Memory.h to reduce file count
#include "Choice.h"
#include <string>
#include <vector>

/**
 * Character - NPC in a memory scene
 */
struct Character {
  std::string name;
  std::vector<std::string> dialogues;
};

/**
 * Echo - The mysterious echo entity
 */
struct Echo {
  std::vector<std::string> dialogues;
};

/**
 * Memory - A memory jump scene in the narrative
 * Contains all data for one complete scene: narration, NPC, choices
 */
struct Memory {
  std::string id;
  std::string title;
  std::vector<std::string> narration;
  Character npc;
  Echo echo; // Optional echo entity
  std::vector<Choice> choices;
};

/**
 * Prologue - The opening sequence before memory jumps
 */
struct Prologue {
  std::vector<std::string> narration;
  std::vector<std::string> player_thoughts;
  std::vector<std::string> system_dialogue;
  std::vector<Choice> choices;
};

/**
 * Ending - A possible story conclusion
 */
struct Ending {
  std::string id;
  std::string title;
  std::vector<std::string> narration;
};
