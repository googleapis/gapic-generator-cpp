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

#ifndef GAPIC_GENERATOR_CPP_GAX_PAGINATION_H_
#define GAPIC_GENERATOR_CPP_GAX_PAGINATION_H_

#include "gax/internal/invoke_result.h"
#include <google/protobuf/repeated_field.h>
#include <functional>
#include <iterator>
#include <string>
#include <type_traits>

namespace google {
namespace gax {

template <typename ElementType, typename PageType, typename ElementAccessor>
class PageResult {
  using FieldIterator =
      typename std::remove_pointer<typename gax::internal::invoke_result_t<
          ElementAccessor, PageType&>>::type::iterator;

 public:
  class iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = ElementType;
    using difference_type = std::ptrdiff_t;
    using pointer = ElementType*;
    using reference = ElementType&;

    reference operator*() const { return *current_; }
    pointer operator->() const { return &(*current_); }
    iterator& operator++() {
      ++current_;
      return *this;
    }
    bool operator==(iterator const& rhs) const {
      return current_ == rhs.current_;
    }
    bool operator!=(iterator const& rhs) const { return !(*this == rhs); }

    iterator(FieldIterator current) : current_(std::move(current)) {}

    FieldIterator current_;
  };
  // Note: Since the constructor will eventually be private (with friend
  // access), we don't have to define all the variants for references.
  // We just want to take ownership of a page.
  PageResult(PageType&& raw_page, ElementAccessor accessor)
      : raw_page_(std::move(raw_page)), accessor_(accessor) {}

  iterator begin() { return accessor_(raw_page_)->begin(); }
  iterator end() { return accessor_(raw_page_)->end(); }

  std::string NextPageToken() const { return raw_page_.next_page_token(); }
  PageType const& RawPage() const { return raw_page_; }

 private:
  PageType raw_page_;
  ElementAccessor accessor_;
};

}  // namespace gax
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GAX_PAGINATION_H_
