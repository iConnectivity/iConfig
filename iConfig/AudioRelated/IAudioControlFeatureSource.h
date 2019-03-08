/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __IAUDIOCONTROLFEATURESOURCE_H__
#define __IAUDIOCONTROLFEATURESOURCE_H__

#include "LibTypes.h"
#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#endif

/* This is an interface class used to help develop the audio control feature
 * units. It is used to help develop the audio control view.
 */
struct IAudioControlFeatureSource {
  static const double VolumedBConversionFactor;// = 1.0 / 256.0;

  // Controller features
  virtual std::string controllerName() const = 0;
  virtual Byte numChannels() const = 0;

  // Channel name
  virtual std::string channelName(Byte channelID) const = 0;

  virtual int8_t channelType(Byte channelID) const = 0;
  virtual int8_t channelNumber(Byte channelID) const = 0;

  // Stereo link methods
  virtual bool isStereoLinkAvailable(Byte channelID) const = 0;
  virtual bool isStereoLinkEditable(Byte channelID) const = 0;
  virtual bool stereoLink(Byte channelID) const = 0;
  virtual void stereoLink(Byte channelID, bool value) = 0;

  // High impedence methods
  virtual bool isHighImpedanceAvailable(Byte channelID) const = 0;
  virtual bool isHighImpedanceEditable(Byte channelID) const = 0;
  virtual bool highImpedance(Byte channelID) const = 0;
  virtual void highImpedance(Byte channelID, bool value) = 0;

  // Phantom power methods
  virtual bool isPhantomPowerAvailable(Byte channelID) const = 0;
  virtual bool isPhantomPowerEditable(Byte channelID) const = 0;
  virtual bool phantomPower(Byte channelID) const = 0;
  virtual void phantomPower(Byte channelID, bool value) = 0;

  // Mute methods
  virtual bool isMuteAvailable(Byte channelID) const = 0;
  virtual bool isMuteEditable(Byte channelID) const = 0;
  virtual bool mute(Byte channelID) const = 0;
  virtual void mute(Byte channelID, bool value) = 0;

  // Volume methods
  virtual bool isVolumeAvailable(Byte channelID) const = 0;
  virtual bool isVolumeEditable(Byte channelID) const = 0;
  virtual int16_t volumeMin(Byte channelID) const = 0;
  virtual int16_t volumeMax(Byte channelID) const = 0;
  virtual int16_t volumeResolution(Byte channelID) const = 0;
  virtual int16_t volumePad(Byte channelID) const = 0;
  virtual int16_t trimMin(Byte channelID) const = 0;
  virtual int16_t trimMax(Byte channelID) const = 0;
  virtual int16_t volumeCurrent(Byte channelID) const = 0;
  virtual void volumeCurrent(Byte channelID, int16_t value) = 0;
  virtual int16_t trimCurrent(Byte channelID) const = 0;
  virtual void trimCurrent(Byte channelID, int16_t value) = 0;

  virtual int16_t meterCurrent(Byte channelID) = 0;

};  // struct IAudioControlFeatureSource

typedef boost::shared_ptr<IAudioControlFeatureSource>
    IAudioControlFeatureSourcePtr;

#endif  // __IAUDIOCONTROLFEATURESOURCE_H__
