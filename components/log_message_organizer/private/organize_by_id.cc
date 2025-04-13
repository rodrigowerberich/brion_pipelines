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

/// Type alias for a pair of pipeline log messages that share the same ID and all the valid next IDs
using ElementsUnderSameId = std::pair<class PipelineLogMessagesChain,
                                      std::map<std::string, struct NextIdInfo>>;

}  // namespace pipelines::log_message_organizer::organize_by_id

/******************************************************************************
 * PRIVATE HELPER DECLARATIONS
 *****************************************************************************/

namespace pipelines::log_message_organizer::organize_by_id {

/**
 * @brief Moves the elements of list2 to the end of list1.
 * @param list1 The first list to which elements will be added.
 * @param list2 The second list whose elements will be moved to the first list.
 */
template <typename T>
static inline void AddListAtEnd(T& list1, T&& list2) {
  list1.splice(std::end(list1), std::move(list2));
}

}  // namespace pipelines::log_message_organizer::organize_by_id

/******************************************************************************
 * PRIVATE CLASSES
 *****************************************************************************/

namespace pipelines::log_message_organizer::organize_by_id {

/**
 * @class PipelineLogMessagesChain
 * @brief Class to manage a chain of pipeline log messages.
 *
 * By keeping a split between normal messages, invalid messages, and termination messages,
 * this class allows for better organization and management of log messages. 
 * It provides methods to add messages to the chain, merge chains, and retrieve the complete chain.
 */
class PipelineLogMessagesChain {

 public:
  /// Type alias for a list of pipeline log messages
  using Chain = std::list<PipelineLogMessage>;
  /// Type alias for an iterator over the list of pipeline log messages
  using ChainIterator = Chain::iterator;

  /// Default constructor
  PipelineLogMessagesChain() = default;
  /**
   * @brief Adds a message to the chain of regular messages.
   * @param message The pipeline log message to add.
   * This method adds the message to the normal chain of messages.
   */
  void AddToChain(const PipelineLogMessage& message) {
    chain_.push_back(message);
  }
  /**
   * @brief Adds a message to the chain of termination messages.
   * @param message The pipeline log message to add.
   * This method adds the message to the termination chain of messages.
   */
  void AddToTerminationChain(const PipelineLogMessage& message) {
    termination_chain_.push_back(message);
  }
  /**
   * @brief Adds a message to the chain of invalid messages.
   * @param message The pipeline log message to add.
   * This method adds the message to the invalid chain of messages.
   */
  void AddToInvalidChain(const PipelineLogMessage& message) {
    invalid_chain_.push_back(message);
  }
  /**
   * @brief Merges another PipelineLogMessagesChain into this one. 
   * @param other The other PipelineLogMessagesChain to merge.
   * This method combines the messages from the other chain into this one. 
   * It adds all messages tot the end of their respective chains.
   */
  void Merge(PipelineLogMessagesChain& other);
  /**
   * @brief Same as Merge, but the regular messages are added after the given iterator.
   * @param other The other PipelineLogMessagesChain to merge.
   * @param it The iterator to the position in the chain where the merge should occur.
   * This method combines the messages from the other chain into this one.
   * It adds the regular messages after the given iterator.
   * The termination and invalid messages are added to the end of their respective chains.
   * @note The iterator must be valid and point to a position in the chain.
   */
  void MergeAfter(PipelineLogMessagesChain& other, ChainIterator it);
  /**
   * @brief Same as Merge, but the regular messages are added at the beginning of the chain.
   * @param other The other PipelineLogMessagesChain to merge.
   * This method combines the messages from the other chain into this one.
   * It adds the regular messages at the beginning of the chain.
   * The termination and invalid messages are added to the end of their respective chains.
   */
  void MergeAtBeginning(PipelineLogMessagesChain& other);
  /**
   * @brief Returns an iterator to last element of the chain.
   * @return An iterator to the last element of the chain.
   */
  ChainIterator BeforeChainEnd() { return std::prev(std::end(chain_)); }
  /** 
   * @brief Returns the complete chain of messages, including regular, invalid, and termination messages.
   * @return The complete chain of messages.
   * This method combines all the messages from the three chains into one.
   * The order of the messages is as follows:
   * - Regular messages
   * - Invalid messages
   * - Termination messages
   */
  Chain CompleteChain();

 private:
  /// The chain of regular messages
  Chain chain_;
  /// The chain of termination messages (Messages with next_id == "-1")
  Chain termination_chain_;
  /// The chain of invalid messages (Messages with next_id that do not exist)
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

/**
 * @class Organizer
 * @brief Class to organize log messages by their IDs.
 *
 * This class takes a list of log messages and organizes them into a chain based on their IDs.
 * It handles the merging of chains, marking messages as visited, and retrieving the organized list.
 */
class Organizer {
 public:
  /// @brief Default constructor is deleted
  Organizer() = delete;
  /**
   * @brief Constructor that initializes the Organizer with a list of log messages.
   * @param log_messages The list of log messages to organize.
   */
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
  /**
   * @brief Returns the organized list of log messages.
   * @return The organized list of log messages.
   * @note This method is meant to be called once, after that the Organizer should be destroyed.
   */
  PipelineLogMessages GetOrganizedList();

 private:
  /// The list of log messages to organize
  const PipelineLogMessages& log_messages_;
  /// The organized list of log messages
  PipelineLogMessagesChain organized_list_;
  /// The map of log messages grouped by ID
  MessagesById messages_by_id_;
  /// The map of messages visited (true if the message was already processed)
  MessagesVisited messages_visited_;

  /**
   * @brief Creates the organized list of log messages.
   * @note This method is called internally to create the organized list.
   * It iterates through the log messages and organizes them into chains.
   * It marks messages as visited to avoid processing them multiple times.
   */
  void CreateOrganizedList();
  /**
   * @brief Marks a message as visited.
   * @param id The ID of the message to mark as visited.
   * This method updates the messages_visited_ map to indicate that the message with the given ID has been processed.
   */
  void MarkMessageAsVisited(const std::string& id) {
    messages_visited_.at(id) = true;
  }
  /**
   * @brief Checks if a message has been visited.
   * @param id The ID of the message to check.
   * @return true if the message has been visited, false otherwise.
   */
  bool IsMessageVisited(const std::string& id) const {
    return messages_visited_.at(id);
  }
  /**
   * @brief Checks if a message has been visited.
   * @param message The message to check.
   * @return true if the message has been visited, false otherwise.
   */
  bool IsMessageVisited(const PipelineLogMessage& message) const {
    return IsMessageVisited(message.id());
  }
  /**
   * @brief Retrieves the elements under the same ID as the current message.
   * @param current_id The ID of the current message.
   * @return A pair containing a chain of messages with the same ID and a map of next IDs.
   */
  ElementsUnderSameId GetElementsUnderSameId(const std::string& current_id);
  /**
   * @brief Adds branches from the next elements to the current chain.
   * @param next_ids The map of next IDs to process.
   * @param current_chain The current chain of messages.
   * This method iterates through the next IDs and adds their corresponding chains to the current chain.
   */
  void AddBranchesFromNextElements(
      const std::map<std::string, NextIdInfo>& next_ids,
      PipelineLogMessagesChain& current_chain);

  /**
   * @brief Retrieves the next elements in the log message chain.
   * @param current_message The current message to process.
   * @return A chain of messages that follow the current message in the log message chain.
   */
  PipelineLogMessagesChain GetNextElements(
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

void PipelineLogMessagesChain::Merge(PipelineLogMessagesChain& other) {
  AddListAtEnd(chain_, std::move(other.chain_));
  AddListAtEnd(termination_chain_, std::move(other.termination_chain_));
  AddListAtEnd(invalid_chain_, std::move(other.invalid_chain_));
}
void PipelineLogMessagesChain::MergeAfter(PipelineLogMessagesChain& other,
                                          ChainIterator it) {
  chain_.splice(std::next(it), std::move(other.chain_));
  AddListAtEnd(termination_chain_, std::move(other.termination_chain_));
  AddListAtEnd(invalid_chain_, std::move(other.invalid_chain_));
}

void PipelineLogMessagesChain::MergeAtBeginning(
    PipelineLogMessagesChain& other) {
  chain_.splice(std::begin(chain_), std::move(other.chain_));
  AddListAtEnd(termination_chain_, std::move(other.termination_chain_));
  AddListAtEnd(invalid_chain_, std::move(other.invalid_chain_));
}

PipelineLogMessagesChain::Chain PipelineLogMessagesChain::CompleteChain() {
  auto complete_chain = Chain{};
  AddListAtEnd(complete_chain, std::move(chain_));
  AddListAtEnd(complete_chain, std::move(invalid_chain_));
  AddListAtEnd(complete_chain, std::move(termination_chain_));
  return complete_chain;
}

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
  // We need to reverse the order of the messages in the list
  return {organized_list.rbegin(), organized_list.rend()};
}

ElementsUnderSameId Organizer::GetElementsUnderSameId(
    const std::string& current_id) {
  auto same_element_chain = PipelineLogMessagesChain{};
  auto next_ids = std::map<std::string, NextIdInfo>{};

  // Get all the messages with the same ID
  auto [it_current_id_first, it_current_id_end] =
      messages_by_id_.equal_range(current_id);

  // Iterate through the messages with the same ID
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
    PipelineLogMessagesChain& current_chain) {

  // All branches are added after the current last element in the chain
  // This guarantees that the order of the messages is preserved because
  // any possible sub-chain will be added before the previous sub-chain
  // and if the sub-chain would have conflicting messages, they will already
  // be marked as visited
  auto last_element_in_chain = current_chain.BeforeChainEnd();
  for (const auto& [next_id, next_id_info] : next_ids) {
    // The next ID can be visited during the last iteration of the loop
    // so we need to check again if the message was already visited
    if (!IsMessageVisited(next_id)) {
      if (auto it = messages_by_id_.find(next_id);
          it != std::end(messages_by_id_)) {
        auto next_elements = GetNextElements(it->second);

        current_chain.MergeAfter(next_elements, last_element_in_chain);
      }
    }
  }
}

PipelineLogMessagesChain Organizer::GetNextElements(
    const PipelineLogMessage& current_message) {
  auto current_chain = PipelineLogMessagesChain{};
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