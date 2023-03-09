//
// Created by Andrey Solovyev on 16/02/2023.
//

#pragma once

#include <type_traits>
#include <functional>

#ifndef TYPE_REQUIREMENTS_CONTAINER_H
#define TYPE_REQUIREMENTS_CONTAINER_H

namespace culib::requirements {
  /**
   * @details
   * Requirements to restrict containers only to be used as a serie
   * */

  template<typename Container, typename = void>
  struct MaybeContainer : std::false_type {};

  template<typename Container>
  struct MaybeContainer<Container,
					 std::void_t<
							 decltype(std::declval<Container>().begin()),
							 decltype(std::declval<Container>().end())
					 >
  > : std::true_type {};

  template<typename Container>
  constexpr bool isContainer_v() { return MaybeContainer<Container>::value; }

  template<typename Container>
  using IsContainer = std::enable_if_t<isContainer_v<Container>(), bool>;

}//!namespace

#endif //TYPE_REQUIREMENTS_CONTAINER_H