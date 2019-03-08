/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICPortRoutingProvider.h"

#include "MyAlgorithms.h"
#include "MIDIInfo.h"
#include "MIDIPortInfo.h"
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/range/adaptors.hpp>

using namespace MyAlgorithms;
using namespace GeneSysLib;
using namespace boost::adaptors;

@implementation ICPortRoutingProvider

- (id)initWithSelectedPort:(Word)_selectedPort {
  self = [super init];

  if (self) {
    selectedPort = _selectedPort;
    buttonNames = @[];
  }

  return self;
}

- (void)initializeProviderButtons:(ICViewController *)sender {
  [super initializeProviderButtons:sender];

  auto *const tempButtonNames = [NSMutableArray array];
  auto *const tempIsStandardArray = [NSMutableArray array];
  auto *const tempIsEnabledArray = [NSMutableArray array];

  assert(sender.device);
  assert(sender.device->contains<MIDIPortRoute>(selectedPort));
  assert(sender.device->containsCommandData(Command::RetMIDIInfo));

  portRoute = sender.device->get<MIDIPortRoute>(selectedPort);

  const auto &midiInfo = sender.device->get<MIDIInfo>();

  setMap.clear();
  getMap.clear();

  for (auto portID = 1; portID <= midiInfo.numMIDIPorts(); ++portID) {
    const auto &midiPortInfo = sender.device->get<MIDIPortInfo>(portID);

    // i is 0 based where portID is 1 based
    int i = (portID - 1);

    const auto &isEnabled = midiPortInfo.isOutputEnabled();

    if (isEnabled) {
      setMap[i] =
          bind(&MIDIPortRoute::setRoutedTo, boost::ref(portRoute), (i + 1), _1);
      getMap[i] =
          bind(&MIDIPortRoute::isRoutedTo, boost::ref(portRoute), i + 1);
    } else {
      setMap[i] = [](bool value) {};
      getMap[i] = []() { return false; };
    }

    [tempIsEnabledArray addObject:@(isEnabled)];

    const auto &portInfo = midiPortInfo.portInfo();
    const auto &isDIN = midiPortInfo.isOfType(PortType::DIN);
    const auto &isUSBDevice = midiPortInfo.isOfType(PortType::USBDevice);
    const auto &isUSBHost = midiPortInfo.isOfType(PortType::USBHost);
    const auto &isEthernet = midiPortInfo.isOfType(PortType::Ethernet);
    auto overallJackNumber = portInfo.common.jack;

    NSString *jackName = @"";
    if (isDIN) {
      jackName = [NSString stringWithFormat:@"DIN %d", portInfo.din.jack];
    } else if (isUSBDevice) {
      jackName =
          [NSString stringWithFormat:@"USB %d.%d", portInfo.usbDevice.jack,
                                     portInfo.usbDevice.devicePort];
      overallJackNumber += 1;  // group din jacks together
    } else if (isUSBHost) {
      jackName =
          [NSString stringWithFormat:@"Host %d", portInfo.usbHost.hostPort];
      overallJackNumber += 1;  // group din jacks together
      overallJackNumber += midiInfo.numUSBDeviceJacks();
    } else if (isEthernet) {
      jackName =
          [NSString stringWithFormat:@"Eth %d", portInfo.ethernet.session];
      overallJackNumber += 1;  // group din jacks together
      overallJackNumber += midiInfo.numUSBDeviceJacks();
      overallJackNumber += midiInfo.numUSBHostJacks();
    }
    NSString *portName = @(midiPortInfo.portName().c_str());
    NSString *buttonName =
        (([[portName uppercaseString] compare:[jackName uppercaseString]] ==
          NSOrderedSame)
             ? (jackName)
             : ([NSString stringWithFormat:@"%@\n\"%@\"", jackName, portName]));
    [tempButtonNames addObject:buttonName];

    const auto &isJackEven = (overallJackNumber % 2) == 0;
    const auto &isStandard = isDIN || !isJackEven;

    [tempIsStandardArray addObject:@(isStandard)];
  }
  buttonNames = tempButtonNames;
  isStandardArray = tempIsStandardArray;
  isEnabledArray = tempIsEnabledArray;

  title = [super title];

  assert(sender.device);
  const auto &midiPortInfo = sender.device->get<MIDIPortInfo>(selectedPort);
  title = [title stringByAppendingString:@(midiPortInfo.portName().c_str())];
  NSString *jackName = @"";
  const auto &portInfo = midiPortInfo.portInfo();
  const auto &isDIN = midiPortInfo.isOfType(PortType::DIN);
  const auto &isUSBDevice = midiPortInfo.isOfType(PortType::USBDevice);
  const auto &isUSBHost = midiPortInfo.isOfType(PortType::USBHost);
  const auto &isEthernet = midiPortInfo.isOfType(PortType::Ethernet);
  if (isDIN) {
    jackName = [NSString stringWithFormat:@"DIN %d", portInfo.din.jack];
  } else if (isUSBDevice) {
    jackName = [NSString stringWithFormat:@"USB %d.%d", portInfo.usbDevice.jack,
                                          portInfo.usbDevice.devicePort];
  } else if (isUSBHost) {
    jackName =
        [NSString stringWithFormat:@"Host %d", portInfo.usbHost.hostPort];
  } else if (isEthernet) {
    jackName = [NSString stringWithFormat:@"Eth %d", portInfo.ethernet.session];
  }
  title = [NSString stringWithFormat:@"%@ \"%@\"", jackName,
                                     @(midiPortInfo.portName().c_str())];
}

- (void)providerWillAppear:(ICViewController *)sender {
  assert(sender.device);
  assert(sender.device->contains<MIDIPortRoute>(selectedPort));
  for (const auto &mapPair : getMap) {
    [(sender.providerButtons)[mapPair.first] setSelected:mapPair.second()];
  }
}

- (NSString *)providerName {
  return @"PortRoutingRouteSelection";
}

- (NSString *)title {
  return title;
}

- (NSArray *)buttonNames {
  return buttonNames;
}

- (NSUInteger)numberOfRows {
  auto result = MAX(1, (NSUInteger)ceil((float)[buttonNames count] /
                                        (float)[self numberOfColumns]));

  // For the iCM2 which has 10 ports
  if ([buttonNames count] == 10) {
    result = 5;
  }
  // For the iCM4 which has 64 ports
  else if ([buttonNames count] == 64) {
    // For iPhone
    if ([[UIDevice currentDevice] userInterfaceIdiom] ==
        UIUserInterfaceIdiomPhone) {
      result = 32;
    }
    // For iPad
    else {
      result = 16;
    }
  }

  return result;
}

- (NSUInteger)numberOfColumns {
  auto result = MAX(1, (int)ceil(sqrt([buttonNames count])));

  // For the iCM2 which has 10 ports
  if ([buttonNames count] == 10) {
    result = 2;
  }
  // For the iCM4 which has 64 ports
  else if ([buttonNames count] == 64) {
    // For iPhone
    if ([[UIDevice currentDevice] userInterfaceIdiom] ==
        UIUserInterfaceIdiomPhone) {
      result = 2;
    }
    // For iPad
    else {
      result = 4;
    }
  }

  return result;
}

- (CGSize)contentScale {
  return (([buttonNames count] != 64)
              ? CGSizeMake(1.0f, 1.0f)
              : (([[UIDevice currentDevice] userInterfaceIdiom] ==
                  UIUserInterfaceIdiomPhone)
                     ? (CGSizeMake(1.0f, 4.8f))
                     : (CGSizeMake(1.0f, 1.4f))));
}

- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  if (setMap.count(buttonIndex) > 0) {
    bool isSet = [(sender.providerButtons)[buttonIndex] isSelected];
    setMap[buttonIndex](!isSet);
    [(sender.providerButtons)[buttonIndex] setSelected:!isSet];

    // Send the change 1 second from the last user input
    [self startUpdateTimer];
  }
}

- (void)onUpdate:(DeviceID)deviceID transID:(Word)transID {
  assert(self.parent);
  assert(self.parent.device);
  self.parent.device->send<SetMIDIPortRouteCommand>(portRoute);
}

- (BOOL)isIndexStandard:(NSUInteger)index {
  return [[isStandardArray objectAtIndex:index] boolValue];
}

- (BOOL)isIndexEnabled:(NSUInteger)index {
  return [[isEnabledArray objectAtIndex:index] boolValue];
}

@end
