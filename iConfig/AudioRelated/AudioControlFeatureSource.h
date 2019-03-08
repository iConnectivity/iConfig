/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOCONTROLFEATURESOURCE_H__
#define __AUDIOCONTROLFEATURESOURCE_H__

#include "IAudioControlFeatureSource.h"
#include "DeviceInfo.h"
#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#endif

struct AudioControlFeatureSource : public IAudioControlFeatureSource {
  // Constructor/destructor
  AudioControlFeatureSource(DeviceInfoPtr device, Word audioPortID,
                            Byte controllerNumber);
  virtual ~AudioControlFeatureSource();

  // Controller features
  std::string controllerName() const;
  Byte numChannels() const;

  // Channel nam
  std::string channelName(Byte channelID) const;

  int8_t channelType(Byte channelID) const;
  int8_t channelNumber(Byte channelID) const;

  // Stereo Link methods
  bool isStereoLinkAvailable(Byte channelID) const;
  bool isStereoLinkEditable(Byte channelID) const;
  bool stereoLink(Byte channelID) const;
  void stereoLink(Byte channelID, bool value);

  // High impedence methods
  bool isHighImpedanceAvailable(Byte channelID) const;
  bool isHighImpedanceEditable(Byte channelID) const;
  bool highImpedance(Byte channelID) const;
  void highImpedance(Byte channelID, bool value);

  // Phantom power methods
  bool isPhantomPowerAvailable(Byte channelID) const;
  bool isPhantomPowerEditable(Byte channelID) const;
  bool phantomPower(Byte channelID) const;
  void phantomPower(Byte channelID, bool value);

  // Mute methods
  bool isMuteAvailable(Byte channelID) const;
  bool isMuteEditable(Byte channelID) const;
  bool mute(Byte channelID) const;
  void mute(Byte channelID, bool value);

  // Volume methods
  bool isVolumeAvailable(Byte channelID) const;
  bool isVolumeEditable(Byte channelID) const;
  int16_t volumeMin(Byte channelID) const;
  int16_t volumeMax(Byte channelID) const;
  int16_t volumeResolution(Byte channelID) const;
  int16_t volumePad(Byte channelID) const;
  int16_t volumeCurrent(Byte channelID) const;
  int16_t trimCurrent(Byte channelID) const;
  int16_t trimMin(Byte channelID) const;
  int16_t trimMax(Byte channelID) const;
  void volumeCurrent(Byte channelID, int16_t value);
  void trimCurrent(Byte channelID, int16_t value);

  int16_t meterCurrent(Byte channelID);

 private:
  DeviceInfoPtr device;
  Word audioPortID;
  Byte controllerNumber;
};

typedef boost::shared_ptr<AudioControlFeatureSource>
    AudioControlFeatureSourcePtr;

#endif  // __AUDIOCONTROLFEATURESOURCE_H__
