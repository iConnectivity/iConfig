/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICInfoTextEditCellDelegate.h"
#import "Info.h"
#import "InfoList.h"
#import <string>

using namespace GeneSysLib;
using namespace std;

@interface ICInfoTextEditCellDelegate ()

@property(nonatomic, assign) DeviceInfoPtr device;
@property(nonatomic, assign) InfoIDEnum infoID;

@end

@implementation ICInfoTextEditCellDelegate

+ (NSString *)stringForInfoID:(InfoIDEnum)infoID {
  NSString *result = @"Unknown Information";
  switch (infoID) {
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

- (id)initWithDevice:(DeviceInfoPtr)device infoID:(InfoIDEnum)infoID {
  self = [super init];

  if (self) {
    NSParameterAssert(device);
    self.device = device;
    self.infoID = infoID;
  }

  return self;
}

- (NSString *)title {
  return [ICInfoTextEditCellDelegate stringForInfoID:self.infoID];
}

- (NSString *)getValue {
  const Info &info = self.device->infoData(self.infoID);
  return @(info.infoString().c_str());
}

- (void)setValue:(NSString *)value {
  Info &info = self.device->infoData(self.infoID);
  info.infoString(string([value cStringUsingEncoding:NSASCIIStringEncoding],
                         [value length]));
  self.device->send<SetInfoCommand>(info);
}

- (NSInteger)maxLength {
  const InfoList &infoList = self.device->get<InfoList>();
  const auto &record = infoList.record_at(self.infoID);
  return (NSInteger)record.maxLength();
}

@end
