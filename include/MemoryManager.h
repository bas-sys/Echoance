#pragma once
#include "Memory.h"
#include <string>
#include <vector>


/**
 * MemoryManager - Loads and manages all story content from JSON
 *
 * Responsible for:
 * - Parsing story.json
 * - Storing prologue, memory jumps, and endings
 * - Providing access by ID for branching
 */
class MemoryManager {
public:
  std::string gameTitle;
  std::string subtitle;
  Prologue prologue;
  std::vector<Memory> memories;
  std::vector<Ending> endings;

  // Load entire story from JSON file
  void loadFromJSON(const std::string &filename);

  // Get memory by ID for branching
  Memory *getMemoryById(const std::string &id);

  // Get ending by ID
  Ending *getEndingById(const std::string &id);

  // Get the first memory (after prologue)
  Memory *getFirstMemory();
};
