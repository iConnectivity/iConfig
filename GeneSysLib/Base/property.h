/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef PROPERTY_H
#define PROPERTY_H

#include <string>
#include <vector>
#ifndef Q_MOC_RUN
#include <boost/array.hpp>
#include <boost/function.hpp>
#endif

typedef boost::array<unsigned char, 5> SerialNumber;

typedef boost::array<unsigned char, 4> NetAddr;

template <typename T>
struct readwrite_property {
  typedef boost::function<T(void)> test_point;
  explicit readwrite_property() : prop(), min(), max() {}

  explicit readwrite_property(const T& t) : prop(t), min(), max() {}

  explicit readwrite_property(test_point _min, test_point _max)
      : prop(), min(_min), max(_max) {}

  explicit readwrite_property(const T& t, test_point _min, test_point _max)
      : prop(t), min(_min), max(_max) {}

  T& operator()(void) { return prop; }
  const T& operator()(void) const { return prop; }

  void operator()(T t) {
    prop = t;
    if ((min) && (t < min())) {
      prop = min();
    }
    if ((max) && (t > max())) {
      prop = max();
    }
  }

 private:
  T prop;
  test_point min;
  test_point max;
};

template <typename T>
struct readonly_property {
  explicit readonly_property() : prop() {}
  explicit readonly_property(const T& t) : prop(t) {}

  const T& operator()(void) const { return prop; }

 private:
  T prop;
};

typedef readonly_property<bool> roBool;
typedef readwrite_property<bool> rwBool;

typedef readonly_property<uint8_t> roByte;
typedef readwrite_property<uint8_t> rwByte;

typedef readonly_property<uint16_t> roWord;
typedef readwrite_property<uint16_t> rwWord;

typedef readonly_property<std::string> roString;
typedef readwrite_property<std::string> rwString;

typedef readonly_property<NetAddr> roNetAddr;
typedef readwrite_property<NetAddr> rwNetAddr;

typedef readonly_property<SerialNumber> roSerialNumber;
typedef readwrite_property<SerialNumber> rwSerialNumber;

#endif  // PROPERTY_H
