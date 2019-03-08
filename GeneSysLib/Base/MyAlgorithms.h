/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __MYALGORITHMS_H__
#define __MYALGORITHMS_H__

#include "LibTypes.h"
#include <map>
#ifndef Q_MOC_RUN
#include <boost/tr1/unordered_map.hpp>
#endif

namespace MyAlgorithms {

template <class InputIterator, class UnaryPredicate>
bool all_of(InputIterator first, InputIterator last, UnaryPredicate pred) {
  while (first != last) {
    if (!pred(*first)) return false;
    ++first;
  }
  return true;
}

template <class InputIterator, class UnaryPredicate>
bool none_of(InputIterator first, InputIterator last, UnaryPredicate pred) {
  while (first != last) {
    if (pred(*first)) return false;
    ++first;
  }
  return true;
}

template <class Key, class T>
bool contains(const std::map<Key, T>& map, const Key& key) {
  return map.find(key) != map.end();
}

template <class Key, class T>
bool contains(const std::tr1::unordered_map<Key, T>& map, const Key& key) {
  return map.find(key) != map.end();
}

template <class T>
bool contains(const std::vector<T>& vec, const T& val) {
  return count(vec.begin(), vec.end(), val) > 0;
}

template <class T>
bool contains(const std::list<T>& l, const T& val) {
  return count(l.begin(), l.end(), val) > 0;
}

template <class T>
bool contains(const std::set<T>& s, const T& val) {
  return count(s.begin(), s.end(), val) > 0;
}

template <class Container, typename VariableType>
void memberSet(Container& container, VariableType Container::*variable,
               VariableType value) {
  container.*variable = value;
}

template <class Container, typename VariableType>
VariableType memberGet(Container& container,
                       VariableType Container::*variable) {
  return container.*variable;
}

void setBit(Bytes& bytes, size_t byte, Byte bit, bool value);

bool getBit(const Bytes& bytes, size_t byte, Byte bit);

template <class InputIterator, class Predicate, class Function>
Function for_each_if(InputIterator first, InputIterator last, Predicate pred,
                     Function fn) {
  while (first != last) {
    if (pred(*first)) {
      fn(*first);
    }
    ++first;
  }
  return fn;
}

}  // namespace MyAlgorithms

#endif  // __MYALGORITHMS_H__
