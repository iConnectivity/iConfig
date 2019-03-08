/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __COMMANDDATA_H__
#define __COMMANDDATA_H__

#include "LibTypes.h"
#include "CommandDataKey.h"
#ifndef Q_MOC_RUN
#include <boost/scoped_ptr.hpp>
#endif

namespace GeneSysLib {

template <typename T>
const commandDataKey_t key(const T &x) {
  return x.key();
}

template <typename T>
Bytes generate(const T &x) {
  return x.generate();
}

template <typename T>
void parse(T &t, BytesIter &b, BytesIter &e) {
  t.parse(b, e);
}

struct commandData_t {
  template <typename T>
  commandData_t(T x)
      : self_(new model_t<T>(x)) {}

  commandData_t() : self_(0) {}

  commandData_t(commandData_t const &that)
      : self_(that.self_ ? that.self_->clone() : 0) {}

  friend void swap(commandData_t &lhs, commandData_t &rhs) {
    boost::swap(lhs.self_, rhs.self_);
  }

  bool empty() const { return (bool)self_; }

  commandData_t &operator=(commandData_t rhs) {
    swap(*this, rhs);
    return *this;
  }

  template <typename T>
  commandData_t &operator=(T const &x) {
    self_.reset(new model_t<T>(x));
    return *this;
  }

  // Forwarding functions
  friend const commandDataKey_t key(const commandData_t &x) {
    return x.self_->key();
  }
  const commandDataKey_t key() const { return self_->key(); }

  friend Bytes generate(const commandData_t &x) { return x.self_->generate(); }
  Bytes generate() const { return self_->generate(); }

  friend void parse(commandData_t &x, BytesIter &b, BytesIter &e) {
    x.self_->parse(b, e);
  }

  void parse(BytesIter &b, BytesIter &e) { self_->parse(b, e); }

  template <typename T>
  const T &get() const {
    assert(self_);
    assert(typeid(T) == self_->type());
    return static_cast<model_t<T> *>(self_.get())->data_;
  }

  template <typename T>
  T &get() {
    assert(self_);
    assert(typeid(T) == self_->type());
    return static_cast<model_t<T> *>(self_.get())->data_;
  }

 private:
  struct concept_t {
    virtual ~concept_t() {}
    virtual concept_t *clone() const = 0;
    virtual const std::type_info &type() const = 0;

    // Forwarding functions
    virtual const commandDataKey_t key() const = 0;
    virtual Bytes generate() const = 0;
    virtual void parse(BytesIter &b, BytesIter &e) = 0;
  };

  template <typename T>
  struct model_t : concept_t {
    model_t(T const &x) : data_(x) {}

    virtual model_t *clone() const { return new model_t(data_); }

    virtual const commandDataKey_t key() const {
      return GeneSysLib::key(data_);
    }
    virtual Bytes generate() const { return GeneSysLib::generate(data_); }
    virtual void parse(BytesIter &b, BytesIter &e) {
      return GeneSysLib::parse(data_, b, e);
    }

    virtual const std::type_info &type() const { return typeid(T); }

    T data_;
  };

  template <typename ValueType>
  friend ValueType *command_cast(concept_t *);

  boost::scoped_ptr<concept_t> self_;
};  // struct commandData_t

struct EmptyCommandData {
  const commandDataKey_t key() const { return generateKey(Command::Unknown); }
  Bytes generate() const { return Bytes(); }
  void parse(BytesIter, BytesIter) {}
};  // struct EmptyCommandData

}  // namespace GeneSysLib

#endif  // __COMMANDDATA_H__
