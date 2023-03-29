#include <boost/test/unit_test.hpp>
#include "relation/value/date.h"
#include "relation/value/value.h"

using namespace query_process_engine;

BOOST_AUTO_TEST_CASE(test_interger_comparasion) {
  Integer n1(2);
  Integer n2(3);
  BOOST_CHECK(n1 < n2);
  BOOST_CHECK(n1 <= n2);
  BOOST_CHECK(n2 >= n1);
  BOOST_CHECK(n2 > n1);
  BOOST_CHECK(n1 != n2);
  BOOST_CHECK(n2 != n1);
  Integer n3(2);
  BOOST_CHECK(n1 == n3);
  BOOST_CHECK(n2 != n3);
}

BOOST_AUTO_TEST_CASE(test_float_comparasion) {
  Float n1(2.5);
  Float n2(12.5);
  BOOST_CHECK(n1 < n2);
  BOOST_CHECK(n1 <= n2);
  BOOST_CHECK(n2 >= n1);
  BOOST_CHECK(n2 > n1);
  BOOST_CHECK(n1 != n2);
  BOOST_CHECK(n2 != n1);
  Float n3(12.5);
  BOOST_CHECK(n2 == n3);
  BOOST_CHECK(n1 != n3);
}

BOOST_AUTO_TEST_CASE(test_string_comparasion) {
  String s1("\"abc\"");
  String s2("\"abd\"");
  BOOST_CHECK(std::string("abc") < std::string("abd"));
  BOOST_CHECK(s1 < s2);
  BOOST_CHECK(s1 <= s2);
  BOOST_CHECK(s2 >= s1);
  BOOST_CHECK(s2 > s1);
  BOOST_CHECK(s1 != s2);
  BOOST_CHECK(s2 != s1);
  String s3("\"abd\"");
  BOOST_CHECK(s2 == s3);
  BOOST_CHECK(s1 != s2);
}

BOOST_AUTO_TEST_CASE(test_date_valid) {
  // length check
  BOOST_CHECK(!Date::is_valid_date("2019-01-021"));
  // invalid char
  BOOST_CHECK(!Date::is_valid_date("----------"));
  BOOST_CHECK(!Date::is_valid_date("102-01-022"));
  // invalid month
  BOOST_CHECK(!Date::is_valid_date("2324-13-01"));
  // invalid date and bound check
  BOOST_CHECK(!Date::is_valid_date("2019-01-32"));
  BOOST_CHECK(Date::is_valid_date("2019-01-31"));
  BOOST_CHECK(!Date::is_valid_date("2019-01-00"));
  BOOST_CHECK(!Date::is_valid_date("2019-02-30"));
  BOOST_CHECK(!Date::is_valid_date("2019-03-33"));
  BOOST_CHECK(!Date::is_valid_date("2019-04-31"));
  BOOST_CHECK(!Date::is_valid_date("2019-05-51"));
  BOOST_CHECK(!Date::is_valid_date("2019-06-31"));
  // special leak year
  BOOST_CHECK(Date::is_valid_date("2020-02-29"));
  BOOST_CHECK(!Date::is_valid_date("2019-02-29"));
  BOOST_CHECK(!Date::is_valid_date("1900-02-29"));
  BOOST_CHECK(!Date::is_valid_date("1900-02-00"));
  BOOST_CHECK(Date::is_valid_date("1900-02-28"));
  BOOST_CHECK(Date::is_valid_date("2029-01-12"));
}

BOOST_AUTO_TEST_CASE(test_date_comparasion) {
  Date d1("2029-01-12");
  Date d2("2029-02-01");
  BOOST_CHECK(d1 < d2);
  BOOST_CHECK(d1 <= d2);
  BOOST_CHECK(d2 > d1);
  BOOST_CHECK(d2 >= d1);
  BOOST_CHECK(d1 != d2);
  Date d3("2029-01-12");
  BOOST_CHECK(d1 == d3);
  BOOST_CHECK_EQUAL(d3.get_day(), 12);
  BOOST_CHECK_EQUAL(d3.get_month(), 1);
  BOOST_CHECK_EQUAL(d3.get_year(), 2029);
  BOOST_CHECK_EQUAL(d3.to_string(), "2029-01-12");
}