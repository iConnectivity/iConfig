/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIXEROUTPUTINTERFACE_H
#define MIXEROUTPUTINTERFACE_H

#include "DeviceInfo.h"

struct MixerOutputInterface {
  // Constructor/destructor
  MixerOutputInterface(DeviceInfoPtr device);
  virtual ~MixerOutputInterface();

  // Invert methods
  bool isInvertAvailable(Word audioPortID) const;
  bool isInvertEditable(Word audioPortID) const;
  bool invert(Word audioPortID, Byte outChannelID) const;
  void invert(Word audioPortID, Byte outChannelID, bool value);

  // Stereo Link methods
  bool isStereoLinkAvailable(Word audioPortID) const;
  bool isStereoLinkEditable(Word audioPortID) const;
  bool stereoLink(Word audioPortID, Byte outChannelID) const;
  void stereoLink(Word audioPortID, Byte outChannelID, bool value);

  // Solo PFL methods
  bool isSoloPFLAvailable(Word audioPortID) const;
  bool isSoloPFLEditable(Word audioPortID) const;
  bool soloPFL(Word audioPortID, Byte outChannelID) const;
  void soloPFL(Word audioPortID, Byte outChannelID, bool value);

  // Solo methods
  bool isSoloAvailable(Word audioPortID) const;
  bool isSoloEditable(Word audioPortID) const;
  int16_t solo(Word audioPortID, Byte outChannelID) const;
  void solo(Word audioPortID, Byte outChannelID, int16_t value);

  // Mute methods
  bool isMuteAvailable(Word audioPortID) const;
  bool isMuteEditable(Word audioPortID) const;
  bool mute(Word audioPortID, Byte outChannelID) const;
  void mute(Word audioPortID, Byte outChannelID, bool value);

  // Volume methods
  bool isVolumeAvailable(Word audioPortID) const;
  bool isVolumeEditable(Word audioPortID) const;
  int16_t volumeMin(Word audioPortID) const;
  int16_t volumeMax(Word audioPortID) const;
  int16_t volumeResolution(Word audioPortID) const;
  int16_t volumeCurrent(Word audioPortID, Byte outChannelID) const;
  void volumeCurrent(Word audioPortID, Byte outChannelID, int16_t value);

  int16_t meterCurrent(Word audioPortID, Byte outChannelID);

  // Pan methods
  bool isPanAvailable(Word audioPortID) const;
  bool isPanEditable(Word audioPortID) const;
  int16_t panMin(Word audioPortID) const;
  int16_t panMax(Word audioPortID) const;
  int16_t panCurrent(Word audioPortID, Byte outChannelID) const;
  void panCurrent(Word audioPortID, Byte outChannelID, int16_t value);

  int8_t numPanCurveLaws(Word audioPortID) const;
  int8_t panCurveLawAt(Word audioPortID, Byte whichCurveLaw) const;
  int8_t panCurveLawCurrent(Word audioPortID, Byte outChannelID) const;
  void panCurveLawCurrent(Word audioPortID, Byte outChannelID, int8_t value);

 private:
  DeviceInfoPtr device;
};

#endif // MIXEROUTPUTINTERFACE_H

