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
#include "gax/status.h"
#include <google/protobuf/util/message_differencer.h>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <iterator>
#include <sstream>
#include <vector>

namespace {

using namespace ::google;

class OperationsAccessor {
 public:
  protobuf::RepeatedPtrField<longrunning::Operation>* operator()(
      longrunning::ListOperationsResponse& lor) const {
    return lor.mutable_operations();
  }
};

class PageRetriever {
 public:
  // Start at 1 to count number of pages seen total, including the first.
  PageRetriever(int max_pages) : i_(1), max_pages_(max_pages) {}
  gax::Status operator()(longrunning::ListOperationsResponse* lor) {
    if (i_ < max_pages_) {
      std::stringstream ss;
      ss << "NextPage" << i_;
      lor->set_next_page_token(ss.str());
      i_++;
    } else {
      lor->clear_next_page_token();
    }

    return gax::Status{};
  }

 private:
  int i_;
  const int max_pages_;
};

using TestPages =
    gax::Pages<longrunning::Operation, longrunning::ListOperationsResponse,
               OperationsAccessor, PageRetriever>;

using TestedPageResult = TestPages::PageResult;

TestedPageResult MakeTestedPageResult() {
  longrunning::ListOperationsResponse response;
  response.set_next_page_token("NextPage");

  for (int i = 0; i < 10; i++) {
    std::stringstream ss;
    ss << "TestOperation" << i;
    auto operation = response.add_operations();
    operation->set_name(ss.str());
  }

  return TestedPageResult(std::move(response));
}

TEST(PageResult, RawPage) {
  TestedPageResult page_result = MakeTestedPageResult();

  EXPECT_EQ(page_result.NextPageToken(), "NextPage");
  EXPECT_EQ(page_result.NextPageToken(),
            page_result.RawPage().next_page_token());
  EXPECT_EQ(page_result.RawPage().operations_size(), 10);
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
    EXPECT_TRUE(protobuf::util::MessageDifferencer::Equals(*prIt, *reIt));
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

TEST(Pages, Basic) {
  TestPages terminal(
      // The output param is pristine, which means its next_page_token
      // is empty.
      PageRetriever(0), 0);

  EXPECT_EQ(terminal.begin(), terminal.end());
  EXPECT_EQ(terminal.end()->NextPageToken(), "");
}

TEST(Pages, Iteration) {
  int i = 1;
  TestPages pages(PageRetriever(10), 0);
  for (auto const& p : pages) {
    std::stringstream ss;
    ss << "NextPage" << i;

    EXPECT_EQ(p.NextPageToken(), ss.str());
    i++;
  }
  EXPECT_EQ(i, 10);
}

TEST(Pages, PageCap) {
  int i = 1;
  TestPages pages(PageRetriever(10), 5);
  auto iter = pages.begin();
  for (; iter != pages.end(); ++iter) {
    std::stringstream ss;
    ss << "NextPage" << i;

    EXPECT_EQ(iter->NextPageToken(), ss.str());
    i++;
  }
  EXPECT_EQ(i, 5);
  EXPECT_EQ(iter->NextPageToken(), "NextPage5");
}

}  // namespace
