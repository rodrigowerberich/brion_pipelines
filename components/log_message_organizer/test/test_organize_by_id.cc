#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include "log_message_organizer/organize_by_id.h"

using ::testing::Contains;
using ::testing::Eq;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAreArray;

static bool MessageLessThan(
    const pipelines::log_message_organizer::PipelineLogMessage& lhs,
    const pipelines::log_message_organizer::PipelineLogMessage& rhs) {
  return lhs.id() < rhs.id();
}

static constexpr pipelines::log_message_organizer::PipelineLogMessage
CreateMessageIndexNextIndex(const std::string& id, const std::string& next_id) {
  return pipelines::log_message_organizer::PipelineLogMessage{id, "body",
                                                              next_id};
}

static constexpr pipelines::log_message_organizer::PipelineLogMessage
CreateFinalMessage(const std::string& id) {
  return pipelines::log_message_organizer::PipelineLogMessage{id, "body", "-1"};
}

MATCHER_P2(ElementsAreInOrder, first, second,
           "The elements are not in the expected order") {
  auto result = false;
  auto it_first = std::ranges::find(arg, first);
  auto it_second = std::ranges::find(arg, second);
  if (it_first != std::end(arg) && it_second != std::end(arg)) {
    result = std::distance(it_first, it_second) > 0;
  }
  return result;
}

MATCHER_P2(ElementIsAfterAnyIn, first, second,
           "The elements are not in the expected order") {

  auto it_first = std::ranges::find(arg, first);
  auto result = it_first != std::end(arg);
  auto orders = std::vector<int>{};
  for (const auto& element : second) {
    auto it_second = std::ranges::find(arg, element);
    result &= it_second != std::end(arg);
    orders.push_back(std::distance(it_first, it_second));
  }
  result &= std::all_of(orders.begin(), orders.end(),
                        [](int order) { return order > 0; });
  return result;
}

MATCHER_P2(ElementsAreAfterAnyIn, first, second,
           "Any elemenent in " + ::testing::PrintToString(first) +
               " should be after any element in " +
               ::testing::PrintToString(second)) {
  auto first_iterators = std::vector<std::ranges::iterator_t<decltype(arg)>>{};
  auto result = true;
  for (const auto& element : first) {
    auto it_first = std::ranges::find(arg, element);
    result &= it_first != std::end(arg);
    first_iterators.push_back(it_first);
  }
  if (result) {
    auto orders = std::vector<int>{};
    for (const auto& element : second) {
      auto it_second = std::ranges::find(arg, element);
      result &= it_second != std::end(arg);
      if (!result) {
        break;
      }
      for (const auto& it_first : first_iterators) {
        orders.push_back(std::distance(it_first, it_second));
      }
    }
    result &= std::all_of(orders.begin(), orders.end(),
                          [](int order) { return order > 0; });
  }
  return result;
}

class OrganizeByIdTest : public ::testing::Test {
  // Setup and teardown methods can be added here if needed
};

TEST_F(OrganizeByIdTest, EmptyInput) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{};
  auto expected_output = PipelineLogMessages{};

  auto organizer = OrganizeById{input};

  ASSERT_THAT(organizer.Organize(), Eq(expected_output));
}

/*******************************************************************
 * Test cases for Single node
 ******************************************************************/

TEST_F(OrganizeByIdTest, SingleMessagePoitingToTermination) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{CreateFinalMessage("sdna=123")};
  auto expected_output = PipelineLogMessages{CreateFinalMessage("sdna=123")};

  auto organizer = OrganizeById{input};

  ASSERT_THAT(organizer.Organize(), Eq(expected_output));
}

TEST_F(OrganizeByIdTest, SingleMessagePointingToNonExistingIndex) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input =
      PipelineLogMessages{CreateMessageIndexNextIndex("sdna=123", "23")};
  auto expected_output =
      PipelineLogMessages{CreateMessageIndexNextIndex("sdna=123", "23")};

  auto organizer = OrganizeById{input};

  ASSERT_THAT(organizer.Organize(), Eq(expected_output));
}

TEST_F(OrganizeByIdTest, SingleMessagePointingToSelf) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input =
      PipelineLogMessages{CreateMessageIndexNextIndex("sdna=123", "sdna=123")};
  auto expected_output =
      PipelineLogMessages{CreateMessageIndexNextIndex("sdna=123", "sdna=123")};

  auto organizer = OrganizeById{input};

  ASSERT_THAT(organizer.Organize(), Eq(expected_output));
}

TEST_F(OrganizeByIdTest, SingleMessageWithTerminationId) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{CreateFinalMessage("-1")};
  auto expected_output = PipelineLogMessages{CreateFinalMessage("-1")};

  auto organizer = OrganizeById{input};

  ASSERT_THAT(organizer.Organize(), Eq(expected_output));
}

/*******************************************************************
 * Test cases for Two nodes
 ******************************************************************/

TEST_F(OrganizeByIdTest, TwoMessagesPointingToTermination) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateFinalMessage("95"),
      CreateFinalMessage("-1"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(2));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));
}

TEST_F(OrganizeByIdTest, TwoMessagesOnePointingToSelfOnToInvalidIndex) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;
  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("sdna=123", "sdna=123"),
      CreateMessageIndexNextIndex("l", "4"),  // Pointing to invalid index
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(2));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));
}

TEST_F(OrganizeByIdTest, TwoMessagesBothPointingToSelf) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("-65-2", "-65-2"),
      CreateMessageIndexNextIndex("m", "m"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(2));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));
}

TEST_F(OrganizeByIdTest, TwoMessages) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("z", "b"),
      CreateFinalMessage("b"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(2));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));
  ASSERT_THAT(result,
              ElementsAreAfterAnyIn(
                  PipelineLogMessages{CreateFinalMessage("b")},
                  PipelineLogMessages{CreateMessageIndexNextIndex("z", "b")}));
}

TEST_F(OrganizeByIdTest, TwoMessagesSeriesOnePointingToSelf) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("sdna=123", "sdna=123"),
      CreateMessageIndexNextIndex("l", "sdna=123"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(2));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));

  ASSERT_THAT(
      result,
      ElementsAreAfterAnyIn(
          PipelineLogMessages{
              CreateMessageIndexNextIndex("sdna=123", "sdna=123")},
          PipelineLogMessages{CreateMessageIndexNextIndex("l", "sdna=123")}));
}

TEST_F(OrganizeByIdTest, TwoMessagesCyclic) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("1", "2"),
      CreateMessageIndexNextIndex("2", "1"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(2));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));
}

TEST_F(OrganizeByIdTest, TwoMessagesSameId) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("1", "2"),
      CreateMessageIndexNextIndex("1", "3"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(2));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));
}

TEST_F(OrganizeByIdTest, TwoMessagesSameIdOneCyclic) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateFinalMessage("1"),
      CreateMessageIndexNextIndex("1", "1"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(2));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));
}

TEST_F(OrganizeByIdTest, TwoMessagesSameIdBothCyclic) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("1", "1"),
      CreateMessageIndexNextIndex("1", "1"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(2));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));
}

/*******************************************************************
 * Test cases for three nodes
 ******************************************************************/

TEST_F(OrganizeByIdTest, ThreeMessages) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("1", "2"),
      CreateMessageIndexNextIndex("2", "3"),
      CreateFinalMessage("3"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(3));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{CreateFinalMessage("3")},
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("1", "2"),
                              CreateMessageIndexNextIndex("2", "3"),
                          }));
  ASSERT_THAT(result,
              ElementsAreAfterAnyIn(
                  PipelineLogMessages{CreateMessageIndexNextIndex("2", "3")},
                  PipelineLogMessages{
                      CreateMessageIndexNextIndex("1", "2"),
                  }));
}

TEST_F(OrganizeByIdTest, ThreeMessagesCyclic) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("1", "2"),
      CreateMessageIndexNextIndex("2", "3"),
      CreateMessageIndexNextIndex("3", "1"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(3));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));
}

TEST_F(OrganizeByIdTest, ThreeMessagesCyclicTwoSameId) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("a", "j"),
      CreateMessageIndexNextIndex("a", "j"),
      CreateMessageIndexNextIndex("j", "a"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(3));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));
}

/*******************************************************************
 * Test cases for Multiple nodes
 ******************************************************************/

TEST_F(OrganizeByIdTest, MultipleMessages1) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      {"1", "Hello, World!", "2"},
      {"2", "Goodbye, World!", "-1"},
  };
  auto expected_output = PipelineLogMessages{
      {"2", "Goodbye, World!", "-1"},
      {"1", "Hello, World!", "2"},
  };

  auto organizer = OrganizeById{input};

  ASSERT_THAT(organizer.Organize(), Eq(expected_output));
}

TEST_F(OrganizeByIdTest, MultipleMessages2) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      {"3", "See you later, World!", "-1"},
      {"1", "Hello, World!", "2"},
      {"2", "Goodbye, World!", "3"},
  };
  auto expected_output = PipelineLogMessages{
      {"3", "See you later, World!", "-1"},
      {"2", "Goodbye, World!", "3"},
      {"1", "Hello, World!", "2"},
  };

  auto organizer = OrganizeById{input};

  ASSERT_THAT(organizer.Organize(), Eq(expected_output));
}

TEST_F(OrganizeByIdTest, MultipleMessages3) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      {"999:123-b", "See you later, World!", "-1"},
      {"23801923:0-65", "Hello, World!", "3595:6-65"},
      {"3595:6-65", "Goodbye, World!", "999:123-b"},
  };

  auto expected_output = PipelineLogMessages{
      {"999:123-b", "See you later, World!", "-1"},
      {"3595:6-65", "Goodbye, World!", "999:123-b"},
      {"23801923:0-65", "Hello, World!", "3595:6-65"},
  };

  auto organizer = OrganizeById{input};

  ASSERT_THAT(organizer.Organize(), Eq(expected_output));
}

TEST_F(OrganizeByIdTest, MissingTerminator) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      {"1", "Hello, World!", "2"},
      {"3", "See you later, World!", "4"},
      {"2", "Goodbye, World!", "3"},
  };
  auto expected_output = PipelineLogMessages{
      {"3", "See you later, World!", "4"},
      {"2", "Goodbye, World!", "3"},
      {"1", "Hello, World!", "2"},
  };

  auto organizer = OrganizeById{input};

  ASSERT_THAT(organizer.Organize(), Eq(expected_output));
}

TEST_F(OrganizeByIdTest, MultipleTerminators1) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      {"1", "A", "2"},
      {"3", "B", "-1"},
      {"2", "C", "3"},
      {"4", "D", "-1"},
  };
  auto expected_output = PipelineLogMessages{
      {"4", "D", "-1"},
      {"3", "B", "-1"},
      {"2", "C", "3"},
      {"1", "A", "2"},
  };

  auto organizer = OrganizeById{input};

  ASSERT_THAT(organizer.Organize(), Eq(expected_output));
}

TEST_F(OrganizeByIdTest, MultipleTerminators2) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      {"1", "A", "2"},  {"3", "B", "-1"}, {"2", "C", "3"},
      {"4", "D", "-1"}, {"5", "E", "6"},  {"6", "F", "4"},
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();
  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              {"4", "D", "-1"},
                          },
                          PipelineLogMessages{
                              {"6", "F", "4"},
                              {"5", "E", "6"},
                          }));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              {"6", "F", "4"},
                          },
                          PipelineLogMessages{
                              {"5", "E", "6"},
                          }));
  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              {"3", "B", "-1"},
                          },
                          PipelineLogMessages{
                              {"2", "C", "3"},
                              {"1", "A", "2"},
                          }));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              {"2", "C", "3"},
                          },
                          PipelineLogMessages{
                              {"1", "A", "2"},
                          }));
}

TEST_F(OrganizeByIdTest, MultipleTerminators3) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      {"1", "A", "2"}, {"4", "D", "-1"}, {"3", "B", "-1"},
      {"2", "C", "3"}, {"5", "E", "6"},  {"6", "F", "4"},
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              {"4", "D", "-1"},
                          },
                          PipelineLogMessages{
                              {"6", "F", "4"},
                              {"5", "E", "6"},
                          }));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              {"6", "F", "4"},
                          },
                          PipelineLogMessages{
                              {"5", "E", "6"},
                          }));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              {"3", "B", "-1"},
                          },
                          PipelineLogMessages{
                              {"2", "C", "3"},
                              {"1", "A", "2"},
                          }));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              {"2", "C", "3"},
                          },
                          PipelineLogMessages{
                              {"1", "A", "2"},
                          }));
}

TEST_F(OrganizeByIdTest, MultiplePointingToSameMessage1) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("j", "a"),
      CreateMessageIndexNextIndex("a", "c"),
      CreateFinalMessage("c"),
      CreateMessageIndexNextIndex("k", "a"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateFinalMessage("c"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("a", "c"),
                              CreateMessageIndexNextIndex("k", "a"),
                              CreateMessageIndexNextIndex("j", "a"),
                          }));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("a", "c"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("k", "a"),
                              CreateMessageIndexNextIndex("j", "a"),
                          }));
}

TEST_F(OrganizeByIdTest, MultiplePointingToSameMessage2) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateFinalMessage("c"),
      CreateMessageIndexNextIndex("a", "c"),
      CreateMessageIndexNextIndex("k", "a"),
      CreateMessageIndexNextIndex("j", "a"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();
  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateFinalMessage("c"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("a", "c"),
                              CreateMessageIndexNextIndex("k", "a"),
                              CreateMessageIndexNextIndex("j", "a"),
                          }));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("a", "c"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("k", "a"),
                              CreateMessageIndexNextIndex("j", "a"),
                          }));
}

TEST_F(OrganizeByIdTest, MultipleElementsWithSameId1) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("z", "a"),
      CreateMessageIndexNextIndex("a", "h"),
      CreateMessageIndexNextIndex("a", "t"),
      CreateMessageIndexNextIndex("a", "b"),
      CreateFinalMessage("h"),
      CreateFinalMessage("t"),
      CreateFinalMessage("b"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateFinalMessage("t"),
                              CreateFinalMessage("h"),
                              CreateFinalMessage("b"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("a", "h"),
                              CreateMessageIndexNextIndex("a", "t"),
                              CreateMessageIndexNextIndex("a", "b"),
                              CreateMessageIndexNextIndex("z", "a"),
                          }));
  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("a", "h"),
                              CreateMessageIndexNextIndex("a", "t"),
                              CreateMessageIndexNextIndex("a", "b"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("z", "a"),
                          }));
}

TEST_F(OrganizeByIdTest, MultipleElementsWithSameId2) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateFinalMessage("c"),
      CreateFinalMessage("z"),
      CreateMessageIndexNextIndex("z", "c"),
  };

  std::ranges::sort(input, MessageLessThan);

  do {
    auto organizer = OrganizeById{input};
    auto result = organizer.Organize();

    ASSERT_THAT(result, ElementsAreAfterAnyIn(
                            PipelineLogMessages{
                                CreateFinalMessage("c"),
                            },
                            PipelineLogMessages{
                                CreateFinalMessage("z"),
                                CreateMessageIndexNextIndex("z", "c"),
                            }));
  } while (std::ranges::next_permutation(input, MessageLessThan).found);
}

TEST_F(OrganizeByIdTest, MultipleElementsWithSameId3) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("7", "3"),
      CreateMessageIndexNextIndex("3", "z"),
      CreateMessageIndexNextIndex("3", "kl"),
      CreateMessageIndexNextIndex("3", "m"),
      CreateMessageIndexNextIndex("z", "2"),
      CreateMessageIndexNextIndex("m", "j"),
      CreateFinalMessage("2"),
      CreateFinalMessage("kl"),
  };

  std::ranges::sort(input, MessageLessThan);

  do {
    auto organizer = OrganizeById{input};
    auto result = organizer.Organize();

    ASSERT_THAT(result, ElementsAreAfterAnyIn(
                            PipelineLogMessages{
                                CreateFinalMessage("2"),
                            },
                            PipelineLogMessages{
                                CreateMessageIndexNextIndex("3", "kl"),
                                CreateMessageIndexNextIndex("3", "m"),
                                CreateMessageIndexNextIndex("3", "z"),
                                CreateMessageIndexNextIndex("z", "2"),
                            }));
    ASSERT_THAT(result, ElementsAreAfterAnyIn(
                            PipelineLogMessages{
                                CreateFinalMessage("kl"),
                            },
                            PipelineLogMessages{
                                CreateMessageIndexNextIndex("3", "kl"),
                                CreateMessageIndexNextIndex("3", "m"),
                                CreateMessageIndexNextIndex("3", "z"),
                            }));

    ASSERT_THAT(result, ElementsAreAfterAnyIn(
                            PipelineLogMessages{
                                CreateMessageIndexNextIndex("m", "j"),
                            },
                            PipelineLogMessages{
                                CreateMessageIndexNextIndex("3", "kl"),
                                CreateMessageIndexNextIndex("3", "m"),
                                CreateMessageIndexNextIndex("3", "z"),
                            }));
    ASSERT_THAT(result, ElementsAreAfterAnyIn(
                            PipelineLogMessages{
                                CreateMessageIndexNextIndex("3", "kl"),
                                CreateMessageIndexNextIndex("3", "m"),
                                CreateMessageIndexNextIndex("3", "z"),
                            },
                            PipelineLogMessages{
                                CreateMessageIndexNextIndex("7", "3"),
                            }));

  } while (std::ranges::next_permutation(input, MessageLessThan).found);
}

TEST_F(OrganizeByIdTest, SelfPointing) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("4", "1"),
      CreateMessageIndexNextIndex("1", "1"),
      CreateMessageIndexNextIndex("2", "2"),
      CreateFinalMessage("3"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("1", "1"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("4", "1"),
                          }));

  ASSERT_THAT(result, SizeIs(input.size()));
  ASSERT_THAT(result, Contains(CreateMessageIndexNextIndex("1", "1")).Times(1));
  ASSERT_THAT(result, Contains(CreateMessageIndexNextIndex("2", "2")).Times(1));
}

TEST_F(OrganizeByIdTest, MultipleSameIdPointingToSame) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("k", "2"),
      CreateMessageIndexNextIndex("k", "2"),
      CreateMessageIndexNextIndex("k", "3"),
      CreateFinalMessage("2"),
      CreateFinalMessage("2"),
      CreateMessageIndexNextIndex("3", "3"),
      CreateMessageIndexNextIndex("3", "j"),
      CreateMessageIndexNextIndex("j", "l"),
      CreateMessageIndexNextIndex("j", "p"),

  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(input.size()));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateFinalMessage("2"),
                              CreateFinalMessage("2"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("k", "2"),
                              CreateMessageIndexNextIndex("k", "2"),
                              CreateMessageIndexNextIndex("k", "3"),
                          }));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("3", "3"),
                              CreateMessageIndexNextIndex("3", "j"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("k", "2"),
                              CreateMessageIndexNextIndex("k", "2"),
                              CreateMessageIndexNextIndex("k", "3"),
                          }));
  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("j", "l"),
                              CreateMessageIndexNextIndex("j", "p"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("3", "3"),
                              CreateMessageIndexNextIndex("3", "j"),
                          }));
}

TEST_F(OrganizeByIdTest, MultipleBranchesOneTermination) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("z", "1"),
      CreateMessageIndexNextIndex("1", "j"),
      CreateMessageIndexNextIndex("1", "casa"),
      CreateMessageIndexNextIndex("1", "-"),
      CreateMessageIndexNextIndex("j", "3l"),
      CreateMessageIndexNextIndex("casa", "3l"),
      CreateMessageIndexNextIndex("-", "3l"),
      CreateFinalMessage("3l"),

  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(input.size()));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateFinalMessage("3l"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("j", "3l"),
                              CreateMessageIndexNextIndex("casa", "3l"),
                              CreateMessageIndexNextIndex("-", "3l"),
                          }));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("j", "3l"),
                              CreateMessageIndexNextIndex("casa", "3l"),
                              CreateMessageIndexNextIndex("-", "3l"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("1", "j"),
                              CreateMessageIndexNextIndex("1", "casa"),
                              CreateMessageIndexNextIndex("1", "-"),
                          }));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("1", "j"),
                              CreateMessageIndexNextIndex("1", "casa"),
                              CreateMessageIndexNextIndex("1", "-"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("z", "1"),
                          }));
}

TEST_F(OrganizeByIdTest, TerminationAsId) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("-1", "-1"),
      CreateMessageIndexNextIndex("-1", "6"),
      CreateMessageIndexNextIndex("6", "j"),
      CreateMessageIndexNextIndex("j", "m"),
      CreateMessageIndexNextIndex("j", "-1"),
      CreateMessageIndexNextIndex("j", "k"),
      CreateFinalMessage("m"),
      CreateFinalMessage("m"),
      CreateFinalMessage("m"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(input.size()));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateFinalMessage("m"),
                              CreateFinalMessage("m"),
                              CreateFinalMessage("m"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("j", "m"),
                              CreateMessageIndexNextIndex("j", "-1"),
                              CreateMessageIndexNextIndex("j", "k"),
                          }));

  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("j", "m"),
                              CreateMessageIndexNextIndex("j", "-1"),
                              CreateMessageIndexNextIndex("j", "k"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("6", "j"),
                          }));
  ASSERT_THAT(result, ElementsAreAfterAnyIn(
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("6", "j"),
                          },
                          PipelineLogMessages{
                              CreateMessageIndexNextIndex("-1", "-1"),
                              CreateMessageIndexNextIndex("-1", "6"),
                          }));
}

TEST_F(OrganizeByIdTest, TerminationAsId2) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("a", "b"),
      CreateMessageIndexNextIndex("a", "d"),
      CreateMessageIndexNextIndex("b", "d"),
      CreateFinalMessage("d"),
      CreateFinalMessage("-1"),
  };

  std::ranges::sort(input, MessageLessThan);

  do {
    auto organizer = OrganizeById{input};
    auto result = organizer.Organize();

    ASSERT_THAT(result, SizeIs(input.size()));
    ASSERT_THAT(result, UnorderedElementsAreArray(input));

    ASSERT_THAT(result, ElementsAreAfterAnyIn(
                            PipelineLogMessages{
                                CreateFinalMessage("d"),
                            },
                            PipelineLogMessages{
                                CreateMessageIndexNextIndex("b", "d"),
                                CreateMessageIndexNextIndex("a", "b"),
                                CreateMessageIndexNextIndex("a", "d"),
                            }));

    ASSERT_THAT(result, ElementsAreAfterAnyIn(
                            PipelineLogMessages{
                                CreateMessageIndexNextIndex("b", "d"),
                            },
                            PipelineLogMessages{
                                CreateMessageIndexNextIndex("a", "b"),
                                CreateMessageIndexNextIndex("a", "d"),
                            }));
  } while (std::ranges::next_permutation(input, MessageLessThan).found);
}

TEST_F(OrganizeByIdTest, FiveMessagesCyclic) {
  using pipelines::log_message_organizer::OrganizeById;
  using pipelines::log_message_organizer::PipelineLogMessages;

  auto input = PipelineLogMessages{
      CreateMessageIndexNextIndex("j", "k"),
      CreateMessageIndexNextIndex("k", "3"),
      CreateMessageIndexNextIndex("3", "1"),
      CreateMessageIndexNextIndex("1", "4"),
      CreateMessageIndexNextIndex("4", "3"),
  };

  auto organizer = OrganizeById{input};
  auto result = organizer.Organize();

  ASSERT_THAT(result, SizeIs(input.size()));
  ASSERT_THAT(result, UnorderedElementsAreArray(input));
}
