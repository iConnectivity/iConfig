/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __PATCHBAY_SOURCE_H__
#define __PATCHBAY_SOURCE_H__

#include <QString>
#include <map>
#ifndef Q_MOC_RUN
#include <boost/function.hpp>
#endif

typedef std::pair<int, int> device_port_t;
typedef std::map<device_port_t, device_port_t> channel_map_t;
typedef std::pair<device_port_t, device_port_t> device_pair_t;

struct IAudioPatchbaySource {
  virtual void init(const boost::function<void(void)> &refreshCallback) = 0;
  virtual int numSections() const = 0;

  virtual int numInputsPerSection(int section) const = 0;
  virtual int numOutputsPerSection(int section) const = 0;

  int numInputs() const;
  int numOutputs() const;

  int inputIndexToTotal(device_port_t targetIndex) const;
  device_port_t inputTotalToIndex(int totalInput) const;

  int outputIndexToTotal(device_port_t targetIndex) const;
  device_port_t outputTotalToIndex(int totalOutput) const;

  virtual QString nameForSection(int section) const = 0;

  virtual bool isPatched(device_port_t src, device_port_t dest) const = 0;
  virtual void setPatch(device_port_t src, device_port_t dest, device_port_t toRemove = std::make_pair(0,0)) = 0;

  virtual void toggleCollapseInput(int section) = 0;
  virtual bool isCollapsedInput(int section) const = 0;

  virtual void toggleCollapseOutput(int section) = 0;
  virtual bool isCollapsedOutput(int section) const = 0;

  virtual bool isMixerToo() const = 0;

  virtual void for_each_mixer(
      const boost::function<void(const device_pair_t &)> &func) const = 0;
  virtual void for_each(
      const boost::function<void(const device_pair_t &)> &func) const = 0;
};  // struct IAudioPatchbaySource

#endif  // __PATCHBAY_SOURCE_H__
