/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICPortSelectionProvider.h"
#import "MIDIInfo.h"
#import "MyAlgorithms.h"
#include "MIDIPortInfo.h"

#import <boost/shared_ptr.hpp>
#import <boost/range/adaptors.hpp>

using namespace MyAlgorithms;
using namespace GeneSysLib;
using namespace boost::adaptors;

@implementation ICPortSelectionProvider

- (id)initWithProviderName:(NSString *)_providerName
       destinationProvider:(id<SquareViewProvider>)_destination
                  forInput:(BOOL)_inputType {
  self = [super init];

  if (self) {
    NSParameterAssert(_providerName);
    NSParameterAssert(_destination);
    providerName = _providerName;
    destinationProvider = _destination;
    isInputType = _inputType;
    buttonNames = @[];
  }

  return self;
}

- (id)initForInput:(BOOL)_inputType {
  self = [super init];

  if (self) {
    isInputType = _inputType;
    buttonNames = @[];
  }

  return self;
}

- (NSString *)providerName {
  return providerName;
}

- (void)initializeProviderButtons:(ICViewController *)sender {
  [super initializeProviderButtons:sender];
  auto *const tempButtonNames = [NSMutableArray array];
  auto *const tempIsStandardArray = [NSMutableArray array];
  auto *const tempIsEnabledArray = [NSMutableArray array];

  assert(sender.device);
  assert(sender.device->containsCommandData(Command::RetMIDIInfo));
  const auto &midiInfo = sender.device->get<MIDIInfo>();

  sender.device->for_each<MIDIPortInfo>([&](const MIDIPortInfo &midiPortInfo) {
    auto isEnabled = true;
    if (isInputType) {
      isEnabled = midiPortInfo.isInputEnabled();
    } else {
      isEnabled = midiPortInfo.isOutputEnabled();
    }

    [tempIsEnabledArray addObject:@(isEnabled)];

    auto portInfo = midiPortInfo.portInfo();
    auto const &isDIN = midiPortInfo.isOfType(PortType::DIN);
    auto const &isUSBDevice = midiPortInfo.isOfType(PortType::USBDevice);
    auto const &isUSBHost = midiPortInfo.isOfType(PortType::USBHost);
    auto const &isEthernet = midiPortInfo.isOfType(PortType::Ethernet);
    int overallJackNumber = portInfo.common.jack;

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

    auto const &isJackEven = (overallJackNumber % 2) == 0;
    auto const &isStandard = isDIN || !isJackEven;

    [tempIsStandardArray addObject:@(isStandard)];
  });

  buttonNames = tempButtonNames;
  isStandardArray = tempIsStandardArray;
  isEnabledArray = tempIsEnabledArray;
}

- (NSArray *)buttonNames {
  return buttonNames;
}

- (NSUInteger)numberOfRows {
  auto result = MAX(1, (NSUInteger)ceil((float)[buttonNames count] /
                                        (float)[self numberOfColumns]));

  if ([buttonNames count] == 10) {
    result = 5;
  } else if ([buttonNames count] == 64) {
    if ([[UIDevice currentDevice] userInterfaceIdiom] ==
        UIUserInterfaceIdiomPhone) {
      result = 32;
    } else {
      result = 16;
    }
  }

  return result;
}

- (NSUInteger)numberOfColumns {
  NSUInteger result = MAX(1, (int)ceil(sqrt([buttonNames count])));

  if ([buttonNames count] == 10) {
    result = 2;
  } else if ([buttonNames count] == 64) {
    if ([[UIDevice currentDevice] userInterfaceIdiom] ==
        UIUserInterfaceIdiomPhone) {
      result = 2;
    } else {
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
  [sender.storedUserInformation setValue:@(buttonIndex + 1) forKey:@"inPort"];
  if (destinationProvider != nil) {
    [sender pushToProvider:destinationProvider animated:YES];
  }
}

- (BOOL)isIndexStandard:(NSUInteger)index {
  return [[isStandardArray objectAtIndex:index] boolValue];
}

- (BOOL)isIndexEnabled:(NSUInteger)index {
  return [[isEnabledArray objectAtIndex:index] boolValue];
}

@end
