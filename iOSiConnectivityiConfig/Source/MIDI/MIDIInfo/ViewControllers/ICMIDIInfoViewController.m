/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "Communicator.h"
#import "ICChoiceCellProvider.h"
#import "ICMIDIIndexTableViewController.h"
#import "ICMIDIInfoViewController.h"
#import "ICNormalCellProvider.h"
#import "ICRunOnMain.h"
#import "ICSwitchCellProvider.h"
#import "ICTextEditCellProvider.h"
#import "MIDIInfo.h"
#import "MIDIPortDetail.h"
#import "MMDrawerBarButtonItem.h"
#import "MyAlgorithms.h"
#import "UIViewController+MMDrawerController.h"
#import "MIDIPortInfo.h"
#import "ICVendorNameDelegate.h"
#import "ICProductNameDelegate.h"
#import "ICReservedPortChoiceDelegate.h"
#import "ICMIDIInterPortRoutingSwitchDelegate.h"
#import "ICRunningStatusSwitchDelegate.h"
#import "ICMIDIInputEnabledSwitchDelegate.h"
#import "ICMIDIOutputEnabledSwitchDelegate.h"
#import "ICRangeChoiceDelegate.h"
#import "ICMaxPortsRangeDelegate.h"
#import "ICRangeCellProvider.h"
#import "ICMIDIPortNameTextEditDelegate.h"

#import <cassert>
#import <string>

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace std;

@interface ICMIDIInfoViewController () {
  MIDIInfo midiInfo;
}

@end

@implementation ICMIDIInfoViewController

- (id)initWithMIDIInfo:(MIDIInfo)_midiInfo
          communicator:(CommPtr)comm
                device:(DeviceInfoPtr)device {
  self = [super init];

  if (self) {
    // Custom initialization
    midiInfo = _midiInfo;

    auto* const tempSectionArray = [NSMutableArray array];
    auto* const tempSectionTitleArray = [NSMutableArray array];

    assert(device);

    const auto isiPhone = ([[UIDevice currentDevice] userInterfaceIdiom] ==
                            UIUserInterfaceIdiomPhone);

    if (midiInfo.versionNumber() == 0x01) {
      // add the MIDI Information
      [tempSectionTitleArray addObject:@"MIDI Information"];

      __block auto* const midiInfoArray = [NSMutableArray array];

      auto addStaticIntInfo = ^(NSString* const title, int value) {
        [midiInfoArray
            addObject:[ICNormalCellProvider
                          providerWithTitle:title
                                      value:[NSString stringWithFormat:@"%d",
                                                                       value]]];
      };

      if (midiInfo.numMIDIPorts()) {
        addStaticIntInfo(
            ((isiPhone) ? (@"MIDI Ports") : (@"Number of MIDI Ports")),
            midiInfo.numMIDIPorts());
      }

      if (midiInfo.numDINPairs()) {
        addStaticIntInfo(
            ((isiPhone) ? (@"DIN Pairs") : (@"Number of DIN Pairs")),
            midiInfo.numDINPairs());
      }

      if (midiInfo.numUSBDeviceJacks()) {
        addStaticIntInfo(
            ((isiPhone) ? (@"Device Jacks") : (@"Number of Device Jacks")),
            midiInfo.numUSBDeviceJacks());

        addStaticIntInfo(
            ((isiPhone) ? (@"MIDI Ports/Device Jack")
                        : (@"Number of USB MIDI Ports/Device Jack")),
            midiInfo.numUSBMIDIPortPerDeviceJack());
      }

      if (midiInfo.numUSBHostJacks()) {
        addStaticIntInfo(
            ((isiPhone) ? (@"USB Host Jacks") : (@"Number of USB Host Jacks")),
            midiInfo.numUSBHostJacks());

        addStaticIntInfo(((isiPhone) ? (@"USB MIDI Ports / Host Jack")
                                     : (@"Number of USB MIDI Ports/Host Jack")),
                         midiInfo.numUSBMIDIPortPerHostJack());

        ICMaxPortsRangeDelegate* maxPortsRangeDelegate =
            [[ICMaxPortsRangeDelegate alloc] initWithDevice:device];

        ICRangeCellProvider* cellProvider = [[ICRangeCellProvider alloc]
            initWithDelegate:maxPortsRangeDelegate];

        [midiInfoArray addObject:cellProvider];
      }

      if (midiInfo.numEthernetJacks()) {
        addStaticIntInfo(
            ((isiPhone) ? (@"Ethernet Jacks") : (@"Number of Ethernet Jacks")),
            midiInfo.numEthernetJacks());

        addStaticIntInfo(
            ((isiPhone) ? (@"Sessions/Ethernet Jack")
                        : (@"Number of RTP MIDI Sessions/Ethernet Jack")),
            midiInfo.numRTPMIDISessionsPerEthernetJack());

        addStaticIntInfo(
            ((isiPhone) ? (@"Connections/RTP MIDI Session")
                        : (@"Number of Connections/RTP MIDI Session")),
            midiInfo.numRTPMIDIConnectionsPerSession());
      }

      // Inter port routing switch
      if (midiInfo.numUSBHostJacks()) {
        ICMIDIInterPortRoutingSwitchDelegate* interPortDelegate =
            [[ICMIDIInterPortRoutingSwitchDelegate alloc]
                initWithDevice:device];

        ICSwitchCellProvider* interPortSwitchProvider =
            [ICSwitchCellProvider providerWithDelegate:interPortDelegate];

        [midiInfoArray addObject:interPortSwitchProvider];
      }

      // Running status switch
      if (midiInfo.numDINPairs()) {
        ICRunningStatusSwitchDelegate* runningStatusDelegate =
            [[ICRunningStatusSwitchDelegate alloc] initWithDevice:device];
        ICSwitchCellProvider* runningStatusSwitchProvider =
            [ICSwitchCellProvider providerWithDelegate:runningStatusDelegate];
        [midiInfoArray addObject:runningStatusSwitchProvider];
      }

      [tempSectionArray addObject:midiInfoArray];

      int numPorts = device->typeCount<MIDIPortInfo>();
      for (Word portID = 1; portID <= numPorts; ++portID) {
        // title
        {
          MIDIPortInfo& port = device->get<MIDIPortInfo>(portID);
          const auto portInfo = port.portInfo();
          NSMutableString* title = [NSMutableString
              stringWithFormat:@"Port Information %d\n", portID];

          // Add a title based on port type
          switch (port.portType()) {
            case PortType::DIN: {
              [title appendFormat:@"(DIN %d)", portInfo.din.jack];
            } break;

            case PortType::USBDevice: {
              [title appendFormat:@"(USB Device %d, Port: %d)",
                                  portInfo.usbDevice.jack,
                                  portInfo.usbDevice.devicePort];

            } break;

            case PortType::USBHost: {
              [title appendFormat:@"(USB Host %d, Port: %d)",
                                  portInfo.usbHost.jack,
                                  portInfo.usbHost.hostPort];
            } break;

            case PortType::Ethernet: {
              [title appendFormat:@"(Ethernet %d, Session: %d)",
                                  portInfo.ethernet.jack,
                                  portInfo.ethernet.session];

            } break;

            default:
              break;
          }
          [tempSectionTitleArray addObject:title];
        }

        __block DeviceInfoPtr blockDevice = device;
        MIDIPortDetail midiDetail;
        if (device->contains<MIDIPortDetail>(portID)) {
          midiDetail = device->get<MIDIPortDetail>(portID);
        }

        auto* const portArray = [NSMutableArray array];

        // port name
        {
          ICMIDIPortNameTextEditDelegate* portNameDelegate =
              [[ICMIDIPortNameTextEditDelegate alloc] initWithDevice:device
                                                              portID:portID];

          ICTextEditCellProvider* textCellProvider =
              [ICTextEditCellProvider providerWithDelegate:portNameDelegate];

          [portArray addObject:textCellProvider];
        }

        // Add Input Enabled
        {
          ICMIDIInputEnabledSwitchDelegate* inputEnabledDelegate =
              [[ICMIDIInputEnabledSwitchDelegate alloc] initWithDevice:device
                                                                portID:portID];
          ICSwitchCellProvider* switchCellProvider =
              [ICSwitchCellProvider providerWithDelegate:inputEnabledDelegate];

          [portArray addObject:switchCellProvider];
        }

        // Add Output Enabled
        {
          ICMIDIOutputEnabledSwitchDelegate* outputEnabledDelegate =
              [[ICMIDIOutputEnabledSwitchDelegate alloc] initWithDevice:device
                                                                 portID:portID];
          ICSwitchCellProvider* switchCellProvider =
              [ICSwitchCellProvider providerWithDelegate:outputEnabledDelegate];

          [portArray addObject:switchCellProvider];
        }
        switch (midiDetail.portType()) {
          case PortType::USBDevice: {
            auto& usbDetails = midiDetail.getUSBDevice();
            NSString* hostTypeString = @"Unknown";

            switch (usbDetails.hostType()) {
              case HostType::NoHost:
                hostTypeString = @"No Host";
                break;
              case HostType::iOSDevice:
                hostTypeString = @"iOS Device Host";
                break;
              case HostType::MacPC:
                hostTypeString = @"Mac/PC Host";
                break;
            }

            ICNormalCellProvider* hostTypeProvider =
                [ICNormalCellProvider providerWithTitle:@"Host Type"
                                                  value:hostTypeString];

            [portArray addObject:hostTypeProvider];

            if (usbDetails.hostName().size() > 0) {
              ICNormalCellProvider* hostNameProvider = [ICNormalCellProvider
                  providerWithTitle:@"Host Name"
                              value:@(usbDetails.hostName().c_str())];
              [portArray addObject:hostNameProvider];
            }

          } break;

          case PortType::USBHost: {
            ICReservedPortChoiceDelegate* reservedPortChoiceDelegate =
                [[ICReservedPortChoiceDelegate alloc] initWithDevice:device
                                                              portID:portID];

            ICChoiceCellProvider* choiceProvider = [ICChoiceCellProvider
                providerWithDelegate:reservedPortChoiceDelegate];

            [portArray addObject:choiceProvider];

            // Vendor Name
            {
              ICVendorNameDelegate* vendorNameDelegate =
                  [[ICVendorNameDelegate alloc] initWithDevice:device
                                                        portID:portID];
              ICNormalCellProvider* const venderNameCell = [ICNormalCellProvider
                  providerWithDelegate:vendorNameDelegate];

              [portArray addObject:venderNameCell];
            }

            // Product Name
            {
              ICProductNameDelegate* productNameDelegate =
                  [[ICProductNameDelegate alloc] initWithDevice:device
                                                         portID:portID];

              ICNormalCellProvider* const productNameProvider =
                  [ICNormalCellProvider
                      providerWithDelegate:productNameDelegate];

              [portArray addObject:productNameProvider];
            }
          } break;

          default:
            break;
        }

        [tempSectionArray addObject:portArray];
      }
    }

    sectionArrays = tempSectionArray;
    sectionTitles = tempSectionTitleArray;
  }
  return self;
}

- (void)setupRightMenuButton {
  [self.mm_drawerController
      setOpenDrawerGestureModeMask:MMOpenDrawerGestureModeAll];

  auto* const rightDrawerButton = [[MMDrawerBarButtonItem alloc]
      initWithTarget:self
              action:@selector(rightDrawerButtonPress:)];
  [self.navigationItem setRightBarButtonItem:rightDrawerButton animated:YES];

  auto* const tvc = self.mm_drawerController.rightDrawerViewController;

  if ([tvc isKindOfClass:[ICMIDIIndexTableViewController class]]) {
    [(ICMIDIIndexTableViewController*)tvc rebuildWithMIDIInfo:midiInfo];
    [(ICMIDIIndexTableViewController*)tvc setSidebarDelegate:self];
  }
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [self.navigationController setNavigationBarHidden:FALSE animated:true];

  [self.navigationItem setTitle:@"MIDI Information"];

  [self setupRightMenuButton];

  runOnMainAfter(0.2, ^{ [self.tableView reloadData]; });
}

- (void)viewWillDisappear:(BOOL)animated {
  [super viewWillDisappear:animated];
  [self.mm_drawerController
      setOpenDrawerGestureModeMask:MMOpenDrawerGestureModeNone];

  auto* const tvc = self.mm_drawerController.rightDrawerViewController;

  if ([tvc isKindOfClass:[ICMIDIIndexTableViewController class]]) {
    [(ICMIDIIndexTableViewController*)tvc setSidebarDelegate:nil];
  }
}

- (void)rightDrawerButtonPress:(id)sender {
  [self.mm_drawerController toggleDrawerSide:MMDrawerSideRight
                                    animated:YES
                                  completion:nil];
}

- (void)indexPathSelected:(NSIndexPath*)indexPath {
  const auto& containsDINS = (midiInfo.numDINPairs() > 0);
  const auto& containsUSBDevice = (midiInfo.numUSBDeviceJacks() > 0);
  const auto& containsUSBHost = (midiInfo.numUSBHostJacks() > 0);

  const auto& totalDINPorts = midiInfo.numDINPairs();
  const auto& totalUSBDevicePorts =
      (midiInfo.numUSBDeviceJacks() * midiInfo.numUSBMIDIPortPerDeviceJack());

  const auto& totalUSBHostPorts =
      (midiInfo.numUSBHostJacks() * midiInfo.numUSBMIDIPortPerHostJack());

  auto offset = indexPath.row;
  auto sections = indexPath.section;

  // if greater than 1 then move pointer by one
  if (sections > 0) {
    offset += 1;
    --sections;
  }

  if (sections > 0) {
    if (containsDINS) {
      offset += totalDINPorts;
      --sections;
    } else if (containsUSBDevice) {
      offset += totalUSBDevicePorts;
      --sections;
    } else if (containsUSBHost) {
      offset += totalUSBHostPorts;
      --sections;
    }
  }

  if (sections > 0) {
    if (containsUSBDevice) {
      offset += totalUSBDevicePorts;
      --sections;
    } else if (containsUSBHost) {
      offset += totalUSBHostPorts;
      --sections;
    }
  }

  if (sections > 0) {
    if (containsUSBHost) {
      offset += totalUSBHostPorts;
      --sections;
    }
  }

  [self.tableView
      scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:offset]
            atScrollPosition:UITableViewScrollPositionTop
                    animated:YES];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:
            (UIInterfaceOrientation)interfaceOrientation {
  const auto& isiPad = ([[UIDevice currentDevice] userInterfaceIdiom] ==
                        UIUserInterfaceIdiomPad);
  const auto& isPortait =
      UIInterfaceOrientationIsPortrait(interfaceOrientation);

  // should only rotate with an iPad or if the orientation is portrait
  return (isiPad || isPortait);
}

@end
