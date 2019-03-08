/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ACK.h"
#import "AudioClockParm.h"
#import "MixerParm.h"
#import "AudioGlobalParm.h"
#import "AudioPortParm.h"
#import "CommandData.h"
#import "CommandDefines.h"
#import "ICAudioClockV2ChoiceDelegate.h"
#import "ICAudioFrameBufferV2RangeDelegate.h"
#import "ICAudioInfoViewControllerV2.h"
#import "ICAudioInputChannelsV2RangeDelegate.h"
#import "ICAudioOutputChannelsV2RangeDelegate.h"
#import "ICAudioPortNameV2TextEditCellDelegate.h"
#import "ICAudioSetupV2ChoiceDelegate.h"
#import "ICAudioSyncFactorV2RangeDelegate.h"
#import "ICAudioMixerConfigurationV2ChoiceDelegate.h"
#import "ICAudioMixerChannelV2ChoiceDelegate.h"
#import "ICChoiceCellProvider.h"
#import "ICNormalCellProvider.h"
#import "ICButtonCellProvider.h"
#import "ICPCEnabledV2SwitchDelegate.h"
#import "ICRangeCellProvider.h"
#import "ICRangeSelectionViewController.h"
#import "ICRunOnMain.h"
#import "ICSwitchCellProvider.h"
#import "ICTextEditCellProvider.h"
#import "ICViewController.h"
#import "ICiOSEnabledV2SwitchDelegate.h"
#import "MyAlgorithms.h"
#import "SaveRestore.h"
#import "ICAppDelegate.h"
#import "ICViewController.h"
#import "CommandList.h"

#import <boost/range/adaptors.hpp>
#import <string>

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace boost::adaptors;
using namespace std;

@interface ICAudioInfoViewControllerV2 ()

- (NSArray *)generatePortArray:(Word)portID;
- (void)initAudioV2:(NSMutableArray *)sectionArray
             titles:(NSMutableArray *)titles;

@end

@implementation ICAudioInfoViewControllerV2

- (id)initWithCommunicator:(CommPtr)_comm device:(DeviceInfoPtr)_device {
  self = [super init];
  if (self) {
    comm = _comm;
    device = _device;

    // Custom initialization
    auto *const tempSectionArray = [NSMutableArray array];
    auto *const tempSectionTitleArray = [NSMutableArray array];

    auto &audioGlobalParm = device->get<AudioGlobalParm>();
    if (audioGlobalParm.versionNumber() == 0x01) {
      [self initAudioV2:tempSectionArray titles:tempSectionTitleArray];
    }

    sectionArrays = tempSectionArray;
    sectionTitles = tempSectionTitleArray;
    change = false;
    justCanceled = NO;

    confirmAlert = [[UIAlertView alloc]
            initWithTitle:@"Audio Changes have not been committed."
                  message:nil
                 delegate:self
        cancelButtonTitle:@"Stay on this window"
        otherButtonTitles:@"Continue without saving", nil];

    commitConfirmAlert =
        [[UIAlertView alloc] initWithTitle:@"Commit changes to FLASH and reset?"
                                   message:nil
                                  delegate:self
                         cancelButtonTitle:@"Cancel"
                         otherButtonTitles:@"Commit", nil];

    commitCompleteAlert =
        [[UIAlertView alloc] initWithTitle:@"Commit Successful."
                                   message:@"The device will now reset."
                                  delegate:self
                         cancelButtonTitle:@"Ok"
                         otherButtonTitles:nil];

    commitFailedAlert = [[UIAlertView alloc] initWithTitle:@"Commit Failed."
                                                   message:@"Please try again."
                                                  delegate:nil
                                         cancelButtonTitle:@"Ok"
                                         otherButtonTitles:nil];

  }

  return self;
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [self.navigationController setNavigationBarHidden:FALSE animated:true];

  [self.navigationItem setTitle:@"Audio Information"];

  commitButton =
      [[UIBarButtonItem alloc] initWithTitle:@"Commit"
                                       style:UIBarButtonItemStyleBordered
                                      target:self
                                      action:@selector(onCommitPressed)];

  self.navigationItem.leftBarButtonItem = mainMenuButton;
  self.navigationItem.rightBarButtonItem = commitButton;

  assert(comm);

  [self saveOldValues];
  [self registerExclusiveAckHandler];

  runOnMainAfter(0.5, ^{ [self.tableView reloadData]; });
}

- (void)viewDidDisappear:(BOOL)animated {
  [super viewDidDisappear:animated];
  comm->unRegisterExclusiveHandler();
  NSLog(@"unregistering");
}

- (void) refreshInfo {
  NSLog(@"1justCanceled: %d, change: %d", justCanceled, change);
  change = !justCanceled;
  [super refreshInfo];
  [self registerExclusiveAckHandler];
  justCanceled = NO;
  NSLog(@"2justCanceled: %d, change: %d", justCanceled, change);
}

- (void) registerExclusiveAckHandler {
  comm->registerExclusiveHandler(
                                 Command::ACK, [self](CmdEnum, DeviceID, Word, commandData_t cmdData) {
                                   runOnMain(^{
                                     const auto &ack = cmdData.get<ACK>();

                                     NSLog(@"in special ack handler");

                                     if (ack.commandID() == Command::SaveRestore) {
                                       if (ack.errorCode() == 0) {
                                         change = false;
                                         [commitCompleteAlert show];
                                       } else {
                                         [commitFailedAlert show];
                                       }
                                       
                                     }
                                   });
                                 });
  NSLog(@"unregistering");
}

- (void)onMainMenuPressed {
  if (change) {
    [confirmAlert show];
  }
}

- (bool) confirm {
  NSLog(@"confirming! change: %d", change);
  return !change;
}

- (void)onCommitPressed {
  [commitConfirmAlert show];
}

- (void)alertView:(UIAlertView *)alertView
    clickedButtonAtIndex:(NSInteger)buttonIndex {
  if (alertView == commitCompleteAlert) {
    [[NSNotificationCenter defaultCenter]
        postNotificationName:kRequestResetNotification
                      object:nil];
    ICAppDelegate *appDelegate = (ICAppDelegate *)[[UIApplication sharedApplication] delegate];
    [[appDelegate getNavigationController] popViewControllerAnimated:YES];
  } else {
    if ((alertView == confirmAlert) && (buttonIndex == 1)) {  // continue Button
      change = false;
      justCanceled = YES;
      [self restoreOldValues];

      ICAppDelegate *appDelegate = (ICAppDelegate *)[[UIApplication sharedApplication] delegate];
      if ([[appDelegate getVisibleView] isKindOfClass:[ICViewController class]]) {
        ICViewController *vc = (ICViewController *) [appDelegate getVisibleView];
        [vc continueReplaceMainView];
      }
    } else {
      if ((alertView == commitConfirmAlert) && (buttonIndex == 1 ))
        device->send<SaveRestoreCommand>(SaveRestoreID::SaveToFlash);
    }
  }
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex {
  if (alertView == confirmAlert) {
    ICAppDelegate *appDelegate = (ICAppDelegate *)[[UIApplication sharedApplication] delegate];

    if ([[appDelegate getVisibleView] isKindOfClass:[ICViewController class]]) {
      ICViewController *vc = (ICViewController *) [appDelegate getVisibleView];
      [vc setAudioInfoActive];
    }
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

- (NSArray *)generatePortArray:(Word)portID {
  assert(device->contains<AudioPortParm>(portID));

  AudioPortParm &audioPortParm = device->get<AudioPortParm>(portID);

  AudioGlobalParm &audioGlobalParm = device->get<AudioGlobalParm>();
  const auto &currentActiveConfigID = audioGlobalParm.currentActiveConfig();
  const auto &block = audioPortParm.block_at(currentActiveConfigID);

  NSMutableArray *portArray = [NSMutableArray array];

  // port name
  {
    // is the name fixed?
    if (audioPortParm.maxPortName() == 0) {

      ICNormalCellProvider *cellProvider = [ICNormalCellProvider
          providerWithTitle:@"Name"
                      value:@(audioPortParm.portName().c_str())];
      [portArray addObject:cellProvider];
    }
    // name is not fixed
    else {
      ICAudioPortNameV2TextEditCellDelegate *delegate =
          [[ICAudioPortNameV2TextEditCellDelegate alloc] initWithDevice:device
                                                                 portID:portID];
      ICTextEditCellProvider *provider =
          [ICTextEditCellProvider providerWithDelegate:delegate];
      [portArray addObject:provider];
    }
  }

  // input channels
  if (block.minInputChannels() == block.maxInputChannels()) {
    NSString *numInputChannelsStr =
        [NSString stringWithFormat:@"%d", block.maxInputChannels()];
    ICNormalCellProvider *provider =
        [ICNormalCellProvider providerWithTitle:@"Input Channels"
                                          value:numInputChannelsStr];
    [portArray addObject:provider];
  } else {
    ICAudioInputChannelsV2RangeDelegate *delegate =
        [[ICAudioInputChannelsV2RangeDelegate alloc] initWithDevice:device
                                                             portID:portID];
    ICRangeCellProvider *provider =
        [ICRangeCellProvider providerWithDelegate:delegate];
    [portArray addObject:provider];
  }

  // output channels
  if (block.minOutputChannels() == block.maxOutputChannels()) {
    NSString *numOutputChannelsStr =
        [NSString stringWithFormat:@"%d", block.maxOutputChannels()];
    ICNormalCellProvider *provider =
        [ICNormalCellProvider providerWithTitle:@"Output Channels"
                                          value:numOutputChannelsStr];
    [portArray addObject:provider];
  } else {
    ICAudioOutputChannelsV2RangeDelegate *delegate =
        [[ICAudioOutputChannelsV2RangeDelegate alloc] initWithDevice:device
                                                              portID:portID];
    ICRangeCellProvider *provider =
        [ICRangeCellProvider providerWithDelegate:delegate];
    [portArray addObject:provider];
  }

  if (audioPortParm.isOfType(PortType::USBDevice)) {
    const auto &usbDevice = audioPortParm.usbDevice();

    // pc audio passthrough enabled
    if (usbDevice.supportsPCAudio()) {
      ICPCEnabledV2SwitchDelegate *delegate =
          [[ICPCEnabledV2SwitchDelegate alloc] initWithDevice:device
                                                       portID:portID];

      ICSwitchCellProvider *provider =
          [ICSwitchCellProvider providerWithDelegate:delegate];

      [portArray addObject:provider];
    }

    // iOS audio passthrough enabled
    if (usbDevice.supportsIOSAudio()) {
      ICiOSEnabledV2SwitchDelegate *delegate =
          [[ICiOSEnabledV2SwitchDelegate alloc] initWithDevice:device
                                                        portID:portID];

      ICSwitchCellProvider *provider =
          [[ICSwitchCellProvider alloc] initWithDelegate:delegate];

      [portArray addObject:provider];
    }
  }

  return portArray;
}

- (void)initAudioV2:(NSMutableArray *)sectionArray
             titles:(NSMutableArray *)titles {
  [self initCommitChanges:sectionArray titles:titles];

  [titles addObject:@"Audio Information"];

  __block NSMutableArray *infoSection = [NSMutableArray array];

  auto addInfoSection = ^(NSString *const title, int value) {
    NSString *valueStr = [NSString stringWithFormat:@"%d", value];

    ICNormalCellProvider *titleProvider =
        [ICNormalCellProvider providerWithTitle:title value:valueStr];

    [infoSection addObject:titleProvider];
  };

  assert(device->contains<AudioGlobalParm>());
  AudioGlobalParm audioGlobalParm = device->get<AudioGlobalParm>();

  // Number of ports
  if (audioGlobalParm.numAudioPorts() > 0) {
    addInfoSection(@"Number of Audio Ports", audioGlobalParm.numAudioPorts());
  }

  // Audio Frame
  {
    ICAudioFrameBufferV2RangeDelegate *audioFrameBufferRangeDelegate =
        [[ICAudioFrameBufferV2RangeDelegate alloc] initWithDevice:device];

    ICRangeCellProvider *rangeCellProvider = [ICRangeCellProvider
        providerWithDelegate:audioFrameBufferRangeDelegate];

    [infoSection addObject:rangeCellProvider];
  }

  // Audio Sync
  {
    ICAudioSyncFactorV2RangeDelegate *audioSyncFactorRangeDelegate =
        [ICAudioSyncFactorV2RangeDelegate syncFactorWithDevice:device];

    ICRangeCellProvider *rangeCellProvider =
        [ICRangeCellProvider providerWithDelegate:audioSyncFactorRangeDelegate];

    [infoSection addObject:rangeCellProvider];
  }

  // Audio Setup
  {
    ICAudioSetupV2ChoiceDelegate *audioSetupChoiceDelegate =
        [ICAudioSetupV2ChoiceDelegate audioSetupWithDevice:device];

    ICChoiceCellProvider *setupChoiceProvider =
        [ICChoiceCellProvider providerWithDelegate:audioSetupChoiceDelegate];

    [infoSection addObject:setupChoiceProvider];
  }

  // Audio Clock Selection
  if (device->contains<AudioClockParm>()) {
    ICAudioClockV2ChoiceDelegate *audioClockChoiceDelegate =
        [[ICAudioClockV2ChoiceDelegate alloc] initWithDevice:device];

    ICChoiceCellProvider *clockSelectProvider =
        [ICChoiceCellProvider providerWithDelegate:audioClockChoiceDelegate];
    [infoSection addObject:clockSelectProvider];
  }

  // Add all the info to the section array
  [sectionArray addObject:infoSection];

  [self initMixer:sectionArray titles:titles];

  // Ports
  for (Word portID = 1; portID <= audioGlobalParm.numAudioPorts(); ++portID) {
    assert(device->contains<AudioPortParm>(portID));
    AudioPortParm audioPortParm = device->get<AudioPortParm>(portID);

    NSMutableString *title =
        [NSMutableString stringWithFormat:@"Port %d", portID];

    if (audioPortParm.isOfType(PortType::USBDevice)) {
      [title
          appendFormat:@" (USB Device %d)", audioPortParm.usbDevice().jack()];
    }
    // TODO: add other audio sources here

    [titles addObject:title];
    [sectionArray addObject:[self generatePortArray:portID]];
  }
}

- (void)initMixer:(NSMutableArray *)sectionArray
             titles:(NSMutableArray *)titles {
  [titles addObject:@"Mixer Information"];

  __block NSMutableArray *infoSection = [NSMutableArray array];

//  auto addInfoSection = ^(NSString *const title, int value) {
//    NSString *valueStr = [NSString stringWithFormat:@"%d", value];
//
//    ICNormalCellProvider *titleProvider =
//    [ICNormalCellProvider providerWithTitle:title value:valueStr];
//
//    [infoSection addObject:titleProvider];
//  };

  const auto &audioGlobalParm = device->get<AudioGlobalParm>();

  // Mixer Configuration
  if (device->contains<MixerParm>(audioGlobalParm.currentActiveConfig())) {
    ICAudioMixerConfigurationV2ChoiceDelegate *mixerConfigurationDelegate =
    [[ICAudioMixerConfigurationV2ChoiceDelegate alloc] initWithDevice:device];

    ICChoiceCellProvider *mixerConfigSelectProvider =
    [ICChoiceCellProvider providerWithDelegate:mixerConfigurationDelegate];
    [infoSection addObject:mixerConfigSelectProvider];

    for (int i = 1; i <= audioGlobalParm.numAudioPorts(); i++) {
      ICAudioMixerChannelV2ChoiceDelegate *mixerInDelegate = [[ICAudioMixerChannelV2ChoiceDelegate alloc] initWithDevice:device portID:i isInput:true];
      ICAudioMixerChannelV2ChoiceDelegate *mixerOutDelegate = [[ICAudioMixerChannelV2ChoiceDelegate alloc] initWithDevice:device portID:i isInput:false];

      ICChoiceCellProvider *mixerInSelectProvider =
      [ICChoiceCellProvider providerWithDelegate:mixerInDelegate];
      ICChoiceCellProvider *mixerOutSelectProvider =
      [ICChoiceCellProvider providerWithDelegate:mixerOutDelegate];

      [infoSection addObject:mixerInSelectProvider];
      [infoSection addObject:mixerOutSelectProvider];
    }
  }

  // Add all the info to the section array
  [sectionArray addObject:infoSection];
}

- (void)initCommitChanges:(NSMutableArray *)sectionArray
           titles:(NSMutableArray *)titles {
  [titles addObject:@"Save settings?"];

  __block NSMutableArray *infoSection = [NSMutableArray array];

  ICButtonCellProvider *saveCell = [ICButtonCellProvider
                                     providerWithTitle:@"Commit changes and reset device"
                                     value:@""
                                    func:^(void) {
                                      [commitConfirmAlert show];
                                    }
                                    ];


  ICButtonCellProvider *cancelCell = [ICButtonCellProvider
                                     providerWithTitle:@"Revert to last saved configuration"
                                     value:@""
                                      func:^(void) {
                                        justCanceled = YES;
                                        [self restoreOldValues];
                                        self->device->rereadAudioInfo();

                                        runOnMainAfter(0.5, ^{
                                          [self saveOldValues];
                                          [self refreshInfo];
                                        });
                                      }];

  [infoSection addObject:saveCell];
  [infoSection addObject:cancelCell];

  // Add all the info to the section array
  [sectionArray addObject:infoSection];
}

- (void) saveOldValues
{
  oldAudioGlobalParm = device->get<AudioGlobalParm>();
  oldAudioPortParms.clear();
  for (int i = 1; i <= oldAudioGlobalParm.numAudioPorts(); i++) {
    oldAudioPortParms.push_back(device->get<AudioPortParm>(i));
  }
  oldAudioClockParm = device->get<AudioClockParm>();
  if (device->get<CommandList>().contains(Command::GetMixerParm)) {
    oldMixerParm = device->get<MixerParm>(oldAudioGlobalParm.currentActiveConfig());
    oldMixerPortParm = device->get<MixerPortParm>();
  }
}

- (void) restoreOldValues {
  device->send<SetAudioGlobalParmCommand>(oldAudioGlobalParm);
  for (int i = 0; i < oldAudioGlobalParm.numAudioPorts(); i++) {
    if (oldAudioPortParms.size() > i) {
      device->send<SetAudioPortParmCommand>(oldAudioPortParms.at(i));
    }
  }
  device->send<SetAudioClockParmCommand>(oldAudioClockParm);
  if (device->get<CommandList>().contains(Command::GetMixerParm)) {
    device->send<SetMixerParmCommand>(oldMixerParm);
    device->send<SetMixerPortParmCommand>(oldMixerPortParm);
  }
}

@end
