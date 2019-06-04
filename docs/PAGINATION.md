# C++ GAPIC Paginated Method Interface Design Doc


*dovs@google.com*

Overview
========

Supported workflow requirements
-------------------------------

The following are statements and constraints about providing
abstractions over paginated API methods.

-   Iterating over the elements within a page requires no additional memory or network trips.
-   Iterating over pages may require additional space and does require network trips.
-   There is no point in iterating over elements asynchronously: all elements within a page are available immediately.
-   Because page-continuation is described using a `next_page_token`, at most one subsequent page can be requested asynchronously due to data dependence.
-   Users MAY wish to terminate iteration early.
-   Users MAY wish to view the page token for the next page at any time.

Example client method
=====================

This client method and associated messages will be used in the examples
in this document.

```protobuf
message Book {
  string name = 1;
  string author = 2;
  string title = 3;
  bool read = 4;
  enum Rating {
    GOOD = 0;
    BAD = 1;
  }

  Rating rating = 5;
}

message ListBooksRequest {
  string name = 1;
  int32 page_size = 2;
  string page_token = 3;
  string filter = 4;
}

message ListBooksResponse {
  repeated Book books = 1;
  string next_page_token = 2;
}

rpc ListBooks(ListBooksRequest) returns (ListBooksResponse) {
  //option (google.api.http) = { get: "/v1/{name=bookShelves/*}/books" };
}
```

Terminology
-----------

In the rest of this document, ListBooksResponse is the 'page type' and Book is the 'element type'. The books field of the ListBooksResponse message is the 'element collection', and in C++ is of the type `google::protobuf::RepeatedPtrField<Book>` (for protobuf primitive types, e.g. integers, the element collection would be of type `google::protobuf::RepeatedField<T>`).

Interfaces
==========

Instead of directly returning the message type described in the service's proto file paginated API methods will return a `gax::PaginatedResult<ElementType, PageType>`. Other template parameters may be necessary for compile-time attribute access.

The returned object will have four public methods: `begin()` and `end()` that define iterators over ElementType; `next_page_token()` that returns the `next_page_token()` attribute of the current page; and pages() that returns a `gax::Pages<ElementType, PageType>` structure defined below.

The iterator for `PaginatedResult` describes a ForwardRange[^1] over all the elements that would be listed by the API method if it returned a single, enormous page. The corresponding client method allows users to cap the number of pages iterated over.

The `gax::Pages<ElementType, PageType>` structure is a ForwardRange over `PageResult<ElementType, PageType>`, described below. It has three public methods: `begin()`, `end()`, and `pages_seen()`.

The `gax::PageResult<ElementType, PageType>` is a thin wrapper around the page type. It provides four public methods: `begin()` and `end()`, which iterate over elements; `next_page_token()`, which is an accessor for the field of the same name in the page; and `RawPage()`, which returns a constant reference to the wrapped page. This may be necessary for the user if the page contains metadata or other information that does not belong to the individual elements.

These interfaces have overlapping responsibilities but are designed to enable multiple tiers of users. Most users are expected to iterate over elements directly and want a minimal interface over pagination. Some may want more explicit control over the number of pages retrieved from the network or may need to reference additional fields in the page.

Detailed Implementation
=======================

`PageResult<ElementType, PageType>` - Already implemented, left here for historical purposes
---------------------------------------

```cpp
template <typename ElementType, typename PageType>
class PageResult {
using ElementAccessor =
  std::function<::google::protobuf::RepeatedPtrField<ElementType>*(PageType&)>;
 public:
  class iterator {
   public:
   // Note: this could probably be a random_access_iterator, but
   // it makes the implementation simpler to only support forward iteration,
   // and it is arguably more correct to force users to slurp elements into their
   // own random access container if they want to sort or do anything like that.
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

  bool operator==(iterator const& rhs) const { return current_ == rhs.current_; }
  bool operator!=(iterator const& rhs) const { return !(*this == rhs); }

  iterator(typename ::google::protobuf::RepeatedPtrField<ElementType>::iterator current)
  : current_(std::move(current)) {}

  typename ::google::protobuf::RepeatedPtrField<ElementType>::iterator current_;
  };

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
```

`Pages<ElementType, PageType>`
----------------------------------

Workhorse structure that engages in rpcs.

**Note:** the decision to store a pointer the client stub was previously visited in the design doc for C++ GAPIC LRO. This option was contentious at the time and ultimately was not chosen.

For the `Pages` class, access to the GAPIC stub is necessary in order to retrieve the next page in the sequence, and storing a pointer to it in the Pages struct is necessary to preserve the C++ iterator interface.

In the generated client code, the GAPIC stub will be hidden in the injected `get_next_page` closure.

[Stalled pull request for Pages](https://github.com/googleapis/gapic-generator-cpp/pull/55)

```cpp
// Note: we either need to store the accessor function in the Pages struct and
// pass it in when making iterators or lift it into a template parameter that
// gets passed down through the hierarchy.
// In any case, that's a minor implementation detail that can be hashed out in
// the design doc or in the PR.
template <typename ElementType, typename PageType>
class Pages {
 using NextPageRetriever = std::function<gax::Status(PageType*)>;
 using ElementAccessor = std::function<::google::protobuf::RepeatedPtrField<ElementType>*(PageType&)>;
 public:
  class iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = PageResult<ElementType, PageType>;
    using difference_type = std::ptrdiff_t;
    using pointer = PageResult<ElementType, PageType>*;
    using reference = PageResult<ElementType, PageType>&;

    reference operator*() const { return *page_result_; }
    pointer operator->() const { return &(*page_result_); }

    iterator& operator++() {
      PageType page;
      // Note: if the rpc fails, the page will be untouched, i.e. will still
      // be in a default-constructed state and will have no elements in the
      // element collection.
      get_next_page_(&page);
      page_result_ = PageResult<ElementType, PageType>(std::move(page), accessor_);
      return *this;
    }

    // Comparing page tokens is as good as we can do.
    // Anything else requires comparing the raw pages, and protobuf messages
    // do not define equality operators.
    bool operator==(iterator const& rhs) const {
      return page_result_.NextPageToken() == rhs.NextPageToken();
    }

    bool operator!=(iterator const& rhs) const { return !(*this == rhs); }

    iterator(PageResult<ElementType, PageType>&& page_result,
             ElementAccessor accessor, NextPageRetriever get_next_page)
     : page_result_(std::move(page_result)),
       accessor_(std::move(accessor)),
       get_next_page_(std::move(get_next_page)) {}

 private:
  PageResult<ElementType, PageType> page_result_;
  ElementAccessor accessor_;
  NextPageRetriever get_next_page_;
  };

  Pages(ElementAccessor accessor, NextPageRetriever get_next_page)
   : accessor_(std::move(accessor)),
     get_next_page_(std::move(get_next_page)) {}

  iterator begin() const {
   PageType page;
   // Copying the next-page lambda is necessary to start at the beginning.
   auto fresh_get_next_page_ = get_next_page_;
   fresh_get_next_page_(&page);
   return iterator{std::move(page), accessor_,
                   std::move(fresh_get_next_page_)};
  }

  iterator end() const { return iterator{PageType{}, nullptr, nullptr}; }

 private:
  ElementAccessor accessor_;
  // Note: be sure to capture the initial page request by value so that calling
  // begin() multiple times is valid.
  // This means that whenever get_next_page_ is copied, i.e. whenever the user
  // calls Pages::begin(), a fresh copy of the initial request gets created,
  // which means that begin() _really_ starts at the beginning.
  NextPageRetriever get_next_page_;
};
```

`PaginatedResult<ElementType, PageType>`
--------------------------------------------

**Note:** accessor functors and a get-next-page closure are also necessary template and constructor parameters but are only necessary as implementation details; they do nothing to clarify the interface.

```cpp
template <typename ElementType, typename PageType> class PaginatedResult {
  class iterator {
   public:
   using iterator_category = std::forward_iterator_tag;
   using value_type = ElementType;
   using difference_type = std::ptrdiff_t;
   using pointer = ElementType *;
   using reference = ElementType &;

   reference operator*() const;
   pointer operator->() const;
   iterator &operator++();

   bool operator==(iterator const &rhs) const;
   bool operator!=(iterator const &rhs) const;
  };

  Pages Pages();
  iterator begin();
  iterator end();
  std::string NextPageToken();
  }
};
```

The iterator for PaginatedResult wraps a Pages instance and flattens its iterator and the iterators for the individual pages so that what the user sees is a continuous stream of ElementType objects.

References
==========
