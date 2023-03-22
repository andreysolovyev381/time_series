//
// Created by Andrey Solovyev on 16/02/2023.
//

#include <gtest/gtest.h>

#include "common_usage_library/types_requirements/ctor_input.h"
#include "common_usage_library/types_requirements/container.h"
#include "common_usage_library/types_requirements/hash.h"
#include "common_usage_library/types_requirements/comparator.h"
#include "time_series/element.hpp"

#include <functional>
#include <vector>
#include <map>
#include <deque>
#include <set>
#include <string>
#include <variant>


using namespace culib;

template <typename T>
struct S {
	T value {42};
	operator double () {return static_cast<double>(value);}
};

template <typename T>
struct Container_ok {
	T begin() const { return 42; }
	T end() const { return 42; }
	T value{42};
};

template <typename T>
struct Container_not_ok {
	T value{42};
};


TEST(BasicsTypesRequirements, CtorInputOk) {
	bool ok = requirements::isConveribleOrConstructible_v<double, int>();
	ASSERT_TRUE(ok);
	ok = requirements::isConveribleOrConstructible_v<int, double>();
	ASSERT_TRUE(ok);
	ok = requirements::isConveribleOrConstructible_v<int, bool>();
	ASSERT_TRUE(ok);
	ok = requirements::isConveribleOrConstructible_v<S<int>, double>();
	ASSERT_TRUE(ok);
}

TEST(BasicsTypesRequirements, CtorInputNotOk) {
	bool ok = requirements::isConveribleOrConstructible_v<std::string, int>();
	ASSERT_FALSE(ok);
	ok = requirements::isConveribleOrConstructible_v<double, S<int>>();
	ASSERT_FALSE(ok);
	ok = requirements::isConveribleOrConstructible_v<bool, std::string>();
	ASSERT_FALSE(ok);
}


TEST(BasicsTypesRequirements, ContainerOk) {
	using Elem = time_series::Element<base::Seconds, int>;
	using Map = std::map<base::Timestamp<base::Seconds>, time_series::Value<time_series::value::traits::ValueTypeDefault>>;
	ASSERT_TRUE(requirements::isContainer_v<std::string>());
	ASSERT_TRUE(requirements::isContainer_v<std::vector<Elem>>());
	ASSERT_TRUE(requirements::isContainer_v<Map>());
	ASSERT_TRUE(requirements::isContainer_v<std::deque<Elem>>());
	ASSERT_TRUE(requirements::isContainer_v<std::set<Elem>>());
	ASSERT_TRUE(requirements::isContainer_v<Container_ok<Elem>>());
}

TEST(BasicsTypesRequirements, ContainerNotOk) {
	using Elem = time_series::Element<base::Seconds, int>;
	ASSERT_FALSE(requirements::isContainer_v<double>());
	ASSERT_FALSE(requirements::isContainer_v<int>());
	ASSERT_FALSE(requirements::isContainer_v<Container_not_ok<Elem>>());
	ASSERT_FALSE(requirements::isContainer_v<std::variant<Elem>>());
}


struct S1 {int value {42};};
struct SHasher {
	std::size_t operator () (const S1& s) const {
		return s.value;
	}
};
bool operator == (const S1& lhs, const S1& rhs) {
	return lhs.value == rhs.value;
}
struct SComparator {
	bool operator () (const S1& lhs, const S1& rhs) const {
		return lhs.value < rhs.value;
	}
};


TEST (BasicsTypesRequirements, TypeIsHash) {
	bool result = requirements::isHash_v<S1, SHasher>();
	ASSERT_TRUE(result);
	result = requirements::isHash_v<S1, SComparator>();
	ASSERT_FALSE(result);
	result = requirements::isHash_v<S1, double>();
	ASSERT_FALSE(result);
	std::hash<int> hi;
	result = requirements::isHash_v<int, decltype(hi)>();
	ASSERT_TRUE(result);
}


TEST (BasicsTypesRequirements, TypeIsComparator) {
	bool result = requirements::isComparator_v<S1, SComparator>();
	ASSERT_TRUE(result);
	result = requirements::isComparator_v<S1, SHasher>();
	ASSERT_FALSE(result);
	result = requirements::isComparator_v<S1, double>();
	ASSERT_FALSE(result);
	result = requirements::isComparator_v<S1, std::less<S1>>();
	ASSERT_TRUE(result);
}


#ifdef __cpp_concepts

TEST (BasicsTypesRequirements, ConceptsContainerOk) {
	using Elem = time_series::Element<base::Seconds, int>;
	using Map = std::map<base::Timestamp<base::Seconds>, time_series::Value<time_series::value::traits::ValueTypeDefault>>;
	ASSERT_TRUE(requirements::isContainer_v_<std::string>());
	ASSERT_TRUE(requirements::isContainer_v_<std::vector<Elem>>());
	ASSERT_TRUE(requirements::isContainer_v_<Map>());
	ASSERT_TRUE(requirements::isContainer_v_<std::deque<Elem>>());
	ASSERT_TRUE(requirements::isContainer_v_<std::set<Elem>>());
	ASSERT_TRUE(requirements::isContainer_v_<Container_ok<Elem>>());
}

TEST(BasicsTypesRequirements, ConceptsContainerNotOk) {
	using Elem = time_series::Element<base::Seconds, int>;
	ASSERT_FALSE(requirements::isContainer_v<double>());
	ASSERT_FALSE(requirements::isContainer_v<int>());
	ASSERT_FALSE(requirements::isContainer_v<Container_not_ok<Elem>>());
	ASSERT_FALSE(requirements::isContainer_v<std::variant<Elem>>());
}


#endif
