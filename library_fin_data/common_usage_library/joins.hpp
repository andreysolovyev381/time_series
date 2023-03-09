//
// Created by Andrey Solovyev on 03/03/2023.
//

#pragma once

#include "itertools.hpp"
#include "tupletools.hpp"

#include <algorithm>
#include <type_traits>
#include <cstddef>
#include <tuple>
#include <utility>

#ifndef DATA_JOINS_H
#define DATA_JOINS_H

namespace culib::join {

  namespace details {

	enum class JoinType {
		None = 0,
		Inner,
		OuterFull,
		OuterExcluding,
		LeftFullOuter,
		LeftExcluding,
		RightFullOuter,
		RightExcluding,
	};

	template <class _InputIterator1, class _InputIterator2, class _OutputIterator, class _Compare = std::less<>>
	constexpr _OutputIterator
	set_intersection_and_difference(_InputIterator1 __first1, _InputIterator1 __last1,
			_InputIterator2 __first2, _InputIterator2 __last2, _OutputIterator __result, [[maybe_unused]] _Compare __comp = std::less<>{})
	{
		while (__first1 != __last1) {
			if (__first2 == __last2) {
				return std::copy(__first1, __last1, __result);
			}
			*__result = *__first1;
			++__result;
			++__first1;
			++__first2;
		}
		return __result;
	}

	template <class InputIterator1, class InputIterator2, class OutputIterator, class ResultContainer, class Compare = std::less<>>
	constexpr OutputIterator
	inner(InputIterator1 first1, InputIterator1 last1,
			InputIterator2 first2, InputIterator2 last2,
			OutputIterator result
			, [[maybe_unused]] Compare comp = std::less<>{}) {
		return std::set_intersection(first1, last1, first2, last2, result);
	}

	template <class InputIterator1, class InputIterator2, class OutputIterator, class ResultContainer, class Compare = std::less<>>
	constexpr OutputIterator
	outerFull(InputIterator1 first1, InputIterator1 last1,
			[[maybe_unused]] InputIterator2 first2, [[maybe_unused]] InputIterator2 last2,
			OutputIterator result
			, [[maybe_unused]] Compare comp = std::less<>{}) {

		//union
		std::copy(first1, last1, result);
		return result;
	}

	template <class InputIterator1, class InputIterator2, class OutputIterator, class ResultContainer, class Compare = std::less<>>
	constexpr OutputIterator
	outerExcluding(InputIterator1 first1, InputIterator1 last1,
			InputIterator2 first2, InputIterator2 last2,
			OutputIterator result
			, [[maybe_unused]] Compare comp = std::less<>{}) {

		ResultContainer temp2;
		auto temp2_iter = std::inserter(temp2, temp2.begin());
		std::set_symmetric_difference(first1, last1, first2, last2, temp2_iter);

		return
				std::set_difference (temp2.begin(), temp2.end(), first2, last2, result);
	}

	template <class InputIterator1, class InputIterator2, class OutputIterator, class ResultContainer, class Compare = std::less<>>
	constexpr OutputIterator
	sideFull(InputIterator1 first1, InputIterator1 last1,
			InputIterator2 first2, InputIterator2 last2,
			OutputIterator result
			, [[maybe_unused]] Compare comp = std::less<>{}) {

		//union
		ResultContainer temp2;
		auto temp2_iter = std::inserter(temp2, temp2.begin());
		std::set_union(first1, last1, first2, last2, temp2_iter);

		//difference between the two
		ResultContainer temp1;
		auto temp1_iter = std::inserter(temp1, temp1.begin());
		std::set_difference(first2, last2, first1, last1, temp1_iter);

		return std::set_difference (temp2.begin(), temp2.end(), temp1.begin(), temp1.end(), result);

	}

	template <class InputIterator1, class InputIterator2, class OutputIterator, class ResultContainer, class Compare = std::less<>>
	constexpr OutputIterator
	sideExcluding(InputIterator1 first1, InputIterator1 last1,
			InputIterator2 first2, InputIterator2 last2,
			OutputIterator result
			, [[maybe_unused]] Compare comp = std::less<>{}) {
		return std::set_difference(first1, last1, first2, last2, result);
	}


	template <class InputIterator1, class InputIterator2, class OutputIterator, class ResultContainer, class Compare = std::less<>>
	constexpr OutputIterator
	makeJoin (JoinType join_type,
			InputIterator1 first1, InputIterator1 last1,
			InputIterator2 first2, InputIterator2 last2,
			OutputIterator result
			, [[maybe_unused]] Compare comp = std::less<>{})
	{
		switch (join_type) {
		case JoinType::Inner : {
			return inner
					<InputIterator1, InputIterator2, OutputIterator, ResultContainer, Compare>
					(first1, last1, first2, last2, result, comp);
		}
		case JoinType::OuterFull : {
			return outerFull
					<InputIterator1, InputIterator2, OutputIterator, ResultContainer, Compare>
					(first1, last1, first2, last2, result, comp);
		}
		case JoinType::OuterExcluding : {
			return outerExcluding
					<InputIterator1, InputIterator2, OutputIterator, ResultContainer, Compare>
					(first1, last1, first2, last2, result, comp);
		}
		case JoinType::LeftFullOuter : {
			return sideFull
					<InputIterator1, InputIterator2, OutputIterator, ResultContainer, Compare>
					(first1, last1, first2, last2, result, comp);
		}
		case JoinType::LeftExcluding : {
			return sideExcluding
					<InputIterator1, InputIterator2, OutputIterator, ResultContainer, Compare>
					(first1, last1, first2, last2, result, comp);
		}
		case JoinType::RightFullOuter : {
			return sideFull
					<InputIterator1, InputIterator2, OutputIterator, ResultContainer, Compare>
					(first1, last1, first2, last2, result, comp);
		}
		case JoinType::RightExcluding : {
			return sideExcluding
					<InputIterator1, InputIterator2, OutputIterator, ResultContainer, Compare>
					(first1, last1, first2, last2, result, comp);
		}
		case JoinType::None: {
			/* do nothing */
			//todo: add exception?
		}
		default: {
			//todo: add exception?
			//having it here to suppress the warning
			return result;
		}
		}
	}

	template<typename Serie1, typename Serie2>
	void
	operationOverPair(JoinType join_type, Serie1 &&serie_1, Serie2 &&serie_2)
	{
		//there is no guarantee resulting container is empty, therefore it is end(), not begin()
		using Result = std::remove_reference_t<Serie1>;
		Result result;
		auto res_iter = std::inserter(result, result.end());

		bool const ascending1{itertools::isSortedAscending(serie_1)};
		bool const ascending2{itertools::isSortedAscending(serie_2)};

#define CALL_OPERATION(first_cond, second_cond) \
    auto [b1, e1] = itertools::getIterators<Serie1, first_cond>(serie_1); \
    auto [b2, e2] = itertools::getIterators<Serie2, second_cond>(serie_2); \
    makeJoin <decltype(b1), decltype(b2), decltype(res_iter), Result> (join_type, b1, e1, b2, e2, res_iter);

		if (ascending1 && ascending2) { CALL_OPERATION(true, true) }
		else if (ascending1 && !ascending2) { CALL_OPERATION(true, false) }
		else if (!ascending1 && ascending2) { CALL_OPERATION(false, true) }
		else if (!ascending1 && !ascending2) { CALL_OPERATION(false, false) }

#undef CALL_OPERATION
		serie_1 = std::move(result);
	}

	template <typename Serie, typename... SerieArgs, std::size_t ...Is>
	void
	operationOverTuple (
			JoinType join_type
			, std::tuple<SerieArgs...> &curr_result
			, Serie &serie
			, std::index_sequence<Is...>
	) {
		(operationOverPair(
				join_type, std::get<Is>(curr_result), std::forward<Serie>(serie)), ...);
	}


	template<std::size_t I = 0, typename SerieRet, typename... SerieArgs>
	auto
	callOperationImp(JoinType join_type, SerieRet&& curr_result, std::tuple<SerieArgs...>&& args)
	{
		if constexpr (I==0) {
			auto result_new = tupletools::tuplePushBack(
					std::forward<SerieRet>(curr_result),
					std::forward<decltype(std::get<I>(args))>(std::get<I>(args)));

			return callOperationImp<I+1, decltype(result_new), SerieArgs...>(
					join_type
					, std::move(result_new)
					, std::forward<std::tuple<SerieArgs...>>(args)
			);
		}
		else if constexpr (I!=sizeof...(SerieArgs)) {

			//filter current tuple of results with the Arg[i]
			operationOverTuple(
					join_type
					, curr_result
					, std::get<I>(args)
					, std::make_index_sequence<std::tuple_size_v<SerieRet>>{}
			);

			//append tuple of results with updated new element
			auto result_new = tupletools::tuplePushBack(
					std::move(curr_result),
					std::move(std::get<I>(args)));

#if 0
			//get new elem of tuple of results
			using Serie = std::remove_reference_t<decltype(std::get<I>(args))>;
			Serie appending_tuple_with = std::move(std::get<I>(args));


			//filter new element with existing tuple of results
			JoinType operation_appending;
			operation_appending = JoinType::Inner; //todo: seems like it doesn't work here
			operationOverPair(
					operation_appending
					, appending_tuple_with
					, std::get<0>(curr_result)
			);

			//append tuple of results with updated new element
			auto result_new = tupletools::tuplePushBack(
					std::move(curr_result),
					std::move(appending_tuple_with));
#endif
			//continue recursion
			using NewSerieRet = decltype(result_new);
			return callOperationImp<I+1, NewSerieRet, SerieArgs...>(
					join_type
					, std::move(result_new)
					, std::forward<std::tuple<SerieArgs...>>(args)
			);
		}
		else if constexpr (I == sizeof...(SerieArgs)) {
			//update the last tuple element with the tuple
			operationOverTuple(
					join_type
					, curr_result
					, std::get<std::tuple_size_v<SerieRet> - 1>(curr_result)
					, std::make_index_sequence<std::tuple_size_v<SerieRet> - 1>{}
			);

			//move is required, as serie_ret is an argument, which is not guaranteed to have RVO
			return std::move(curr_result);
		}
	}

	template<typename... SerieArgs>
	auto callOperation(JoinType join_type, SerieArgs&& ...series) {
		std::tuple t;
		if constexpr (sizeof...(SerieArgs) == 0u) {
			return t;
		}
		else if constexpr (sizeof...(SerieArgs) == 1u) {
			return tupletools::tuplePushBack(t, std::forward<SerieArgs>(series)...);
		}
		else {
			auto args = std::make_tuple(std::forward<SerieArgs>(series)...);
			if (join_type == JoinType::RightExcluding || join_type == JoinType::RightFullOuter) {
				args = tupletools::reverseTuple(std::move(args));
				auto res = callOperationImp(join_type, std::move(t), std::move(args));
				return tupletools::reverseTuple(std::move(res));
			}

			return callOperationImp(join_type, std::move(t), std::move(args));
		}
	}

  }//!namespace


  template<typename... SerieArgs>
  auto outerFull(SerieArgs&& ...series)
  {
	  return
			  details::callOperation<SerieArgs...>(
					  details::JoinType::OuterFull,
					  std::forward<SerieArgs>(series)...);
  }

  template<typename... SerieArgs>
  auto outerExcluding(SerieArgs&& ...series)
  {
	  return
			  details::callOperation<SerieArgs...>(
					  details::JoinType::OuterExcluding,
					  std::forward<SerieArgs>(series)...);
  }

  template<typename... SerieArgs>
  auto inner(SerieArgs&& ...series)
  {
	  return
			  details::callOperation<SerieArgs...>(
					  details::JoinType::Inner,
					  std::forward<SerieArgs>(series)...);
  }

  template<typename... SerieArgs>
  auto leftOuter(SerieArgs&& ...series)
  {
	  return
			  details::callOperation<SerieArgs...>(
					  details::JoinType::LeftFullOuter,
					  std::forward<SerieArgs>(series)...);
  }

  template<typename... SerieArgs>
  auto leftExcluding(SerieArgs&& ...series)
  {
	  return
			  details::callOperation<SerieArgs...>(
					  details::JoinType::LeftExcluding,
					  std::forward<SerieArgs>(series)...);
  }

  template<typename... SerieArgs>
  auto rightOuter(SerieArgs&& ...series)
  {
	  return
			  details::callOperation<SerieArgs...>(
					  details::JoinType::RightFullOuter,
					  std::forward<SerieArgs>(series)...);
  }

  template<typename... SerieArgs>
  auto rightExcluding(SerieArgs&& ...series)
  {
	  return
			  details::callOperation<SerieArgs...>(
					  details::JoinType::RightExcluding,
					  std::forward<SerieArgs>(series)...);
  }

}//!namespace


#endif //DATA_JOINS_H