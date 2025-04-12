/**
 * @file organize_by_id.cc
 * @brief Implementation of the OrganizeById class.
 * 
 * This file contains the implementation of the OrganizeById class, which is responsible
 * for organizing log messages based on their identifiers.
 */

/******************************************************************************
  * INCLUDES
  *****************************************************************************/
#include "log_message_organizer/organize_by_id.h"

#include <list>
#include <map>
#include <ranges>
#include <set>
#include <string>
#include <string_view>

#include <iostream>  // TODO: remove this include

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

constexpr auto kTerminator = std::string_view{"-1"};

/******************************************************************************
 * PRIVATE HELPER DECLARATIONS
 *****************************************************************************/

auto print_container(auto& container) -> void {
  for (const auto& item : container) {
    std::cout << item << ", ";
  }
  std::cout << std::endl;
}

/******************************************************************************
 * PRIVATE CLASSES
 *****************************************************************************/

namespace pipelines::log_message_organizer {

struct NextIdInfo {
  bool invalid = false;
  bool terminator = false;

  bool valid_id() const { return !invalid && !terminator; }
};

}  // namespace pipelines::log_message_organizer

/******************************************************************************
 * PRIVATE HELPER IMPLEMENTATIONS
 *****************************************************************************/

namespace pipelines::log_message_organizer {

std::list<PipelineLogMessage> GetNextElements(
    const PipelineLogMessage& current_message,
    const std::multimap<std::string, PipelineLogMessage>& messages_by_id,
    std::map<std::string, bool>& messages_visited,
    std::set<PipelineLogMessage>&
        alread_visited_next_elements_from_current_chain) {
  std::cout << "\n\n<< GetNextElements Current message: " << current_message
            << std::endl;

  const auto& current_id = current_message.id();
  auto [first, end] = messages_by_id.equal_range(current_id);
  auto same_element_chain = std::list<PipelineLogMessage>{};
  auto next_ids = std::map<std::string, NextIdInfo>{};
  auto current_chain = std::list<PipelineLogMessage>{};

  for (auto it = first; it != end; ++it) {
    const auto& next_id = it->second.next_id();
    auto next_id_info = NextIdInfo{};
    std::cout << "Next ID: " << next_id << std::endl;
    if (kTerminator == next_id) {
      next_id_info.terminator = true;
    } else if (!messages_by_id.contains(next_id)) {
      next_id_info.invalid = true;
    }

    if (!next_id_info.valid_id()) {
      same_element_chain.push_back(it->second);
    } else {
      same_element_chain.push_front(it->second);
      if (messages_visited.at(next_id)) {
        auto [next_element_first, next_element_last] =
            messages_by_id.equal_range(next_id);
        for (auto it_next = next_element_first; it_next != next_element_last;
             ++it_next) {
          alread_visited_next_elements_from_current_chain.insert(
              it_next->second);
        }
      } else {
        next_ids.emplace(next_id, next_id_info);
      }
    }
  }
  std::cout << "Same element chain: ";
  print_container(same_element_chain);
  current_chain.splice(std::end(current_chain), same_element_chain);
  std::cout << "Current chain: ";
  print_container(current_chain);
  // We want a message with a next_id if it exists
  messages_visited.at(current_id) = true;
  std::cout << "Next IDs: ";
  for (const auto& [next_id, next_id_info] : next_ids) {
    std::cout << next_id << " ";
  }
  std::cout << std::endl;
  for (const auto& [next_id, next_id_info] : next_ids) {
    auto last_element_in_chain = std::prev(std::end(current_chain));
    if (auto it = messages_by_id.find(next_id);
        it != std::end(messages_by_id)) {
      auto next_elements =
          GetNextElements(it->second, messages_by_id, messages_visited,
                          alread_visited_next_elements_from_current_chain);
      current_chain.splice(std::next(last_element_in_chain), next_elements);
    }
  }

  std::cout << ">>>>>>>> GetNextElements Current message: " << current_message
            << std::endl;
  return current_chain;
}

}  // namespace pipelines::log_message_organizer

/******************************************************************************
 * PRIVATE CLASS METHODS IMPLEMENTATION
 *****************************************************************************/

/******************************************************************************
 * PUBLIC CLASS METHODS IMPLEMENTATION
 *****************************************************************************/

namespace pipelines::log_message_organizer {

PipelineLogMessages OrganizeById::Organize() const {
  auto organized_messages = PipelineLogMessages{};
  auto messages_by_id = std::multimap<std::string, PipelineLogMessage>{};
  // auto messages_by_next_id = std::multimap<std::string, PipelineLogMessage>{};
  // auto messages_without_next_message = PipelineLogMessages{};
  auto organized_list = std::list<PipelineLogMessage>{};
  auto messages_visited = std::map<std::string, bool>{};

  for (const auto& message : log_messages_) {
    messages_by_id.insert({message.id(), message});
    messages_visited.insert({message.id(), false});
    // messages_by_next_id.insert({message.next_id(), message});
  }

  for (const auto& message : log_messages_) {
    const auto& id = message.id();
    if (messages_visited.at(id)) {
      continue;
    }
    auto alread_visited_next_elements_from_current_chain =
        std::set<PipelineLogMessage>{};
    auto current_chain =
        GetNextElements(message, messages_by_id, messages_visited,
                        alread_visited_next_elements_from_current_chain);

    auto next_element_already_visited =
        !alread_visited_next_elements_from_current_chain.empty();

    if (next_element_already_visited) {
      organized_list.splice(std::begin(organized_list), current_chain);
    } else {
      // We need to add the current chain to the organized list
      organized_list.splice(std::end(organized_list), current_chain);
      std::cout << "Adding to the end" << std::endl;
      std::cout << "Organized list: ";
      print_container(organized_list);
    }
  }

  for (const auto& message : organized_list | std::views::reverse) {
    organized_messages.push_back(message);
  }

  // Print the organized messages for debugging
  std::cout << "Organized messages: ";
  print_container(organized_messages);

  return organized_messages;
}

}  // namespace pipelines::log_message_organizer