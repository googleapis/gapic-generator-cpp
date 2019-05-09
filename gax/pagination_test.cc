// Copyright 2019 Google Inc.  All rights reserved
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gax/pagination.h"
#include "google/longrunning/operations.pb.h"
#include <gtest/gtest.h>
#include <iterator>
#include <sstream>
#include <vector>

namespace {

using namespace ::google;

TEST(PageResult, Basic) {
  // Note: I think Carlos is going to want begin(), end() and member access to
  // be done with templates.
  longrunning::ListOperationsResponse tmp;
  tmp.set_next_page_token("NextPage");

  for (int i = 0; i < 10; i++) {
    std::stringstream ss;
    ss << "TestOperation" << i;
    auto operation = tmp.add_operations();
    operation->set_name(ss.str());
  }

  // Make a copy so we can test making move iterators below.
  longrunning::ListOperationsResponse response(tmp);

  gax::PageResult<longrunning::Operation, longrunning::ListOperationsResponse>
  pageResult(std::move(tmp), [](longrunning::ListOperationsResponse& lor) {
    return lor.mutable_operations();
  });

  // Tests for raw page access
  EXPECT_EQ(pageResult.NextPageToken(), "NextPage");
  EXPECT_EQ(pageResult.RawPage().next_page_token(), response.next_page_token());
  EXPECT_EQ(pageResult.RawPage().operations_size(), response.operations_size());

  // Quickly test operator*
  EXPECT_EQ((*pageResult.begin()).name(), "TestOperation0");

  auto prIt = pageResult.begin();
  auto reIt = response.operations().begin();
  for (; prIt != pageResult.end() && reIt != response.operations().end();
       ++prIt, ++reIt) {
    EXPECT_EQ(prIt->name(), reIt->name());
  }
  EXPECT_EQ(prIt, pageResult.end());
  EXPECT_EQ(reIt, response.operations().end());

  // Move iteration test
  using iter_t = decltype(pageResult)::iterator;
  std::vector<longrunning::Operation> ops{
      std::move_iterator<iter_t>(pageResult.begin()),
      std::move_iterator<iter_t>(pageResult.end())};
  EXPECT_EQ(pageResult.begin()->name(), "");
}

}  // namespace
