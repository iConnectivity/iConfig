/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICMainMenuProvider.h"
#import "ICDeviceSelectionViewController.h"
#import "ICViewController.h"
#import "ICBaseInfoViewController.h"
#import "MyAlgorithms.h"
#import "Info.h"
#import "InfoList.h"
#import "DevicePID.h"
#import "Device.h"
#import "DeviceInfo.h"
#import "CommandList.h"
#import "ResetList.h"
#import "SaveRestoreList.h"
#import <UIKit/UIKit.h>

#import "MIDIInfo.h"

#define kSearchTimeoutTime 0.265f //2017-06-07

using namespace MyAlgorithms;
using namespace GeneSysLib;

@interface ICDeviceSelectionViewController () {
  UIAlertView *deviceNotSupportedAlertView;
}
@end

@implementation ICDeviceSelectionViewController

- (id)initWithCommunicator:(CommPtr)communicator {
  self = [super initWithNibName:@"ICDeviceSelectionViewController" bundle:nil];
  if (self) {
    // Custom initialization
    self.comm = communicator;
    inHandler = false;
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view from its nib.
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
  // Dispose of any resources that can be recreated.
}

- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];
  [self startSearch];
}

- (NSString *)tableView:(UITableView *)tableView
    titleForHeaderInSection:(NSInteger)section {
  return (deviceList.size() == 0) ? @"No Devices Found" : @"";
}

- (NSInteger)tableView:(UITableView *)tableView
    numberOfRowsInSection:(NSInteger)section {
  return deviceList.size();
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  static NSString *const cellIdentifiter = @"DiscoveryCell";

  UITableViewCell *cell =
      [tableView dequeueReusableCellWithIdentifier:cellIdentifiter];
  if (cell == NULL) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle
                                  reuseIdentifier:cellIdentifiter];
    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
  }

  const auto &device = [self deviceForIndex:indexPath.row];

  // TPK - may want to look at this as a source of bugs
  if (device) {
    const auto &containsDeviceName = device->containsInfo(InfoID::DeviceName);
    const auto &containsAccessoryName =
        device->containsInfo(InfoID::AccessoryName);

    auto snum = device->getSerialNumber();

    uint32_t serialNumber = snum[0];
    serialNumber = (serialNumber << 7) | snum[1];
    serialNumber = (serialNumber << 7) | snum[2];
    serialNumber = (serialNumber << 7) | snum[3];
    serialNumber = (serialNumber << 7) | snum[4];

    if ((containsDeviceName) && (containsAccessoryName)) {
      const auto &deviceName = device->infoData(InfoID::DeviceName);
      const auto &accessoryName = device->infoData(InfoID::AccessoryName);

      cell.textLabel.text = @(deviceName.infoString().c_str());
      cell.detailTextLabel.text = [NSString
          stringWithFormat:@"%s (%08X)", accessoryName.infoString().c_str(),
                           serialNumber];
    } else if (containsAccessoryName) {
      const auto &accessoryName = device->infoData(InfoID::AccessoryName);
      cell.textLabel.text = @(accessoryName.infoString().c_str());
      cell.detailTextLabel.text =
          [NSString stringWithFormat:@"%08X", serialNumber];
    } else {
      cell.textLabel.text = [NSString stringWithFormat:@"%08X", serialNumber];
    }

    UIImage *image = nil;
    switch (device->getDeviceID().pid()) {
      case DevicePID::MIO:
        image = [UIImage imageNamed:@"block_miodevice.png"];
        break;

      case DevicePID::iConnect1:
        image = [UIImage imageNamed:@"block_icm1device.png"];
        break;

      case DevicePID::iConnect2Plus:
        image = [UIImage imageNamed:@"block_icm2plusdevice.png"];
        break;

      case DevicePID::iConnect4Plus:
        image = [UIImage imageNamed:@"block_icm4plusdevice.png"];
        break;

      default:
        break;
    }

    cell.imageView.image = image;
  }
    
  return cell;
}

- (CGFloat)tableView:(UITableView *)tableView
    heightForRowAtIndexPath:(NSIndexPath *)indexPath {
  return 120.0f;
}

- (void)tableView:(UITableView *)tableView
    didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  auto selectedDevice = [self deviceForIndex:indexPath.row];

  if (selectedDevice) {
    const auto &transID = selectedDevice->getTransID();
    self.comm->setCurrentOutput(transID);

    auto *const baseViewController = [[ICViewController alloc]
        initWithProvider:[[ICMainMenuProvider alloc] init]
            communicator:self.comm
                  device:selectedDevice];

    [self.navigationController setViewControllers:@[ baseViewController ]
                                         animated:YES];
  }
}

- (void)viewDidUnload {
  [self setSearchingView:nil];
  [self setMyTableView:nil];
  [super viewDidUnload];
}

- (DeviceInfoPtr)deviceForIndex:(NSInteger)index {
  DeviceInfoPtr selectedDevice;

  if ((index >= 0) && (index < (int)deviceList.size())) {
    auto item = deviceList.begin();
    for (auto i = 0; i < index; ++i) {
      item++;
    }
    selectedDevice = *item;
  }
  return selectedDevice;
}

- (DeviceInfoPtr)deviceForDeviceID:(DeviceID)deviceID {
  DeviceInfoPtr selectedDevice;

  auto device = find_if(deviceList.begin(), deviceList.end(),
                        [=](const DeviceInfoPtr &dev) {
    return (dev->getDeviceID() == deviceID);
  });

  if (device != deviceList.end()) {
    selectedDevice = *device;
  }
  return selectedDevice;
}

- (IBAction)startSearch {
  //???????
/*  if([NSThread isMainThread] == YES)
  {
      dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0);
      dispatch_async(queue, ^(void) {
          [self startSearch];
      });
      
      return;
  }
 */
  //????????????
    
    
  self.searchingView.frame = self.view.frame;
  [self.view addSubview:self.searchingView];

  deviceList.clear();
  while (!pendingSysex.empty()) {
    pendingSysex.pop();
  }

  bool opened = false;

  foundDevice = NO;

  try {
    if ((opened = self.comm->openAllInputs())) {
      opened = self.comm->openAllOutputs();
    }

    if (opened) {
      for (auto &item : registeredHandlers) {
        self.comm->unRegisterHandler(item.first, item.second);
      }
      registeredHandlers.clear();

      static int handlerID = 0;
      int currHandler = handlerID++;

      const auto &deviceHandler = [self, currHandler](
          CmdEnum cmd, DeviceID deviceID, Word transID, commandData_t cmdData) {
        inHandler = YES;

        auto device = [self deviceForDeviceID:deviceID];

        // was the device already found with prior calls?
        // if not it is added to the deviceList
        if (!device) {
          foundDevice = YES;
          auto &devInfo = cmdData.get<Device>();

          if (devInfo.mode() == BootMode::AppMode) {
            device =
                DeviceInfoPtr(new DeviceInfo(self.comm, deviceID, transID));

            device->addCommandData(cmdData);
            deviceList.push_back(device);

            pendingSysex.push(make_pair(
                transID, sysex(GetCommandListCommand(deviceID, transID))));
          }
        }
        inHandler = NO;
      };
      registeredHandlers[Command::RetDevice] =
          self.comm->registerHandler(Command::RetDevice, deviceHandler);

      const auto &commandListHandler = [self](
          CmdEnum cmd, DeviceID deviceID, Word transID, commandData_t cmdData) {
        inHandler = YES;
        // NSLog(@"Command List");
        const auto &device = [self deviceForDeviceID:deviceID];

        if (device) {
          foundDevice = YES;
          device->addCommandData(cmdData);

          const auto &commandList = cmdData.get<CommandList>();

          self.comm->setCurrentOutput(transID);
          if (commandList.contains(Command::GetMIDIInfo)) {
            pendingSysex.push(make_pair(
                transID, sysex(GetMIDIInfoCommand(deviceID, transID))));
          }

          if (commandList.contains(Command::GetInfoList)) {
            pendingSysex.push(make_pair(
                transID, sysex(GetInfoListCommand(deviceID, transID))));
          }

          if (commandList.contains(Command::GetResetList)) {
            pendingSysex.push(make_pair(
                transID, sysex(GetResetListCommand(deviceID, transID))));
          }

          if (commandList.contains(Command::GetSaveRestoreList)) {
            pendingSysex.push(make_pair(
                transID, sysex(GetSaveRestoreListCommand(deviceID, transID))));
          }
          [self sendNextSysex];
        }
        inHandler = NO;
      };
      registeredHandlers[Command::RetCommandList] = self.comm->registerHandler(
          Command::RetCommandList, commandListHandler);

      const auto &midiInfoHandler = [self](CmdEnum cmd, DeviceID deviceID, Word,
                                           commandData_t cmdData) {
        inHandler = YES;
        // NSLog(@"MIDI Info");
        const auto &device = [self deviceForDeviceID:deviceID];

        if (device) {
          foundDevice = YES;
          device->addCommandData(cmdData);
          [self sendNextSysex];
        }
        inHandler = NO;
      };
      registeredHandlers[Command::RetMIDIInfo] =
          self.comm->registerHandler(Command::RetMIDIInfo, midiInfoHandler);

      const auto &resetListHandler = [self](CmdEnum cmd, DeviceID deviceID,
                                            Word, commandData_t cmdData) {
        inHandler = YES;
        // NSLog(@"Reset List");
        const auto &device = [self deviceForDeviceID:deviceID];

        if (device) {
          foundDevice = YES;
          device->addCommandData(cmdData);
          [self sendNextSysex];
        }
        inHandler = NO;
      };
      registeredHandlers[Command::RetResetList] =
          self.comm->registerHandler(Command::RetResetList, resetListHandler);

      const auto &saveRestoreListHandler = [self](
          CmdEnum cmd, DeviceID deviceID, Word, commandData_t cmdData) {
        inHandler = YES;
        // NSLog(@"Save Restore List");
        const auto &device = [self deviceForDeviceID:deviceID];

        if (device) {
          foundDevice = YES;
          device->addCommandData(cmdData);
          [self sendNextSysex];
        }
        inHandler = NO;
      };
      registeredHandlers[Command::RetSaveRestoreList] =
          self.comm->registerHandler(Command::RetSaveRestoreList,
                                     saveRestoreListHandler);

      const auto &infoListHandler = [self](
          CmdEnum cmd, DeviceID deviceID, Word transID, commandData_t cmdData) {
        inHandler = YES;
        // NSLog(@"Ret Info List");
        const auto &device = [self deviceForDeviceID:deviceID];

        if (device) {
          foundDevice = YES;
          device->addCommandData(cmdData);

          const auto &infoList = cmdData.get<InfoList>();
          const auto &containsDeviceName =
              infoList.contains(InfoID::DeviceName);
          const auto &containsAccessoryName =
              infoList.contains(InfoID::AccessoryName);

          if (containsDeviceName) {
            pendingSysex.push(make_pair(
                transID,
                GetInfoCommand(deviceID, transID, InfoID::DeviceName).sysex()));
          }

          if (containsAccessoryName) {
            pendingSysex.push(make_pair(
                transID, GetInfoCommand(deviceID, transID,
                                        InfoID::AccessoryName).sysex()));
          }

          [self sendNextSysex];
        }
        inHandler = NO;
      };
      registeredHandlers[Command::RetInfoList] =
          self.comm->registerHandler(Command::RetInfoList, infoListHandler);

      const auto &infoHandler = [self](CmdEnum, DeviceID deviceID, Word,
                                       commandData_t cmdData) {
        inHandler = YES;
        const auto &device = [self deviceForDeviceID:deviceID];

        if (device) {
          foundDevice = YES;
          device->addCommandData(cmdData);

          [self sendNextSysex];
        }
        inHandler = NO;
      };
      registeredHandlers[Command::RetInfo] =
          self.comm->registerHandler(Command::RetInfo, infoHandler);

      for (auto i = 0; i < self.comm->getOutCount(); ++i) {
        pendingGetDevice.push(
            make_pair(i, sysex(GetDeviceCommand(DeviceID(), i))));
      }

      // Send first sysex
      [self sendNextGetDevice];
    } else {
      NSLog(@"Could not open device!!!");
    }
  }
  catch (...) {
    NSLog(@"Could not open device!!!");
  }
  [self startSearchTimer];
}

- (void)deviceTimerTick {
  if (!pendingGetDevice.empty()) {
    [self sendNextGetDevice];
  } else {
    [self startInfoTimer];
  }
}

- (void)searchComplete {
/*
    if([NSThread isMainThread] == NO)
    {
        [self performSelectorOnMainThread:@selector(searchComplete) withObject:nil waitUntilDone:NO];
        return;
    }
*/    
   [self stopSearchTimer];

//????????????????
/*    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Test searchComplete"
                                                    message:@"Search Done."
                                                   delegate:self
                                          cancelButtonTitle:@"OK"
                                          otherButtonTitles:nil];
    [alert show];
*/
  if (inHandler)
  {
    [self startSearchTimer];
    return;
  }
  else if (!pendingSysex.empty())
  {
    [self sendNextSysex];
    [self startSearchTimer];
    return;
  }

  for (auto &item : registeredHandlers)
  {
    self.comm->unRegisterHandler(item.first, item.second);
  }
  registeredHandlers.clear();
///*????
  [self.searchingView removeFromSuperview];
  [self.myTableView reloadData];
//*/
}

- (void)sendNextSysex {
   
  if (!pendingSysex.empty()) {
    auto nextSysex = pendingSysex.front();
    NSLog(@"Send next Sysex for Transaction ID:%i", nextSysex.first);
    pendingSysex.pop();
    self.comm->setCurrentOutput(nextSysex.first);
    self.comm->sendSysex(nextSysex.second);
  }
}

- (void)sendNextGetDevice {
  if (!pendingGetDevice.empty()) {
    auto nextGetDevice = pendingGetDevice.front();
    NSLog(@"Sending next GetDevice %i", nextGetDevice.first);
    pendingGetDevice.pop();
    self.comm->setCurrentOutput(nextGetDevice.first);
    self.comm->sendSysex(nextGetDevice.second);
    [self startSearchTimer];
  }
}

- (void)startSearchTimer {
  [self stopSearchTimer];

/*
  searchTimer = [NSTimer timerWithTimeInterval:kSearchTimeoutTime //kTimeoutTime //2017-06-07
                                        target:self
                                      selector:@selector(deviceTimerTick)
                                      userInfo:nil
                                       repeats:NO];
*/
    searchTimer = [NSTimer scheduledTimerWithTimeInterval:kSearchTimeoutTime //kTimeoutTime //2017-06-07
                                          target:self
                                        selector:@selector(deviceTimerTick)
                                        userInfo:nil
                                         repeats:NO];
 //zx, 2017-06-08
 //[[NSRunLoop currentRunLoop] addTimer:searchTimer
 //                              forMode:NSDefaultRunLoopMode];
}

- (void)restartSearchTimer {
  [self startSearchTimer];
}

- (void)stopSearchTimer {
  if (searchTimer != nil) {
    [searchTimer invalidate];
    searchTimer = nil;
  }
}

- (void)startInfoTimer {
  [self stopInfoTimer];

/*
  infoTimer = [NSTimer timerWithTimeInterval:kSearchTimeoutTime //kTimeoutTime //zx, 2017-06-07
                                      target:self
                                    selector:@selector(searchComplete)
                                    userInfo:nil
                                     repeats:NO];
*/
    infoTimer = [NSTimer scheduledTimerWithTimeInterval:kSearchTimeoutTime //kTimeoutTime //zx, 2017-06-07
                                        target:self
                                      selector:@selector(searchComplete)
                                      userInfo:nil
                                       repeats:NO];
    //zx, 2017-06-08
  //[[NSRunLoop currentRunLoop] addTimer:infoTimer forMode:NSDefaultRunLoopMode];
}

- (void)restartInfoTimer {
  [self startInfoTimer];
}

- (void)stopInfoTimer {
  if (infoTimer != nil) {
    [infoTimer invalidate];
    infoTimer = nil;
  }
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

@end
