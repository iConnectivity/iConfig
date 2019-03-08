/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "CommandData.h"
#import "CommandList.h"
#import "ICAudioInfoViewControllerV1.h"
#import "ICAudioInfoViewControllerV2.h"
#import "ICAudioPatchbayV1Delegate.h"
#import "ICAudioPatchbayV2Delegate.h"
#import "ICAudioPatchbayGridViewController.h"
#import "ICMIDIPatchbayDelegate.h"
#import "ICMIDIPatchbayGridViewController.h"
#import "ICMixerViewController.h"
#import "ICDeviceInfoViewController.h"
#import "ICMIDIInfoViewController.h"
#import "ICMainMenuProvider.h"
#import "MyAlgorithms.h"

using namespace GeneSysLib;
using namespace MyAlgorithms;

@interface ICMainMenuProvider ()

- (ButtonActionBlock)createDeviceInfoAction:(CommandList)commandList;
- (ButtonActionBlock)createMIDIInfoAction:(CommandList)commandList;
- (ButtonActionBlock)createAudioInfoActionV1:(CommandList)commandList;
- (ButtonActionBlock)createAudioInfoActionV2:(CommandList)commandList;
- (ButtonActionBlock)createAudioPatchbayV1Action;
- (ButtonActionBlock)createAudioPatchbayV2Action;
- (ButtonActionBlock)createMixerAction;
- (ButtonActionBlock)createMIDIPatchbayAction;

@end

@implementation ICMainMenuProvider

- (NSString *)providerName {
  return @"MainMenu";
}

- (void)providerWillAppear:(ICViewController *)sender {
  if (queryNotificationHandler) {
    [[NSNotificationCenter defaultCenter]
        removeObserver:queryNotificationHandler];
    queryNotificationHandler = nil;
  }
}

- (void)providerWillDissappear:(ICViewController *)sender {
  if (queryNotificationHandler) {
    [[NSNotificationCenter defaultCenter]
        removeObserver:queryNotificationHandler];
    queryNotificationHandler = nil;
  }
}

- (void)initializeProviderButtons:(ICViewController *)sender {
  queryNotificationHandler = nil;

  // This method requires the CommmandList of the device
  if (sender.device->containsCommandData(Command::RetCommandList)) {
    const auto &commandList = sender.device->get<CommandList>();

    // always add the device information menu item
    NSMutableArray *tempButtonNames = [NSMutableArray array];
    NSMutableArray *tempActionArray = [NSMutableArray array];

    // add a "Device Info" button if there is any device info on the device
    if (commandList.contains(Command::GetInfoList) ||
        commandList.contains(Command::GetInfo)) {
      [tempButtonNames addObject:@"Device Info"];
      [tempActionArray addObject:[self createDeviceInfoAction:commandList]];
    }

    // add an "Audio Info" (V1) button if supported by the device
    if (commandList.contains(Command::GetAudioInfo)) {
      [tempButtonNames addObject:@"Audio Info"];
      [tempActionArray addObject:[self createAudioInfoActionV1:commandList]];
    } else if ((commandList.contains(Command::GetAudioGlobalParm)) &&
               (commandList.contains(Command::GetAudioPortParm)) &&
               (commandList.contains(Command::GetAudioDeviceParm)) &&
               (commandList.contains(Command::GetAudioClockParm))) {
      // add an "Audio Info" (V2) button if supported by the device
      [tempButtonNames addObject:@"Audio Info"];
      [tempActionArray addObject:[self createAudioInfoActionV2:commandList]];
    }

    // add an "Audio Patchbay" (V1) button if supported by the device
    if (commandList.contains(Command::GetAudioPortInfo) &&
        commandList.contains(Command::GetAudioPortPatchbay)) {
      [tempButtonNames addObject:@"Audio Patchbay"];
      [tempActionArray addObject:[self createAudioPatchbayV1Action]];
    }

    if ((commandList.contains(Command::GetAudioGlobalParm)) &&
        (commandList.contains(Command::GetAudioPatchbayParm))) {
      [tempButtonNames addObject:@"Audio Patchbay"];
      [tempActionArray addObject:[self createAudioPatchbayV2Action]];
    }

    // add a "Mixer" button if there is any Mixer Parm on the device
    if (commandList.contains(Command::GetMixerParm)) {
      [tempButtonNames addObject:@"Audio Mixer"];
      [tempActionArray addObject:[self createMixerAction]];
    }

    // add a "MIDI Info" button if there is any MIDI info on the device
    if (commandList.contains(Command::GetMIDIInfo)) {
      [tempButtonNames addObject:@"MIDI Info"];
      [tempActionArray addObject:[self createMIDIInfoAction:commandList]];
    }

    // add a "MIDI Info" button if there is any MIDI info on the device
    if (commandList.contains(Command::GetMIDIPortRoute)) {
      [tempButtonNames addObject:@"MIDI Patchbay"];
      [tempActionArray addObject:[self createMIDIPatchbayAction]];
    }

    buttonNames = tempButtonNames;
    actionArray = tempActionArray;
  }
}
- (NSArray *)buttonNames {
  return buttonNames;
}

// This method handles all button presses by calling the action block for the
// corresponding button index
- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex {
  // Make sure that the button index is in range
  if ((buttonIndex >= 0) && (buttonIndex < [actionArray count])) {

    // If there is a query notification handler then remove it so we don't have
    // unwanted query listeners
    if (queryNotificationHandler) {
      [[NSNotificationCenter defaultCenter]
          removeObserver:queryNotificationHandler];
      queryNotificationHandler = nil;
    }

    // Get the corresponding action
    ButtonActionBlock action = actionArray[buttonIndex];

    // Call the action
    action(sender);
  }
}

// This method returns the number of rows to show
- (NSUInteger)numberOfRows {
  // Divide the number of buttons by the number of columns to get the number of
  // rows
  NSUInteger result = MAX(1, (NSUInteger)ceil((float)[buttonNames count] /
                                              (float)[self numberOfColumns]));
  return result;
}

// This method returns the number of columns to show
- (NSUInteger)numberOfColumns {
  // The number of columns is the floor of the the square root of the number of
  // buttons
  NSUInteger result = MAX(1, (int)floor(sqrt([buttonNames count])));
  return result;
}

// This method returns the span at a given index
- (NSUInteger)spanForIndex:(NSUInteger)index {
  NSUInteger result = 1;
  // Stretch the last button the the remaining width of the row
  if (index == ([buttonNames count] - 1)) {
    result = 1 + ([self numberOfColumns] * [self numberOfRows]) %
                     [buttonNames count];
  }
  return result;
}

// This method creates a generic query action for a given query and screen. It
// embeds the action into its own callback
- (ButtonActionBlock)createActionForQuery:(std::list<CmdEnum>)query
                                   screen:(Screen)screen
                                   action:(ActionBlock)action {
  // Create a block local copy of the action
  __block ActionBlock actionCpy = [action copy];

  // Create a block local copy of the screen argument
  __block Screen screenCpy = screen;

  // Create a block local copy of the query
  __block std::list<CmdEnum> queryCpy = query;

  // verify that there is an action
  assert(actionCpy);

  // Return the button action block
  return [^(ICViewController *sender) {
      // Get a block local copy of the sender
      __block ICViewController *blockSender = sender;

      // Create a notification response block
      void (^notifyBlock)(NSNotification *) = ^(NSNotification *) {
        // Hide the reading view

        NSLog(@"Executing passed in action block");
        // Execute passed in action block
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.4 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
          [blockSender hideReadingView];
          actionCpy(sender);
        });


        // Remove Handler
        [[NSNotificationCenter defaultCenter]
            removeObserver:queryNotificationHandler];
      };

      dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [sender stopUpdateTimer];

        // Add query notification handler to the system
        queryNotificationHandler = [[NSNotificationCenter defaultCenter]
                                    addObserverForName:@"queryCompleted"
                                    object:nil
                                    queue:nil
                                    usingBlock:notifyBlock];
        NSLog(@"queryStarting");

        // start the query
        sender.device->startQuery(screenCpy, queryCpy);
      });

      // show the reading view
      [sender showReadingView];

  } copy];
}

// Create a query action for the "Device Info" button
- (ButtonActionBlock)createDeviceInfoAction:(CommandList)commandList {
  // The query required by the "Device Info" button
  std::list<CmdEnum> query = {Command::GetInfoList, Command::GetInfo};

  Communicator::waitForAllTimers();

  // If ethernet port information is supported then add it to the query
  if (commandList.contains(Command::GetEthernetPortInfo)) {
    query.push_back(Command::GetEthernetPortInfo);
  }

  // Create the action block for the button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the view controller to push to
    UIViewController *viewController =
        [[ICDeviceInfoViewController alloc] initWithCommunicator:sender.comm
                                                          device:sender.device];

    // Push the device info view on top of the tiles view controller
    [sender replaceMainView:viewController];
  };

  // Return the device info query action
  return [self createActionForQuery:query
                             screen:Screen::InformationScreen
                             action:actionBlock];
}

// Create a query action for the "Device Info" button
- (ButtonActionBlock)createMixerAction {
  // The query required by the "Device Info" button
  std::list<CmdEnum> query = {
    Command::RetAudioGlobalParm,
    Command::RetAudioPortParm,
    Command::RetAudioPatchbayParm,
    Command::RetAudioDeviceParm,
    Command::RetAudioControlParm,
    Command::RetAudioControlDetail,
    Command::RetAudioControlDetailValue,
    Command::RetAudioPortMeterValue,

    Command::RetMixerParm,
    Command::RetMixerPortParm,
    Command::RetMixerInputParm,
    Command::RetMixerOutputParm,
    Command::RetMixerInputControl,
    Command::RetMixerOutputControl,
    Command::RetMixerInputControlValue,
    Command::RetMixerOutputControlValue,
    Command::RetMixerMeterValue};

  Communicator::waitForAllTimers();

  // Create the action block for the button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the view controller to push to
    UIViewController *viewController =
    [[ICMixerViewController alloc] initWithCommunicator:sender.comm
                                                      device:sender.device];

    
    // Push the device info view on top of the tiles view controller
    [sender replaceMainView:viewController withAutoLayout:YES startMeterUpdate:YES];
  };

  // Return the device info query action
  return [self createActionForQuery:query
                             screen:Screen::MixerScreen
                             action:actionBlock];
}

// Create a query action for the "MIDI Info" button
- (ButtonActionBlock)createMIDIInfoAction:(CommandList)commandList {
  // The query for the "MIDI Info" button
  std::list<CmdEnum> query = {Command::RetMIDIInfo, Command::RetMIDIPortInfo};

  Communicator::waitForAllTimers();

  // If port details are available then add them to the query
  if (commandList.contains(Command::GetMIDIPortDetail)) {
    query.push_back(Command::RetMIDIPortDetail);
  }

  // If usb host MIDI details are available then add them to the query
  if (commandList.contains(Command::GetUSBHostMIDIDeviceDetail)) {
    query.push_back(Command::RetUSBHostMIDIDeviceDetail);
  }

  // Create the action block for the button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the view controller to push to
    UIViewController *viewController = [[ICMIDIInfoViewController alloc]
        initWithMIDIInfo:sender.device->get<MIDIInfo>()
            communicator:sender.comm
                  device:sender.device];

    // Push to the midi info view controller
    [sender replaceMainView:viewController];
  };

  // Return the midi info query action
  return [self createActionForQuery:query
                             screen:Screen::MIDIInformationScreen
                             action:actionBlock];
}

// Create the query action for the "Audio Info" button for Audio V1
- (ButtonActionBlock)createAudioInfoActionV1:(CommandList)commandList {
  // Create the query for the audio info button
  std::list<CmdEnum> query = {Command::RetAudioInfo, Command::RetAudioCfgInfo,
                              Command::RetAudioPortInfo,
                              Command::RetAudioPortCfgInfo};

  Communicator::waitForAllTimers();

  // If the device supports getting the audio clock info then add that to the
  // query
  if (commandList.contains(Command::GetAudioClockInfo)) {
    query.push_back(Command::RetAudioClockInfo);
  }

  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the audio info view controller to push to
    ICAudioInfoViewControllerV1 *viewController =
        [[ICAudioInfoViewControllerV1 alloc]
            initWithCommunicator:sender.comm
                          device:sender.device];

    // Push to the audio info view controller
    [sender replaceMainView:viewController];

  };

  // return the audio info query action
  return [self createActionForQuery:query
                             screen:Screen::AudioInformationFormScreen
                             action:actionBlock];
}

// Create the query action for the "Audio Info" button for Audio V2
- (ButtonActionBlock)createAudioInfoActionV2:(CommandList)commandList {
  // Create the query for the audio info button
  std::list<CmdEnum> query = {
      Command::RetAudioGlobalParm, Command::RetAudioPortParm,
      Command::RetAudioDeviceParm, Command::RetAudioClockParm,
      Command::RetMixerParm, Command::RetMixerPortParm};

  Communicator::waitForAllTimers();

  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the audio info view controller to push to
    ICAudioInfoViewControllerV2 *viewController =
        [[ICAudioInfoViewControllerV2 alloc]
            initWithCommunicator:sender.comm
                          device:sender.device];

    // Push to the audio info view controller
    [sender replaceMainView:viewController];

  };

  // return the audio info query action
  return [self createActionForQuery:query
                             screen:Screen::AudioInformationFormScreen
                             action:actionBlock];
}

// Create the query action for the "Audio Patchbay" V1 button
- (ButtonActionBlock)createAudioPatchbayV1Action {
  // Create the audio patchbay query
  std::list<CmdEnum> query = {Command::RetAudioInfo, Command::RetAudioCfgInfo,
                              Command::RetAudioPortInfo,
                              Command::RetAudioPortCfgInfo,
                              Command::RetAudioPortPatchbay};

  Communicator::waitForAllTimers();

  // Create the action block for the audio patchbay v1 button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the audio patchbay delegate
    ICAudioPatchbayV1Delegate *delegate =
    [[ICAudioPatchbayV1Delegate alloc] initWithDevice:sender.device];

    // Create the audio patchbay view controller to push to
    ICAudioPatchbayGridViewController *viewController =
    [[ICAudioPatchbayGridViewController alloc] initWithDelegate:delegate];

    // Push to the audio patchbay grid view
    [sender replaceMainView:viewController];
  };

  // return the audio patchbay v1 query action
  return [self createActionForQuery:query
                             screen:Screen::AudioPortPatchbayScreen
                             action:actionBlock];
}

// Create the query action for the "Audio Patchbay" V2 button
- (ButtonActionBlock)createAudioPatchbayV2Action {
  // Create the audio patchbay query
  // Create the query for the audio info button
  std::list<CmdEnum> query = {
      Command::RetAudioGlobalParm, Command::RetAudioPortParm,
      Command::RetAudioDeviceParm, Command::RetAudioClockParm,
    Command::RetAudioPatchbayParm, Command::RetMixerParm,
    Command::RetMixerPortParm,
    Command::RetMixerInputParm,
    Command::RetMixerOutputParm};

  Communicator::waitForAllTimers();

  // Create the action block for the audio patchbay V2 button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    ICAudioPatchbayV2Delegate *delegate =
        [[ICAudioPatchbayV2Delegate alloc] initWithDevice:sender.device];

    // Create the audio patchbay view controller to push to
    ICAudioPatchbayGridViewController *viewController =
        [[ICAudioPatchbayGridViewController alloc] initWithDelegate:delegate];

    [sender replaceMainView:viewController];

    // Push to the audio patchbay V2 view controller
  };

  // return the audio patchbay V2 query action
  return [self createActionForQuery:query
                             screen:Screen::AudioPortPatchbayScreen
                             action:actionBlock];
}

- (ButtonActionBlock)createMIDIPatchbayAction {
  // Create the audio patchbay query
  // Create the query for the audio info button

  std::list<CmdEnum> query = {Command::RetMIDIInfo, Command::RetMIDIPortInfo,
    Command::RetMIDIPortDetail, Command::RetMIDIPortFilter,
    Command::RetMIDIPortRemap, Command::RetMIDIPortRoute};

  Communicator::waitForAllTimers();

  // Create the action block for the audio patchbay V2 button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    ICMIDIPatchbayDelegate *delegate =
    [[ICMIDIPatchbayDelegate alloc] initWithDevice:sender.device];

    // Create the audio patchbay view controller to push to
    ICMIDIPatchbayGridViewController *viewController =
    [[ICMIDIPatchbayGridViewController alloc] initWithDelegate:delegate];

    [sender replaceMainView:viewController];

    // Push to the audio patchbay V2 view controller
  };

  // return the audio patchbay V2 query action
  return [self createActionForQuery:query
                             screen:Screen::MIDIPatchbayScreen
                             action:actionBlock];
}

@end
