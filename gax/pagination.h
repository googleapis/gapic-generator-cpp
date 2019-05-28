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

/**
 * Wraps a 'page' message with consistent interface that provides an iterator
 * over its repeated elements and an accessor for its next_page_token field.
 *
 * Picking a repeated field, deducing the type of its elements, and providing an
 * ElementAccessor functor that references the field is outside the scope of the
 * template.
 *
 * @tparam ElementType the type of the repeated elements in the page.
 * @tparam PageType the type of the wrapped message.
 * @tparam ElementAccessor a default-constructable functor that has an overload
 * of operator() that takes a PageType& and returns a mutable pointer to the
 * repeated field that contains ElementType.
 *
 * Note: if there is more than one repeated field in a PageType message, all but
 * one will be efectively hidden except through the RawPage accessor.
 */
template <
    typename ElementType, typename PageType, typename ElementAccessor,
    typename std::enable_if<
        std::is_default_constructible<ElementAccessor>::value, int>::type = 0>
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

  /**
   * @brief Get the next_page_token for the page.
   *
   * @retun the token for the next page in the sequence
   * or the empty string if this is the last page.
   */
  std::string NextPageToken() const { return raw_page_.next_page_token(); }

  /**
   * @brief Get the underlying page message.
   *
   * This can be useful if the page has custom metadata fields.
   *
   * @return a const reference to the underlying raw page.
   */
  PageType const& RawPage() const { return raw_page_; }

  // Note: the non-const variant is intended for internal use only.
  PageType& RawPage() { return raw_page_; }

 private:
  PageType raw_page_;
};

/**
 * Wraps a sequence of pages implied to be serially returned by a paginated API
 * method and provides an iterator that retrieves subsequent pages, usually via
 * synchronous rpc.
 *
 * Determining whether a protobuf message constitutes a valid PageType is
 * outside the scope of the template code.
 *
 * @par Example
 *
 * @code
 * ListElementsRequest request;
 * // Set up request to describe the right resource
 * // and optionally customize the initial page token or page size.
 *
 * class ElementsAccessor {
 *  public:
 *   protobuf::RepeatedPtrField<Element>*
 *   operator()(ListElementsResponse* response) {
 *     return response.mutable_elements();
 *   }
 * };
 *
 * auto get_next_page = [request, stub](ListElementsResponse* response) mutable
 * {
 *   gax::CallContext ctx;
 *   gax::Status status = stub->ListElements(context, request, response);
 *   request.set_next_page_token(response->next_page_token());
 *   return status;
 * };
 *
 * // Only list up to 20 pages, even though there may be more.
 * Pages<EltType, ListElementsResponse, decltype(get_elements),
 *       ElementsAccessor> pages(std::move(get_next_page), 20);
 * for(auto& page : pages) {
 *   // Do something with the page
 * }
 * @endcode
 *
 * @tparam ElementType the type of the repeated elements in the page.
 * @tparam PageType thye type of the wrapped message.
 * @tparam ElementAccessor a default-constructable functor that has an overload
 * of operator() that takes a PageType& and returns a mutable pointer to the
 * repeated field that contains ElementType.
 * @tparam NextPageRetriever a copy-constructable functor that has an overload
 * of operator() that takes a mutable PageType*, initiates an rpc that
 * overwrites the contents of the page with the next page, and returns a
 * gax::Status indicating the success or failure of the rpc.
 *
 * Note: the initial page request MUST be captured by value in the
 * NextPageRetriever functor so that calling begin() multiple times on a Pages
 * instance results in valid behavior.
 */
template <typename ElementType, typename PageType, typename ElementAccessor,
          typename NextPageRetriever,
          typename std::enable_if<
              gax::internal::is_invocable<NextPageRetriever, PageType*>::value,
              int>::type = 0,
          typename std::enable_if<
              std::is_copy_constructible<NextPageRetriever>::value, int>::type =
              0>
class Pages {
 public:
  class iterator {
   public:
    using PageResultT = PageResult<ElementType, PageType, ElementAccessor>;
    using iterator_category = std::forward_iterator_tag;
    using value_type = PageResultT;
    using difference_type = std::ptrdiff_t;
    using pointer = PageResultT*;
    using reference = PageResultT&;

    PageResultT const& operator*() const { return page_result_; }
    PageResultT const* operator->() const { return &page_result_; }
    iterator& operator++() {
      // Note: if the rpc fails, the page will be untouched,
      // i.e. will be have an empty page token and element collection.
      // This invalidates any iterators on the PageResult.
      page_result_.RawPage().Clear();
      get_next_page_(&(page_result_.RawPage()));
      num_pages_++;
      return *this;
    }

    // Just want to compare against end()
    bool operator==(iterator const& rhs) const {
      return num_pages_ == rhs.num_pages_ ||
             page_result_.NextPageToken() == rhs.page_result_.NextPageToken();
    }
    bool operator!=(iterator const& rhs) const { return !(*this == rhs); }

   private:
    friend Pages;
    // Note: copying a message with many repeated elements is expensive.
    // Callers should move pages in when instantiating an iterator.
    iterator(PageType page_result, NextPageRetriever get_next_page,
             int num_pages)
        : page_result_(std::move(page_result)),
          get_next_page_(std::move(get_next_page)),
          num_pages_(num_pages) {}

    PageResultT page_result_;
    NextPageRetriever get_next_page_;
    int num_pages_;
  };

  /**
   * Create a new Pages instance using the provided page retrieval functor.
   * Iterators on the instance will yield a number of pages up to the page cap.
   *
   * @param get_next_page an instance of the page retrieval functor.
   * @param pages_cap the maximum number of pages to retrieve. A value of 0
   * (default) indicates no cap.
   */
  Pages(NextPageRetriever get_next_page, int pages_cap = 0)
      : get_next_page_(std::move(get_next_page)), pages_cap_(pages_cap) {}

  iterator begin() const {
    PageType page;
    // Copying the next-page lambda is necessary to start at the beginning.
    NextPageRetriever fresh_get_next_page_(get_next_page_);
    fresh_get_next_page_(&page);

    return iterator(std::move(page), std::move(fresh_get_next_page_), 1);
  }

  iterator end() const {
    return iterator{PageType{}, get_next_page_, pages_cap_};
  }

 private:
  // Note: be sure to capture the initial page request by value so that calling
  // begin() multiple times is valid.
  // This means that whenever get_next_page_ is copied, i.e. whenever the user
  // calls Pages::begin(), a fresh copy of the initial request gets created,
  // which means that begin() _really_ starts at the beginning.
  NextPageRetriever get_next_page_;
  const int pages_cap_;
};

}  // namespace gax
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GAX_PAGINATION_H_
