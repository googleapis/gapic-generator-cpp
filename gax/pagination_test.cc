// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gax/pagination.h"
#include "google/longrunning/operations.pb.h"
#include <google/protobuf/util/message_differencer.h>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <iterator>
#include <sstream>
#include <vector>

namespace {

using namespace ::google;

// Protobuf messages are not equality comparible by default.
// This complicates testing, so just define a comparison function here.
bool Equal(longrunning::Operation const& lhs,
           longrunning::Operation const& rhs) {
  return protobuf::util::MessageDifferencer::Equals(lhs, rhs);
}

auto constexpr accessor = [](longrunning::ListOperationsResponse& lor) {
  return lor.mutable_operations();
};

using TestedPageResult =
    gax::PageResult<longrunning::Operation, longrunning::ListOperationsResponse,
                    decltype(accessor)>;

TestedPageResult MakeTestedPageResult() {
  longrunning::ListOperationsResponse response;
  response.set_next_page_token("NextPage");

  for (int i = 0; i < 10; i++) {
    std::stringstream ss;
    ss << "TestOperation" << i;
    auto operation = response.add_operations();
    operation->set_name(ss.str());
  }

  return TestedPageResult(std::move(response), accessor);
}

TEST(PageResult, RawPage) {
  TestedPageResult page_result = MakeTestedPageResult();

  EXPECT_EQ(page_result.NextPageToken(), "NextPage");
  EXPECT_EQ(page_result.NextPageToken(),
            page_result.RawPage().next_page_token());
  EXPECT_EQ(page_result.RawPage().operations_size(), 10);

  // Move iteration test
}

TEST(PageResult, Accessors) {
  TestedPageResult page_result = MakeTestedPageResult();
  EXPECT_EQ((*page_result.begin()).name(), "TestOperation0");
  EXPECT_EQ(page_result.begin()->name(), "TestOperation0");
}

TEST(PageResult, BasicIteration) {
  TestedPageResult page_result = MakeTestedPageResult();
  auto prIt = page_result.begin();
  auto reIt = page_result.RawPage().operations().begin();
  for (; prIt != page_result.end() &&
         reIt != page_result.RawPage().operations().end();
       ++prIt, ++reIt) {
    // Note: cannot use EXPECT_EQ for the elements or on vectors constructed
    // from the respective iterators because messages do not define operator==
    // as a member function.
    EXPECT_TRUE(Equal(*prIt, *reIt));
  }
  EXPECT_EQ(prIt, page_result.end());
  EXPECT_EQ(reIt, page_result.RawPage().operations().end());
}

TEST(PageResult, MoveIteration) {
  TestedPageResult page_result = MakeTestedPageResult();
  std::vector<longrunning::Operation> ops{
      std::move_iterator<TestedPageResult::iterator>(page_result.begin()),
      std::move_iterator<TestedPageResult::iterator>(page_result.end())};
  EXPECT_EQ(page_result.begin()->name(), "");
}

}  // namespace
