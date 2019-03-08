/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "DeviceInfo.h"
#import "PureLayout.h"
#import "ICMixerMixerGroup.h"
#import "ICMixerViewController.h"


@implementation ICMixerMixerGroup

- (id)initWithCommunicator:(CommPtr)m_comm device:(DeviceInfoPtr)m_device mvc:(ICMixerViewController*)mvc {
  self = [super init];
  if (self) {
    self->device = m_device;
    self->comm = m_comm;
    self->audioPortID = 3;
    self->outputNumber = 1;

    self->mixerInterface = new MixerInterface(device);
    self->mixerInputInterface = new MixerInputInterface(device);
    self->mixerOutputInterface = new MixerOutputInterface(device);

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

- (void) setMixerAudioPort:(Word)audioPort outputNumber:(Byte)output {
  audioPortID = audioPort;
  outputNumber = output;

  //NSLog(@"Setting mixerGroup: %d %d", audioPortID, outputNumber);
}

- (void) redraw {
  for (UIView* view in [self->topView subviews]) {
    [view removeFromSuperview];
  }
  for (UIView* view in [self->bottomView subviews]) {
    [view removeFromSuperview];
  }
  for (UIView* view in [self subviews]) {
    [view removeFromSuperview];
  }

  [self->mixerStrips removeAllObjects];
  [self->stereoLinkButtons removeAllObjects];

  [self postReadyInit];
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
  //NSLog(@"here in mixer's setupMixerStrips");
  ICMixerStrip* lastMixerStrip = nil;
  for (int i = 1; i <= mixerInterface->numberInputs(audioPortID); ++i) {
    //NSLog(@"in for loop");
    ICMixerStrip* mixerStrip;
    mixerStrip = [[ICMixerStrip alloc] initWithCommunicator:comm device:device isAnalog:false audioPortID:audioPortID inputNumber:i outputNumber:outputNumber];
    //NSLog(@"post mixerstrip init in for loop");
    mixerStrip.translatesAutoresizingMaskIntoConstraints = NO;
    [mixerStrip setUserInteractionEnabled:YES];
    [self addSubview:mixerStrip];
    //NSLog(@"added a mixer mixerStrip");

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
  // make output strip 2x as wide
  ICMixerStrip* mixerStrip = [[ICMixerStrip alloc] initWithCommunicator:comm device:device isAnalog:false audioPortID:audioPortID inputNumber:0 outputNumber:outputNumber];
  mixerStrip.translatesAutoresizingMaskIntoConstraints = NO;
  [mixerStrip setUserInteractionEnabled:YES];
  [self addSubview:mixerStrip];

  NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:[mixerStrip(128)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(mixerStrip)];
  [self addConstraints:constraint_POS_H];

  NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[mixerStrip]-|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(mixerStrip)];
  [self addConstraints:constraint_POS_V];

  [mixerStrips addObject:mixerStrip];

  [mixerStrips autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeTop withFixedSpacing:2 insetSpacing:YES matchedSizes:NO];
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

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

@end
