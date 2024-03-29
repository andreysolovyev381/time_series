//
// Created by Andrey Solovyev on 29/01/2023.
//

#pragma once

#include "numerics.h"

#include <type_traits>
#include <iterator>

#ifndef TYPE_REQUIREMENTS_ITERATORS_H
#define TYPE_REQUIREMENTS_ITERATORS_H

namespace culib::requirements {

  /**
   * @details
   * The first part of the code (C++20) was taken from an example\n
   * used by Konstantin Vladimirov while delivering\n
   * his lectures on C++. Example name is "junk_iterator".\n\n
   * His work on Iter requirements by using the\n
   * C++20 features provided a framework for the same\n
   * functionality developed in C++17, that is roughly\n
   * the second part of this source file.\n
   * See the appropriate comment.\n
   * */

  // -------------------------------- stdlib-like part
#ifdef __cpp_concepts
  template <class T, class U>
  concept DerivedFrom = std::is_base_of<U, T>::value;

  template <typename I>
  concept Iterator = requires() {
	  typename I::value_type;
	  typename I::difference_type;
	  typename I::reference;
	  typename I::pointer;
	  typename I::iterator_category;
  };

  template <Iterator Iter>
  inline constexpr bool is_iterator_v { Iterator<Iter> };

  template<typename... MaybeIterator>
  concept AreAllIterators = requires () {requires ((Iterator<MaybeIterator>),...);};

  template <typename I>
  concept InputIterator = Iterator<I> && requires {
	  typename I::iterator_category;
  } && DerivedFrom<typename I::iterator_category, std::input_iterator_tag>;

  template <typename I>
  concept ForwardIterator =
  InputIterator<I> && Incrementable<I> &&
		  DerivedFrom<typename I::iterator_category, std::forward_iterator_tag>;


  template <typename I>
  concept BidirectionalIterator =
  ForwardIterator<I> && Decrementable<I> &&
		  DerivedFrom<typename I::iterator_category, std::bidirectional_iterator_tag>;

  template <typename I>
  concept RandomAccess = requires(I it, typename I::difference_type n) {
	  it + n;
	  it - n;
  };

  template <typename I>
  concept RandomAccessIterator =
  BidirectionalIterator<I> && RandomAccess<I> &&
		  DerivedFrom<typename I::iterator_category, std::random_access_iterator_tag>;

  /** @brief
   * Block of code, written by Konstantin Valdimirov,\n
   * ends here.\n
   * */

  template <typename IterCategory>
  concept BiDirectionalOrRandomAccess =
  DerivedFrom<IterCategory, std::bidirectional_iterator_tag> ||
		  DerivedFrom<IterCategory, std::random_access_iterator_tag>;


  template <typename... IterCategory>
  inline constexpr bool areAllRandomAccess_v (){
	  bool result{true};
	  ((result = result && DerivedFrom<IterCategory, std::random_access_iterator_tag>), ...);
	  return result;
  }

#else

  namespace details {
	template<typename Iter, typename = void>
	struct MaybeIterator : std::false_type { };

	template<typename Iter>
	struct MaybeIterator<Iter, std::void_t<
			typename Iter::value_type,
			typename Iter::difference_type,
			typename Iter::reference,
			typename Iter::pointer,
			typename Iter::iterator_category>> : std::true_type {};
  }//!namespace
  template <typename Iter>
  inline constexpr bool is_iterator_v { details::MaybeIterator<Iter>::value };

  template <typename Iter>
  using IsIterator = std::enable_if_t<is_iterator_v<Iter>, bool>;

  template <typename... MaybeIterators>
  constexpr bool areAllIterators_v () {
	  bool result {true};
	  return ((result = result && is_iterator_v<MaybeIterators>),...);
  }

  template<typename... MaybeIterators>
  using AreAllIterators = std::enable_if_t<areAllIterators_v<MaybeIterators...>(), bool>;



  template <typename Iter, IsIterator<Iter> = true>
  using InputIterator = std::enable_if_t<
		  std::is_base_of_v<
				  std::input_iterator_tag,
				  typename std::iterator_traits<Iter>::iterator_category>
				  , bool>;

  template <typename Iter, IsIterator<Iter> = true>
  using ForwardIterator = std::enable_if_t<std::conjunction_v<
		  InputIterator<Iter>,
		  IsIncrementable<Iter>,
		  std::is_base_of<
				  std::forward_iterator_tag,
				  typename std::iterator_traits<Iter>::iterator_category>>
				  , bool>;

  template <typename Iter, IsIterator<Iter> = true>
  using BidirectionalIterator = std::enable_if_t<std::conjunction_v<
		  ForwardIterator<Iter>,
		  IsDecrementable<Iter>,
		  std::is_base_of<
				  std::bidirectional_iterator_tag,
				  typename std::iterator_traits<Iter>::iterator_category>>
				  , bool>;

  namespace details {

  template <typename I, typename = void>
  struct RandomAccessType : std::false_type {} ;
  template <typename I>
  struct RandomAccessType<I, std::void_t<
		  decltype(std::declval<I::difference_type>()),
		  decltype(std::declval<I>() + std::declval<I::difference_type>()),
		  decltype(std::declval<I>() - std::declval<I::difference_type>())>
		  > : std::true_type {};

  template <typename I>
  inline constexpr bool is_random_access_type_v {RandomAccessType<I>::value} ;

  template <typename Iter>
  using HasRandomAccess = std::enable_if_t<is_random_access_type_v<Iter>, bool>;
}//!namespace

  template <typename Iter, IsIterator<Iter> = true>
  using RandomAccessIterator = std::enable_if_t<std::conjunction_v<
		  BidirectionalIterator<Iter>,
		  details::HasRandomAccess<Iter>,
		  std::is_base_of<
				  std::random_access_iterator_tag,
				  typename std::iterator_traits<Iter>::iterator_category>>
				  , bool>;

  namespace details {

	template <typename IterCategory, typename = void>
	struct MaybeBiDirectionalOrRandomAccess : std::false_type {};

	template <typename IterCategory>
	struct MaybeBiDirectionalOrRandomAccess <IterCategory, std::void_t<
			std::disjunction <
					std::is_base_of<std::bidirectional_iterator_tag, IterCategory>,
					std::is_base_of<std::random_access_iterator_tag, IterCategory>
			>
	>> : std::true_type {};

  }//!namespace

  template <typename IterCategory>
  inline constexpr bool is_bidirectional_or_random_access_v {
		  details::MaybeBiDirectionalOrRandomAccess<IterCategory>::value };

  template <typename IterCategory>
  using BiDirectionalOrRandomAccess = std::enable_if_t<
		  is_bidirectional_or_random_access_v<IterCategory>
		  , bool>;

  template <typename... IterCategory>
  inline constexpr bool areAllRandomAccess_v (){
	  bool result{true};
	  ((result = result && std::is_base_of_v<std::random_access_iterator_tag, IterCategory>), ...);
	  return result;
  }

#endif

}//!namespace
#endif //TYPE_REQUIREMENTS_ITERATORS_H
