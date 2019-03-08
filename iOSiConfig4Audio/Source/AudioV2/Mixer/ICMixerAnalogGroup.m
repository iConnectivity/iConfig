/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "DeviceInfo.h"
#import "AudioPortParm.h"
#import "ICMixerAnalogGroup.h"
#import "PureLayout.h"
#import "ICMixerViewController.h"


using namespace GeneSysLib;

@implementation ICMixerAnalogGroup

- (id)initWithCommunicator:(CommPtr)m_comm device:(DeviceInfoPtr)m_device isInput:(bool)m_isInput mvc:(ICMixerViewController*)mvc {
  self = [super init];
  if (self) {
    self->device = m_device;
    self->comm = m_comm;
    self->isInput = m_isInput;
    self->numDisplayedPorts = 0;
    self.translatesAutoresizingMaskIntoConstraints = NO;
    [self setUserInteractionEnabled:YES];

    self->mixerStrips = [[NSMutableArray alloc] init];
    self->stereoLinkButtons = [[NSMutableArray alloc] init];

    self->topView = [UIView new];
    self->topView.translatesAutoresizingMaskIntoConstraints = NO;
    self->bottomView = [UIView new];
    self->bottomView.translatesAutoresizingMaskIntoConstraints = NO;

    self->topMVC = mvc;
  }
  return self;
}

- (int) postReadyInit {
  [self addSubview:topView];
  [self addSubview:bottomView];

  NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[topView]|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(topView)];
  NSArray *constraint_POS_H2 = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[bottomView]|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(bottomView)];
  NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[topView][bottomView(30)]|"
                                                                       options:0
                                                                       metrics:nil
                                                                         views:NSDictionaryOfVariableBindings(topView, bottomView)];
  [self addConstraints:constraint_POS_H];
  [self addConstraints:constraint_POS_H2];
  [self addConstraints:constraint_POS_V];

  [self setupMixerStrips];
  [self setupStereoLinkButtons];

  return self->numDisplayedPorts;
}

- (void) updateMeters {
  for (ICMixerStrip *mixerStrip in mixerStrips) {
    [mixerStrip updateMeters];
  }
}

- (void) setupMixerStrips {
  const AudioPortParm &audioPortParm = device->get<AudioPortParm>(3);
  int numPorts;
  if (isInput) {
    numPorts = audioPortParm.numOutputChannels();
  }
  else {
    numPorts = audioPortParm.numInputChannels();
  }

  ICMixerStrip* lastMixerStrip = nil;
  for (int i = 1; i <= numPorts; i++) {
    ICMixerStrip* mixerStrip;
    if (isInput)
      mixerStrip = [[ICMixerStrip alloc] initWithCommunicator:comm device:device isAnalog:true audioPortID:3 inputNumber:i outputNumber:0];
    else {
      mixerStrip = [[ICMixerStrip alloc] initWithCommunicator:comm device:device isAnalog:true audioPortID:3 inputNumber:0 outputNumber:i];
    }
    mixerStrip.translatesAutoresizingMaskIntoConstraints = NO;
    [mixerStrip setUserInteractionEnabled:YES];
    [topView addSubview:mixerStrip];

    NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[mixerStrip]-|"
                                                                        options:0
                                                                        metrics:nil
                                                                          views:NSDictionaryOfVariableBindings(mixerStrip)];
    [self addConstraints:constraint_POS_V];

    [mixerStrips addObject:mixerStrip];

    if ([mixerStrip isStereoLinked]) {
      i++;
    }
    if (lastMixerStrip != nil) {
      [mixerStrip autoMatchDimension:ALDimensionWidth toDimension:ALDimensionWidth ofView:lastMixerStrip];
    }
    lastMixerStrip = mixerStrip;
    self->numDisplayedPorts++;
  }
  [mixerStrips autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeTop withFixedSpacing:2 insetSpacing:YES matchedSizes:YES];
}

- (void) setupStereoLinkButtons {
  for (int i = 0; i < [mixerStrips count]; i++) {
    ICMixerStrip* mixerStrip = [mixerStrips objectAtIndex:i];
    UIButton *stereoLinkButton = [UIButton new];
    stereoLinkButton.translatesAutoresizingMaskIntoConstraints = NO;
    [stereoLinkButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    stereoLinkButton.tag = i;
    if ([mixerStrip isStereoLinkable]) {
      [stereoLinkButton addTarget:self action:@selector(stereoLinkTouched:) forControlEvents:UIControlEventTouchUpInside];
      if (![mixerStrip isStereoLinked]) {
        [stereoLinkButton setBackgroundImage:[UIImage imageNamed:@"btn_link_off.png"] forState:UIControlStateNormal];
      }
      else {
        [stereoLinkButton setBackgroundImage:[UIImage imageNamed:@"btn_link_on.png"] forState:UIControlStateNormal];
      }
    }

    [bottomView addSubview:stereoLinkButton];
    if (![mixerStrip isStereoLinked]) {
      i++;
      [self addConstraint:[NSLayoutConstraint constraintWithItem:stereoLinkButton
                                                       attribute:NSLayoutAttributeCenterX
                                                       relatedBy:NSLayoutRelationEqual
                                                          toItem:mixerStrip
                                                       attribute:NSLayoutAttributeRight
                                                      multiplier:1.0
                                                        constant:1.0]];
    }
    else {
      [self addConstraint:[NSLayoutConstraint constraintWithItem:stereoLinkButton
                                                       attribute:NSLayoutAttributeCenterX
                                                       relatedBy:NSLayoutRelationEqual
                                                          toItem:mixerStrip
                                                       attribute:NSLayoutAttributeCenterX
                                                      multiplier:1.0
                                                        constant:0.0]];
    }
    NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[stereoLinkButton(19)]"
                                                                        options:0
                                                                        metrics:nil
                                                                          views:NSDictionaryOfVariableBindings(stereoLinkButton)];
    [bottomView addConstraints:constraint_POS_V];
    [stereoLinkButton autoPinEdgeToSuperviewEdge:ALEdgeBottom];
    [stereoLinkButtons addObject:stereoLinkButton];
  }
//  [stereoLinkButtons autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeBottom withFixedSpacing:2];
}

- (void) stereoLinkTouched:(UIButton*)but {
  [[mixerStrips objectAtIndex:but.tag] toggleStereoLink];
  [self->topMVC resetView];
  NSLog(@"View Resetting!");
}

@end


