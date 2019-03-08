/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ACK.h"
#import "AudioCfgInfo.h"
#import "AudioClockInfo.h"
#import "AudioInfo.h"
#import "AudioPortCfgInfo.h"
#import "AudioPortInfo.h"
#import "CommandData.h"
#import "CommandDefines.h"
#import "ICAudioClockV1ChoiceDelegate.h"
#import "ICAudioFrameBufferV1RangeDelegate.h"
#import "ICAudioInfoViewControllerV1.h"
#import "ICAudioInputChannelsV1RangeDelegate.h"
#import "ICAudioOutputChannelsV1RangeDelegate.h"
#import "ICAudioSetupV1ChoiceDelegate.h"
#import "ICAudioSyncFactorV1RangeDelegate.h"
#import "ICChoiceCellProvider.h"
#import "ICNormalCellProvider.h"
#import "ICPCEnabledV1SwitchDelegate.h"
#import "ICRangeCellProvider.h"
#import "ICRangeSelectionViewController.h"
#import "ICRunOnMain.h"
#import "ICSwitchCellProvider.h"
#import "ICTextEditCellProvider.h"
#import "ICViewController.h"
#import "ICiOSEnabledV1SwitchDelegate.h"
#import "MyAlgorithms.h"
#import "SaveRestore.h"
#import "ICAudioPortNameV1TextEditCellDelegate.h"
#import "ICAppDelegate.h"
#import "ICViewController.h"

#import <boost/range/adaptors.hpp>
#import <string>

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace boost::adaptors;
using namespace std;

@interface ICAudioInfoViewControllerV1 ()

- (NSArray *)generatePortArray:(Word)portID;
- (void)initAudioV1:(NSMutableArray *)sectionArray
             titles:(NSMutableArray *)titles;

@end

@implementation ICAudioInfoViewControllerV1

- (id)initWithCommunicator:(CommPtr)_comm device:(DeviceInfoPtr)_device {
  self = [super init];
  if (self) {
    comm = _comm;
    device = _device;

    // Custom initialization
    auto *const tempSectionArray = [NSMutableArray array];
    auto *const tempSectionTitleArray = [NSMutableArray array];

    assert(device->containsCommandData(Command::RetAudioInfo));
    const auto &audioInfo = device->get<AudioInfo>();
    if (audioInfo.versionNumber() == 0x01) {
      [self initAudioV1:tempSectionArray titles:tempSectionTitleArray];
    }

    sectionArrays = tempSectionArray;
    sectionTitles = tempSectionTitleArray;
    change = false;

    confirmAlert = [[UIAlertView alloc]
            initWithTitle:@"Audio Changes have not been committed."
                  message:nil
                 delegate:self
        cancelButtonTitle:@"Stay here"
        otherButtonTitles:@"Continue without saving", nil];

    commitConfirmAlert =
        [[UIAlertView alloc] initWithTitle:@"Commit Changes to FLASH and reset."
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

  comm->registerExclusiveHandler(
      Command::ACK, [self](CmdEnum, DeviceID, Word, commandData_t cmdData) {
        runOnMain(^{
            const auto &ack = cmdData.get<ACK>();

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

  runOnMainAfter(0.5, ^{ [self.tableView reloadData]; });
}

- (void)viewDidDisappear:(BOOL)animated {
  [super viewDidDisappear:animated];
  comm->unRegisterExclusiveHandler();
}

- (void)onMainMenuPressed {
  if (change) {
    [confirmAlert show];
  }
}

- (bool) confirm {
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
    [self.navigationController popViewControllerAnimated:YES];
  } else {
    if ((alertView == confirmAlert) && (buttonIndex == 1)) {  // continue Button
      change = false;
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
  assert(device->containsCommandData(Command::RetAudioCfgInfo));
  __block auto &audioCfgInfo = device->get<AudioCfgInfo>();
  __block auto &audioPortInfo = device->get<AudioPortInfo>(portID);
  __block NSMutableArray *portArray = [NSMutableArray array];

  // audio Port Name
  {
    if (audioPortInfo.maxPortName() == 0) {
      ICNormalCellProvider *provider = [ICNormalCellProvider
          providerWithTitle:@"Name"
                      value:@(audioPortInfo.portName().c_str())];
      [portArray addObject:provider];
    } else {
      ICAudioPortNameV1TextEditCellDelegate *delegate =
          [[ICAudioPortNameV1TextEditCellDelegate alloc] initWithDevice:device
                                                                 portID:portID];
      ICTextEditCellProvider *provider =
          [ICTextEditCellProvider providerWithDelegate:delegate];
      [portArray addObject:provider];
    }
  }

  if (device->contains<AudioPortCfgInfo>(audioPortInfo.portID())) {
    __block auto &audioPortCfgInfo =
        device->get<AudioPortCfgInfo>(audioPortInfo.portID());

    if (audioCfgInfo.currentActiveConfig() <=
        audioPortCfgInfo.numConfigBlocks()) {

      const auto &block =
          audioPortCfgInfo.block_at(audioCfgInfo.currentActiveConfig() - 1);

      if (block.minInputChannels() == block.maxInputChannels()) {
        NSString *numInputChannelStr = [NSString
            stringWithFormat:@"%d", audioPortCfgInfo.numInputChannels()];

        ICNormalCellProvider *inputChannelProvider =
            [ICNormalCellProvider providerWithTitle:@"Input Channels"
                                              value:numInputChannelStr];

        [portArray addObject:inputChannelProvider];
      } else {
        ICAudioInputChannelsV1RangeDelegate *inputChannelsRangeDelegate =
            [[ICAudioInputChannelsV1RangeDelegate alloc] initWithDevice:device
                                                                 portID:portID];

        ICRangeCellProvider *rangeCellProvider = [ICRangeCellProvider
            providerWithDelegate:inputChannelsRangeDelegate];

        [portArray addObject:rangeCellProvider];
      }

      if (block.minOutputChannels() == block.maxOutputChannels()) {
        NSString *numOutChannelStr = [NSString
            stringWithFormat:@"%d", audioPortCfgInfo.numOutputChannels()];

        ICNormalCellProvider *outputChannelProvider =
            [ICNormalCellProvider providerWithTitle:@"Output Channels"
                                              value:numOutChannelStr];

        [portArray addObject:outputChannelProvider];
      } else {
        ICAudioOutputChannelsV1RangeDelegate *outputChannelsRangeDelegate =
            [[ICAudioOutputChannelsV1RangeDelegate alloc]
                initWithDevice:device
                        portID:portID];

        ICRangeCellProvider *rangeCellProvider = [ICRangeCellProvider
            providerWithDelegate:outputChannelsRangeDelegate];

        [portArray addObject:rangeCellProvider];
      }
    }
  }

  if (audioPortInfo.portType() == PortType::USBDevice) {
    // PC audio passthrough
    if (audioPortInfo.supportsPCAudio()) {
      ICPCEnabledV1SwitchDelegate *pcEnabledSwichDelegate =
          [[ICPCEnabledV1SwitchDelegate alloc] initWithDevice:device
                                                     portID:portID];
      ICSwitchCellProvider *pcEnabledProvider =
          [ICSwitchCellProvider providerWithDelegate:pcEnabledSwichDelegate];
      [portArray addObject:pcEnabledProvider];
    }

    // iOS audio passthrough
    if (audioPortInfo.supportsIOSAudio()) {
      ICiOSEnabledV1SwitchDelegate *iOSEnabledSwitchDelegate =
          [[ICiOSEnabledV1SwitchDelegate alloc] initWithDevice:device
                                                        portID:portID];
      ICSwitchCellProvider *iosEnabledProvider =
          [ICSwitchCellProvider providerWithDelegate:iOSEnabledSwitchDelegate];
      [portArray addObject:iosEnabledProvider];
    }
  }

  return portArray;
}

- (void)initAudioV1:(NSMutableArray *)sectionArray
             titles:(NSMutableArray *)titles {
  [titles addObject:@"Audio Information"];

  const AudioInfo &audioInfo = device->get<AudioInfo>();
  __block NSMutableArray *infoSection = [NSMutableArray array];

  auto addInfoSecion = ^(NSString *const title, int value) {
    NSString *valueStr = [NSString stringWithFormat:@"%d", value];

    ICNormalCellProvider *titleProvider =
        [ICNormalCellProvider providerWithTitle:title value:valueStr];

    [infoSection addObject:titleProvider];
  };

  addInfoSecion(@"Number of Audio Ports", audioInfo.numberOfAudioPorts());

  if (audioInfo.numberOfUSBDeviceJacks()) {
    addInfoSecion(@"Audio USB Device Jacks",
                  audioInfo.numberOfUSBDeviceJacks());
  }

  if (audioInfo.numberOfUSBHostJacks()) {
    addInfoSecion(@"Audio USB Host Jacks", audioInfo.numberOfUSBHostJacks());

    addInfoSecion(@"Ports / USB Host Jacks",
                  audioInfo.numberOfPortsPerUSBHostJack());
  }

  if (audioInfo.numberOfEthernetJacks()) {
    addInfoSecion(@"Audio Ethernet Jacks", audioInfo.numberOfEthernetJacks());

    addInfoSecion(@"Ports / Ethernet Jack",
                  audioInfo.numberOfPortsPerEthernetJack());
  }

  if (device->containsCommandData(Command::RetAudioCfgInfo)) {
    // audio frame buffer
    {
      ICAudioFrameBufferV1RangeDelegate *audioFrameBufferRangeDelegate =
          [[ICAudioFrameBufferV1RangeDelegate alloc] initWithDevice:device];

      ICRangeCellProvider *rangeCellProvider = [ICRangeCellProvider
          providerWithDelegate:audioFrameBufferRangeDelegate];

      [infoSection addObject:rangeCellProvider];
    }

    // audio sync factor
    {
      ICAudioSyncFactorV1RangeDelegate *audioSyncFactorRangeDelegate =
          [ICAudioSyncFactorV1RangeDelegate syncFactorWithDevice:device];

      ICRangeCellProvider *rangeCellProvider = [ICRangeCellProvider
          providerWithDelegate:audioSyncFactorRangeDelegate];

      [infoSection addObject:rangeCellProvider];
    }

    // audio setup configuration
    {
      ICAudioSetupV1ChoiceDelegate *audioSetupChoiceDelegate =
          [ICAudioSetupV1ChoiceDelegate audioSetupWithDevice:device];

      ICChoiceCellProvider *setupChoiceProvider =
          [ICChoiceCellProvider providerWithDelegate:audioSetupChoiceDelegate];

      [infoSection addObject:setupChoiceProvider];

      [sectionArray addObject:infoSection];
    }

    // audio port
    for (int portID = 1; portID <= device->audioPortInfoCount(); ++portID) {
      NSMutableString *title =
          [NSMutableString stringWithFormat:@"Port %d", portID];

      auto &audioPortInfo = device->get<AudioPortInfo>(portID);

      if (audioPortInfo.portType() == PortType::USBDevice) {
        [title appendFormat:@" (USB Device %d)",
                            audioPortInfo.portInfo().usbDevice.jack];
      }
      // TODO: add other audio sources here

      [titles addObject:title];
      [sectionArray addObject:[self generatePortArray:portID]];
    }
  }

  // Audio Clock Selection
  if (device->containsCommandData(Command::RetAudioClockInfo)) {
    ICAudioClockV1ChoiceDelegate *audioClockChoiceDelegate =
        [[ICAudioClockV1ChoiceDelegate alloc] initWithDevice:device];

    ICChoiceCellProvider *clockSelectProvider =
        [ICChoiceCellProvider providerWithDelegate:audioClockChoiceDelegate];
    [infoSection addObject:clockSelectProvider];
  }
}

- (void) saveOldValues
{
    oldAudioInfo = device->get<AudioInfo>();
    oldAudioCfgInfo = device->get<AudioCfgInfo>();

    for (int i = 1; i <= oldAudioInfo.numberOfAudioPorts(); i++) {
      const auto &portCfgInfo = device->get<AudioPortCfgInfo>(i);
      const auto &audioPortInfo = device->get<AudioPortInfo>(i);

      oldPortNames.push_back(audioPortInfo.portName());
      oldPortIn.push_back(portCfgInfo.numInputChannels());
      oldPortOut.push_back(portCfgInfo.numOutputChannels());
      oldPortIOSEnabled.push_back(audioPortInfo.isIOSAudioEnabled());
      oldPortMacPCEnabled.push_back(audioPortInfo.isPCAudioEnabled());
    }
    oldAudioClockInfo = device->get<AudioClockInfo>();
}

- (void) restoreOldValues {
  device->send<SetAudioCfgInfoCommand>(oldAudioCfgInfo);
  oldAudioInfo = device->get<AudioInfo>();
  for (int i = 0; i < oldAudioInfo.numberOfAudioPorts(); i++) {
    auto &portCfgInfo = device->get<AudioPortCfgInfo>(i + 1);
    auto &audioPortInfo = device->get<AudioPortInfo>(i + 1);

    audioPortInfo.portName(oldPortNames.at(i));
    portCfgInfo.numInputChannels(oldPortIn.at(i));
    portCfgInfo.numOutputChannels(oldPortIn.at(i));
    audioPortInfo.setIOSAudioEnabled(oldPortIOSEnabled.at(i));
    audioPortInfo.setPCAudioEnabled(oldPortMacPCEnabled.at(i));

    device->send<SetAudioPortCfgInfoCommand>(portCfgInfo);
    usleep(2000);
    device->send<SetAudioPortInfoCommand>(audioPortInfo);
  }
  device->send<SetAudioClockInfoCommand>(oldAudioClockInfo);
}


@end
