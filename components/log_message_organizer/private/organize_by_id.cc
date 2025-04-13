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

/// Type alias with a set of all the ids that should have been added to the current chain, but are already visited
using MessagesVisitedSet = std::set<PipelineLogMessage>;

using ElementsUnderSameId = std::pair<class PipelineLogMessagesChain2,
                                      std::map<std::string, class NextIdInfo>>;

}  // namespace pipelines::log_message_organizer::organize_by_id

/******************************************************************************
 * PRIVATE HELPER DECLARATIONS
 *****************************************************************************/

namespace pipelines::log_message_organizer::organize_by_id {

template <typename T>
static inline void AddListAtEnd(T& list1, T&& list2) {
  list1.splice(std::end(list1), std::move(list2));
}

}  // namespace pipelines::log_message_organizer::organize_by_id

/******************************************************************************
 * PRIVATE CLASSES
 *****************************************************************************/

namespace pipelines::log_message_organizer::organize_by_id {

class PipelineLogMessagesChain2 {

 public:
  using Chain = std::list<PipelineLogMessage>;
  using ChainIterator = Chain::iterator;

  PipelineLogMessagesChain2() = default;

  void AddToChain(const PipelineLogMessage& message) {
    chain_.push_back(message);
  }

  void AddToTerminationChain(const PipelineLogMessage& message) {
    termination_chain_.push_back(message);
  }

  void AddToInvalidChain(const PipelineLogMessage& message) {
    invalid_chain_.push_back(message);
  }

  void Merge(PipelineLogMessagesChain2& other) {
    AddListAtEnd(chain_, std::move(other.chain_));
    AddListAtEnd(termination_chain_, std::move(other.termination_chain_));
    AddListAtEnd(invalid_chain_, std::move(other.invalid_chain_));
  }

  void MergeAfter(PipelineLogMessagesChain2& other, ChainIterator it) {
    chain_.splice(std::next(it), std::move(other.chain_));
    AddListAtEnd(termination_chain_, std::move(other.termination_chain_));
    AddListAtEnd(invalid_chain_, std::move(other.invalid_chain_));
  }

  void MergeAtBeginning(PipelineLogMessagesChain2& other) {
    chain_.splice(std::begin(chain_), std::move(other.chain_));
    AddListAtEnd(termination_chain_, std::move(other.termination_chain_));
    AddListAtEnd(invalid_chain_, std::move(other.invalid_chain_));
  }

  ChainIterator BeforeChainEnd() { return std::prev(std::end(chain_)); }

  Chain CompleteChain() {
    auto complete_chain = Chain{};
    AddListAtEnd(complete_chain, std::move(chain_));
    AddListAtEnd(complete_chain, std::move(invalid_chain_));
    AddListAtEnd(complete_chain, std::move(termination_chain_));
    return complete_chain;
  }

 private:
  Chain chain_;
  Chain termination_chain_;
  Chain invalid_chain_;
};

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

  PipelineLogMessages GetOrganizedList();

 private:
  const PipelineLogMessages& log_messages_;
  PipelineLogMessagesChain2 organized_list_;
  MessagesById messages_by_id_;
  MessagesVisited messages_visited_;

  void CreateOrganizedList();

  void MarkMessageAsVisited(const std::string& id) {
    messages_visited_.at(id) = true;
  }

  bool IsMessageVisited(const std::string& id) const {
    return messages_visited_.at(id);
  }

  bool IsMessageVisited(const PipelineLogMessage& message) const {
    return IsMessageVisited(message.id());
  }

  ElementsUnderSameId GetElementsUnderSameId(const std::string& current_id);

  void AddBranchesFromNextElements(
      const std::map<std::string, NextIdInfo>& next_ids,
      PipelineLogMessagesChain2& current_chain);

  PipelineLogMessagesChain2 GetNextElements(
      const PipelineLogMessage& current_message);
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
    if (!IsMessageVisited(message)) {
      auto current_chain = GetNextElements(message);
      organized_list_.MergeAtBeginning(current_chain);
    }
  }
}

PipelineLogMessages Organizer::GetOrganizedList() {
  CreateOrganizedList();

  auto organized_list = organized_list_.CompleteChain();
  auto organized_messages = PipelineLogMessages{};
  for (const auto& message : organized_list | std::views::reverse) {
    organized_messages.push_back(message);
  }
  return organized_messages;
}

ElementsUnderSameId Organizer::GetElementsUnderSameId(
    const std::string& current_id) {
  auto same_element_chain = PipelineLogMessagesChain2{};
  auto next_ids = std::map<std::string, NextIdInfo>{};

  auto [it_current_id_first, it_current_id_end] =
      messages_by_id_.equal_range(current_id);
  for (auto it = it_current_id_first; it != it_current_id_end; ++it) {
    const auto& next_id = it->second.next_id();
    auto next_id_info = NextIdInfo{};
    if (kTerminator == next_id) {
      next_id_info.terminator = true;
      same_element_chain.AddToTerminationChain(it->second);
    } else if (next_id == current_id) {
      next_id_info.same_id = true;
      same_element_chain.AddToChain(it->second);
    } else if (!messages_by_id_.contains(next_id)) {
      next_id_info.invalid = true;
      same_element_chain.AddToInvalidChain(it->second);
    }

    if (next_id_info.valid_id()) {
      same_element_chain.AddToChain(it->second);
      if (!IsMessageVisited(next_id)) {
        next_ids.emplace(next_id, next_id_info);
      }
    }
  }

  return {same_element_chain, next_ids};
}

void Organizer::AddBranchesFromNextElements(
    const std::map<std::string, NextIdInfo>& next_ids,
    PipelineLogMessagesChain2& current_chain) {

  auto last_element_in_chain = current_chain.BeforeChainEnd();
  for (const auto& [next_id, next_id_info] : next_ids) {
    if (!IsMessageVisited(next_id)) {
      if (auto it = messages_by_id_.find(next_id);
          it != std::end(messages_by_id_)) {
        auto next_elements = GetNextElements(it->second);

        current_chain.MergeAfter(next_elements, last_element_in_chain);
      }
    }
  }
}

PipelineLogMessagesChain2 Organizer::GetNextElements(
    const PipelineLogMessage& current_message) {
  auto current_chain = PipelineLogMessagesChain2{};
  const auto& current_id = current_message.id();

  auto [same_element_chain, next_ids] = GetElementsUnderSameId(current_id);

  current_chain.Merge(same_element_chain);
  MarkMessageAsVisited(current_id);

  AddBranchesFromNextElements(next_ids, current_chain);

  return current_chain;
}

}  // namespace pipelines::log_message_organizer::organize_by_id

/******************************************************************************
 * PUBLIC CLASS METHODS IMPLEMENTATION
 *****************************************************************************/

namespace pipelines::log_message_organizer {

PipelineLogMessages OrganizeById::Organize() const {
  using namespace pipelines::log_message_organizer::organize_by_id;

  return Organizer(log_messages_).GetOrganizedList();
}

}  // namespace pipelines::log_message_organizer