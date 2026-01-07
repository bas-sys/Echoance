#include "MemoryManager.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>


using json = nlohmann::json;

void MemoryManager::loadFromJSON(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Cannot open JSON file: " << filename << "\n";
    return;
  }

  json story;
  file >> story;

  // Load game metadata
  if (story.contains("game_title"))
    gameTitle = story["game_title"];
  if (story.contains("subtitle"))
    subtitle = story["subtitle"];

  // =========================================================================
  // LOAD PROLOGUE
  // =========================================================================
  if (story.contains("prologue")) {
    auto &prologueData = story["prologue"];

    if (prologueData.contains("narration")) {
      for (auto &line : prologueData["narration"])
        prologue.narration.push_back(line);
    }

    if (prologueData.contains("player_thoughts")) {
      for (auto &line : prologueData["player_thoughts"])
        prologue.player_thoughts.push_back(line);
    }

    if (prologueData.contains("system_dialogue")) {
      for (auto &line : prologueData["system_dialogue"])
        prologue.system_dialogue.push_back(line);
    }

    if (prologueData.contains("player_choices")) {
      for (auto &ch : prologueData["player_choices"]) {
        Choice choice;
        choice.id = ch["id"];
        choice.text = ch["text"];
        if (ch.contains("outcome"))
          choice.outcome = ch["outcome"];
        if (ch.contains("next_memory_id"))
          choice.next_memory_id = ch["next_memory_id"];
        prologue.choices.push_back(choice);
      }
    }
  }

  // =========================================================================
  // LOAD MEMORY JUMPS
  // =========================================================================
  if (story.contains("memory_jumps")) {
    for (auto &jump : story["memory_jumps"]) {
      Memory memory;
      memory.id = jump["id"];
      memory.title = jump["title"];

      for (auto &line : jump["narration"])
        memory.narration.push_back(line);

      // Load NPC
      if (jump.contains("npc")) {
        memory.npc.name = jump["npc"]["name"];
        for (auto &d : jump["npc"]["dialogues"])
          memory.npc.dialogues.push_back(d);
      }

      // Load Echo (if present)
      if (jump.contains("echo")) {
        for (auto &d : jump["echo"]["dialogues"])
          memory.echo.dialogues.push_back(d);
      }

      // Load choices
      if (jump.contains("player_choices")) {
        for (auto &ch : jump["player_choices"]) {
          Choice choice;
          choice.id = ch["id"];
          choice.text = ch["text"];
          if (ch.contains("outcome"))
            choice.outcome = ch["outcome"];
          if (ch.contains("next_memory_id"))
            choice.next_memory_id = ch["next_memory_id"];

          // Load sub-choices
          if (ch.contains("sub_choices")) {
            for (auto &sub : ch["sub_choices"]) {
              Choice subChoice;
              subChoice.id = sub["id"];
              subChoice.text = sub["text"];
              subChoice.outcome = sub["outcome"];
              if (sub.contains("next_memory_id"))
                subChoice.next_memory_id = sub["next_memory_id"];
              choice.sub_choices.push_back(subChoice);
            }
          }
          memory.choices.push_back(choice);
        }
      }

      memories.push_back(memory);
    }
  }

  // =========================================================================
  // LOAD ENDINGS
  // =========================================================================
  if (story.contains("endings")) {
    for (auto &endData : story["endings"]) {
      Ending ending;
      ending.id = endData["id"];
      ending.title = endData["title"];
      for (auto &line : endData["narration"])
        ending.narration.push_back(line);
      endings.push_back(ending);
    }
  }

  std::cout << "Loaded story: " << gameTitle << " - " << subtitle << "\n";
  std::cout << "  Memories: " << memories.size() << "\n";
  std::cout << "  Endings: " << endings.size() << "\n";
}

Memory *MemoryManager::getMemoryById(const std::string &id) {
  for (auto &mem : memories) {
    if (mem.id == id)
      return &mem;
  }
  return nullptr;
}

Ending *MemoryManager::getEndingById(const std::string &id) {
  for (auto &ending : endings) {
    if (ending.id == id)
      return &ending;
  }
  return nullptr;
}

Memory *MemoryManager::getFirstMemory() {
  if (!memories.empty())
    return &memories[0];
  return nullptr;
}
