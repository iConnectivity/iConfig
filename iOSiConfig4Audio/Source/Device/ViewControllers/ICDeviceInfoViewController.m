/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "Device.h"
#import "DeviceInfo.h"
#import "EthernetPortInfo.h"
#import "ICChoiceCellProvider.h"
#import "ICDeviceInfoViewController.h"
#import "ICNormalCellProvider.h"
#import "ICTextEditCellProvider.h"
#import "Info.h"
#import "InfoList.h"
#import "SysexCommand.h"
#import "MyConverters.h"
#import "ICMacAddressDelegate.h"
#import "ICIPModeChoiceDelegate.h"
#import "ICIPAddressTextEditCellDelegate.h"
#import "ICSubnetMaskTextEditCellDelegate.h"
#import "ICGatewayTextEditCellDelegate.h"
#import "ICInfoTextEditCellDelegate.h"

#import <boost/shared_ptr.hpp>
#import <boost/range/adaptors.hpp>

using namespace GeneSysLib;
using namespace NetAddrTools;
using namespace boost::adaptors;

@interface ICDeviceInfoViewController ()

@property(nonatomic, assign) DeviceInfoPtr device;

+ (NSString *)stringForInfoID:(Byte)infoID;

- (NSObject *)createInfoProvider:(InfoIDEnum)infoID;

@end

@implementation ICDeviceInfoViewController

- (id)initWithCommunicator:(CommPtr)comm device:(DeviceInfoPtr)device {
  self = [super init];
  if (self) {
    self.device = device;

    const auto &isiPhone = ([[UIDevice currentDevice] userInterfaceIdiom] ==
                            UIUserInterfaceIdiomPhone);

    auto *const tempSectionArray = [NSMutableArray array];
    auto *const tempSectionTitles = [NSMutableArray array];

    assert(device);
    [tempSectionTitles addObject:@"Device Information"];
    auto *const tempInfo = [NSMutableArray array];

    /* we loop through all the stored information. Then we get the associated
     * info record to determine the maximum length. Then we create a label is
     * the max length is zero and a text edit provider otherwise. We do this so
     * that we can pass in the context to each provider.
     */
    self.device->for_each<Info>([&](Info &info) {
      NSObject *provider = [self createInfoProvider:info.infoID()];
      if (provider) {
        [tempInfo addObject:provider];
      }
    });

    [tempSectionArray addObject:tempInfo];

    auto count = device->typeCount<EthernetPortInfo>();

    for (int portID = 1; portID <= count; ++portID) {
      auto &ethPort = device->get<EthernetPortInfo>(portID);
      NSString *const ethernetSectionTitle =
          [NSString stringWithFormat:@"Ethernet Jack %d", ethPort.portJackID()];
      [tempSectionTitles addObject:ethernetSectionTitle];

      __block auto *const tempEthInfo = [NSMutableArray array];

      ICMacAddressDelegate *macAddressDelegate =
          [[ICMacAddressDelegate alloc] initWithDevice:self.device
                                                portID:portID];
      ICNormalCellProvider *macProvider =
          [ICNormalCellProvider providerWithDelegate:macAddressDelegate];
      [tempEthInfo addObject:macProvider];

      // IP mode
      {
        ICIPModeChoiceDelegate *ipModeChoiceDelegate =
            [[ICIPModeChoiceDelegate alloc] initWithDevice:device
                                                    portID:portID];

        ICChoiceCellProvider *cellProvider =
            [ICChoiceCellProvider providerWithDelegate:ipModeChoiceDelegate];
        [tempEthInfo addObject:cellProvider];
      }

      // static IP text edit
      {
        ICIPAddressTextEditCellDelegate *delegate =
            [[ICIPAddressTextEditCellDelegate alloc] initWithDevice:device
                                                             portID:portID];
        ICTextEditCellProvider *provider =
            [ICTextEditCellProvider providerWithDelegate:delegate];
        [tempEthInfo addObject:provider];
      }

      // static subnet mask text edit
      {
        ICSubnetMaskTextEditCellDelegate *delegate =
            [[ICSubnetMaskTextEditCellDelegate alloc] initWithDevice:device
                                                              portID:portID];
        ICTextEditCellProvider *provider =
            [ICTextEditCellProvider providerWithDelegate:delegate];
        [tempEthInfo addObject:provider];
      }

      // static gateway text edit
      {
        ICGatewayTextEditCellDelegate *delegate =
            [[ICGatewayTextEditCellDelegate alloc] initWithDevice:device
                                                           portID:portID];
        ICTextEditCellProvider *provider =
            [ICTextEditCellProvider providerWithDelegate:delegate];
        [tempEthInfo addObject:provider];
      }

      const auto &addStaticToEthSection =
          ^(NSString *const title, const NetAddr &value) {
        ICNormalCellProvider *provider =
            [ICNormalCellProvider providerWithTitle:title
                                              value:fromNetAddr(value)];
        [tempEthInfo addObject:provider];
      };

      addStaticToEthSection(@"Active IP", ethPort.currentIPAddress());

      addStaticToEthSection(
          ((isiPhone) ? (@"Active Subnet") : (@"Active Subnet Mask")),
          ethPort.currentSubnetMask());

      addStaticToEthSection(@"Active Gateway", ethPort.currentGateway());

      NSString *nameString = @(ethPort.deviceName().c_str());

      [tempEthInfo
          addObject:[ICNormalCellProvider providerWithTitle:@"Device Name"
                                                      value:nameString]];

      [tempSectionArray addObject:tempEthInfo];
    }

    sectionArrays = tempSectionArray;
    sectionTitles = tempSectionTitles;
  }
  return self;
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [self.navigationController setNavigationBarHidden:FALSE animated:true];

  [self.navigationItem setTitle:@"Device Information"];

  [self.tableView reloadData];
}

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view from its nib.
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
  // Dispose of any resources that can be recreated.
}

+ (NSString *)stringForInfoID:(Byte)infoID {
  NSString *result = @"Unknown Information";

  InfoID::Enum infoIDEnum = (InfoID::Enum)infoID;

  switch (infoIDEnum) {
    case InfoID::AccessoryName:
      result = @"Accessory Name";
      break;

    case InfoID::ManufacturerName:
      result = @"Manufacturer Name";
      break;

    case InfoID::ModelNumber:
      result = @"Model Number";
      break;

    case InfoID::SerialNumber:
      result = @"Serial Number";
      break;

    case InfoID::FirmwareVersion:
      result = @"Firmware Version";
      break;

    case InfoID::HardwareVersion:
      result = @"Hardware Version";
      break;

    case InfoID::MACAddress1:
      result = @"MAC address 1";
      break;

    case InfoID::MACAddress2:
      result = @"MAC address 2";
      break;

    case InfoID::DeviceName:
      result = @"Device Name";
      break;

    default:
      break;
  }

  return result;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:
            (UIInterfaceOrientation)interfaceOrientation {

  const auto &isiPad = ([[UIDevice currentDevice] userInterfaceIdiom] ==
                        UIUserInterfaceIdiomPad);
  const auto &isPortait =
      UIInterfaceOrientationIsPortrait(interfaceOrientation);

  // should only rotate with an iPad or if the orientation is portrait
  return (isiPad || isPortait);
}

- (NSObject *)createInfoProvider:(InfoIDEnum)infoID {
  NSObject *provider = nil;

  if ((self.device->contains<Info>(infoID)) &&
      (self.device->contains<InfoList>())) {

    const Info &info = self.device->infoData(infoID);
    const InfoList &infoList = self.device->get<InfoList>();
    if (infoList.contains(infoID)) {
      const auto &record = infoList.record_at(infoID);

      // Is it static?
      if (record.maxLength() == 0) {
        provider = [ICNormalCellProvider
            providerWithTitle:[ICDeviceInfoViewController
                                  stringForInfoID:infoID]
                        value:@(info.infoString().c_str())];
      }
      // It is editable
      else {
        ICInfoTextEditCellDelegate *delegate =
            [[ICInfoTextEditCellDelegate alloc] initWithDevice:self.device
                                                        infoID:infoID];

        provider = [ICTextEditCellProvider providerWithDelegate:delegate];
      }
    }
  }

  return provider;
}

@end
