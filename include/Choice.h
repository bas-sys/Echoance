#pragma once
#include <string>
#include <vector>

/**
 * Choice - Represents a player choice in the narrative
 *
 * Fields:
 * - id: Unique identifier for this choice
 * - text: Display text shown to player
 * - outcome: Narrative result of choosing this option
 * - next_memory_id: ID of the next memory/scene to load (for branching)
 * - sub_choices: Nested choices (for multi-step decisions)
 */
struct Choice {
  std::string id;
  std::string text;
  std::string outcome;
  std::string next_memory_id; // For branching: which memory to load next
  std::vector<Choice> sub_choices;
};
