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

/******************************************************************************
 * CONSTANTS AND TYPEDEFS
 ******************************************************************************/
namespace pipelines::log_message_organizer::organize_by_id {

/// Constant for the terminator ID
constexpr auto kTerminator = std::string_view{"-1"};

/// Type alias for pipeline log messages grouped by ID
using MessagesById = std::multimap<std::string, PipelineLogMessage>;

/// Type alias for an id map which tells if a message was already processed
using MessagesVisited = std::map<std::string, bool>;

/// Type alias for a list used to build the organized messages
using PipelineLogMessagesChain = std::list<PipelineLogMessage>;

/// Type alias with a set of all the ids that should have been added to the current chain, but are already visited
using MessagesVisitedSet = std::set<PipelineLogMessage>;

using ElementsUnderSameId = std::pair<PipelineLogMessagesChain,
                                      std::map<std::string, class NextIdInfo>>;

}  // namespace pipelines::log_message_organizer::organize_by_id

/******************************************************************************
 * PRIVATE HELPER DECLARATIONS
 *****************************************************************************/

/******************************************************************************
 * PRIVATE CLASSES
 *****************************************************************************/

namespace pipelines::log_message_organizer::organize_by_id {

/**
 * @struct NextIdInfo
 * @brief Structure to hold information about the next ID in the log message chain.
 *
 * This structure contains flags to indicate whether the next ID is invalid,
 * a terminator, or the same as the current ID.
 */
struct NextIdInfo {
  /// Indicates if there is an element with this ID in the log messages
  bool invalid{false};
  /// Indicates if this ID is a terminator (e.g., "-1")
  bool terminator{false};
  /// Indicates if this ID is the same as the current ID
  bool same_id{false};

  /**
   * @brief Checks if the ID is valid, i.e., an ID that should be followed.
   * @return true if the ID is valid (not invalid, not a terminator, and not the same ID),
   */
  bool valid_id() const { return !invalid && !terminator && !same_id; }
};

class Organizer {
 public:
  Organizer() = delete;

  explicit Organizer(const PipelineLogMessages& log_messages)
      : log_messages_{log_messages},
        organized_list_{},
        messages_by_id_{},
        messages_visited_{} {
    for (const auto& message : log_messages) {
      messages_by_id_.insert({message.id(), message});
      messages_visited_.insert({message.id(), false});
    }
  }

  void CreateOrganizedList();

  PipelineLogMessages GetOrganizedList() const;

 private:
  const PipelineLogMessages& log_messages_;
  PipelineLogMessagesChain organized_list_;
  MessagesById messages_by_id_;
  MessagesVisited messages_visited_;

  void AddChainsFromThisMessageToList(const PipelineLogMessage& message);

  PipelineLogMessagesChain GetNextElements(
      const PipelineLogMessage& current_message,
      MessagesVisitedSet& already_visited_next_elements_from_current_chain);
  ElementsUnderSameId GetElementsUnderSameId(
      const std::string& current_id,
      MessagesVisitedSet& already_visited_next_elements_from_current_chain);
};

}  // namespace pipelines::log_message_organizer::organize_by_id

/******************************************************************************
 * PRIVATE HELPER IMPLEMENTATIONS
 *****************************************************************************/

/******************************************************************************
 * PRIVATE CLASS METHODS IMPLEMENTATION
 *****************************************************************************/

namespace pipelines::log_message_organizer::organize_by_id {

void Organizer::CreateOrganizedList() {
  for (const auto& message : log_messages_) {
    if (!messages_visited_.at(message.id())) {
      AddChainsFromThisMessageToList(message);
    }
  }
}

PipelineLogMessages Organizer::GetOrganizedList() const {
  auto organized_messages = PipelineLogMessages{};
  for (const auto& message : organized_list_ | std::views::reverse) {
    organized_messages.push_back(message);
  }
  return organized_messages;
}

void Organizer::AddChainsFromThisMessageToList(
    const PipelineLogMessage& message) {
  auto already_visited_next_elements_from_current_chain = MessagesVisitedSet{};
  auto current_chain = GetNextElements(
      message, already_visited_next_elements_from_current_chain);

  auto next_element_already_visited =
      !already_visited_next_elements_from_current_chain.empty();

  if (next_element_already_visited) {
    organized_list_.splice(std::begin(organized_list_), current_chain);
  } else {
    // We need to add the current chain to the organized list
    organized_list_.splice(std::end(organized_list_), current_chain);
  }
}

ElementsUnderSameId Organizer::GetElementsUnderSameId(
    const std::string& current_id,
    MessagesVisitedSet& already_visited_next_elements_from_current_chain) {
  auto [it_current_id_first, it_current_id_end] =
      messages_by_id_.equal_range(current_id);
  auto same_element_chain = PipelineLogMessagesChain{};
  auto next_ids = std::map<std::string, NextIdInfo>{};

  for (auto it = it_current_id_first; it != it_current_id_end; ++it) {
    const auto& next_id = it->second.next_id();
    auto next_id_info = NextIdInfo{};
    if (kTerminator == next_id) {
      next_id_info.terminator = true;
    } else if (next_id == current_id) {
      next_id_info.same_id = true;
    } else if (!messages_by_id_.contains(next_id)) {
      next_id_info.invalid = true;
    }

    if (!next_id_info.valid_id()) {
      same_element_chain.push_back(it->second);
    } else {
      same_element_chain.push_front(it->second);
      if (messages_visited_.at(next_id)) {
        auto [next_element_first, next_element_last] =
            messages_by_id_.equal_range(next_id);
        for (auto it_next = next_element_first; it_next != next_element_last;
             ++it_next) {
          already_visited_next_elements_from_current_chain.insert(
              it_next->second);
        }
      } else {
        next_ids.emplace(next_id, next_id_info);
      }
    }
  }

  return {same_element_chain, next_ids};
}

PipelineLogMessagesChain Organizer::GetNextElements(
    const PipelineLogMessage& current_message,
    MessagesVisitedSet& already_visited_next_elements_from_current_chain) {
  const auto& current_id = current_message.id();

  auto current_chain = PipelineLogMessagesChain{};

  auto [same_element_chain, next_ids] = GetElementsUnderSameId(
      current_id, already_visited_next_elements_from_current_chain);

  current_chain.splice(std::end(current_chain), same_element_chain);
  messages_visited_.at(current_id) = true;
  auto last_element_in_chain = std::prev(std::end(current_chain));
  for (const auto& [next_id, next_id_info] : next_ids) {
    if (messages_visited_.at(next_id)) {
      continue;
    }
    if (auto it = messages_by_id_.find(next_id);
        it != std::end(messages_by_id_)) {
      auto next_elements = GetNextElements(
          it->second, already_visited_next_elements_from_current_chain);
      current_chain.splice(std::next(last_element_in_chain), next_elements);
    }
  }

  return current_chain;
}

}  // namespace pipelines::log_message_organizer::organize_by_id

/******************************************************************************
 * PUBLIC CLASS METHODS IMPLEMENTATION
 *****************************************************************************/

namespace pipelines::log_message_organizer {

PipelineLogMessages OrganizeById::Organize() const {
  using namespace pipelines::log_message_organizer::organize_by_id;

  auto organizer = Organizer(log_messages_);
  organizer.CreateOrganizedList();

  return organizer.GetOrganizedList();
}

}  // namespace pipelines::log_message_organizer