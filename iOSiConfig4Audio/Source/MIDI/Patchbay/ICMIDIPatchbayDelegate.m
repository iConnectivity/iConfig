/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICMIDIPatchbayDelegate.h"
#import "MIDIPortInfo.h"
#import "MIDIPortRoute.h"
#import "MIDIPortDetail.h"
#import "MIDIPortDetailTypes.h"
#import "MIDIPortRemap.h"
#import "MIDIPortFilter.h"
#import "MIDIInfo.h"
#import "MIDIEndPoint.h"
#import "PortType.h"

#include "CommandDefines.h"
#include "CommandList.h"

#import "MixerInterface.h"
#import "MixerPortParm.h"

using namespace GeneSysLib;

@interface ICMIDIPatchbayDelegate () {
  std::vector<bool> isCollapsedInputVec;
  std::vector<bool> isCollapsedOutputVec;

  Byte numDevicePorts;
  Byte numPorts;

  NSMutableArray *portsPerDevicePort;
  NSMutableArray *devicePortNames;

  std::map<int, GeneSysLib::ChannelFilterStatusBitEnum> rowChannelStatusMap;
  std::map<int, GeneSysLib::FilterStatusBitEnum> rowFilterMap;

  std::map<int, GeneSysLib::ChannelBitmapBitEnum> colChBitmapMap;

  std::multimap<GeneSysLib::FilterIDEnum, Word> portFilterUpdateList;
  std::multimap<RemapTypeEnum, Word> channelRemapUpdateList;
  std::multimap<GeneSysLib::FilterIDEnum, Word> controllerFilterUpdateList;
  std::multimap<RemapTypeEnum, Word> controllerRemapUpdateList;

  NSTimer* sendTimer;
  NSLock* sendLock;
}

@property(nonatomic) DeviceInfoPtr device;

@end

@implementation ICMIDIPatchbayDelegate

- (id)initWithDevice:(DeviceInfoPtr)device {
  self = [super init];

  //NSLog(@"patchbay: 1");
  if (self) {
    NSParameterAssert(device);
    self.device = device;

    //NSLog(@"in self");

    const auto &midiInfo = device->get<MIDIInfo>();
    //NSLog(@"audioInfo.numAudioPorts(): %i", audioInfo.numAudioPorts());

    portsPerDevicePort = [[NSMutableArray alloc] init];
    devicePortNames = [[NSMutableArray alloc] init];

    numPorts = midiInfo.numMIDIPorts();
    numDevicePorts = 0;

    sendTimer = nil;
    sendLock = [[NSLock alloc] init];

    for (int i = 1; i <= numPorts;) {
      const auto &portInfo = device->get<MIDIPortInfo>(i);
      if (portInfo.portType() == PortType::USBDevice) {
        i += midiInfo.numUSBMIDIPortPerDeviceJack();
        numDevicePorts++;
        [portsPerDevicePort addObject:[NSNumber numberWithInt:midiInfo.numUSBMIDIPortPerDeviceJack()]];
        [devicePortNames addObject:[NSString stringWithFormat:@"USB Device %d", portInfo.portInfo().usbDevice.jack]];
      } else if (portInfo.portType() == PortType::USBHost) {
        i += midiInfo.numUSBMIDIPortPerHostJack();
        numDevicePorts++;
        [portsPerDevicePort addObject:[NSNumber numberWithInt:midiInfo.numUSBMIDIPortPerHostJack()]];
        [devicePortNames addObject:[NSString stringWithFormat:@"USB Host %d", portInfo.portInfo().common.jack]];
      } else if (portInfo.portType() == PortType::Ethernet) {
        i += midiInfo.numRTPMIDISessionsPerEthernetJack();
        numDevicePorts++;
        [portsPerDevicePort addObject:[NSNumber numberWithInt:midiInfo.numRTPMIDISessionsPerEthernetJack()]];
        [devicePortNames addObject:[NSString stringWithFormat:@"Ethernet %d", portInfo.portInfo().common.jack]];
      } else if (portInfo.portType() == PortType::DIN) {
        i++;
        numDevicePorts++;
        [portsPerDevicePort addObject:[NSNumber numberWithInt:1]];
        [devicePortNames addObject:[NSString stringWithFormat:@"DIN %d", portInfo.portInfo().din.jack]];
      }
      else {
        //NSLog(@"MIDI PORT ERROR!");
        i++;
      }

      self->isCollapsedInputVec.push_back(false);
      self->isCollapsedOutputVec.push_back(false);

      // INITIALIZE Port Filter things

      rowChannelStatusMap[EventRows::PitchBendEvents] =
      ChannelFilterStatusBit::pitchBendEvents;
      rowChannelStatusMap[EventRows::ChannelPressureEvents] =
      ChannelFilterStatusBit::channelPressureEvents;
      rowChannelStatusMap[EventRows::ProgramChangeEvents] =
      ChannelFilterStatusBit::programChangeEvents;
      rowChannelStatusMap[EventRows::ControlChangeEvents] =
      ChannelFilterStatusBit::controlChangeEvents;
      rowChannelStatusMap[EventRows::PolyKeyPressureEvents] =
      ChannelFilterStatusBit::polyKeyPressureEvents;
      rowChannelStatusMap[EventRows::NoteEvents] =
      ChannelFilterStatusBit::noteEvents;

      rowFilterMap[EventRows::ResetEvents] = FilterStatusBit::resetEvents;
      rowFilterMap[EventRows::ActiveSensingEvents] =
      FilterStatusBit::activeSensingEvents;
      rowFilterMap[EventRows::RealtimeEvents] =
      FilterStatusBit::realtimeEvents;
      rowFilterMap[EventRows::TuneRequestEvents] =
      FilterStatusBit::tuneRequestEvents;
      rowFilterMap[EventRows::SongSelectEvents] =
      FilterStatusBit::songSelectEvents;
      rowFilterMap[EventRows::SongPositionPointerEvents] =
      FilterStatusBit::songPositionPointerEvents;
      rowFilterMap[EventRows::TimeCodeEvents] =
      FilterStatusBit::timeCodeEvents;
      rowFilterMap[EventRows::SystemExclusiveEvents] =
      FilterStatusBit::systemExclusiveEvents;

      // INITIALIZE CONTROLLER FILTER THINGS

      colChBitmapMap[0] = ChannelBitmapBit::channel1;
      colChBitmapMap[1] = ChannelBitmapBit::channel2;
      colChBitmapMap[2] = ChannelBitmapBit::channel3;
      colChBitmapMap[3] = ChannelBitmapBit::channel4;
      colChBitmapMap[4] = ChannelBitmapBit::channel5;
      colChBitmapMap[5] = ChannelBitmapBit::channel6;
      colChBitmapMap[6] = ChannelBitmapBit::channel7;
      colChBitmapMap[7] = ChannelBitmapBit::channel8;
      colChBitmapMap[8] = ChannelBitmapBit::channel9;
      colChBitmapMap[9] = ChannelBitmapBit::channel10;
      colChBitmapMap[10] = ChannelBitmapBit::channel11;
      colChBitmapMap[11] = ChannelBitmapBit::channel12;
      colChBitmapMap[12] = ChannelBitmapBit::channel13;
      colChBitmapMap[13] = ChannelBitmapBit::channel14;
      colChBitmapMap[14] = ChannelBitmapBit::channel15;
      colChBitmapMap[15] = ChannelBitmapBit::channel16;
    }
  }

  return self;
}

- (void) resetTimer {
  if (sendTimer != nil) {
    [sendTimer invalidate];
    sendTimer = nil;
  }
  sendTimer = [NSTimer scheduledTimerWithTimeInterval:2.0
                                   target:self
                                 selector:@selector(processSends)
                                 userInfo:nil
                                  repeats:NO];
}

- (void) processSends {
  std::multimap<GeneSysLib::FilterIDEnum, Word> lportFilterUpdateList = portFilterUpdateList;
  portFilterUpdateList.clear();

  std::multimap<RemapTypeEnum, Word> lchannelRemapUpdateList = channelRemapUpdateList;
  channelRemapUpdateList.clear();

  std::multimap<GeneSysLib::FilterIDEnum, Word> lcontrollerFilterUpdateList = controllerFilterUpdateList;
  controllerFilterUpdateList.clear();

  std::multimap<RemapTypeEnum, Word> lcontrollerRemapUpdateList = controllerRemapUpdateList;
  controllerRemapUpdateList.clear();

  [sendLock lock];
  //NSLog(@"Processing sends");
  // iterate through the hash
  for (std::multimap<GeneSysLib::FilterIDEnum, Word>::iterator it = lportFilterUpdateList.begin(); it != lportFilterUpdateList.end(); it++) {
    const auto &filterID = it->first;
    const auto &portID = it->second;

    // get the filterID for the portID and filterID
    const auto &filterData = self.device->midiPortFilter(portID, filterID);

    // send the set sysex method to the device
    self.device->send<SetMIDIPortFilterCommand>(filterData);
    //NSLog(@"Sending: %d, %d",portID, filterID);
  }

  for (std::multimap<RemapTypeEnum, Word>::iterator it = lchannelRemapUpdateList.begin(); it != lchannelRemapUpdateList.end(); it++) {
    const auto &remapID = it->first;
    const auto &portID = it->second;

    const auto &remapData = self.device->midiPortRemap(portID, remapID);
    self.device->send<SetMIDIPortRemapCommand>(remapData);
  }

  for (std::multimap<GeneSysLib::FilterIDEnum, Word>::iterator it = lcontrollerFilterUpdateList.begin(); it != lcontrollerFilterUpdateList.end(); it++) {
    const auto &filterID = it->first;
    const auto &portID = it->second;

    const auto &filterData = self.device->midiPortFilter(portID, filterID);
    self.device->send<SetMIDIPortFilterCommand>(filterData);
    //NSLog(@"Sending: %d, %d",portID, filterID);
  }

  for (std::multimap<RemapTypeEnum, Word>::iterator it = lcontrollerRemapUpdateList.begin(); it != lcontrollerRemapUpdateList.end(); it++) {
    const auto &remapID = it->first;
    const auto &portID = it->second;

    const auto &remapData = self.device->midiPortRemap(portID, remapID);
    self.device->send<SetMIDIPortRemapCommand>(remapData);
  }

  [sendLock unlock];
}

-(NSString*)nameForDevicePort:(Byte)popupDevicePort andPort:(Byte)popupPort {
  //NSLog(@"Trying to look up port name for: %d, %d", popupDevicePort, popupPort);
  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  NSString *toRet;
  if (portNum <= numPorts) {
    const auto &portInfo = self.device->get<MIDIPortInfo>(portNum);
    toRet = [NSString stringWithUTF8String:portInfo.portName().c_str()];
  }
  else {
    toRet = @"ERROR";
  }

  return toRet;
}

- (void)setCallback:(RefreshCallback)refreshCallback {
}

- (int)numSourceSections {
  //NSLog(@"patchbay: 2");

  int toReturn = numDevicePorts;
  return toReturn;
}

- (int)numDestinationSections {
  //NSLog(@"patchbay: 3");

  int toReturn = numDevicePorts;
  return toReturn;
}

- (int)numDestinationsPerSection:(int)section {
  //NSLog(@"patchbay: 4");

  int toReturn;
  toReturn = [[portsPerDevicePort objectAtIndex:section - 1] intValue];
  if (toReturn >= 1) {
    if ([self isCollapsedOutput:section])
      toReturn = 1;
  }
  return toReturn;
}

- (int)numSourcesPerSection:(int)section {
  //NSLog(@"patchbay: 5");

  int toReturn;
  toReturn = [[portsPerDevicePort objectAtIndex:section - 1] intValue];
  if (toReturn >= 1) {
    if ([self isCollapsedInput:section])
      toReturn = 1;
  }
  return toReturn;
}

- (int)realNumDestinationsPerSection:(int)section {
  //NSLog(@"patchbay: 4");

  int toReturn;
  toReturn = [[portsPerDevicePort objectAtIndex:section - 1] intValue];
  return toReturn;
}

- (int)realNumSourcesPerSection:(int)section {
  //NSLog(@"patchbay: 5");

  int toReturn;
  toReturn = [[portsPerDevicePort objectAtIndex:section - 1] intValue];
  return toReturn;
}

- (int)totalSources {
  //NSLog(@"patchbay: 6");

  int count = 0;
  for (int sect = 1; sect <= [self numSourceSections]; ++sect) {
    count += [self numSourcesPerSection:sect];
  }
  return count;
}

- (int)totalDestinations {
  //NSLog(@"patchbay: 7");

  int count = 0;
  for (int sect = 1; sect <= [self numDestinationSections]; ++sect) {
    count += [self numDestinationsPerSection:sect];
  }
  return count;
}

- (int)sourceIndexToTotal:(midi_port_t)targetIndex {
  int index = 0;
  for (int sect = 1; sect <= [self numSourceSections]; ++sect) {
    if (targetIndex.devicePort == sect) {
      index += targetIndex.port - 1;
      break;
    } else {
      index += [self numSourcesPerSection:sect];
    }
  }
  return index;
}

- (midi_port_t)realSourceTotalToIndex:(int)totalIndex {
  //NSLog(@"patchbay: 8");

  midi_port_t result = {0, 0};

  if (totalIndex > 0) {
    result = {1, 1};
    --totalIndex;
  }

  for (int sect = 1; sect <= [self numSourceSections]; ++sect) {
    if ((totalIndex - [self realNumSourcesPerSection:sect]) < 0) {
      result.port += totalIndex;
      break;
    } else {
      ++result.devicePort;
      totalIndex -= [self realNumSourcesPerSection:sect];
    }
  }

  return result;
}

// This method will return the port and channel given the total 1 based offset
- (midi_port_t)sourceTotalToIndex:(int)totalIndex {
  //NSLog(@"patchbay: 8");

  midi_port_t result = {0, 0};

  if (totalIndex > 0) {
    result = {1, 1};
    --totalIndex;
  }

  for (int sect = 1; sect <= [self numSourceSections]; ++sect) {
    if ((totalIndex - [self numSourcesPerSection:sect]) < 0) {
      result.port += totalIndex;
      break;
    } else {
      ++result.devicePort;
      totalIndex -= [self numSourcesPerSection:sect];
    }
  }

  return result;
}

- (int)destinationIndexToTotal:(midi_port_t)targetIndex {
  //NSLog(@"patchbay: 9");

  int index = 0;
  for (int sect = 1; sect <= [self numDestinationSections]; ++sect) {
    if (targetIndex.devicePort == sect) {
      index += targetIndex.port - 1;
      break;
    } else {
      //NSLog(@"here, weirdly");
      index += [self numDestinationsPerSection:sect];
    }
  }
  return index;
}

- (midi_port_t)destinationTotalToIndex:(int)totalIndex {
  //NSLog(@"patchbay: 10");

  midi_port_t result = {0, 0};

  if (totalIndex > 0) {
    result = {1, 1};
    --totalIndex;
  }

  for (int sect = 1; sect <= [self numDestinationSections]; ++sect) {
    if ((totalIndex - [self numDestinationsPerSection:sect]) < 0) {
      result.port += totalIndex;
      break;
    } else {
      ++result.devicePort;
      totalIndex -= [self numDestinationsPerSection:sect];
    }
  }

  return result;
}

- (midi_port_t)realDestinationTotalToIndex:(int)totalIndex {
  //NSLog(@"patchbay: 10");

  midi_port_t result = {0, 0};

  if (totalIndex > 0) {
    result = {1, 1};
    --totalIndex;
  }

  for (int sect = 1; sect <= [self numDestinationSections]; ++sect) {
    if ((totalIndex - [self realNumDestinationsPerSection:sect]) < 0) {
      result.port += totalIndex;
      break;
    } else {
      ++result.devicePort;
      totalIndex -= [self realNumDestinationsPerSection:sect];
    }
  }

  return result;
}

- (NSString *)nameForSection:(int)section {
  //NSLog(@"patchbay: 11");

  NSString *toReturn;
  toReturn = [devicePortNames objectAtIndex:section-1];
  //NSLog(@"done patchbay: 11");
  return toReturn;
}

- (bool)isPatchedFrom:(midi_port_t)src to:(midi_port_t)dst {
  //NSLog(@"patchbay: 13 with data: %d, %d, %d, %d", src.devicePort, src.port, dst.devicePort, dst.port);

  bool result = false;

  int dstPort = [self destinationIndexToTotal:dst] + 1;
  int srcPort = [self sourceIndexToTotal:src] + 1;

  //NSLog(@"patchbay 13 src: %d, dst: %d", dstPort, srcPort);

  if (([self isCollapsedInput:src.devicePort]) || ([self isCollapsedOutput:dst.devicePort])) {
    return true;
  }

  const auto &portRoute = self.device->get<MIDIPortRoute>(srcPort);
  result = portRoute.isRoutedTo(dstPort);
  //NSLog(@"patchbay: done 13");

  return result;
}

- (void)setPatchedFrom:(midi_port_t)src to:(midi_port_t)dst andRemove:(midi_port_t)toRemove {
  //NSLog(@"patchbay: 14 with data: %d, %d, %d, %d, %d, %d", src.devicePort, src.port, dst.devicePort, dst.port, toRemove.devicePort, toRemove.port);

  int dstPort = [self destinationIndexToTotal:dst] + 1;
  int srcPort = [self sourceIndexToTotal:src] + 1;

  //NSLog(@"patchbay 14 src: %d, dst: %d", dstPort, srcPort);

  if (toRemove.devicePort == 0) {
    auto &portRoute = self.device->get<MIDIPortRoute>(srcPort);
    portRoute.setRoutedTo(dstPort, true);
    self.device->send<SetMIDIPortRouteCommand>(portRoute);
  }
  else {
    int deletePort = [self sourceIndexToTotal:toRemove] + 1;

    auto &portRoute = self.device->get<MIDIPortRoute>(deletePort);
    portRoute.setRoutedTo(dstPort, false);
    self.device->send<SetMIDIPortRouteCommand>(portRoute);
  }

  //NSLog(@"patchbay: done 14");

}

- (void)toggleCollapseInput:(int)section
{
  //NSLog(@"patchbay: 15 collapsing %d", section);

  self->isCollapsedInputVec.at(section - 1) = !self->isCollapsedInputVec.at(section - 1);
}

- (bool)isCollapsedInput:(int)section
{
  //NSLog(@"patchbay: 16 & section is %i", section);
  //NSLog(@"size is: %lu", self->isCollapsedInputVec.size());


  if (section == 0 || section == 65535)
    return false;
  return self->isCollapsedInputVec.at(section - 1);
}

- (void)toggleCollapseOutput:(int)section
{
  //NSLog(@"patchbay: 17");

  self->isCollapsedOutputVec.at(section - 1) = !self->isCollapsedOutputVec.at(section - 1);

}

- (bool)isCollapsedOutput:(int)section
{
  //NSLog(@"patchbay: 18 & section is %i", section);
  //NSLog(@"size is: %lu", self->isCollapsedOutputVec.size());

  if (section == 0 || section == 65535)
    return false;
  return self->isCollapsedOutputVec.at(section - 1);
}


- (void)foreach:(MIDIPairOperator)midiOperator {
  //NSLog(@"patchbay: 20");

  for (int srcPort = 1; srcPort <= numPorts; ++srcPort) {
    const auto &portRoute = self.device->get<MIDIPortRoute>(srcPort);
    for (int dstPort = 1; dstPort <= numPorts; ++dstPort) {
      if (portRoute.isRoutedTo(dstPort)) {
        midi_pair_t pair;

        midi_port_t src = [self realSourceTotalToIndex:srcPort];
        midi_port_t dst = [self realDestinationTotalToIndex:dstPort];

        pair.source.devicePort = src.devicePort;
        pair.source.port = src.port;
        pair.destination.devicePort = dst.devicePort;
        pair.destination.port = dst.port;

        if ([self isCollapsedInput:(pair.source.devicePort)])
          pair.source.port = 1;
        if ([self isCollapsedOutput:(pair.destination.devicePort)])
          pair.destination.port = 1;

        //NSLog(@"doing device Operator: %d, %d, %d, %d\n",pair.source.devicePort, pair.source.port, pair.destination.devicePort, pair.destination.port);
        midiOperator(&pair);
      }
    }
  }
  //NSLog(@"done patchbay: 20");

}

- (BOOL)isSourceDeviceInBounds:(midi_port_t)device {
  //NSLog(@"patchbay: 21 w/ info: %i, %i", device.devicePort, device.port);

  return ((device.devicePort >= 1) && (device.devicePort <= [self numSourceSections]) &&
          (device.port >= 1) &&
          (device.port <= [self numSourcesPerSection:device.devicePort]));
}

- (BOOL)isDestinationDeviceInBounds:(midi_port_t)device {
  //NSLog(@"patchbay: 22 w/ info: %i, %i", device.devicePort, device.port);

  return ((device.devicePort >= 1) &&
          (device.devicePort <= [self numDestinationSections]) &&
          (device.port >= 1) &&
          (device.port <= [self numDestinationsPerSection:device.devicePort]));
}

- (bool) isPortFilterCellSetAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut {
  bool result = false;

  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  //NSLog(@"isPortFilterCellSetAtRow: %d, %d, %d, %d", row, col, portNum, popupPut);

  // determine if this is a channel status map
  const auto &isChannelStatusMap = MyAlgorithms::contains(rowChannelStatusMap, row);
  // determine if this is a filter map
  const auto &isFilterMap = MyAlgorithms::contains(rowFilterMap, row);

  // get the filter data for the current port ID
  const auto &filterData = self.device->midiPortFilter(portNum, (GeneSysLib::FilterIDEnum)popupPut);

  // if this is the channel status map
  if (isChannelStatusMap) {
    // determine if channel statis is set
    result = filterData.channelFilterStatus_at(col)[rowChannelStatusMap.at(row)];
  } else if (isFilterMap) {
    // determine if filter status is set
    result = filterData.filterStatus()[rowFilterMap.at(row)];
  }
  return result;
}

- (void) setPortFilterAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(bool)state {
  const auto &isChannelStatus = MyAlgorithms::contains(rowChannelStatusMap, row);
  // determine if it is a filter status
  const auto &isFilterStatus = MyAlgorithms::contains(rowFilterMap, row);

  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  auto &filterData = self.device->midiPortFilter(portNum, (GeneSysLib::FilterIDEnum)popupPut);

  // if it is a channel status
  if (isChannelStatus) {
    // get column filter status
    auto &colChFilterStatus = filterData.channelFilterStatus_at(col);
    colChFilterStatus.set(rowChannelStatusMap.at(row), state);
  } else if (isFilterStatus) {  // if it is a filter status
    filterData.filterStatus().set(rowFilterMap.at(row), state);
  }

  // add current portID and current filter ID to update list
  portFilterUpdateList.insert(std::pair<GeneSysLib::FilterIDEnum, Word>((GeneSysLib::FilterIDEnum)popupPut, portNum));
  [self resetTimer];
}

- (bool) isChannelRemapSetAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut {
  bool result = false;

  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  const auto &remapMap = self.device->midiPortRemap(portNum, (RemapID::Enum)popupPut);
  const auto &remapStatus = remapMap.remapStatus_at(col);
  
  switch (row) {
    case EventRows::PitchBendEvents:
      result = remapStatus.pitchBendEvents;
      break;
    case EventRows::ChannelPressureEvents:
      result = remapStatus.channelPressureEvents;
      break;
    case EventRows::ProgramChangeEvents:
      result = remapStatus.programChangeEvents;
      break;
    case EventRows::ControlChangeEvents:
      result = remapStatus.controlChangeEvents;
      break;
    case EventRows::PolyKeyPressureEvents:
      result = remapStatus.polyKeyPressureEvents;
      break;
    case EventRows::NoteEvents:
      result = remapStatus.noteEvents;
      break;
    default:
      break;
  }

  return result;
}

- (NSArray*) getChannelRemapsForDevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut {
  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  auto &remapMap = self.device->midiPortRemap(portNum, (RemapTypeEnum) popupPut);
  NSMutableArray *retArray = [[NSMutableArray alloc] init];
  for (auto col = 0; col < 16; ++col) {
    const auto &remapStatus = remapMap.remapStatus_at(col);
    [retArray addObject:[NSString stringWithFormat:@"%d",remapStatus.channelNumber + 1]];
  }
  return retArray;
}

- (void) setRemapStatusBit:(MIDIPortRemap::RemapStatus*)remapStatus Row:(int)row Value:(int)value {
  switch (row) {
    case EventRows::PitchBendEvents:
      remapStatus->pitchBendEvents = value;
      break;
    case EventRows::ChannelPressureEvents:
      remapStatus->channelPressureEvents = value;
      break;
    case EventRows::ProgramChangeEvents:
      remapStatus->programChangeEvents = value;
      break;
    case EventRows::ControlChangeEvents:
      remapStatus->controlChangeEvents = value;
      break;
    case EventRows::PolyKeyPressureEvents:
      remapStatus->polyKeyPressureEvents = value;
      break;
    case EventRows::NoteEvents:
      remapStatus->noteEvents = value;
      break;
    default:
      break;
  }
}

- (void) setChannelRemapAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(bool)state {
  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  auto &remapMap = self.device->midiPortRemap(portNum, (RemapTypeEnum)popupPut);
  auto &remapStatus = remapMap.remapStatus_at(col);

  [self setRemapStatusBit:&remapStatus Row:row Value:state];

//  addToUpdateList(portID);

  // add current portID and current filter ID to update list
  channelRemapUpdateList.insert(std::pair<RemapTypeEnum, Word>((RemapTypeEnum)popupPut,portNum));
  [self resetTimer];
}

- (void) setChannelRemapAtCol:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(int)state {
  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  auto &remapMap = self.device->midiPortRemap(portNum, (RemapTypeEnum)popupPut);
  auto &remapStatus = remapMap.remapStatus_at(col);

  remapStatus.channelNumber = (uint16_t)(state - 1) & 0x0F;

  channelRemapUpdateList.insert(std::pair<RemapTypeEnum, Word>((RemapTypeEnum)popupPut, portNum));
  [self resetTimer];
}

- (bool) isControllerFilterSetAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut {
  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  const auto &filterData = self.device->midiPortFilter(portNum, (GeneSysLib::FilterIDEnum) popupPut);

  const auto &controllerFilter = filterData.controllerFilter_at(row);
  return (controllerFilter.channelBitmap[colChBitmapMap.at(col)]);
}

- (void) setControllerFilterAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(bool)state {
  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  const auto &chBitmap = colChBitmapMap.at(col);
  auto &filterData = self.device->midiPortFilter(portNum, (GeneSysLib::FilterIDEnum)popupPut);

  auto &controllerFilter = filterData.controllerFilter_at(row);
  controllerFilter.channelBitmap.set(chBitmap, state);

  // add current portID and current filter ID to update list
  controllerFilterUpdateList.insert(std::pair<GeneSysLib::FilterIDEnum, Word>((GeneSysLib::FilterIDEnum)popupPut, portNum));
  [self resetTimer];
}

- (void) setControllerFilterIDAtRow:(int)row DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(int)state {
  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  auto &filterData = self.device->midiPortFilter(portNum, (GeneSysLib::FilterIDEnum)popupPut);
  auto &controllerFilter = filterData.controllerFilter_at(row);
  controllerFilter.controllerID = state;

  // add current portID and current filter ID to update list
  controllerFilterUpdateList.insert(std::pair<GeneSysLib::FilterIDEnum, Word>((GeneSysLib::FilterIDEnum)popupPut, portNum));
  [self resetTimer];
}

- (NSArray*) getControllerFilterIDsForDevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut {
  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;
  const auto &filterData = self.device->midiPortFilter(portNum, (GeneSysLib::FilterIDEnum) popupPut);
  NSMutableArray *retArray = [[NSMutableArray alloc] init];

  for (auto row = 0; row < 8; ++row) {
    const auto &controllerFilter = filterData.controllerFilters().at(row);

    [retArray addObject:[NSString stringWithFormat:@"%d", controllerFilter.controllerID]];
  }

  return retArray;
}

- (bool) isControllerRemapSetAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut {
  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  const auto &remapMap = self.device->midiPortRemap(portNum, (RemapTypeEnum)popupPut);
  const auto &remapFlags = remapMap.controller_at(row);

  return (remapFlags.channelBitmap[colChBitmapMap.at(col)]);
}

- (NSArray*) getControllerRemapIDsForDevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut {
  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;
  const auto &remapMap = self.device->midiPortRemap(portNum, (RemapTypeEnum)popupPut);
  NSMutableArray *retArray = [[NSMutableArray alloc] init];

  for (auto row = 0; row < 8; ++row) {
    const auto &remapController = remapMap.controller_at(row);

    NSArray *srcDst = [[NSArray alloc] initWithObjects:[NSString stringWithFormat:@"%d", remapController.controllerSource], [NSString stringWithFormat:@"%d", remapController.controllerDestination], nil];

    //NSLog(@"adding controller remap: %d, %d", remapController.controllerSource, remapController.controllerDestination);

    [retArray addObject:srcDst];
  }

  return retArray;
}

- (void) setControllerRemapAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(bool)state {
  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  auto &remapMap = self.device->midiPortRemap(portNum, (RemapTypeEnum)popupPut);
  auto &remapController = remapMap.controller_at(row);

  remapController.channelBitmap[colChBitmapMap.at(col)] = state;

  controllerRemapUpdateList.insert(std::pair<RemapTypeEnum, Word>((RemapTypeEnum)popupPut, portNum));
  [self resetTimer];
}

- (void) setControllerRemapIDAtRow:(int)row DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(int)state isSource:(bool)isSource {
  //NSLog(@"setControllerRemapIDAtRow: %d", row);
  midi_port_t port = {popupDevicePort, popupPort};
  int portNum = [self destinationIndexToTotal:port] + 1;

  auto &remapMap = self.device->midiPortRemap(portNum, (RemapTypeEnum)popupPut);
  auto &remapController = remapMap.controller_at(row);

  if (isSource)
    remapController.controllerSource = state;
  else
    remapController.controllerDestination = state;

  controllerRemapUpdateList.insert(std::pair<RemapTypeEnum, Word>((RemapTypeEnum)popupPut, portNum));
  [self resetTimer];
}
@end
