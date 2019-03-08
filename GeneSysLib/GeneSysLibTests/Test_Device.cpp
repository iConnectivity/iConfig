/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Device testcases
#include <boost/test/unit_test.hpp>
#include "Device.h"
#include "Device.cpp"

using namespace GeneSysLib;


// Test that the default constructor zero fills all variables
BOOST_AUTO_TEST_CASE(construct_1) {
  Device dev;
  BOOST_CHECK_EQUAL(dev.protocol, 0);
  BOOST_CHECK_EQUAL(dev.mode, 0);
  BOOST_CHECK_EQUAL(dev.maxLength, 0);
}

// Test that the specialized constructor sets variable correctly
BOOST_AUTO_TEST_CASE(construct_2) {
  Device dev(0x7F, 0x6F, 0x6FFF);
  BOOST_CHECK_EQUAL(dev.protocol, 0x7F);
  BOOST_CHECK_EQUAL(dev.mode, 0x6F);
  BOOST_CHECK_EQUAL(dev.maxLength, 0x6FFF);
}

