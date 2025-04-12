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

/******************************************************************************
 * PRIVATE HELPER IMPLEMENTATIONS
 *****************************************************************************/

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
    auto current_chain = std::list<PipelineLogMessage>{};
    auto current_message = message;
    auto next_id_invalid = false;
    auto next_element_already_visited = false;
    auto next_element_id_terminator = false;
    while (!next_element_id_terminator && !next_element_already_visited &&
           !next_id_invalid) {

      std::cout << "\n\nCurrent message: " << current_message << std::endl;
      const auto& current_id = current_message.id();
      auto [first, end] = messages_by_id.equal_range(current_id);
      auto same_element_chain = std::list<PipelineLogMessage>{};
      auto all_are_terminators = true;
      auto all_are_not_found = true;
      for (auto it = first; it != end; ++it) {
        const auto& next_id = it->second.next_id();
        std::cout << "Next ID: " << next_id << std::endl;
        auto is_element_at_end = false;
        if (kTerminator == next_id) {
          is_element_at_end = true;
          all_are_not_found = false;
        } else if (messages_by_id.contains(next_id)) {
          all_are_terminators = false;
          all_are_not_found = false;
        } else {
          is_element_at_end = true;
          all_are_terminators = false;
        }

        if (is_element_at_end) {
          same_element_chain.push_back(it->second);
        } else {
          same_element_chain.push_front(it->second);
          if (messages_visited.at(next_id)) {
            auto [next_element_first, next_element_last] =
                messages_by_id.equal_range(next_id);
            for (auto it_next = next_element_first;
                 it_next != next_element_last; ++it_next) {
              alread_visited_next_elements_from_current_chain.insert(
                  it_next->second);
            }
          }
        }
      }
      next_element_already_visited =
          !alread_visited_next_elements_from_current_chain.empty();
      next_element_id_terminator = all_are_terminators;
      next_id_invalid = all_are_not_found;
      std::cout << "Same element chain: ";
      print_container(same_element_chain);
      current_message = same_element_chain.front();
      std::cout << "Current message: " << current_message << std::endl;
      current_chain.splice(std::end(current_chain), same_element_chain);
      std::cout << "Current chain: ";
      print_container(current_chain);
      // We want a message with a next_id if it exists
      messages_visited.at(current_id) = true;
      std::cout << "next_element_already_visited: "
                << next_element_already_visited << std::endl;
      std::cout << "next_id_invalid: " << next_id_invalid << std::endl;
      std::cout << "next_element_id_terminator: " << next_element_id_terminator
                << std::endl;
      if (!next_element_already_visited && !next_id_invalid &&
          !next_element_id_terminator) {
        if (auto it = messages_by_id.find(current_message.next_id());
            it != std::end(messages_by_id)) {
          current_message = it->second;
        }
      }
    }

    if (next_element_already_visited) {
      // We need to find it in the organized list and add our current chain before it
      if (auto it = std::ranges::find_if(
              organized_list,
              [&alread_visited_next_elements_from_current_chain](
                  const auto& message) {
                std::cout << "Searching for: " << message << std::endl;
                std::cout << "In: ";
                print_container(
                    alread_visited_next_elements_from_current_chain);
                return alread_visited_next_elements_from_current_chain.contains(
                    message);
              });
          it != std::end(organized_list)) {
        organized_list.splice(it, current_chain);
        std::cout << "Adding before: " << *it << std::endl;
        std::cout << "Organized list: ";
        print_container(organized_list);
      } else {
        std::cout << "Did not find the element in the organized list"
                  << std::endl;
      }
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