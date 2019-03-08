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
#import "ICAudioPatchbayV1InputSelection.h"
#import "ICAudioPatchbayV1OutputSelection.h"
#import "ICAudioPatchbayV1ViewController.h"
#import "ICAudioPatchbayV1Delegate.h"
#import "ICAudioPatchbayV2Delegate.h"
#import "ICAudioPatchbayGridViewController.h"
#import "ICChannelRemapPortSelection.h"
#import "ICChannelRemapSelectionProvider.h"
#import "ICControllerFilterIDProvider.h"
#import "ICControllerFilterPortSelectionProvider.h"
#import "ICControllerRemapIDProvider.h"
#import "ICControllerRemapPortSelectionProvider.h"
#import "ICDeviceInfoViewController.h"
#import "ICFilterPortSelection.h"
#import "ICMIDIInfoViewController.h"
#import "ICMainMenuProvider.h"
#import "ICPortFiltersProvider.h"
#import "ICPortRoutingPortSelectionProvider.h"
#import "ICPortRoutingProvider.h"
#import "ICPortSelectionProvider.h"
#import "MyAlgorithms.h"

using namespace GeneSysLib;
using namespace MyAlgorithms;

@interface ICMainMenuProvider ()

- (ButtonActionBlock)createDeviceInfoAction:(CommandList)commandList;
- (ButtonActionBlock)createMIDIInfoAction:(CommandList)commandList;
- (ButtonActionBlock)createPortRoutingAction;
- (ButtonActionBlock)createInputFilterAction;
- (ButtonActionBlock)createOutputFilterAction;
- (ButtonActionBlock)createInputChannelRemapAction;
- (ButtonActionBlock)createOutputChannelRemapAction;
- (ButtonActionBlock)createInputControllerFilterAction;
- (ButtonActionBlock)createOutputControllerFilterAction;
- (ButtonActionBlock)createControllerInputRemapAction;
- (ButtonActionBlock)createControllerOutputRemapAction;
- (ButtonActionBlock)createAudioInfoActionV1:(CommandList)commandList;
- (ButtonActionBlock)createAudioInfoActionV2:(CommandList)commandList;
- (ButtonActionBlock)createAudioPatchbayV1Action;
- (ButtonActionBlock)createAudioPatchbayV2Action;

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
  [super initializeProviderButtons:sender];
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

    // add a "MIDI Info" button if there is any MIDI info on the device
    if (commandList.contains(Command::GetMIDIInfo)) {
      [tempButtonNames addObject:@"MIDI Info"];
      [tempActionArray addObject:[self createMIDIInfoAction:commandList]];
    }

    // add a "Port Routing" button if supported by the device
    if (commandList.contains(Command::GetMIDIInfo) &&
        commandList.contains(Command::GetMIDIPortRoute)) {
      [tempButtonNames addObject:@"Port Routing"];
      [tempActionArray addObject:[self createPortRoutingAction]];
    }

    // add a "Port Input Filters" and "Port Output Filters" button if supported
    // by the device
    if (commandList.contains(Command::GetMIDIInfo) &&
        commandList.contains(Command::GetMIDIPortFilter)) {
      [tempButtonNames addObject:@"Port Input Filters"];
      [tempActionArray addObject:[self createInputFilterAction]];

      [tempButtonNames addObject:@"Port Output Filters"];
      [tempActionArray addObject:[self createOutputFilterAction]];
    }

    // add a "Channel Input Remap" and "Channel Output Remap" button if
    // supported by the device
    if (commandList.contains(Command::GetMIDIInfo) &&
        commandList.contains(Command::GetMIDIPortRemap)) {
      [tempButtonNames addObject:@"Channel Input Remap"];
      [tempActionArray addObject:[self createInputChannelRemapAction]];

      [tempButtonNames addObject:@"Channel Output Remap"];
      [tempActionArray addObject:[self createOutputChannelRemapAction]];
    }

    // add a "Controller Input Filters" and "Controller Output Filters" button
    // if supported by the device
    if (commandList.contains(Command::GetMIDIInfo) &&
        commandList.contains(Command::GetMIDIPortFilter)) {
      [tempButtonNames addObject:@"Controller Input Filters"];
      [tempActionArray addObject:[self createInputControllerFilterAction]];

      [tempButtonNames addObject:@"Controller Output Filters"];
      [tempActionArray addObject:[self createOutputControllerFilterAction]];
    }

    // add a "Controller Input Remap" and "Controller Output Remap" button if
    // supported by the device
    if (commandList.contains(Command::GetMIDIInfo) &&
        commandList.contains(Command::GetMIDIPortRemap)) {
      [tempButtonNames addObject:@"Controller Input Remap"];
      [tempActionArray addObject:[self createControllerInputRemapAction]];

      [tempButtonNames addObject:@"Controller Output Remap"];
      [tempActionArray addObject:[self createControllerOutputRemapAction]];
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
        [blockSender hideReadingView];

        // Execute passed in action block
        actionCpy(sender);

        // Remove Handler
        [[NSNotificationCenter defaultCenter]
            removeObserver:queryNotificationHandler];
      };

      // Add query notification handler to the system
      queryNotificationHandler = [[NSNotificationCenter defaultCenter]
          addObserverForName:@"queryCompleted"
                      object:nil
                       queue:nil
                  usingBlock:notifyBlock];

      // show the reading veiw
      [sender showReadingView];

      // start the query
      sender.device->startQuery(screenCpy, queryCpy);
  } copy];
}

// Create a query action for the "Device Info" button
- (ButtonActionBlock)createDeviceInfoAction:(CommandList)commandList {
  // The query required by the "Device Info" button
  std::list<CmdEnum> query = {Command::GetInfoList, Command::GetInfo};

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
    [sender.navigationController pushViewController:viewController
                                           animated:YES];
  };

  // Return the device info query action
  return [self createActionForQuery:query
                             screen:Screen::InformationScreen
                             action:actionBlock];
}

// Create a query action for the "MIDI Info" button
- (ButtonActionBlock)createMIDIInfoAction:(CommandList)commandList {
  // The query for the "MIDI Info" button
  std::list<CmdEnum> query = {Command::RetMIDIInfo, Command::RetMIDIPortInfo};

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
    [sender.navigationController pushViewController:viewController
                                           animated:YES];
  };

  // Return the midi info query action
  return [self createActionForQuery:query
                             screen:Screen::MIDIInformationScreen
                             action:actionBlock];
}

// Create the button action for the "Port Routing" button
- (ButtonActionBlock)createPortRoutingAction {
  // Create the port routing query (This only needs to be the port info. The
  // routing information is queried at a later point in time)
  std::list<CmdEnum> query = {Command::RetMIDIPortInfo};

  // Create the button action for the port routing button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create a provider to push to
    ICPortRoutingPortSelectionProvider *provider =
        [[ICPortRoutingPortSelectionProvider alloc] init];

    // Push to the port routing selection provider
    [sender pushToProvider:provider animated:YES];
  };

  // Return the port routing query action
  return [self createActionForQuery:query
                             screen:Screen::PortRoutingScreen
                             action:actionBlock];
}

// Create the button action for the "Port Input Filters" button
- (ButtonActionBlock)createInputFilterAction {
  // Create the query for the port input filters button. (At this point only
  // need the port info. The filter information is queried at a later point)
  std::list<CmdEnum> query = {Command::RetMIDIPortInfo};

  // Create the button action for the port input filters button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the port selection provider to push to
    ICFilterPortSelection *provider =
        [[ICFilterPortSelection alloc] initForInput:YES];

    // Push to the port selection provider
    [sender pushToProvider:provider animated:YES];
  };

  // Return the port input filters query action
  return [self createActionForQuery:query
                             screen:Screen::PortFiltersScreen
                             action:actionBlock];
}

// Create the button action for the "Port Output Filters" button
- (ButtonActionBlock)createOutputFilterAction {
  // Create the query for the port output filters button. (At this point only
  // need the port info. The filter information is queried at a later point)
  std::list<CmdEnum> query = {Command::RetMIDIPortInfo};

  // Create the button action for the port output filters button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the filter port selection provider to push to
    ICFilterPortSelection *provider =
        [[ICFilterPortSelection alloc] initForInput:NO];

    // Push to the provider
    [sender pushToProvider:provider animated:YES];
  };

  // Return the port output filters query action
  return [self createActionForQuery:query
                             screen:Screen::PortFiltersScreen
                             action:actionBlock];
}

// Create the query action for the "Channel Input Remap" button
- (ButtonActionBlock)createInputChannelRemapAction {
  // Create the query for the channel input remap button. (At this point only
  // need the port info. The remap info is queried at a later point)
  std::list<CmdEnum> query = {Command::RetMIDIPortInfo};

  // Create the button action for the channel input remap button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the port selection provider to push to
    ICChannelRemapPortSelection *provider =
        [[ICChannelRemapPortSelection alloc] initForInput:YES];

    // Push to the port selection provider
    [sender pushToProvider:provider animated:true];
  };

  // Return the channel input remap query action
  return [self createActionForQuery:query
                             screen:Screen::ChannelRemapScreen
                             action:actionBlock];
}

// Create the query action for the "Channel Output Remap" button
- (ButtonActionBlock)createOutputChannelRemapAction {
  // Create the query for the channel output remap button. (At this point only
  // need the port info. The remap info is queried at a later point)
  std::list<CmdEnum> query = {Command::RetMIDIPortInfo};

  // Create the button action for the channel output remap button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the port selection provider to push to
    ICChannelRemapPortSelection *provider =
        [[ICChannelRemapPortSelection alloc] initForInput:NO];

    // Push to the port selection provider
    [sender pushToProvider:provider animated:true];
  };

  // Return the channel output remap query action
  return [self createActionForQuery:query
                             screen:Screen::ChannelRemapScreen
                             action:actionBlock];
}

// Create the query action for the "Controller Input Filters" button
- (ButtonActionBlock)createInputControllerFilterAction {
  // Create the query for the controller input filters button. (At this point
  // only need the port info. The filters info is queried at a later point)
  std::list<CmdEnum> query = {Command::RetMIDIPortInfo};

  // Create the controller input filters action
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the port selection provider to push to
    ICControllerFilterPortSelectionProvider *provider =
        [[ICControllerFilterPortSelectionProvider alloc] initForInput:YES];

    // Push to the port selection provider
    [sender pushToProvider:provider animated:YES];
  };

  // Return the controller input filters query action
  return [self createActionForQuery:query
                             screen:Screen::CCFiltersScreen
                             action:actionBlock];
}

// Create the query action for the "Controller Output Filters" button
- (ButtonActionBlock)createOutputControllerFilterAction {
  // Create the query for the controller output filters button. (At this point
  // only need the port info. The filters info is queried at a later point)
  std::list<CmdEnum> query = {Command::RetMIDIPortInfo};

  // Create the controller output filters action
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the port selection provider to push to
    ICControllerFilterPortSelectionProvider *provider =
        [[ICControllerFilterPortSelectionProvider alloc] initForInput:NO];

    // Push to the port selection provider
    [sender pushToProvider:provider animated:YES];
  };

  // Return the controller output filters query action
  return [self createActionForQuery:query
                             screen:Screen::CCFiltersScreen
                             action:actionBlock];
}

// Create the query action for the "Controller Input Remap" button
- (ButtonActionBlock)createControllerInputRemapAction {
  // Create the query for the controller input remap button. (At this point
  // only need the port info. The remap info is queried at a later point)
  std::list<CmdEnum> query = {Command::RetMIDIPortInfo};

  // Create the controller input remap action
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the port selection provider to push to
    ICControllerRemapPortSelectionProvider *provider =
        [[ICControllerRemapPortSelectionProvider alloc] initForInput:YES];

    // Push to the port selection provider
    [sender pushToProvider:provider animated:YES];
  };

  // Return the controller input remap query action
  return [self createActionForQuery:query
                             screen:Screen::CCRemapScreen
                             action:actionBlock];
}

// Create the query action for the "Controller Output Remap" button
- (ButtonActionBlock)createControllerOutputRemapAction {
  // Create the query for the controller output remap button. (At this point
  // only need the port info. The remap info is queried at a later point)
  std::list<CmdEnum> query = {Command::RetMIDIPortInfo};

  // Create the controller output remap action
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the port selection provider to push to
    ICControllerRemapPortSelectionProvider *provider =
        [[ICControllerRemapPortSelectionProvider alloc] initForInput:NO];

    // Push to the port selection provider
    [sender pushToProvider:provider animated:YES];
  };

  // Return the controller output remap query action
  return [self createActionForQuery:query
                             screen:Screen::CCRemapScreen
                             action:actionBlock];
}

// Create the query action for the "Audio Info" button for Audio V1
- (ButtonActionBlock)createAudioInfoActionV1:(CommandList)commandList {
  // Create the query for the audio info button
  std::list<CmdEnum> query = {Command::RetAudioInfo, Command::RetAudioCfgInfo,
                              Command::RetAudioPortInfo,
                              Command::RetAudioPortCfgInfo};

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
    [sender.navigationController pushViewController:viewController
                                           animated:YES];
  };

  // return the audio info query action
  return [self createActionForQuery:query
                             screen:Screen::AudioInformationScreen
                             action:actionBlock];
}

// Create the query action for the "Audio Info" button for Audio V1
- (ButtonActionBlock)createAudioInfoActionV2:(CommandList)commandList {
  // Create the query for the audio info button
  std::list<CmdEnum> query = {
      Command::RetAudioGlobalParm, Command::RetAudioPortParm,
      Command::RetAudioDeviceParm, Command::RetAudioClockParm};

  ActionBlock actionBlock = ^(ICViewController *sender) {
    // Create the audio info view controller to push to
    ICAudioInfoViewControllerV2 *viewController =
        [[ICAudioInfoViewControllerV2 alloc]
            initWithCommunicator:sender.comm
                          device:sender.device];

    // Push to the audio info view controller
    [sender.navigationController pushViewController:viewController
                                           animated:YES];
  };

  // return the audio info query action
  return [self createActionForQuery:query
                             screen:Screen::AudioInformationScreen
                             action:actionBlock];
}

// Create the query action for the "Audio Patchbay" V1 button
- (ButtonActionBlock)createAudioPatchbayV1Action {
  // Create the audio patchbay query
  std::list<CmdEnum> query = {Command::RetAudioInfo, Command::RetAudioCfgInfo,
                              Command::RetAudioPortInfo,
                              Command::RetAudioPortCfgInfo,
                              Command::RetAudioPortPatchbay};

  // Create the action block for the audio patchbay v1 button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    // is iPhone
    bool isiPhone = ([[UIDevice currentDevice] userInterfaceIdiom] ==
                     UIUserInterfaceIdiomPhone);

    if (isiPhone) {
      // Create the audio patchbay view controller to push to
      ICAudioPatchbayV1ViewController *viewController =
          [[ICAudioPatchbayV1ViewController alloc]
              initWithCommunicator:sender.comm
                            device:sender.device];

      // Push to the audio patchbay v1 view controller
      [sender.navigationController pushViewController:viewController
                                             animated:YES];
    } else {
      // Create the audio patchbay delegate
      ICAudioPatchbayV1Delegate *delegate =
          [[ICAudioPatchbayV1Delegate alloc] initWithDevice:sender.device];

      // Create the audio patchbay view controller to push to
      ICAudioPatchbayGridViewController *viewController =
          [[ICAudioPatchbayGridViewController alloc] initWithDelegate:delegate];

      // Push to the audio patchbay grid view
      [sender.navigationController pushViewController:viewController
                                             animated:YES];
    }
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
      Command::RetAudioPatchbayParm};

  // Create the action block for the audio patchbay V2 button
  ActionBlock actionBlock = ^(ICViewController *sender) {
    ICAudioPatchbayV2Delegate *delegate =
        [[ICAudioPatchbayV2Delegate alloc] initWithDevice:sender.device];

    // Create the audio patchbay view controller to push to
    ICAudioPatchbayGridViewController *viewController =
        [[ICAudioPatchbayGridViewController alloc] initWithDelegate:delegate];

    // Push to the audio patchbay V2 view controller
    [sender.navigationController pushViewController:viewController
                                           animated:YES];
  };

  // return the audio patchbay V2 query action
  return [self createActionForQuery:query
                             screen:Screen::AudioPortPatchbayScreen
                             action:actionBlock];
}

@end
