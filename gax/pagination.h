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
#include "gax/status.h"
#include <google/protobuf/repeated_field.h>
#include <iterator>
#include <string>
#include <type_traits>

namespace google {
namespace gax {

template <
    typename ElementType, typename PageType, typename ElementAccessor,
    typename NextPageRetriever,
    typename std::enable_if<
        std::is_default_constructible<ElementAccessor>::value, int>::type = 0,
    typename std::enable_if<
        gax::internal::is_invocable<NextPageRetriever, PageType*>::value,
        int>::type = 0>
class Pages {
 public:
  // Forward declare for friend access in PageResult
  class iterator;

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

      ElementType& operator*() const { return *current_; }
      ElementType* operator->() const { return &(*current_); }
      iterator& operator++() {
        ++current_;
        return *this;
      }
      bool operator==(iterator const& rhs) const {
        return current_ == rhs.current_;
      }
      bool operator!=(iterator const& rhs) const { return !(*this == rhs); }

     private:
      friend PageResult;
      iterator(FieldIterator current) : current_(std::move(current)) {}

      FieldIterator current_;
    };

    PageResult(PageType const& raw_page) : raw_page_(raw_page) {}
    PageResult(PageType&& raw_page) : raw_page_(std::move(raw_page)) {}

    iterator begin() { return ElementAccessor{}(raw_page_)->begin(); }
    iterator end() { return ElementAccessor{}(raw_page_)->end(); }

    // Const overloads
    iterator begin() const { return ElementAccessor{}(raw_page_)->cbegin(); }
    iterator end() const { return ElementAccessor{}(raw_page_)->cend(); }

    std::string NextPageToken() const { return raw_page_.next_page_token(); }
    PageType const& RawPage() const { return raw_page_; }

   private:
    friend Pages::iterator;
    PageType raw_page_;
  };

  class iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = PageResult;
    using difference_type = std::ptrdiff_t;
    using pointer = PageResult*;
    using reference = PageResult&;

    PageResult const& operator*() const { return page_result_; }
    PageResult const* operator->() const { return &page_result_; }
    iterator& operator++() {
      // Note: if the rpc fails, the page will be untouched,
      // i.e. will be have an empty page token and element collection.
      // This invalidates any iterators on the PageResult.
      page_result_.raw_page_.Clear();
      get_next_page_(&page_result_.raw_page_);
      num_pages_++;
      return *this;
    }

    // Just want to compare against end()
    bool operator==(iterator const& rhs) const {
      return page_result_.NextPageToken() == rhs.page_result_.NextPageToken() ||
             num_pages_ == rhs.num_pages_;
    }
    bool operator!=(iterator const& rhs) const { return !(*this == rhs); }

   private:
    friend Pages;
    // Note: don't need to provide a const-lvalue receiving version of the
    // constructor since iterators always take ownership of a page and pages can
    // be very expensive to copy if they have a lot of repeated elements.
    iterator(PageType&& page_result, NextPageRetriever get_next_page,
             int num_pages)
        : page_result_(std::move(page_result)),
          get_next_page_(std::move(get_next_page)),
          num_pages_(num_pages) {}

    PageResult page_result_;
    NextPageRetriever get_next_page_;
    int num_pages_;
  };

  // Note that a cap of 0 indicates that an iterator will only terminate
  // when the next page token is empty.
  Pages(NextPageRetriever get_next_page, int cap)
      : get_next_page_(std::move(get_next_page)), cap_(cap) {}

  iterator begin() const {
    PageType page;
    // Copying the next-page lambda is necessary to start at the beginning.
    NextPageRetriever fresh_get_next_page_(get_next_page_);
    fresh_get_next_page_(&page);

    return iterator(std::move(page), std::move(fresh_get_next_page_), 1);
  }

  iterator end() const { return iterator{PageType{}, get_next_page_, cap_}; }

 private:
  // Note: be sure to capture the initial page request by value so that calling
  // begin() multiple times is valid.
  // This means that whenever get_next_page_ is copied, i.e. whenever the user
  // calls Pages::begin(), a fresh copy of the initial request gets created,
  // which means that begin() _really_ starts at the beginning.
  NextPageRetriever get_next_page_;
  const int cap_;
};

}  // namespace gax
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GAX_PAGINATION_H_
