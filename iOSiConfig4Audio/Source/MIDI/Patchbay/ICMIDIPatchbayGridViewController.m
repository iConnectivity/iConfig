/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICMIDIPatchbayGridViewController.h"
#import <QuartzCore/QuartzCore.h>
#import "NSString+RotatedDraw.h"
#import "PureLayout.h"

using namespace GeneSysLib;

#define MoveToPoint(pt) \
  CGContextMoveToPoint(UIGraphicsGetCurrentContext(), pt.x, pt.y)

#define AddPoint(pt) \
  CGContextAddLineToPoint(UIGraphicsGetCurrentContext(), pt.x, pt.y)

#define kRefreshRate (41.6f / 1000.0f)

#define kNumberOfChannels 16
#define kNumberOfMIDIChannelCommands 6
#define kNumberOfMIDIGeneralCommands 8

static const CGFloat gutterSize = 35.0f;
static const CGFloat arrowSize = 5.0f;
static const CGFloat textHeight = 20.0f;
static const CGFloat arrowWidth = 5.0f;
static const CGFloat arrowHeight = 9.0f;
static const CGFloat arrowXOffset = 5.0f;
static const CGFloat arrowYOffset = 2.0f;
static const CGFloat circleRadius = 6.0f;
static const CGFloat crossWidth = 6.0f;
static const CGFloat horizontalHeaderHeight = 50.0f;
static const CGFloat verticalHeaderWidth = 50.0f;

static const CGFloat sectionColorRed = 1.0f;
static const CGFloat sectionColorGreen = 0.8627f;
static const CGFloat sectionColorBlue = 0.705888f;
static const CGFloat sectionColorAlpha = 0.5f;

static const CGFloat additionColorRed = 0.0f;
static const CGFloat additionColorGreen = 0.0f;
static const CGFloat additionColorBlue = 1.0f;
static const CGFloat additionColorAlpha = 1.0f;

static const CGFloat numberFontSize = 14;
static const CGFloat fullAlpha = 1.0f;
static const CGFloat blendedAlpha = 0.75f;
static const CGFloat headerAlpha = 0.2f;
static const CGFloat gridBackAlpha = 0.07f;
static const CGFloat verticalHeaderAngle = 1.57079632679f;

typedef unsigned int (^PortCount)(unsigned int);

@interface ICMIDIPatchbayGridViewController () {
  UIImage *staticImage;
  UIImage *dynamicImage;
  NSPredicate *filterTouchPredicate;
  NSTimer *redrawTimer;
  NSSet *currentProposed;

  CAShapeLayer *pathLayer;
  CAShapeLayer *bluePathLayer;
  CAShapeLayer *proposedLayer;

  UILongPressGestureRecognizer* lpgr;
}

@property(nonatomic, strong) NSSet *touches;
@property (nonatomic, strong) UITextField *pickerViewTextField;
@property (nonatomic, strong) UIPickerView *pickerView;

- (CGFloat)horizontalHeaderHeight;
- (CGFloat)horizontalHeaderWidth:(CGSize)viewSize;
- (CGFloat)verticalHeaderWidth;
- (CGFloat)verticalHeaderHeight:(CGSize)viewSize;
- (CGFloat)gridWidth:(CGSize)viewSize;
- (CGFloat)gridHeight:(CGSize)viewSize;
- (CGFloat)gutterSize;
- (CGFloat)arrowSize;
- (CGFloat)horizontalChannelNumOffset:(CGSize)viewSize;
- (CGFloat)horizontalArrowOffset:(CGSize)viewSize;
- (CGFloat)verticalChannelNumOffset;
- (CGFloat)verticalArrowOffset;
- (CGRect)horizontalHeaderRect:(CGPoint)offset size:(CGSize)viewSize;
- (CGRect)verticalHeaderRect:(CGPoint)offset size:(CGSize)viewSize;
- (CGRect)horizontalGutterRect:(CGPoint)offset size:(CGSize)viewSize;
- (CGRect)verticalGutterRect:(CGPoint)offset size:(CGSize)viewSize;
- (CGRect)topRightRect:(CGPoint)offset size:(CGSize)viewSize;
- (CGRect)gridRect:(CGPoint)offset size:(CGSize)viewSize;

- (UIImage *)generateStaticImage;
- (void)updateDynamicPaths:(NSSet *)touches;

- (void)inContextDrawTriangle:(CGPoint)a b:(CGPoint)b c:(CGPoint)c;
- (void)inContextDrawDownTriangle:(CGPoint)point;
- (void)inContextDrawRightTriangle:(CGPoint)point;

- (void)inPathDrawTriangle:(CGMutablePathRef *)path
                         a:(CGPoint)a
                         b:(CGPoint)b
                         c:(CGPoint)c;
- (void)inPathDrawDownTriangle:(CGMutablePathRef *)path point:(CGPoint)point;
- (void)inPathDrawRightTriangle:(CGMutablePathRef *)path point:(CGPoint)point;

- (void)inContextDrawLine:(CGPoint)a to:(CGPoint)b;
- (void)inContextDrawHorizontalHeaderBackground:(CGSize)viewSize;
- (void)inContextDrawVerticalGutterBackground:(CGSize)viewSize;

- (void)redrawTimerTick:(id)sender;

@end

@implementation ICMIDIPatchbayGridViewController

@synthesize pickerViewTextField = _pickerViewTextField;

- (id)initWithDelegate:(ICMIDIPatchbayDelegate*)patchbayDelegate {
  self = [super initWithNibName:@"ICMIDIPatchbayGrid_iPad" bundle:nil];

  if (self) {
    NSParameterAssert(patchbayDelegate);
    self.midiPatchbayDelegate = patchbayDelegate;
    currentPopupType = PortFilters;
    popupPut = Input;
    popupDevicePort = 1;
    popupPort = 1;

    pickerType = Port;

    popupControllerIDCol = [[NSMutableArray alloc] init];
    popupControllerIDSrcCol = [[NSMutableArray alloc] init];
    popupControllerIDDstCol = [[NSMutableArray alloc] init];

    controllerNames = [NSArray arrayWithObjects:
                       @"0 - Bank Select (coarse)",
                       @"1 - Modulation Wheel (coarse)",
                       @"2 - Breath Control (coarse)",
                       @"3 - Continuous controller #3",
                       @"4 - Foot Controller (coarse)",
                       @"5 - Portamento Time (coarse)",
                       @"6 - Data Entry Slider (coarse)",
                       @"7 - Main Volume (coarse)",
                       @"8 - Stereo Balance (coarse)",
                       @"9 - Continuous controller #9",
                       @"10 - Pan (coarse)",
                       @"11 - Expression (sub-Volume) (coarse)",
                       @"12 - Effect Control 1 (coarse)",
                       @"13 - Effect Control 2 (coarse)",
                       @"14 - Continuous controller #14",
                       @"15 - Continuous controller #15",
                       @"16 - General Purpose Slider 1",
                       @"17 - General Purpose Slider 2",
                       @"18 - General Purpose Slider 3",
                       @"19 - General Purpose Slider 4",
                       @"20 - Continuous controller #20",
                       @"21 - Continuous controller #21",
                       @"22 - Continuous controller #22",
                       @"23 - Continuous controller #23",
                       @"24 - Continuous controller #24",
                       @"25 - Continuous controller #25",
                       @"26 - Continuous controller #26",
                       @"27 - Continuous controller #27",
                       @"28 - Continuous controller #28",
                       @"29 - Continuous controller #29",
                       @"30 - Continuous controller #30",
                       @"31 - Continuous controller #31",
                       @"32 - Bank Select (fine)",
                       @"33 - Modulation Wheel (fine)",
                       @"34 - Breath Control (fine)",
                       @"35 - Continuous controller #3 (fine)",
                       @"36 - Foot Controller (fine)",
                       @"37 - Portamento Time (fine)",
                       @"38 - Data Entry Slider (fine)",
                       @"39 - Main Volume (fine)",
                       @"40 - Stereo Balance (fine)",
                       @"41 - Continuous controller #9 (fine)",
                       @"42 - Pan (fine)",
                       @"43 - Expression (sub-Volume) (fine)",
                       @"44 - Effect Control 1 (fine)",
                       @"45 - Effect Control 2 (fine)",
                       @"46 - Continuous controller #14 (fine)",
                       @"47 - Continuous controller #15 (fine)",
                       @"48 - Continuous controller #16",
                       @"49 - Continuous controller #17",
                       @"50 - Continuous controller #18",
                       @"51 - Continuous controller #19",
                       @"52 - Continuous controller #20 (fine)",
                       @"53 - Continuous controller #21 (fine)",
                       @"54 - Continuous controller #22 (fine)",
                       @"55 - Continuous controller #23 (fine)",
                       @"56 - Continuous controller #24 (fine)",
                       @"57 - Continuous controller #25 (fine)",
                       @"58 - Continuous controller #26 (fine)",
                       @"59 - Continuous controller #27 (fine)",
                       @"60 - Continuous controller #28 (fine)",
                       @"61 - Continuous controller #29 (fine)",
                       @"62 - Continuous controller #30 (fine)",
                       @"63 - Continuous controller #31 (fine)",
                       @"64 - Hold pedal (Sustain) on/off",
                       @"65 - Portamento on/off",
                       @"66 - Sustenuto Pedal on/off",
                       @"67 - Soft Pedal on/off",
                       @"68 - Legato Pedal on/off",
                       @"69 - Hold Pedal 2 on/off",
                       @"70 - Sound Variation",
                       @"71 - Sound Timbre",
                       @"72 - Sound Release Time",
                       @"73 - Sound Attack Time",
                       @"74 - Sound Brightness",
                       @"75 - Sound Control 6",
                       @"76 - Sound Control 7",
                       @"77 - Sound Control 8",
                       @"78 - Sound Control 9",
                       @"79 - Sound Control 10",
                       @"80 - General Purpose Button",
                       @"81 - General Purpose Button",
                       @"82 - General Purpose Button",
                       @"83 - General Purpose Button",
                       @"84 - Undefined on/off",
                       @"85 - Undefined on/off",
                       @"86 - Undefined on/off",
                       @"87 - Undefined on/off",
                       @"88 - Undefined on/off",
                       @"89 - Undefined on/off",
                       @"90 - Undefined on/off",
                       @"91 - Effects Level",
                       @"92 - Tremulo Level",
                       @"93 - Chorus Level",
                       @"94 - Celeste (Detune) Level",
                       @"95 - Phaser Level",
                       @"96 - Data entry +1",
                       @"97 - Data entry -1",
                       @"98 - Non-Registered Parameter Number (coarse)",
                       @"99 - Non-Registered Parameter Number (fine)",
                       @"100 - Registered Parameter Number (coarse)",
                       @"101 - Registered Parameter Number (fine)",
                       @"102 - Undefined",
                       @"103 - Undefined",
                       @"104 - Undefined",
                       @"105 - Undefined",
                       @"106 - Undefined",
                       @"107 - Undefined",
                       @"108 - Undefined",
                       @"109 - Undefined",
                       @"110 - Undefined",
                       @"111 - Undefined",
                       @"112 - Undefined",
                       @"113 - Undefined",
                       @"114 - Undefined",
                       @"115 - Undefined",
                       @"116 - Undefined",
                       @"117 - Undefined",
                       @"118 - Undefined",
                       @"119 - Undefined",
                       @"120 - All Sound Off",
                       @"121 - All Controllers Off",
                       @"122 - Local Keyboard On/Off",
                       @"123 - All Notes Off",
                       @"124 - Omni Mode Off",
                       @"125 - Omni Mode On",
                       @"126 - Monophonic Mode On",
                       @"127 - Polyphonic Mode On (mono=off)",
                       nil];
  }

  return self;
}

- (IBAction)popupTabChanged:(id)sender {
  [portFiltersButton setTintColor:[UIColor blueColor]];
  [channelRemapButton setTintColor:[UIColor blueColor]];
  [controllerRemapButton setTintColor:[UIColor blueColor]];
  [controllerFiltersButton setTintColor:[UIColor blueColor]];

  if (sender == portFiltersButton) {
    currentPopupType = PortFilters;
    [portFiltersButton setTintColor:[UIColor redColor]];
  }
  else if (sender == channelRemapButton) {
    currentPopupType = ChannelRemap;
    [channelRemapButton setTintColor:[UIColor redColor]];
  }
  else if (sender == controllerRemapButton) {
    currentPopupType = ControllerRemap;
    [controllerRemapButton setTintColor:[UIColor redColor]];
  }
  else if (sender == controllerFiltersButton) {
    currentPopupType = ControllerFilters;
    [controllerFiltersButton setTintColor:[UIColor redColor]];
  }

  [self redrawPopup];
}

- (IBAction)popupPortTouched:(id)sender {
  pickerType = Port;
  [self.pickerView reloadAllComponents];

  [self.pickerView selectRow:(popupPort - 1) inComponent:0 animated:NO];
  [self.pickerViewTextField becomeFirstResponder];
}

- (IBAction)bubbleButtonTouched:(id)sender {
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

  [defaults setObject:[NSNumber numberWithBool:NO] forKey:@"showBubbleOnMidiPatchbay"];
  [defaults synchronize];

  [bubbleButton setEnabled:NO];
  [bubbleButton setHidden:YES];
}

- (void) redrawPopup {
  NSMutableString *str = [[NSMutableString alloc] init];
  if (popupPut == Input) {
    [str appendString:@"Input"];
  }
  else {
    [str appendString:@"Output"];
  }
  switch (currentPopupType) {
    case PortFilters:
      [str appendString:@" Port Filters"];
      break;
    case ChannelRemap:
      [str appendString:@" Channel Remap"];
      break;
    case ControllerFilters:
      [str appendString:@" Controller Filters"];
      break;
    default:
    case ControllerRemap:
      [str appendString:@" Controller Remap"];
      break;

  }
  [str appendString:@" for "];
  [str appendString:[self.midiPatchbayDelegate nameForSection:popupDevicePort]];
  [str appendString:@" & Port:"];

  [popupTitle setText:str];

  if ((popupPut == Input && [self.midiPatchbayDelegate realNumSourcesPerSection:popupDevicePort] == 1) ||
      (popupPut == Output && [self.midiPatchbayDelegate realNumDestinationsPerSection:popupDevicePort] == 1)) {
    popupPortButton.enabled = NO;
    [popupPortButton setTitle:@"N/A - only 1 port" forState:UIControlStateNormal];
  }
  else {
    popupPortButton.enabled = YES;
    NSString *portName = [self.midiPatchbayDelegate nameForDevicePort:popupDevicePort andPort:popupPort];
    [popupPortButton setTitle:portName forState:UIControlStateNormal];
  }
  [self fillPopup];

  if (currentPopupType == PortFilters) {
    [self updatePortFilters];
  }
  else if (currentPopupType == ChannelRemap) {
    [self updateChannelRemap];
  }
  else if (currentPopupType == ControllerFilters) {
    [self updateControllerFilters];
  }
  else if (currentPopupType == ControllerRemap) {
    [self updateControllerRemap];
  }
}

- (void) updatePortFilters {
  for (int row = 0; row < kNumberOfMIDIChannelCommands; row++) {
    for (int col = 0; col < kNumberOfChannels; col++) {
      bool isSet = [self.midiPatchbayDelegate isPortFilterCellSetAtRow:row Col:col DevicePort:popupDevicePort Port:popupPort Put:popupPut];
      if ([popupButtonRows count] > row) {
        if ([[popupButtonRows objectAtIndex:row] count] > col) {
          UIButton* but = [[popupButtonRows objectAtIndex:row] objectAtIndex:col];
          but.selected = isSet;
          [self updateButtonColor:but];
        }
      }
    }
  }
  for (int row = kNumberOfMIDIChannelCommands; row < kNumberOfMIDIChannelCommands + kNumberOfMIDIGeneralCommands; row++) {
    bool isSet = [self.midiPatchbayDelegate isPortFilterCellSetAtRow:row Col:0 DevicePort:popupDevicePort Port:popupPort Put:popupPut];
    if ([popupButtonRows count] > row) {
      if ([[popupButtonRows objectAtIndex:row] count] > 0) {
        UIButton* but = [[popupButtonRows objectAtIndex:row] objectAtIndex:0];
        but.selected = isSet;
        [self updateButtonColor:but];
      }
    }
  }
}

- (void) updateChannelRemap {

  NSArray *channelRemaps = [self.midiPatchbayDelegate getChannelRemapsForDevicePort:popupDevicePort Port:popupPort Put:popupPut];

  for (int i = 0; i < [channelRemaps count]; i++) {
    [[popupChannelRemapChannelRow objectAtIndex:i] setTitle:[channelRemaps objectAtIndex:i] forState:UIControlStateNormal];
  }

  for (int row = 0; row < kNumberOfMIDIChannelCommands; row++) {
    for (int col = 0; col < kNumberOfChannels; col++) {
      bool isSet = [self.midiPatchbayDelegate isChannelRemapSetAtRow:row Col:col DevicePort:popupDevicePort Port:popupPort Put:popupPut];
      if ([popupButtonRows count] > row) {
        if ([[popupButtonRows objectAtIndex:row] count] > col) {
          UIButton* but = [[popupButtonRows objectAtIndex:row] objectAtIndex:col];
          but.selected = isSet;
          [self updateButtonColor:but];
        }
      }
    }
  }
}

- (void) updateControllerFilters {

  NSArray *controllerIDs = [self.midiPatchbayDelegate getControllerFilterIDsForDevicePort:popupDevicePort Port:popupPort Put:popupPut];

  for (int i = 0; i < [controllerIDs count]; i++) {
    [[popupControllerIDCol objectAtIndex:i] setTitle:[controllerIDs objectAtIndex:i] forState:UIControlStateNormal];
  }

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < kNumberOfChannels; col++) {
      bool isSet = [self.midiPatchbayDelegate isControllerFilterSetAtRow:row Col:col DevicePort:popupDevicePort Port:popupPort Put:popupPut];
      if ([popupButtonRows count] > row) {
        if ([[popupButtonRows objectAtIndex:row] count] > col) {
          UIButton* but = [[popupButtonRows objectAtIndex:row] objectAtIndex:col];
          but.selected = isSet;
          [self updateButtonColor:but];
        }
      }
    }
  }
}

- (void) updateControllerRemap {

  NSArray *controllerIDs = [self.midiPatchbayDelegate getControllerRemapIDsForDevicePort:popupDevicePort Port:popupPort Put:popupPut];

  for (int i = 0; i < [controllerIDs count]; i++) {
    [[popupControllerIDSrcCol objectAtIndex:i] setTitle:[[controllerIDs objectAtIndex:i] objectAtIndex:0] forState:UIControlStateNormal];
    [[popupControllerIDDstCol objectAtIndex:i] setTitle:[[controllerIDs objectAtIndex:i] objectAtIndex:1] forState:UIControlStateNormal];
    //NSLog(@"Set some titles! %@, %@", [[controllerIDs objectAtIndex:i] objectAtIndex:0], [[controllerIDs objectAtIndex:i] objectAtIndex:1]);
  }

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < kNumberOfChannels; col++) {
      bool isSet = [self.midiPatchbayDelegate isControllerRemapSetAtRow:row Col:col DevicePort:popupDevicePort Port:popupPort Put:popupPut];
      if ([popupButtonRows count] > row) {
        if ([[popupButtonRows objectAtIndex:row] count] > col) {
          UIButton* but = [[popupButtonRows objectAtIndex:row] objectAtIndex:col];
          but.selected = isSet;
          [self updateButtonColor:but];
        }
      }
    }
  }
}

- (void) fillPopup {
  for (UIView *subV in [popupSettingsView subviews]) {
    [subV removeFromSuperview];
  }
  for (UIView *subV in [popupSettingsLeft subviews]) {
    [subV removeFromSuperview];
  }
  for (UIView *subV in [popupSettingsTop subviews]) {
    [subV removeFromSuperview];
  }

  [popupControllerIDCol removeAllObjects];
  [popupControllerIDSrcCol removeAllObjects];
  [popupControllerIDDstCol removeAllObjects];

  if (currentPopupType == PortFilters) {

    NSArray *leftTitles = [[NSArray alloc] initWithObjects:
                           @"Pitch Bend Events", @"Channel Pressure Events", @"Program Change Events", @"Control Change (CC) Events", @"Poly Key Pressure Events", @"Note On/Off Events", @"Reset Events", @"Active Sensing Events", @"Realtime Events", @"Tune Request Events", @"Song Select Events", @"Song Position Pointer Events", @"Time Code (MTC) Events", @"System Exclusive Events",nil];

    //  popupSettingsView.translatesAutoresizingMaskIntoConstraints = NO;
    if (popupButtonRows != nil) {
      [popupButtonRows removeAllObjects];
    }
    else {
      popupButtonRows = [[NSMutableArray alloc] init];
    }
    NSMutableArray *leftLabels = [[NSMutableArray alloc] init];
    NSMutableArray *topLabels = [[NSMutableArray alloc] init];
    NSMutableArray *firstButtons = [[NSMutableArray alloc] init];

    for (int i = 0; i < kNumberOfChannels; i++) { // 16 channels per port
      UIButton *topLabel = [UIButton new];
      topLabel.translatesAutoresizingMaskIntoConstraints = NO;
      [topLabel setBackgroundColor:[UIColor whiteColor]];
      [topLabel setTitle:[NSString stringWithFormat:@"%d",i + 1] forState:UIControlStateNormal];
      topLabel.titleLabel.font = [UIFont systemFontOfSize:10.0];

      [topLabel setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];

      [[topLabel layer] setBorderWidth:1.0f];
      [[topLabel layer] setBorderColor:[UIColor lightGrayColor].CGColor];

      topLabel.enabled = NO;
      [popupSettingsTop addSubview:topLabel];

      [topLabel autoPinEdgeToSuperviewEdge:ALEdgeTop];
      [topLabel autoPinEdgeToSuperviewEdge:ALEdgeBottom];

      if (i == 0) {
        [topLabel autoPinEdgeToSuperviewEdge:ALEdgeLeft];
      }
      [topLabels addObject:topLabel];
    }

    [topLabels autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeTop withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];

    for (int i = 0; i < [leftTitles count]; i++) {
      UIButton *leftLabel = [UIButton new];
      leftLabel.translatesAutoresizingMaskIntoConstraints = NO;
      [leftLabel setBackgroundColor:[UIColor whiteColor]];
      [leftLabel setTitle:[leftTitles objectAtIndex:i] forState:UIControlStateNormal];
      leftLabel.titleLabel.font = [UIFont systemFontOfSize:10.0];

      [leftLabel setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];

      [[leftLabel layer] setBorderWidth:1.0f];
      [[leftLabel layer] setBorderColor:[UIColor lightGrayColor].CGColor];

      leftLabel.enabled = NO;
      [popupSettingsLeft addSubview:leftLabel];
      [leftLabels addObject:leftLabel];

      [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeLeft];
      [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeRight];

      if (i == 0) {
        [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeTop];
      }
      if (i == [leftTitles count] - 1) {
        [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeBottom];
      }

      NSMutableArray *buttonLine = [[NSMutableArray alloc] init];
      for (int j = 0; j < kNumberOfChannels; j++) { // 16 channels per port
        UIButton *button = [UIButton new];
        button.tag = i << 8 | j;

        button.translatesAutoresizingMaskIntoConstraints = NO;
        [button setBackgroundColor:[UIColor whiteColor]];
        [button setTitle:@"" forState:UIControlStateNormal];
        [button setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];

        [[button layer] setBorderWidth:1.0f];
        [[button layer] setBorderColor:[UIColor lightGrayColor].CGColor];

        button.selected = NO;
        button.enabled = NO;

        [popupSettingsView addSubview:button];

        [buttonLine addObject:button];
        if (j == 0) {
          [firstButtons addObject:button];
          if (i == 0) {
            [button autoPinEdgeToSuperviewEdge:ALEdgeLeft];
            [button autoPinEdgeToSuperviewEdge:ALEdgeTop];
          }
          if (i == 15) {
            [button autoPinEdgeToSuperviewEdge:ALEdgeLeft];
            [button autoPinEdgeToSuperviewEdge:ALEdgeBottom];
          }
        }
        else {
          [button autoMatchDimension:ALDimensionHeight toDimension:ALDimensionHeight ofView:[firstButtons objectAtIndex:i]];
          //[button autoMatchDimension:ALDimensionWidth toDimension:ALDimensionWidth ofView:[firstButtons objectAtIndex:i] withMultiplier:0.5];
        }
        if (i >= kNumberOfMIDIChannelCommands)
          break;
      }
      if ([buttonLine count] > 1) {
        [buttonLine autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeTop withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];
      }
      else {
        [[buttonLine objectAtIndex:0] autoPinEdgeToSuperviewEdge:ALEdgeLeft];
        [[buttonLine objectAtIndex:0] autoPinEdgeToSuperviewEdge:ALEdgeRight];
      }
      [popupButtonRows addObject:buttonLine];
    }
    [leftLabels autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeLeft withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];
    [firstButtons autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeLeft withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];
  }
  else if (currentPopupType == ChannelRemap) {

    NSArray *leftTitles = [[NSArray alloc] initWithObjects:
                           @"Remap to Channel", @"Pitch Bend", @"Channel Pressure", @"Program Change", @"Control Change", @"Poly Key Pressure", @"Note On/Off", nil];

    //  popupSettingsView.translatesAutoresizingMaskIntoConstraints = NO;
    if (popupButtonRows != nil) {
      [popupButtonRows removeAllObjects];
    }
    else {
      popupButtonRows = [[NSMutableArray alloc] init];
    }
    NSMutableArray *leftLabels = [[NSMutableArray alloc] init];
    NSMutableArray *topLabels = [[NSMutableArray alloc] init];
    NSMutableArray *firstButtons = [[NSMutableArray alloc] init];

    for (int i = 0; i < kNumberOfChannels; i++) { // 16 channels per port
      UIButton *topLabel = [UIButton new];
      topLabel.translatesAutoresizingMaskIntoConstraints = NO;
      [topLabel setBackgroundColor:[UIColor whiteColor]];
      [topLabel setTitle:[NSString stringWithFormat:@"%d",i + 1] forState:UIControlStateNormal];
      topLabel.titleLabel.font = [UIFont systemFontOfSize:10.0];

      [topLabel setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];

      [[topLabel layer] setBorderWidth:1.0f];
      [[topLabel layer] setBorderColor:[UIColor lightGrayColor].CGColor];

      topLabel.enabled = NO;
      [popupSettingsTop addSubview:topLabel];

      [topLabel autoPinEdgeToSuperviewEdge:ALEdgeTop];
      [topLabel autoPinEdgeToSuperviewEdge:ALEdgeBottom];

      if (i == 0) {
        [topLabel autoPinEdgeToSuperviewEdge:ALEdgeLeft];
      }
      [topLabels addObject:topLabel];
    }

    [topLabels autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeTop withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];

    for (int i = 0; i < [leftTitles count]; i++) {
      UIButton *leftLabel = [UIButton new];
      leftLabel.translatesAutoresizingMaskIntoConstraints = NO;
      [leftLabel setBackgroundColor:[UIColor whiteColor]];
      [leftLabel setTitle:[leftTitles objectAtIndex:i] forState:UIControlStateNormal];
      leftLabel.titleLabel.font = [UIFont systemFontOfSize:10.0];

      [leftLabel setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];

      [[leftLabel layer] setBorderWidth:1.0f];
      [[leftLabel layer] setBorderColor:[UIColor lightGrayColor].CGColor];

      leftLabel.enabled = NO;
      [popupSettingsLeft addSubview:leftLabel];
      [leftLabels addObject:leftLabel];

      [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeLeft];
      [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeRight];

      if (i == 0) {
        [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeTop];
      }
      if (i == [leftTitles count] - 1) {
        [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeBottom];
      }

      NSMutableArray *buttonLine = [[NSMutableArray alloc] init];
      for (int j = 0; j < kNumberOfChannels; j++) { // 16 channels per port
        UIButton *button = [UIButton new];

        button.translatesAutoresizingMaskIntoConstraints = NO;
        [button setBackgroundColor:[UIColor whiteColor]];
        if (i == 0) {
          [button setTitle:@"1" forState:UIControlStateNormal];
          [button addTarget:self action:@selector(remapToChannelButtonTouched:) forControlEvents:UIControlEventTouchUpInside];

          button.enabled = YES;
          button.tag = j;
        }
        else {
          button.tag = (i - 1) << 8 | j;

          [button setTitle:@"" forState:UIControlStateNormal];
          button.enabled = NO;
        }
        [button setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];

        [[button layer] setBorderWidth:1.0f];
        [[button layer] setBorderColor:[UIColor lightGrayColor].CGColor];

        button.selected = NO;

        [popupSettingsView addSubview:button];

        [buttonLine addObject:button];
        if (j == 0) {
          [firstButtons addObject:button];
          if (i == 0) {
            [button autoPinEdgeToSuperviewEdge:ALEdgeLeft];
            [button autoPinEdgeToSuperviewEdge:ALEdgeTop];
          }
          if (i == 15) {
            [button autoPinEdgeToSuperviewEdge:ALEdgeLeft];
            [button autoPinEdgeToSuperviewEdge:ALEdgeBottom];
          }
        }
        else {
          [button autoMatchDimension:ALDimensionHeight toDimension:ALDimensionHeight ofView:[firstButtons objectAtIndex:i]];
        }
      }
      [buttonLine autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeTop withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];
      if (i != 0) {
        [popupButtonRows addObject:buttonLine];
      }
      else {
        popupChannelRemapChannelRow = [[NSArray alloc] initWithArray:buttonLine];
      }
    }
    [leftLabels autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeLeft withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];
    [firstButtons autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeLeft withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];
  }
  else if (currentPopupType == ControllerFilters) {

    NSArray *leftTitles = [[NSArray alloc] initWithObjects:
                           @"Controller Filter 1", @"Controller Filter 2", @"Controller Filter 3", @"Controller Filter 4", @"Controller Filter 5", @"Controller Filter 6", @"Controller Filter 7", @"Controller Filter 8",nil];


    //  popupSettingsView.translatesAutoresizingMaskIntoConstraints = NO;
    if (popupButtonRows != nil) {
      [popupButtonRows removeAllObjects];
    }
    else {
      popupButtonRows = [[NSMutableArray alloc] init];
    }
    NSMutableArray *leftLabels = [[NSMutableArray alloc] init];
    NSMutableArray *topLabels = [[NSMutableArray alloc] init];
    NSMutableArray *firstButtons = [[NSMutableArray alloc] init];

    for (int i = 0; i < kNumberOfChannels + 1; i++) { // 16 channels per port + controller ID
      UIButton *topLabel = [UIButton new];
      topLabel.translatesAutoresizingMaskIntoConstraints = NO;
      [topLabel setBackgroundColor:[UIColor whiteColor]];
      if (i == kNumberOfChannels)
        [topLabel setTitle:@"ID" forState:UIControlStateNormal];
      else
        [topLabel setTitle:[NSString stringWithFormat:@"%d",i + 1] forState:UIControlStateNormal];
      topLabel.titleLabel.font = [UIFont systemFontOfSize:10.0];

      [topLabel setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];

      [[topLabel layer] setBorderWidth:1.0f];
      [[topLabel layer] setBorderColor:[UIColor lightGrayColor].CGColor];

      topLabel.enabled = NO;
      [popupSettingsTop addSubview:topLabel];

      [topLabel autoPinEdgeToSuperviewEdge:ALEdgeTop];
      [topLabel autoPinEdgeToSuperviewEdge:ALEdgeBottom];

      if (i == 0) {
        [topLabel autoPinEdgeToSuperviewEdge:ALEdgeLeft];
      }
      [topLabels addObject:topLabel];
    }

    [topLabels autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeTop withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];

    for (int i = 0; i < [leftTitles count]; i++) {
      UIButton *leftLabel = [UIButton new];
      leftLabel.translatesAutoresizingMaskIntoConstraints = NO;
      [leftLabel setBackgroundColor:[UIColor whiteColor]];
      [leftLabel setTitle:[leftTitles objectAtIndex:i] forState:UIControlStateNormal];
      leftLabel.titleLabel.font = [UIFont systemFontOfSize:10.0];

      [leftLabel setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];

      [[leftLabel layer] setBorderWidth:1.0f];
      [[leftLabel layer] setBorderColor:[UIColor lightGrayColor].CGColor];

      leftLabel.enabled = NO;
      [popupSettingsLeft addSubview:leftLabel];
      [leftLabels addObject:leftLabel];

      [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeLeft];
      [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeRight];

      if (i == 0) {
        [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeTop];
      }
      if (i == [leftTitles count] - 1) {
        [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeBottom];
      }

      NSMutableArray *buttonLine = [[NSMutableArray alloc] init];
      for (int j = 0; j < kNumberOfChannels + 1; j++) { // 16 channels per port
        UIButton *button = [UIButton new];

        button.translatesAutoresizingMaskIntoConstraints = NO;
        [button setBackgroundColor:[UIColor whiteColor]];
        if (j >= kNumberOfChannels) { // controller id
          [button setTitle:@"0" forState:UIControlStateNormal];
          button.enabled = YES;
          [button addTarget:self action:@selector(controllerSelectorButtonTouched:) forControlEvents:UIControlEventTouchUpInside];
          button.tag = i;
        }
        else {
          [button setTitle:@"" forState:UIControlStateNormal];
          button.enabled = NO;
          button.tag = i << 8 | j;
        }
        [button setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];

        [[button layer] setBorderWidth:1.0f];
        [[button layer] setBorderColor:[UIColor lightGrayColor].CGColor];

        button.selected = NO;

        [popupSettingsView addSubview:button];

        [buttonLine addObject:button];
        if (j == 0) {
          [firstButtons addObject:button];
          if (i == 0) {
            [button autoPinEdgeToSuperviewEdge:ALEdgeLeft];
            [button autoPinEdgeToSuperviewEdge:ALEdgeTop];
          }
          if (i == 15) {
            [button autoPinEdgeToSuperviewEdge:ALEdgeLeft];
            [button autoPinEdgeToSuperviewEdge:ALEdgeBottom];
          }
        }
        else {
          [button autoMatchDimension:ALDimensionHeight toDimension:ALDimensionHeight ofView:[firstButtons objectAtIndex:i]];
          //[button autoMatchDimension:ALDimensionWidth toDimension:ALDimensionWidth ofView:[firstButtons objectAtIndex:i] withMultiplier:0.5];
        }
      }
      [buttonLine autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeTop withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];
      [popupControllerIDCol addObject:[buttonLine objectAtIndex:[buttonLine count] - 1]];
      [buttonLine removeObjectAtIndex:[buttonLine count] - 1]; // remove controller id
      [popupButtonRows addObject:buttonLine];
    }
    [leftLabels autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeLeft withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];
    [firstButtons autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeLeft withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];
  }
  else if (currentPopupType == ControllerRemap) {

    NSArray *leftTitles = [[NSArray alloc] initWithObjects:
                           @"Controller Remap 1", @"Controller Remap 2", @"Controller Remap 3", @"Controller Remap 4", @"Controller Remap 5", @"Controller Remap 6", @"Controller Remap 7", @"Controller Remap 8",nil];

    //  popupSettingsView.translatesAutoresizingMaskIntoConstraints = NO;
    if (popupButtonRows != nil) {
      [popupButtonRows removeAllObjects];
    }
    else {
      popupButtonRows = [[NSMutableArray alloc] init];
    }
    NSMutableArray *leftLabels = [[NSMutableArray alloc] init];
    NSMutableArray *topLabels = [[NSMutableArray alloc] init];
    NSMutableArray *firstButtons = [[NSMutableArray alloc] init];

    for (int i = 0; i < kNumberOfChannels + 2; i++) { // 16 channels per port + src/dst
      UIButton *topLabel = [UIButton new];
      topLabel.translatesAutoresizingMaskIntoConstraints = NO;
      [topLabel setBackgroundColor:[UIColor whiteColor]];
      if (i == kNumberOfChannels)
        [topLabel setTitle:@"SRC" forState:UIControlStateNormal];
      else if (i == kNumberOfChannels + 1)
        [topLabel setTitle:@"DST" forState:UIControlStateNormal];
      else
        [topLabel setTitle:[NSString stringWithFormat:@"%d",i + 1] forState:UIControlStateNormal];
      topLabel.titleLabel.font = [UIFont systemFontOfSize:10.0];

      [topLabel setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];

      [[topLabel layer] setBorderWidth:1.0f];
      [[topLabel layer] setBorderColor:[UIColor lightGrayColor].CGColor];

      topLabel.enabled = NO;
      [popupSettingsTop addSubview:topLabel];

      [topLabel autoPinEdgeToSuperviewEdge:ALEdgeTop];
      [topLabel autoPinEdgeToSuperviewEdge:ALEdgeBottom];

      if (i == 0) {
        [topLabel autoPinEdgeToSuperviewEdge:ALEdgeLeft];
      }
      [topLabels addObject:topLabel];
    }

    [topLabels autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeTop withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];

    for (int i = 0; i < [leftTitles count]; i++) {
      UIButton *leftLabel = [UIButton new];
      leftLabel.translatesAutoresizingMaskIntoConstraints = NO;
      [leftLabel setBackgroundColor:[UIColor whiteColor]];
      [leftLabel setTitle:[leftTitles objectAtIndex:i] forState:UIControlStateNormal];
      leftLabel.titleLabel.font = [UIFont systemFontOfSize:10.0];

      [leftLabel setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];

      [[leftLabel layer] setBorderWidth:1.0f];
      [[leftLabel layer] setBorderColor:[UIColor lightGrayColor].CGColor];

      leftLabel.enabled = NO;
      [popupSettingsLeft addSubview:leftLabel];
      [leftLabels addObject:leftLabel];

      [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeLeft];
      [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeRight];

      if (i == 0) {
        [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeTop];
      }
      if (i == [leftTitles count] - 1) {
        [leftLabel autoPinEdgeToSuperviewEdge:ALEdgeBottom];
      }

      NSMutableArray *buttonLine = [[NSMutableArray alloc] init];
      for (int j = 0; j < kNumberOfChannels + 2; j++) { // 16 channels per port + src/dst
        UIButton *button = [UIButton new];

        button.translatesAutoresizingMaskIntoConstraints = NO;
        [button setBackgroundColor:[UIColor whiteColor]];
        if (j >= kNumberOfChannels) { // src/dst
          [button setTitle:@"0" forState:UIControlStateNormal];
          button.enabled = YES;
          [button addTarget:self action:@selector(controllerSelectorButtonTouched:) forControlEvents:UIControlEventTouchUpInside];
          if (j == kNumberOfChannels) {// source
            button.tag = 0x100 | i;
          }
          else {
            button.tag = i;
          }
        }
        else {
          [button setTitle:@"" forState:UIControlStateNormal];
          button.enabled = NO;

          button.tag = i << 8 | j;
        }
        [button setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];

        [[button layer] setBorderWidth:1.0f];
        [[button layer] setBorderColor:[UIColor lightGrayColor].CGColor];

        button.selected = NO;

        [popupSettingsView addSubview:button];

        [buttonLine addObject:button];
        if (j == 0) {
          [firstButtons addObject:button];
          if (i == 0) {
            [button autoPinEdgeToSuperviewEdge:ALEdgeLeft];
            [button autoPinEdgeToSuperviewEdge:ALEdgeTop];
          }
          if (i == 15) {
            [button autoPinEdgeToSuperviewEdge:ALEdgeLeft];
            [button autoPinEdgeToSuperviewEdge:ALEdgeBottom];
          }
        }
        else {
          [button autoMatchDimension:ALDimensionHeight toDimension:ALDimensionHeight ofView:[firstButtons objectAtIndex:i]];
          //[button autoMatchDimension:ALDimensionWidth toDimension:ALDimensionWidth ofView:[firstButtons objectAtIndex:i] withMultiplier:0.5];
        }
      }
      [buttonLine autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeTop withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];
      [popupControllerIDDstCol addObject:[buttonLine objectAtIndex:[buttonLine count] - 1]];
      [buttonLine removeObjectAtIndex:[buttonLine count] - 1]; // remove dst
      [popupControllerIDSrcCol addObject:[buttonLine objectAtIndex:[buttonLine count] - 1]];
      [buttonLine removeObjectAtIndex:[buttonLine count] - 1]; // remove src
      [popupButtonRows addObject:buttonLine];
    }
    [leftLabels autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeLeft withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];
    [firstButtons autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeLeft withFixedSpacing:0.0 insetSpacing:NO matchedSizes:YES];
  }
}

- (void) buttonToggled:(UIButton*)but {
  but.selected = !but.selected;
  [self updateButtonColor:but];
  [self sendUpdateForButton:but];
}

- (void) sendUpdateForButton:(UIButton*)but {
  int row = but.tag >> 8;
  int col = but.tag & 0xff;


  if (currentPopupType == PortFilters) {
    [self sendPortFilterUpdateForRow:row Col:col State:but.selected];
  }
  else if (currentPopupType == ChannelRemap) {
    [self sendChannelRemapUpdateForRow:row Col:col State:but.selected];
  }
  else if (currentPopupType == ControllerFilters) {
    [self sendControllerFilterUpdateForRow:row Col:col State:but.selected];
  }
  else if (currentPopupType == ControllerRemap) {
    [self sendControllerRemapUpdateForRow:row Col:col State:but.selected];
  }
}

- (void) sendPortFilterUpdateForRow:(int)row Col:(int)col State:(int)state {
  [self.midiPatchbayDelegate setPortFilterAtRow:row Col:col DevicePort:popupDevicePort Port:popupPort Put:popupPut ToState:state];
}

- (void) sendChannelRemapUpdateForRow:(int)row Col:(int)col State:(int)state {
  [self.midiPatchbayDelegate setChannelRemapAtRow:row Col:col DevicePort:popupDevicePort Port:popupPort Put:popupPut ToState:state];
}

- (void) sendControllerFilterUpdateForRow:(int)row Col:(int)col State:(int)state {
  [self.midiPatchbayDelegate setControllerFilterAtRow:row Col:col DevicePort:popupDevicePort Port:popupPort Put:popupPut ToState:state];
}

- (void) sendControllerRemapUpdateForRow:(int)row Col:(int)col State:(int)state {
  [self.midiPatchbayDelegate setControllerRemapAtRow:row Col:col DevicePort:popupDevicePort Port:popupPort Put:popupPut ToState:state];
}


- (void) updateButtonColor:(UIButton*)but {
  if (but.selected) {
    [but setBackgroundColor:[UIColor grayColor]];
  }
  else {
    [but setBackgroundColor:[UIColor whiteColor]];
  }

}

- (void) remapToChannelButtonTouched:(UIButton*)but {
  int currentNumber = [[but titleForState:UIControlStateNormal] intValue];
  int col = but.tag;
  
  currentNumber++;
  if (currentNumber > kNumberOfChannels)
    currentNumber = 1;
  [but setTitle:[NSString stringWithFormat:@"%d", currentNumber] forState:UIControlStateNormal];

  [self.midiPatchbayDelegate setChannelRemapAtCol:col DevicePort:popupDevicePort Port:popupPort Put:popupPut ToState:currentNumber];
}

- (void) controllerSelectorButtonTouched:(UIButton*)but {
  pickerType = Controller;
  [self.pickerView reloadAllComponents];
  currentButtonBeingPickered = nil;

  int currentController = [[but titleForState:UIControlStateNormal] intValue];

  [self.pickerView selectRow:currentController inComponent:0 animated:NO];
  [self.pickerViewTextField becomeFirstResponder];

  currentButtonBeingPickered = but;
}

- (void)cancelTouched:(UIBarButtonItem *)sender
{
  // hide the picker view
  [self.pickerViewTextField resignFirstResponder];
}

- (void)doneTouched:(UIBarButtonItem *)sender
{
  // hide the picker view
  [self.pickerViewTextField resignFirstResponder];

  if (pickerType == Controller) {
    if (currentButtonBeingPickered != nil) {
      [self.pickerViewTextField resignFirstResponder];

      [currentButtonBeingPickered setTitle:[NSString stringWithFormat:@"%ld", (long)[self.pickerView selectedRowInComponent:0]] forState:UIControlStateNormal];

      if (currentPopupType == ControllerFilters) {
        int row = currentButtonBeingPickered.tag;
        [self.midiPatchbayDelegate setControllerFilterIDAtRow:row DevicePort:popupDevicePort Port:popupPort Put:popupPut ToState:[self.pickerView selectedRowInComponent:0]];
      }
      else if (currentPopupType == ControllerRemap) {
        bool isSource = currentButtonBeingPickered.tag >> 8;
        int row = currentButtonBeingPickered.tag & 0xff;
        [self.midiPatchbayDelegate setControllerRemapIDAtRow:row DevicePort:popupDevicePort Port:popupPort Put:popupPut ToState:[self.pickerView selectedRowInComponent:0] isSource:isSource];
      }
    }
    currentButtonBeingPickered = nil;
  }
  else if (pickerType == Port){
    popupPort = [self.pickerView selectedRowInComponent:0] + 1;
    [self redrawPopup];
  }
}

#pragma mark - UIPickerViewDataSource
- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
  return 1;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
  if (pickerType == Controller) {
    return [controllerNames count];
  }
  else {
    if (popupPut == Input) {
      return [self.midiPatchbayDelegate realNumSourcesPerSection:popupDevicePort];
    }
    else {
      return [self.midiPatchbayDelegate realNumDestinationsPerSection:popupDevicePort];

    }
  }
}

#pragma mark - UIPickerViewDelegate
- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
  if (pickerType == Controller) {
    NSString *item = [controllerNames objectAtIndex:row];
    return item;
  }
  else {
    NSString *item = [self.midiPatchbayDelegate nameForDevicePort:popupDevicePort andPort:row + 1];
    return item;
  }
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
  // perform some action

}

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view, typically from a nib.

  filterTouchPredicate = [NSPredicate
      predicateWithFormat:@"(phase = %d) OR (phase = %d) OR (phase = %d)",
                          UITouchPhaseBegan, UITouchPhaseMoved,
                          UITouchPhaseStationary];
  redrawTimer = nil;

  pathLayer = [[CAShapeLayer alloc] init];
  bluePathLayer = [[CAShapeLayer alloc] init];
  proposedLayer = [[CAShapeLayer alloc] init];

  [imageView.layer addSublayer:pathLayer];
  [imageView.layer addSublayer:bluePathLayer];
  [imageView.layer addSublayer:proposedLayer];

  [self.view setContentScaleFactor:[[UIScreen mainScreen] scale]];
  [imageView setContentScaleFactor:[[UIScreen mainScreen] scale]];

  self.view.multipleTouchEnabled = NO;
  self.view.exclusiveTouch = YES;

/*  grayView.exclusiveTouch = YES;
  popupView.exclusiveTouch = YES;*/

  popupShowing = NO;

  grayView.hidden = YES;
  popupView.hidden = YES;

  self.pickerViewTextField = [[UITextField alloc] initWithFrame:CGRectZero];
  [self.view addSubview:self.pickerViewTextField];

  self.pickerView = [[UIPickerView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
  self.pickerView.showsSelectionIndicator = YES;
  self.pickerView.dataSource = self;
  self.pickerView.delegate = self;

  self.pickerViewTextField.inputView = self.pickerView;

  UIToolbar *toolBar = [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, 320, 44)];
  toolBar.barStyle = UIBarStyleDefault;

  UIBarButtonItem *doneButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(doneTouched:)];
  UIBarButtonItem *cancelButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemCancel target:self action:@selector(cancelTouched:)];

  // the middle button is to make the Done button align to right
  [toolBar setItems:[NSArray arrayWithObjects:cancelButton, [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil], doneButton, nil]];
  self.pickerViewTextField.inputAccessoryView = toolBar;

  lpgr = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleLongPressGestures:)];
  lpgr.minimumPressDuration = 0.4f;
  lpgr.allowableMovement = 20.0f;
  lpgr.cancelsTouchesInView = NO;

  [self.view addGestureRecognizer:lpgr];

  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  BOOL showBubble = YES;
  if ([defaults objectForKey:@"showBubbleOnMidiPatchbay"] != nil) {
    showBubble = [defaults boolForKey:@"showBubbleOnMidiPatchbay"];
  }
  [bubbleButton setEnabled:showBubble];
  [bubbleButton setHidden:!showBubble];
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [self.navigationController setNavigationBarHidden:NO animated:YES];

  [portFiltersButton setTintColor:[UIColor redColor]];
}

- (void)inContextSetSectionColor {
  CGContextSetRGBStrokeColor(UIGraphicsGetCurrentContext(), sectionColorRed,
                             sectionColorGreen, sectionColorBlue,
                             sectionColorAlpha);
  CGContextSetRGBFillColor(UIGraphicsGetCurrentContext(), sectionColorRed,
                           sectionColorGreen, sectionColorBlue,
                           sectionColorAlpha);
}

- (void)inContextSetBlackColor:(CGFloat)alpha {
  CGContextSetRGBStrokeColor(UIGraphicsGetCurrentContext(), 0, 0, 0, alpha);
  CGContextSetRGBFillColor(UIGraphicsGetCurrentContext(), 0, 0, 0, alpha);
}

- (void)inContextSetAdditionColor {
  CGContextSetRGBStrokeColor(UIGraphicsGetCurrentContext(), additionColorRed,
                             additionColorGreen, additionColorBlue,
                             additionColorAlpha);
  CGContextSetRGBFillColor(UIGraphicsGetCurrentContext(), additionColorRed,
                           additionColorGreen, additionColorBlue,
                           additionColorAlpha);
}

- (void)inContextSetSolidLine {
  CGContextSetLineDash(UIGraphicsGetCurrentContext(), 0, NULL, 0);
}

- (void)inContextSetDottedLine {
  CGFloat pattern[] = {1, 5};
  CGContextSetLineDash(UIGraphicsGetCurrentContext(), 0, pattern,
                       sizeof(pattern) / sizeof(CGFloat));
}

- (void)inContextDrawHorizontalHeaderBackground:(CGSize)viewSize {
  CGContextSaveGState(UIGraphicsGetCurrentContext());
  [self inContextSetBlackColor:headerAlpha];
  CGContextFillRect(
      UIGraphicsGetCurrentContext(),
      [self horizontalHeaderRect:CGPointMake(0, 0) size:viewSize]);

  CGContextRestoreGState(UIGraphicsGetCurrentContext());
}

- (void)inContextDrawVerticalGutterBackground:(CGSize)viewSize {
  [self inContextSetBlackColor:blendedAlpha];

  CGContextSaveGState(UIGraphicsGetCurrentContext());
  [self inContextSetBlackColor:headerAlpha];

  CGPoint topRightGrid =
      CGPointMake([self gridWidth:viewSize],
                  [self horizontalHeaderHeight] + [self gutterSize]);
  CGPoint bottomRightGrid =
      CGPointMake([self gridWidth:viewSize], viewSize.height);
  CGPoint topRightView = CGPointMake(viewSize.width, 0);

  CGPoint bottomLeftVerticalGutter = CGPointMake(
      [self gridWidth:viewSize] + [self gutterSize], viewSize.height);
  CGPoint topLeftVerticalGutter =
      CGPointMake([self gridWidth:viewSize] + [self gutterSize],
                  [self horizontalHeaderHeight] + [self gutterSize]);
  CGPoint topRightVerticalGutter = CGPointMake(
      viewSize.width, [self horizontalHeaderHeight] + [self gutterSize]);

  MoveToPoint(topRightGrid);
  AddPoint(topRightView);
  AddPoint(topRightVerticalGutter);
  AddPoint(topLeftVerticalGutter);
  AddPoint(bottomLeftVerticalGutter);
  AddPoint(bottomRightGrid);
  CGContextClosePath(UIGraphicsGetCurrentContext());

  CGContextDrawPath(UIGraphicsGetCurrentContext(), kCGPathFillStroke);

  CGContextRestoreGState(UIGraphicsGetCurrentContext());
}

- (void)inContextDrawVericalSections:(CGSize)viewSize {
  [self inContextSetSectionColor];

  CGFloat totalSources = (CGFloat)[self.midiPatchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:viewSize] / totalSources;
  CGFloat xOffset = 0.0f;
  CGFloat yOffset = 0;
  CGFloat height = viewSize.height;

  bool drawFlipFlop = false;
  for (int section = 1; section <= [self.midiPatchbayDelegate numSourceSections];
       ++section) {
    CGFloat numInputs =
        (CGFloat)[self.midiPatchbayDelegate numSourcesPerSection : section];
    if (numInputs) {
      CGFloat width = pinDX * numInputs;

      if (drawFlipFlop) {
        CGContextFillRect(UIGraphicsGetCurrentContext(),
                          CGRectMake(xOffset, yOffset, width, height));
      }

      drawFlipFlop = !drawFlipFlop;
      xOffset += width;
    }
  }
}

- (void)inContextDrawHorizontalSections:(CGSize)viewSize {
  [self inContextSetSectionColor];

  CGFloat totalDestinations =
      (CGFloat)[self.midiPatchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:viewSize] / totalDestinations;
  CGFloat xOffset = 0;
  CGFloat yOffset = [self horizontalHeaderHeight] + [self gutterSize];
  CGFloat width = viewSize.width;

  bool drawFlipFlop = false;
  for (int section = 1;
       section <= [self.midiPatchbayDelegate numDestinationSections]; ++section) {
    CGFloat numOutputs =
        (CGFloat)[self.midiPatchbayDelegate numDestinationsPerSection : section];
    if (numOutputs) {
      CGFloat height = pinDY * numOutputs;

      if (drawFlipFlop) {
        CGContextFillRect(UIGraphicsGetCurrentContext(),
                          CGRectMake(xOffset, yOffset, width, height));
      }

      drawFlipFlop = !drawFlipFlop;
      yOffset += height;
    }
  }
}

- (void)inContextDrawBorder:(CGSize)viewSize {
  [self inContextSetBlackColor:fullAlpha];
  [self inContextSetSolidLine];
  CGContextSetLineWidth(UIGraphicsGetCurrentContext(), 1.0f);

  /* Draw the lines that seperate the sections */

  CGPoint topLeftGrid =
      CGPointMake(0, [self horizontalHeaderHeight] + [self gutterSize]);
  CGPoint topRightGrid =
      CGPointMake([self gridWidth:viewSize],
                  [self horizontalHeaderHeight] + [self gutterSize]);
  CGPoint bottomRightGrid =
      CGPointMake([self gridWidth:viewSize], viewSize.height);
  CGPoint topRightView = CGPointMake(viewSize.width, 0);

  CGPoint bottomLeftHorizontalGutter =
      CGPointMake(0, [self horizontalHeaderHeight]);
  CGPoint bottomRightHorizontalGutter =
      CGPointMake([self gridWidth:viewSize], [self horizontalHeaderHeight]);
  CGPoint topRightHorizontalGutter = CGPointMake([self gridWidth:viewSize], 0);

  CGPoint bottomLeftVerticalGutter = CGPointMake(
      [self gridWidth:viewSize] + [self gutterSize], viewSize.height);
  CGPoint topLeftVerticalGutter =
      CGPointMake([self gridWidth:viewSize] + [self gutterSize],
                  [self horizontalHeaderHeight] + [self gutterSize]);
  CGPoint topRightVerticalGutter = CGPointMake(
      viewSize.width, [self horizontalHeaderHeight] + [self gutterSize]);

  [self inContextDrawLine:topLeftGrid to:topRightGrid];
  [self inContextDrawLine:topRightGrid to:bottomRightGrid];
  [self inContextDrawLine:topRightGrid to:topRightView];
  [self inContextDrawLine:bottomLeftHorizontalGutter
                       to:bottomRightHorizontalGutter];
  [self inContextDrawLine:bottomRightHorizontalGutter
                       to:topRightHorizontalGutter];
  [self inContextDrawLine:bottomLeftVerticalGutter to:topLeftVerticalGutter];
  [self inContextDrawLine:topLeftVerticalGutter to:topRightVerticalGutter];
  CGContextStrokePath(UIGraphicsGetCurrentContext());
}

- (void)inContextDrawDottedGrid:(CGSize)viewSize {
  CGFloat totalSources = (CGFloat)[self.midiPatchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:viewSize] / totalSources;
  CGFloat totalDestinations =
      (CGFloat)[self.midiPatchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:viewSize] / totalDestinations;

  CGFloat xOffset, yOffset;

  xOffset = pinDX * 0.5f;
  yOffset = [self horizontalHeaderHeight] + [self gutterSize] - arrowYOffset;

  CGContextSetLineWidth(UIGraphicsGetCurrentContext(), 0.5);
  CGContextSetLineCap(UIGraphicsGetCurrentContext(), kCGLineCapRound);
  [self inContextSetDottedLine];
  [self inContextSetBlackColor:blendedAlpha];

  for (int i = 0; i < (int)totalSources; ++i) {
    [self inContextDrawLine:CGPointMake(xOffset, yOffset)
                         to:CGPointMake(xOffset, viewSize.height)];
    xOffset += pinDX;
  }

  xOffset = 0.0f;
  yOffset = [self horizontalHeaderHeight] + [self gutterSize] + pinDY * 0.5f;
  for (int i = 0; i < (int)totalDestinations; ++i) {
    [self
        inContextDrawLine:CGPointMake(xOffset, yOffset)
                       to:CGPointMake([self gridWidth:viewSize] + arrowXOffset,
                                      yOffset)];
    yOffset += pinDY;
  }

  CGContextStrokePath(UIGraphicsGetCurrentContext());

  [self inContextSetBlackColor:gridBackAlpha];
  CGContextFillRect(UIGraphicsGetCurrentContext(),
                    [self gridRect:CGPointMake(0, 0) size:viewSize]);
}

- (void)inContextDrawHorizontalPortNumbers:(CGSize)viewSize {
  [self inContextSetBlackColor:fullAlpha];

  int numSections = [self.midiPatchbayDelegate numSourceSections];
  CGFloat totalSources = (CGFloat)[self.midiPatchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:viewSize] / totalSources;

  CGFloat xOffset, yOffset;

  xOffset = 0.0f;
  yOffset = [self horizontalHeaderHeight];

  for (int i = 1; i <= numSections; ++i) {
    int pinsPerSection = [self.midiPatchbayDelegate numSourcesPerSection:i];
    for (int pin = 1; pin <= pinsPerSection; ++pin) {
      NSString *text = [NSString stringWithFormat:@"%d", pin];
      if (![self.midiPatchbayDelegate isCollapsedInput:i]) {
        [text drawInRect:CGRectMake(xOffset, yOffset, pinDX, [self gutterSize])
                withFont:[UIFont systemFontOfSize:numberFontSize]
           lineBreakMode:NSLineBreakByCharWrapping
               alignment:NSTextAlignmentCenter];
      }

      xOffset += pinDX;
    }
  }
}

- (void)inContextDrawHorizontalPortNames:(CGSize)viewSize {
  [self inContextSetBlackColor:fullAlpha];

  int numSections = [self.midiPatchbayDelegate numSourceSections];
  CGFloat totalSources = (CGFloat)[self.midiPatchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:viewSize] / totalSources;

  CGFloat xOffset, yOffset;

  xOffset = 0.0f;
  yOffset = [self horizontalHeaderHeight] * 0.5f;

  for (int i = 1; i <= numSections; ++i) {
    int pinsPerSection = [self.midiPatchbayDelegate numSourcesPerSection:i];
    NSString *text = [self.midiPatchbayDelegate nameForSection:i];
    if (pinsPerSection)
    {
      [text drawInRect:CGRectMake(xOffset, yOffset, pinsPerSection * pinDX,
                                  [self horizontalHeaderHeight])
              withFont:[UIFont systemFontOfSize:19]
                 angle:0
         lineBreakMode:NSLineBreakByWordWrapping];
      xOffset += pinsPerSection * pinDX;
    }
  }
}

- (void)inContextDrawVerticalPortNames:(CGSize)viewSize {
  [self inContextSetBlackColor:fullAlpha];

  int numSections = [self.midiPatchbayDelegate numSourceSections];
  CGFloat totalDestinations =
      (CGFloat)[self.midiPatchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:viewSize] / totalDestinations;

  CGFloat xOffset, yOffset;

  xOffset = [self gridWidth:viewSize] + [self gutterSize] +
            [self verticalHeaderWidth] * 0.5f;
  yOffset = [self horizontalHeaderHeight] + [self gutterSize];

  for (int i = 1; i <= numSections; ++i) {
    int pinsPerSection = [self.midiPatchbayDelegate numDestinationsPerSection:i];
    NSString *text = [self.midiPatchbayDelegate nameForSection:i];
    if (pinsPerSection) {
      [text drawInRect:CGRectMake(xOffset, yOffset, pinDY * pinsPerSection,
                                  [self verticalHeaderWidth])
              withFont:[UIFont systemFontOfSize:19]
                 angle:verticalHeaderAngle
         lineBreakMode:NSLineBreakByWordWrapping];
      yOffset += pinsPerSection * pinDY;
    }
  }
}

- (void)inContextDrawVerticalPortNumbers:(CGSize)viewSize {
  [self inContextSetBlackColor:fullAlpha];

  int numSections = [self.midiPatchbayDelegate numDestinationSections];
  CGFloat totalDestinations =
      (CGFloat)[self.midiPatchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:viewSize] / totalDestinations;

  CGFloat xOffset, yOffset;

  xOffset = [self gridWidth:viewSize] + arrowXOffset + arrowHeight;
  yOffset = [self horizontalHeaderHeight] + [self gutterSize] + pinDY * 0.5f;

  for (int i = 1; i <= numSections; ++i) {
    int pinsPerSection = [self.midiPatchbayDelegate numDestinationsPerSection:i];
    for (int pin = 1; pin <= pinsPerSection; ++pin) {
      NSString *text = [NSString stringWithFormat:@"%d", pin];

      if (![self.midiPatchbayDelegate isCollapsedOutput:i]) {
        [text drawInRect:CGRectMake(
                                    xOffset, yOffset,
                                    [self gutterSize] - arrowHeight - arrowXOffset,
                                    pinDY)
                withFont:[UIFont systemFontOfSize:numberFontSize]
                   angle:0
           lineBreakMode:NSLineBreakByClipping];
      }

      yOffset += pinDY;
    }
  }
}

- (void)inContextDrawOutArrows:(CGSize)viewSize {
  [self inContextSetBlackColor:fullAlpha];

  CGFloat totalSources = (CGFloat)[self.midiPatchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:viewSize] / totalSources;

  CGFloat xOffset, yOffset;

  xOffset = pinDX * 0.5f;
  yOffset = [self horizontalHeaderHeight] + [self gutterSize] - arrowYOffset;

  for (int i = 1; i <= (int)totalSources; ++i) {
    [self inContextDrawDownTriangle:CGPointMake(xOffset, yOffset)];
    xOffset += pinDX;
  }
}

- (void)inContextDrawInArrows:(CGSize)viewSize {
  CGFloat totalDestinations =
      (CGFloat)[self.midiPatchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:viewSize] / totalDestinations;

  CGFloat xOffset, yOffset;

  xOffset = [self gridWidth:viewSize] + arrowXOffset;
  yOffset = [self horizontalHeaderHeight] + [self gutterSize] + pinDY * 0.5f;
  for (int i = 1; i <= (int)totalDestinations; ++i) {
    [self inContextDrawRightTriangle:CGPointMake(xOffset, yOffset)];
    yOffset += pinDY;
  }
}

- (void)inContextDrawLegend:(CGSize)viewSize {
  CGFloat xOffset, yOffset;

  CGFloat angle;

  CGPoint topRightGrid =
      CGPointMake([self gridWidth:viewSize],
                  [self horizontalHeaderHeight] + [self gutterSize]);

  CGPoint topRightView = CGPointMake(viewSize.width, 0);

  angle =
      atan2f(topRightView.y - topRightGrid.y, topRightView.x - topRightGrid.x);

  CGPoint p2 = topRightView;
  CGPoint p1 = topRightGrid;
  // Assign the coord of p2 and p1...
  // End Assign...
  CGFloat xDist = (p2.x - p1.x);                               //[2]
  CGFloat yDist = (p2.y - p1.y);                               //[3]
  CGFloat distance = sqrt((xDist * xDist) + (yDist * yDist));  //[4]

  NSString *outText = @"Sources";
  xOffset = topRightGrid.x + abs(topRightView.x - topRightGrid.x) * 0.4f;
  yOffset = abs(topRightView.y - topRightGrid.y) * 0.4f;

  [outText drawAtPoint:CGPointMake(xOffset, yOffset)
              forWidth:distance
              withFont:[UIFont boldSystemFontOfSize:14]
                 angle:angle
         lineBreakMode:NSLineBreakByCharWrapping];

  NSString *inText = @"Destinations";
  xOffset = topRightGrid.x + abs(topRightView.x - topRightGrid.x) * 0.6f;
  yOffset = abs(topRightView.y - topRightGrid.y) * 0.6f;
  [inText drawAtPoint:CGPointMake(xOffset, yOffset)
             forWidth:distance
             withFont:[UIFont boldSystemFontOfSize:14]
                angle:angle
        lineBreakMode:NSLineBreakByCharWrapping];
}

- (void)inContextDrawBackgroundWithViewSize:(CGSize)viewSize
                           rightGutterWidth:(CGFloat)rightGutterWidth
                            topGutterHeight:(CGFloat)topGutterHeight
                          numSourceSections:(unsigned int)numSourceSections
                     numDestinationSections:
                         (unsigned int)numDestinationSections {
  assert(self.midiPatchbayDelegate);

  [self inContextSetSectionColor];

  [self inContextDrawHorizontalHeaderBackground:viewSize];

  [self inContextDrawVerticalGutterBackground:viewSize];

  [self inContextDrawVericalSections:viewSize];

  [self inContextDrawHorizontalSections:viewSize];

  [self inContextDrawDottedGrid:viewSize];

  [self inContextDrawHorizontalPortNumbers:viewSize];

  [self inContextDrawHorizontalPortNames:viewSize];

  [self inContextDrawVerticalPortNames:viewSize];

  [self inContextDrawVerticalPortNumbers:viewSize];

  [self inContextDrawBorder:viewSize];

  [self inContextDrawOutArrows:viewSize];

  [self inContextDrawInArrows:viewSize];

  [self inContextDrawLegend:viewSize];
}

- (void)inContextDrawPortsWithViewSize:(CGSize)viewSize
                      rightGutterWidth:(CGFloat)rightGutterWidth
                       topGutterHeight:(CGFloat)topGutterHeight
                     numSourceSections:(unsigned int)numSourceSections
                     portsPerInSection:(PortCount)portsPerInSection
                numDestinationSections:(unsigned int)numDestinationSections
                    portsPerOutSection:(PortCount)portsPerOutSection {
  CGFloat width = viewSize.width;
  CGFloat gridWidth = width - rightGutterWidth;
  CGFloat yOffset = topGutterHeight;

  CGContextSetLineCap(UIGraphicsGetCurrentContext(), kCGLineCapRound);
  CGContextSetLineWidth(UIGraphicsGetCurrentContext(), 1.0);
  CGContextSetRGBStrokeColor(UIGraphicsGetCurrentContext(), 0, 0, 0, 0.8);

  CGContextSetBlendMode(UIGraphicsGetCurrentContext(), kCGBlendModeNormal);
  for (unsigned int i = 1; i <= numSourceSections; ++i) {
    unsigned int inPortCount = portsPerInSection(i);
    for (unsigned int j = 1; j <= inPortCount; ++j) {
      CGContextMoveToPoint(UIGraphicsGetCurrentContext(), 0, yOffset);
      CGContextAddLineToPoint(UIGraphicsGetCurrentContext(), gridWidth,
                              yOffset);
    }
  }

  CGContextStrokePath(UIGraphicsGetCurrentContext());
}

- (void)inContextDrawPatches {
  CGMutablePathRef patchPath = CGPathCreateMutable();
  CGMutablePathRef bluePatchPath = CGPathCreateMutable();

  CGFloat totalSources = (CGFloat)[self.midiPatchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:imageView.frame.size] / totalSources;
  CGFloat totalDestinations =
      (CGFloat)[self.midiPatchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:imageView.frame.size] / totalDestinations;

  if (self.midiPatchbayDelegate) {
    [self.midiPatchbayDelegate foreach:^(midi_pair_t *pair) {
        if ((pair) &&
            ([self.midiPatchbayDelegate isSourceDeviceInBounds:pair->source]) &&
            ([self.midiPatchbayDelegate
                isDestinationDeviceInBounds:pair->destination])) {
          CGPoint a, b, c;

          int totalSrcs =
              [self.midiPatchbayDelegate sourceIndexToTotal:pair->source];
          int totalDests =
              [self.midiPatchbayDelegate destinationIndexToTotal:pair->destination];

          CGFloat horX = (CGFloat)(totalSrcs) * pinDX + pinDX * 0.5f;
          CGFloat horY = (CGFloat)(totalDests) * pinDY + pinDY * 0.5f;

          CGRect grid =
              [self gridRect:imageView.frame.origin size:imageView.frame.size];

          a = CGPointMake(grid.origin.x + horX, [self verticalArrowOffset]);
          b = CGPointMake(grid.origin.x + horX, grid.origin.y + horY);
          c = CGPointMake([self horizontalArrowOffset:imageView.frame.size],
                          grid.origin.y + horY);

          if ([self.midiPatchbayDelegate isCollapsedInput:pair->source.devicePort] ||
              [self.midiPatchbayDelegate isCollapsedOutput:pair->destination.devicePort]) {
            CGPathMoveToPoint(bluePatchPath, NULL, a.x, a.y);
            CGPathAddLineToPoint(bluePatchPath, NULL, b.x, b.y);
            CGPathAddLineToPoint(bluePatchPath, NULL, c.x, c.y);
            CGPathAddLineToPoint(bluePatchPath, NULL, b.x, b.y);
            CGPathAddLineToPoint(bluePatchPath, NULL, a.x, a.y);
            CGPathAddEllipseInRect(
                                   bluePatchPath, NULL,
                                   CGRectMake(b.x - circleRadius, b.y - circleRadius,
                                              circleRadius * 2.0f, circleRadius * 2.0f));
          }
          else {
            CGPathMoveToPoint(patchPath, NULL, a.x, a.y);
            CGPathAddLineToPoint(patchPath, NULL, b.x, b.y);
            CGPathAddLineToPoint(patchPath, NULL, c.x, c.y);
            CGPathAddLineToPoint(patchPath, NULL, b.x, b.y);
            CGPathAddLineToPoint(patchPath, NULL, a.x, a.y);
            CGPathAddEllipseInRect(
                                   patchPath, NULL,
                                   CGRectMake(b.x - circleRadius, b.y - circleRadius,
                                              circleRadius * 2.0f, circleRadius * 2.0f));
          }
        }
    }];

  }

  pathLayer.allowsEdgeAntialiasing = NO;
  pathLayer.lineWidth = 1.2f;
  pathLayer.fillColor = [[UIColor blackColor] CGColor];
  pathLayer.strokeColor = [[UIColor blackColor] CGColor];
  pathLayer.path = patchPath;

  bluePathLayer.allowsEdgeAntialiasing = NO;
  bluePathLayer.lineWidth = 1.2f;
  bluePathLayer.fillColor = [[UIColor blueColor] CGColor];
  bluePathLayer.strokeColor = [[UIColor blueColor] CGColor];
  bluePathLayer.path = bluePatchPath;

  CGPathRelease(bluePatchPath);
  CGPathRelease(patchPath);
}

- (void)inPathDrawProposedPatches:(NSSet *)touches ofSize:(CGSize)viewSize {
  CGMutablePathRef path = CGPathCreateMutable();
  BOOL isAdd = FALSE;

  CGFloat totalSources = (CGFloat)[self.midiPatchbayDelegate totalSources];
  CGFloat pinDX = [self gridWidth:imageView.frame.size] / totalSources;
  CGFloat totalDestinations =
      (CGFloat)[self.midiPatchbayDelegate totalDestinations];
  CGFloat pinDY = [self gridHeight:imageView.frame.size] / totalDestinations;

  // loop through all touchs
  for (UITouch *touch in touches) {
    CGPoint location = [touch locationInView:imageView];
    CGRect grid = [self gridRect:CGPointMake(0, 0) size:viewSize];

    // filter out all non begin and move phases
    if (CGRectContainsPoint(grid, location)) {
      // modify the location to inside the grid
      location.x -= grid.origin.x;
      location.y -= grid.origin.y;

      // determine the corresponding row and column
      int col = (int)floor(location.x / pinDX) + 1;
      int row = (int)floor(location.y / pinDY) + 1;

      auto srcIndex = [self.midiPatchbayDelegate sourceTotalToIndex:col];
      auto dstIndex = [self.midiPatchbayDelegate destinationTotalToIndex:row];

      // check the bounds
      if (([self.midiPatchbayDelegate isSourceDeviceInBounds:srcIndex]) &&
          ([self.midiPatchbayDelegate isDestinationDeviceInBounds:dstIndex])) {
        isAdd = ![self.midiPatchbayDelegate isPatchedFrom:srcIndex to:dstIndex];
        CGFloat horX = (CGFloat)(col - 1) * pinDX + pinDX * 0.5f;
        CGFloat horY = (CGFloat)(row - 1) * pinDY + pinDY * 0.5f;

        CGPoint a = CGPointMake(grid.origin.x + horX,
                                [self verticalArrowOffset] + arrowHeight);
        CGPoint b = CGPointMake(grid.origin.x + horX, grid.origin.y + horY);
        CGPoint c = CGPointMake([self gridWidth:viewSize] + arrowXOffset,
                                grid.origin.y + horY);

        CGPathMoveToPoint(path, NULL, a.x, a.y);
        CGPathAddLineToPoint(path, NULL, b.x, b.y);
        CGPathAddLineToPoint(path, NULL, c.x, c.y);
        CGPathAddLineToPoint(path, NULL, b.x, b.y);
        CGPathAddLineToPoint(path, NULL, a.x, a.y);
        [self inPathDrawDownTriangle:&path point:a];
        [self inPathDrawRightTriangle:&path point:c];

        if (isAdd) {
          CGPathAddEllipseInRect(
              path, NULL,
              CGRectMake(b.x - circleRadius * 2, b.y - circleRadius * 2,
                         circleRadius * 4.0f, circleRadius * 4.0f));
        } else {
          CGPoint ab, bc;
          ab.x = (a.x + b.x) * 0.5f;
          ab.y = (a.y + b.y) * 0.5f;
          bc.x = (b.x + c.x) * 0.5f;
          bc.y = (b.y + c.y) * 0.5f;

          CGPathMoveToPoint(path, NULL, ab.x - crossWidth, ab.y - crossWidth);
          CGPathAddLineToPoint(path, NULL, ab.x + crossWidth,
                               ab.y + crossWidth);
          CGPathMoveToPoint(path, NULL, ab.x + crossWidth, ab.y - crossWidth);
          CGPathAddLineToPoint(path, NULL, ab.x - crossWidth,
                               ab.y + crossWidth);

          CGPathMoveToPoint(path, NULL, b.x - crossWidth, b.y - crossWidth);
          CGPathAddLineToPoint(path, NULL, b.x + crossWidth, b.y + crossWidth);
          CGPathMoveToPoint(path, NULL, b.x + crossWidth, b.y - crossWidth);
          CGPathAddLineToPoint(path, NULL, b.x - crossWidth, b.y + crossWidth);

          CGPathMoveToPoint(path, NULL, bc.x - crossWidth, bc.y - crossWidth);
          CGPathAddLineToPoint(path, NULL, bc.x + crossWidth,
                               bc.y + crossWidth);
          CGPathMoveToPoint(path, NULL, bc.x + crossWidth, bc.y - crossWidth);
          CGPathAddLineToPoint(path, NULL, bc.x - crossWidth,
                               bc.y + crossWidth);
        }
      }
    }


  }
  proposedLayer.lineWidth = 3.0f;
  proposedLayer.fillColor =
      (isAdd ? [[UIColor blueColor] CGColor] : [[UIColor redColor] CGColor]);
  proposedLayer.strokeColor =
      (isAdd ? [[UIColor blueColor] CGColor] : [[UIColor redColor] CGColor]);
  proposedLayer.path = path;
  proposedLayer.lineJoin = kCALineJoinMiter;
  proposedLayer.miterLimit = 1000.0f;
  proposedLayer.lineCap = kCALineCapSquare;
  CGPathRelease(path);
}

- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];
  imageView.layer.contents = (id)[[self generateStaticImage] CGImage];
  [self updateDynamicPaths:[NSSet set]];
  [self becomeFirstResponder];

  if (!redrawTimer) {
    redrawTimer = [NSTimer timerWithTimeInterval:kRefreshRate
                                          target:self
                                        selector:@selector(redrawTimerTick:)
                                        userInfo:nil
                                         repeats:YES];

    [[NSRunLoop currentRunLoop] addTimer:redrawTimer
                                 forMode:NSDefaultRunLoopMode];
  }
}

- (void)viewDidDisappear:(BOOL)animated {
  [super viewDidDisappear:animated];
  if (redrawTimer) {
    [redrawTimer invalidate];
    redrawTimer = nil;
  }
}

/// AGH FIXME TODO
- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
  //NSLog(@"viewWillTransitionToSize");
  [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
  imageView.layer.contents = (id)[[self generateStaticImage] CGImage];
  [self updateDynamicPaths:[NSSet set]];
}

- (UIImage *)generateStaticImage {
  CGFloat scale = [[UIScreen mainScreen] scale];

  UIGraphicsBeginImageContextWithOptions(imageView.frame.size, NO, scale);
  CGContextClearRect(UIGraphicsGetCurrentContext(), imageView.frame);

  CGContextSetLineCap(UIGraphicsGetCurrentContext(), kCGLineCapRound);

  CGContextSetLineWidth(UIGraphicsGetCurrentContext(), 1.0f);

  CGSize viewSize = imageView.frame.size;
  CGFloat topGutterHeight = 100.0f;
  CGFloat rightGutterWidth = 100.0f;
  unsigned int numSourceSections = [self.midiPatchbayDelegate numSourceSections];
  unsigned int numDestinationSections =
      [self.midiPatchbayDelegate numDestinationSections];

  [self inContextDrawBackgroundWithViewSize:viewSize
                           rightGutterWidth:rightGutterWidth
                            topGutterHeight:topGutterHeight
                          numSourceSections:numSourceSections
                     numDestinationSections:numDestinationSections];

  staticImage = UIGraphicsGetImageFromCurrentImageContext();

  UIGraphicsEndImageContext();

  return staticImage;
}

- (void)updateDynamicPaths:(NSSet *)touches {
  [self inContextDrawPatches];

  [self inPathDrawProposedPatches:touches ofSize:imageView.frame.size];
}

- (CGFloat)horizontalHeaderHeight {
  return horizontalHeaderHeight;
}

- (CGFloat)horizontalHeaderWidth:(CGSize)viewSize {
  return viewSize.width - [self verticalHeaderWidth] - [self gutterSize];
}

- (CGFloat)verticalHeaderWidth {
  return verticalHeaderWidth;
}

- (CGFloat)verticalHeaderHeight:(CGSize)viewSize {
  return viewSize.height - [self horizontalHeaderHeight] - [self gutterSize];
}

- (CGFloat)gridWidth:(CGSize)viewSize {
  return [self horizontalHeaderWidth:viewSize];
}

- (CGFloat)gridHeight:(CGSize)viewSize {
  return [self verticalHeaderHeight:viewSize];
}

- (CGFloat)gutterSize {
  return gutterSize;
}

- (CGFloat)arrowSize {
  return arrowSize;
}

- (CGFloat)horizontalChannelNumOffset:(CGSize)viewSize {
  return [self horizontalArrowOffset:viewSize];
}

- (CGFloat)horizontalArrowOffset:(CGSize)viewSize {
  return [self gridWidth:viewSize] + [self arrowSize] + arrowXOffset;
}

- (CGFloat)verticalChannelNumOffset {
  return [self verticalArrowOffset] - [self arrowSize] - textHeight;
}

- (CGFloat)verticalArrowOffset {
  return [self horizontalHeaderHeight] + [self gutterSize] - arrowYOffset -
         arrowHeight;
}

- (CGRect)horizontalHeaderRect:(CGPoint)offset size:(CGSize)viewSize {
  return CGRectMake(0, 0, [self horizontalHeaderWidth:viewSize],
                    [self horizontalHeaderHeight]);
}

- (CGRect)verticalHeaderRect:(CGPoint)offset size:(CGSize)viewSize {
  return CGRectMake([self horizontalHeaderWidth:viewSize] + [self gutterSize],
                    [self horizontalHeaderHeight] + [self gutterSize],
                    [self verticalHeaderWidth],
                    [self verticalHeaderHeight:viewSize]);
}

- (CGRect)horizontalGutterRect:(CGPoint)offset size:(CGSize)viewSize {
  return CGRectMake(0, [self horizontalHeaderHeight],
                    [self horizontalHeaderWidth:viewSize], [self gutterSize]);
}

- (CGRect)verticalGutterRect:(CGPoint)offset size:(CGSize)viewSize {
  return CGRectMake([self horizontalHeaderWidth:viewSize],
                    [self horizontalHeaderHeight] + [self gutterSize],
                    [self gutterSize], [self verticalHeaderHeight:viewSize]);
}

- (CGRect)topRightRect:(CGPoint)offset size:(CGSize)viewSize {
  return CGRectMake([self horizontalHeaderWidth:viewSize], 0,
                    [self verticalHeaderWidth] + [self gutterSize],
                    [self horizontalHeaderHeight] + [self gutterSize]);
}

- (CGRect)gridRect:(CGPoint)offset size:(CGSize)viewSize {
  return CGRectMake(0, [self horizontalHeaderHeight] + [self gutterSize],
                    [self gridWidth:viewSize], [self gridHeight:viewSize]);
}

- (void)inContextDrawLine:(CGPoint)a to:(CGPoint)b {
  CGContextMoveToPoint(UIGraphicsGetCurrentContext(), a.x, a.y);
  CGContextAddLineToPoint(UIGraphicsGetCurrentContext(), b.x, b.y);
}

- (void)inContextDrawTriangle:(CGPoint)a b:(CGPoint)b c:(CGPoint)c {
  CGContextMoveToPoint(UIGraphicsGetCurrentContext(), a.x, a.y);
  CGContextAddLineToPoint(UIGraphicsGetCurrentContext(), b.x, b.y);
  CGContextAddLineToPoint(UIGraphicsGetCurrentContext(), c.x, c.y);
  CGContextAddLineToPoint(UIGraphicsGetCurrentContext(), a.x, a.y);
  CGContextFillPath(UIGraphicsGetCurrentContext());
}

- (void)inContextDrawDownTriangle:(CGPoint)point {
  CGPoint mid_p, top_l, top_r;
  mid_p = point;
  top_l.x = mid_p.x + arrowWidth;
  top_l.y = mid_p.y - arrowHeight;

  top_r.x = mid_p.x - arrowWidth;
  top_r.y = mid_p.y - arrowHeight;

  [self inContextDrawTriangle:mid_p b:top_l c:top_r];
}

- (void)inContextDrawRightTriangle:(CGPoint)point {
  CGPoint mid_p, top_l, top_r;
  mid_p.x = point.x + arrowHeight;
  mid_p.y = point.y;

  top_l.x = mid_p.x - arrowHeight;
  top_l.y = mid_p.y + arrowWidth;

  top_r.x = mid_p.x - arrowHeight;
  top_r.y = mid_p.y - arrowWidth;
  [self inContextDrawTriangle:mid_p b:top_l c:top_r];
}

- (void)inPathDrawTriangle:(CGMutablePathRef *)path
                         a:(CGPoint)a
                         b:(CGPoint)b
                         c:(CGPoint)c {
  CGPathMoveToPoint(*path, NULL, a.x, a.y);
  CGPathAddLineToPoint(*path, NULL, b.x, b.y);
  CGPathAddLineToPoint(*path, NULL, c.x, c.y);
  CGPathAddLineToPoint(*path, NULL, a.x, a.y);
}

- (void)inPathDrawDownTriangle:(CGMutablePathRef *)path point:(CGPoint)point {
  CGPoint mid_p, top_l, top_r;
  mid_p = point;
  top_l.x = mid_p.x + arrowWidth;
  top_l.y = mid_p.y - arrowHeight;

  top_r.x = mid_p.x - arrowWidth;
  top_r.y = mid_p.y - arrowHeight;

  [self inPathDrawTriangle:path a:mid_p b:top_l c:top_r];
}

- (void)inPathDrawRightTriangle:(CGMutablePathRef *)path point:(CGPoint)point {
  CGPoint mid_p, top_l, top_r;
  mid_p.x = point.x + arrowHeight;
  mid_p.y = point.y;

  top_l.x = mid_p.x - arrowHeight;
  top_l.y = mid_p.y + arrowWidth;

  top_r.x = mid_p.x - arrowHeight;
  top_r.y = mid_p.y - arrowWidth;
  [self inPathDrawTriangle:path a:mid_p b:top_l c:top_r];
}

- (BOOL)canBecomeFirstResponder {
  return YES;
}

- (void) checkButtons:(NSSet *)touches {
  UITouch *oneTouch;
  for (UITouch *touch in touches) {
    oneTouch = touch;
  }
  if (popupButtonRows) {
    for (NSMutableArray *row : popupButtonRows) {
      for (UIButton *but : row) {
        if (but != lastButtonToggled) {
          CGPoint location = [oneTouch locationInView:but];
          CGRect butRect = CGRectMake(0, 0, but.frame.size.width, but.frame.size.height);
          if (CGRectContainsPoint(butRect, location)) {
            [self buttonToggled:but];
            lastButtonToggled = but;
          }
        }
      }
    }
  }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
  [super touchesBegan:touches withEvent:event];
  if (!popupShowing)
    currentProposed = [NSSet setWithObject:[touches anyObject]];
  else {
    [self checkButtons:touches];
  }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
  [super touchesCancelled:touches withEvent:event];
  currentProposed = [NSSet set];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
  [super touchesMoved:touches withEvent:event];
  if (!popupShowing)
    currentProposed = [NSSet setWithObject:[touches anyObject]];
  else {
    [self checkButtons:touches];
  }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
  [super touchesEnded:touches withEvent:event];
  lastButtonToggled = nil;
  if (!popupShowing) {
    currentProposed = [NSSet set];

    CGFloat totalSources = (CGFloat)[self.midiPatchbayDelegate totalSources];
    CGFloat pinDX = [self gridWidth:imageView.frame.size] / totalSources;

    CGFloat totalDestinations =
    (CGFloat)[self.midiPatchbayDelegate totalDestinations];
    CGFloat pinDY = [self gridHeight:imageView.frame.size] / totalDestinations;

    // loop through all touchs
    for (UITouch *touch in touches) {
      CGPoint location = [touch locationInView:imageView];
      CGRect grid = [self gridRect:CGPointMake(0, 0) size:imageView.frame.size];

      CGRect horizontalHeaderRect = [self horizontalHeaderRect:CGPointMake(0,0) size:imageView.frame.size];
      CGRect verticalHeaderRect = [self verticalHeaderRect:CGPointMake(0,0) size:imageView.frame.size];

      // filter out all non begin and move phases
      if (CGRectContainsPoint(grid, location)) {

        // modify the location to inside the grid
        location.x -= grid.origin.x;
        location.y -= grid.origin.y;

        // determine the corresponding row and column
        int col = (int)floor(location.x / pinDX) + 1;
        int row = (int)floor(location.y / pinDY) + 1;

        auto srcIndex = [self.midiPatchbayDelegate sourceTotalToIndex:col];
        auto dstIndex = [self.midiPatchbayDelegate destinationTotalToIndex:row];

        // check the bounds
        if (([self.midiPatchbayDelegate isSourceDeviceInBounds:srcIndex]) &&
            ([self.midiPatchbayDelegate isDestinationDeviceInBounds:dstIndex])) {
          midi_port_t empty = {0, 0};
          if ([self.midiPatchbayDelegate isPatchedFrom:srcIndex to:dstIndex]) {
            [self.midiPatchbayDelegate setPatchedFrom:empty to:dstIndex andRemove:srcIndex];
          } else {
            [self.midiPatchbayDelegate setPatchedFrom:srcIndex to:dstIndex andRemove:empty];
          }
        }
      }
      else if (CGRectContainsPoint(horizontalHeaderRect, location)) {
        // modify the location to inside the grid
        location.x -= horizontalHeaderRect.origin.x;
        location.y -= horizontalHeaderRect.origin.y;

        // determine the corresponding row and column
        int col = (int)floor(location.x / pinDX) + 1;

        auto srcIndex = [self.midiPatchbayDelegate sourceTotalToIndex:col];
        [self.midiPatchbayDelegate toggleCollapseInput:srcIndex.devicePort];
        imageView.layer.contents = (id)[[self generateStaticImage] CGImage];
        [self updateDynamicPaths:[NSSet set]];

      }
      else if (CGRectContainsPoint(verticalHeaderRect, location)) {
        // modify the location to inside the grid
        location.x -= verticalHeaderRect.origin.x;
        location.y -= verticalHeaderRect.origin.y;

        // determine the corresponding row and column
        int row = (int)floor(location.y / pinDY) + 1;

        auto dstIndex = [self.midiPatchbayDelegate destinationTotalToIndex:row];
        [self.midiPatchbayDelegate toggleCollapseOutput:dstIndex.devicePort];
        imageView.layer.contents = (id)[[self generateStaticImage] CGImage];
        [self updateDynamicPaths:[NSSet set]];
        
      }
    }
  }
  else {
    for (UITouch *touch in touches) {
      if (!firstTouchUpSinceLPGR) {
        CGPoint location = [touch locationInView:popupView];
        CGRect popupRect = CGRectMake(0, 0, popupView.frame.size.width, popupView.frame.size.height);
        if (!CGRectContainsPoint(popupRect, location)) {
          popupShowing = NO;
          lpgr.enabled = YES;

          grayView.hidden = YES;
          popupView.hidden = YES;
        }
      }
      else {
        firstTouchUpSinceLPGR = NO;
      }
    }
  }
}

- (void)handleLongPressGestures:(UILongPressGestureRecognizer *)sender
{
  if ([sender isEqual:lpgr]) {
    if (sender.state == UIGestureRecognizerStateBegan)
    {
      CGFloat totalSources = (CGFloat)[self.midiPatchbayDelegate totalSources];
      CGFloat pinDX = [self gridWidth:imageView.frame.size] / totalSources;

      CGFloat totalDestinations =
      (CGFloat)[self.midiPatchbayDelegate totalDestinations];
      CGFloat pinDY = [self gridHeight:imageView.frame.size] / totalDestinations;

      CGPoint location = [sender locationInView:imageView];

      CGRect horizontalHeaderRect = [self horizontalHeaderRect:CGPointMake(0,0) size:imageView.frame.size];
      CGRect verticalHeaderRect = [self verticalHeaderRect:CGPointMake(0,0) size:imageView.frame.size];

      bool inside = false;

      if (CGRectContainsPoint(horizontalHeaderRect, location)) {
        // modify the location to inside the grid
        location.x -= horizontalHeaderRect.origin.x;
        location.y -= horizontalHeaderRect.origin.y;

        // determine the corresponding row and column
        int col = (int)floor(location.x / pinDX) + 1;

        auto srcIndex = [self.midiPatchbayDelegate sourceTotalToIndex:col];

        popupDevicePort = srcIndex.devicePort;
        popupPort = 1;
        popupPut = Input;

        lpgr.cancelsTouchesInView = YES;
        inside = true;
      }
      else if (CGRectContainsPoint(verticalHeaderRect, location)) {
        // modify the location to inside the grid
        location.x -= verticalHeaderRect.origin.x;
        location.y -= verticalHeaderRect.origin.y;

        // determine the corresponding row and column
        int row = (int)floor(location.y / pinDY) + 1;

        auto dstIndex = [self.midiPatchbayDelegate destinationTotalToIndex:row];

        popupDevicePort = dstIndex.devicePort;
        popupPort = 1;

        popupPut = Output;

        lpgr.cancelsTouchesInView = YES;
        inside = true;
      }
      else {
        lpgr.cancelsTouchesInView = NO;
      }

      if (inside) {
        popupShowing = YES;
        lpgr.enabled = NO;

        firstTouchUpSinceLPGR = YES;

        grayView.hidden = NO;
        popupView.hidden = NO;
        [self redrawPopup];
      }
    }
  }
}

- (void)redrawTimerTick:(id)sender {
  [self updateDynamicPaths:currentProposed];
}

@end
