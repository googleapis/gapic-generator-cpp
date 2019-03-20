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

#include "status_or.h"
#include "googletest/include/gtest/gtest.h"
#include "status.h"

#include <memory>
#include <string>
#include <type_traits>

namespace {

using namespace google;

class Observable {
 public:
  static int default_constructor;
  static int value_constructor;
  static int copy_constructor;
  static int move_constructor;
  static int destructor;

  static void reset_counters() {
    default_constructor = 0;
    value_constructor = 0;
    copy_constructor = 0;
    move_constructor = 0;
    destructor = 0;
  }
  Observable() { ++default_constructor; }
  explicit Observable(std::string str) : str_(std::move(str)) {
    ++value_constructor;
  }
  Observable(Observable const& rhs) : str_(rhs.str_) { ++copy_constructor; }
  Observable(Observable&& rhs) : str_(std::move(rhs.str_)) {
    rhs.str_ = "moved-out";
    ++move_constructor;
  }
  ~Observable() { ++destructor; }

  bool operator==(Observable const& rhs) const { return str_ == rhs.str_; }

  std::string const& str() const { return str_; }

 private:
  std::string str_;
};

// Have to define these outside of the class so that the linker
// actually makes some space for them.
int Observable::default_constructor;
int Observable::value_constructor;
int Observable::copy_constructor;
int Observable::move_constructor;
int Observable::destructor;

static_assert(!std::is_default_constructible<gax::StatusOr<int>>::value,
              "Default constructed StatusOr is unhelpful.");

// Although production use is not going to use simple types,
// testing StatusOr<int> is useful for very basic tests.
TEST(StatusOr, ConstructFromStatus) {
  gax::StatusOr<int> tested(gax::Status(gax::StatusCode::kUnknown, "Because"));
  EXPECT_FALSE(tested.ok());
  EXPECT_EQ(tested.status(), gax::Status(gax::StatusCode::kUnknown, "Because"));
  EXPECT_FALSE(tested);

  gax::StatusOr<int> tested2(
      gax::Status(gax::StatusCode::kCancelled, "Why not?"));
  EXPECT_FALSE(tested2.ok());
  EXPECT_EQ(tested2.status(),
            gax::Status(gax::StatusCode::kCancelled, "Why not?"));
  EXPECT_FALSE(tested2);

  gax::StatusOr<int> copy(tested2);
  EXPECT_FALSE(copy.ok());
  EXPECT_EQ(copy.status(),
            gax::Status(gax::StatusCode::kCancelled, "Why not?"));
  EXPECT_FALSE(copy);
}

TEST(StatusOr, ConstructFromOkStatusFails) {
  EXPECT_DEATH(gax::StatusOr<int>(gax::Status()),
               "Constructing StatusOr<T> from OK status is not allowed");
}

TEST(StatusOr, ValueOnFailedAborts) {
  gax::StatusOr<int> failed(
      gax::Status(gax::StatusCode::kCancelled, "Because"));
  EXPECT_DEATH(failed.value(), "Because \\[CANCELLED\\]");
}

TEST(StatusOr, ValueOnFailedAbortsConst) {
  gax::StatusOr<int> const failed(
      gax::Status(gax::StatusCode::kCancelled, "Because"));
  EXPECT_DEATH(failed.value(), "Because \\[CANCELLED\\]");
}

TEST(StatusOr, ValueOnFailedAbortsMove) {
  gax::StatusOr<int> failed(
      gax::Status(gax::StatusCode::kCancelled, "Because"));
  EXPECT_DEATH(std::move(failed).value(), "Because \\[CANCELLED\\]");
}

TEST(StatusOr, ConstructFromValue) {
  gax::StatusOr<int> tested(42);
  EXPECT_TRUE(tested.ok());
  EXPECT_EQ(tested.status(), gax::Status());
  EXPECT_TRUE(tested);
  EXPECT_EQ(tested.value(), 42);

  gax::StatusOr<int> tested2(666);
  EXPECT_TRUE(tested2.ok());
  EXPECT_EQ(tested2.status(), gax::Status());
  EXPECT_TRUE(tested2);
  EXPECT_EQ(tested2.value(), 666);

  gax::StatusOr<int> copy(tested2);
  EXPECT_TRUE(copy.ok());
  EXPECT_EQ(copy.status(), gax::Status());
  EXPECT_TRUE(copy);
  EXPECT_EQ(copy.value(), 666);
}

TEST(StatusOr, CopyConstruct) {
  Observable::reset_counters();
  gax::StatusOr<Observable> failed(
      gax::Status(gax::StatusCode::kUnknown, "Because"));
  gax::StatusOr<Observable> copy_failed(failed);
  EXPECT_EQ(Observable::default_constructor, 0);
  EXPECT_EQ(Observable::copy_constructor, 0);

  Observable o("Passed");
  gax::StatusOr<Observable> succeeded(o);
  Observable::reset_counters();
  gax::StatusOr<Observable> copy_succeeded(succeeded);
  EXPECT_EQ(Observable::copy_constructor, 1);
}

TEST(StatusOr, MoveConstruct) {
  Observable::reset_counters();
  gax::StatusOr<Observable> failed(
      gax::Status(gax::StatusCode::kUnknown, "Because"));
  gax::StatusOr<Observable> move_failed(std::move(failed));
  EXPECT_EQ(Observable::default_constructor, 0);
  EXPECT_EQ(Observable::copy_constructor, 0);
  EXPECT_EQ(Observable::move_constructor, 0);

  Observable o("Passed");
  gax::StatusOr<Observable> succeeded(o);
  Observable::reset_counters();
  gax::StatusOr<Observable> move_succeeded(std::move(succeeded));
  EXPECT_EQ(Observable::copy_constructor, 0);
  EXPECT_EQ(Observable::move_constructor, 1);
}

TEST(StatusOr, ValueByConstRef) {
  Observable o;
  Observable::reset_counters();

  gax::StatusOr<Observable> tested(o);
  EXPECT_EQ(Observable::default_constructor, 0);
  EXPECT_EQ(Observable::copy_constructor, 1);
  EXPECT_EQ(Observable::move_constructor, 0);
}

TEST(StatusOr, ValueByRvalueRef) {
  Observable o;
  Observable::reset_counters();

  gax::StatusOr<Observable> tested(std::move(o));
  EXPECT_EQ(Observable::default_constructor, 0);
  EXPECT_EQ(Observable::copy_constructor, 0);
  EXPECT_EQ(Observable::move_constructor, 1);
}

TEST(StatusOr, Destructor) {
  Observable::reset_counters();
  {
    gax::StatusOr<Observable> invalid(
        gax::Status(gax::StatusCode::kUnknown, "Because"));
  }
  EXPECT_EQ(Observable::destructor, 0);

  Observable o;
  Observable::reset_counters();
  { gax::StatusOr<Observable> valid(o); }
  EXPECT_EQ(Observable::destructor, 1);
}

TEST(StatusOr, ValueAccessor) {
  Observable o("test string");
  gax::StatusOr<Observable> tested(o);
  EXPECT_EQ(o, *tested);
  EXPECT_EQ(tested->str(), o.str());

  Observable::reset_counters();
  auto& ref = tested.value();
  EXPECT_EQ(Observable::copy_constructor, 0);

  gax::StatusOr<Observable> const const_tested(o);
  EXPECT_EQ(o, *const_tested);
  EXPECT_EQ(const_tested->str(), o.str());

  Observable::reset_counters();
  auto const& constref = tested.value();
  EXPECT_EQ(Observable::copy_constructor, 0);
}

TEST(StatusOrObservableTest, MoveStar) {
  gax::StatusOr<Observable> tested(Observable("testing"));
  EXPECT_EQ("testing", tested->str());

  Observable::reset_counters();
  auto observed = *std::move(tested);
  EXPECT_EQ("testing", observed.str());
  EXPECT_EQ(tested.status(), gax::Status());
  EXPECT_EQ("moved-out", tested->str());
}

TEST(StatusOrObservableTest, MoveValue) {
  gax::StatusOr<Observable> tested(Observable("testing"));
  EXPECT_EQ("testing", tested->str());

  Observable::reset_counters();
  auto observed = std::move(tested).value();
  EXPECT_EQ("testing", observed.str());
  EXPECT_EQ(tested.status(), gax::Status());
  EXPECT_EQ("moved-out", tested->str());
}

}  // namespace
