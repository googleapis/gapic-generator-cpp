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

#ifndef GAPIC_GENERATOR_CPP_GAX_PAGINATION_H_
#define GAPIC_GENERATOR_CPP_GAX_PAGINATION_H_

#include <google/protobuf/repeated_field.h>
#include <functional>
#include <iterator>
#include <string>

namespace google {
namespace gax {

template <typename ElementType, typename PageType>
class PageResult {
  using ElementAccessor =
      std::function<::google::protobuf::RepeatedPtrField<ElementType>*(
          PageType&)>;

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

    iterator(typename ::google::protobuf::RepeatedPtrField<
             ElementType>::iterator current)
        : current_(std::move(current)) {}

    typename ::google::protobuf::RepeatedPtrField<ElementType>::iterator
        current_;
  };
  // TODO: Need to take into account RepeatedField vs RepeatedPtrField.
  // They have different iterator types,
  // and will probably need some templating magic.
  // Note: Since the constructor will eventually be private (with friend
  // access), we don't have to define all the variants for references.
  // We just want to take ownership of a page.
  PageResult(PageType&& page, ElementAccessor accessor)
      : page_(std::move(page)),
        begin_(accessor(page_)->begin()),
        end_(accessor(page_)->end()) {}

  iterator begin() const { return begin_; }
  iterator end() const { return end_; }

  std::string NextPageToken() const { return page_.next_page_token(); }
  PageType const& RawPage() const { return page_; }

 private:
  PageType page_;
  iterator const begin_;
  iterator const end_;
};

}  // namespace gax
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GAX_PAGINATION_H_
