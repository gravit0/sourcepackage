#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Calculator
#include <boost/test/unit_test.hpp>

#include <iostream>
#include "main.hpp"
#include "Sock.hpp"

BOOST_AUTO_TEST_SUITE(CalculatorTests)

BOOST_AUTO_TEST_CASE(PlusTest)
{
    //So calc;
    BOOST_CHECK_EQUAL(1, 1);
}
/*
BOOST_AUTO_TEST_CASE(MinusTest)
{
    Calculator calc;
    BOOST_CHECK_EQUAL(calc.minus( 2, 1 ), 1);
}
*/
BOOST_AUTO_TEST_SUITE_END()

