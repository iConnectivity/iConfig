/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICMixerStrip.h"
#import "PureLayout.h"
#import "AudioGlobalParm.h"
#import "AudioPortParm.h"
#import <CoreGraphics/CoreGraphics.h>
#import "DevicePID.h"

//64bit conversion. zx, 2017-07-28
#define UIColorFromRGB(rgbValue) [UIColor colorWithRed:(CGFloat)(((CGFloat)((rgbValue & 0xFF0000) >> 16))/255.0) green:(CGFloat)(((CGFloat)((rgbValue & 0xFF00) >> 8))/255.0) blue:(CGFloat)(((CGFloat)(rgbValue & 0xFF))/255.0) alpha:1.0]

using namespace GeneSysLib;

@implementation ICMixerStrip 

- (id)initWithCommunicator:(CommPtr)m_comm device:(DeviceInfoPtr)m_device isAnalog:(bool)m_isAnalog audioPortID:(Word)m_audioPortID inputNumber:(Byte)m_inputNumber outputNumber:(Byte)m_outputNumber {
  self = [super init];
  if (self) {
    self->device = m_device;
    self->comm = m_comm;

    self->changingVolume = NO;
    self->changingPan = NO;
    self->changingSolo = NO;

    mixerInterface = new MixerInterface(device);
    mixerInputInterface = new MixerInputInterface(device);
    mixerOutputInterface = new MixerOutputInterface(device);

    self->inputNumber = m_inputNumber;
    self->outputNumber = m_outputNumber;
    self->audioPortID = m_audioPortID;

    self.translatesAutoresizingMaskIntoConstraints = NO;
    [self removeConstraints:self.constraints];

    if (self->inputNumber) {
      self->mixerType = in;
    }
    else {
      self->mixerType = out;
    }

    if (m_isAnalog) {
      self->isAudioStrip = true;
      if (self->mixerType == in) {
        audioController = 1;
      }
      else {
        if (outputNumber <= 4)
          audioController = 2;
        else {
          audioController = 3;
          outputNumber -= 4;
        }
      }
    }
    else {
      self->isAudioStrip = false;
      self->outputStereoLinked = true;
    }

    if (isAudioStrip) {
      analogInterface = new ICAnalogInterface(self->device, self->audioPortID, audioController);
    }
    if ((self->mixerType == out) && (!self->isAudioStrip)){
      self->stereoLinked = true;
    }
    else if ((self->mixerType == in) && (!self->isAudioStrip)){
      //NSLog(@"1");
      self->stereoLinked = mixerInputInterface->stereoLink(self->audioPortID, self->outputNumber, self->inputNumber);
      //NSLog(@"2");
    }
    else {
      if (self->mixerType == in)
        self->stereoLinked = analogInterface->stereoLink(inputNumber);
      else
        self->stereoLinked = analogInterface->stereoLink(outputNumber);
    }

    [self setUserInteractionEnabled:YES];
    [self postReadyInit];
  }
  return self;
}

- (bool) isStereoLinked {
  return self->stereoLinked;
}

- (bool) isStereoLinkable {
  if ((self->mixerType == out) && (!self->isAudioStrip)){
    return false;
  }
  return true;
}

- (void) toggleStereoLink {
  if ((self->mixerType == in) && (!self->isAudioStrip)){
    mixerInputInterface->stereoLink(self->audioPortID, self->outputNumber, self->inputNumber, !self->stereoLinked);
  }
  else {
    if (self->mixerType == in)
      analogInterface->stereoLink(inputNumber, !self->stereoLinked);
    else
      analogInterface->stereoLink(outputNumber, !self->stereoLinked);
  }
}

-(void) postReadyInit {
  [self buildAll];
  [self alignAll];
  [self updateAll];
}

-(void) alignAll {
  UIView *bottomView = [UIView new];
  [self addSubview:bottomView];

  NSArray *constraint_POS_VBV = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[bottomView(>=10)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(bottomView)];
  [self addConstraints:constraint_POS_VBV];

  if (self->isAudioStrip) {
    if (self->mixerType == in) {
      NSArray *viewsToAlign = [NSArray arrayWithObjects: channelNameButton, panSoloContainer, panSoloLabelContainer, totalVolumeSliderContainer, volumeLabel, muteButton, phantomPowerButton, highImpedenceButton, bottomView, nil];
      [viewsToAlign autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeVertical withFixedSpacing:4 insetSpacing:YES matchedSizes:NO];
    }
    else {
      NSArray *viewsToAlign = [NSArray arrayWithObjects: channelNameButton, panSoloContainer, panSoloLabelContainer, totalVolumeSliderContainer, volumeLabel, muteButton, bottomView, nil];
      [viewsToAlign autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeVertical withFixedSpacing:4 insetSpacing:YES matchedSizes:NO];
    }
  }
  else {
    if (self->mixerType == in) {
      NSArray *viewsToAlign = [NSArray arrayWithObjects: channelNameButton, panSoloContainer, panSoloLabelContainer, totalVolumeSliderContainer, volumeLabel, muteButton, soloButton, invertButtonContainer, soloPFLButton, bottomView, nil];
      [viewsToAlign autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeVertical withFixedSpacing:4 insetSpacing:YES matchedSizes:NO];
    }
    else {
      NSArray *viewsToAlign = [NSArray arrayWithObjects: channelNameButton, panSoloContainer, panSoloLabelContainer, totalVolumeSliderContainer, volumeLabel, muteButton, invertButtonContainer, soloPFLButton, bottomView, nil];
      [viewsToAlign autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeVertical withFixedSpacing:4 insetSpacing:YES matchedSizes:NO];
    }
  }
}

-(void) buildAll {
  [self buildChannelName];
  [self buildPanDials];
  [self buildPanLabel];
  [self buildVolumeSlider];
  [self buildVolumeLabel];
  [self buildMuteButton];
  if (self->isAudioStrip) {
    if (self->mixerType == in) {
      [self buildPhantomPowerButton];
      [self buildHighImpedenceButton];
    }
  }
  else {
    if (self->mixerType == in) {
      [self buildSoloButton];
    }
    [self buildInvertButtons];
    [self buildSoloPFLButton];
  }
}

-(void) updateAll {
  if (stereoLinked) {
    [self updatePanDials];
    [self updatePanLabels];
  }
  [self updateVolumeSlider];
  [self updateMeters];
  [self updateMuteButton];
  if (self->isAudioStrip) {
    if (self->mixerType == in) {
      [self updatePhantomPowerButton];
      [self updateHighImpedenceButton];
    }
  }
  else {
    if (self->mixerType == in) {
      [self updateSoloButton];
    }
    [self updateInvertButtons];
    [self updateSoloPFLButton];
  }
}

-(void) buildChannelName {
  channelNameButton = [[UIButton alloc] init];
  channelNameButton.translatesAutoresizingMaskIntoConstraints = NO;
  [channelNameButton setTitle:@"Channel Name" forState:UIControlStateNormal];
  [channelNameButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
  [channelNameButton.titleLabel setFont:[UIFont boldSystemFontOfSize:8]];
  channelNameButton.selected = NO;

  [self addSubview:channelNameButton];

  NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-2-[channelNameButton]-2-|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(channelNameButton)];
  NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[channelNameButton(25)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(channelNameButton)];
  [self addConstraints:constraint_POS_H];
  [self addConstraints:constraint_POS_V];

  if (isAudioStrip) {
    if (stereoLinked) {
      if (mixerType == in) {
        [channelNameButton setTitle:[NSString stringWithFormat:@"%@ %@ & %@",@"Analog", [[NSNumber numberWithInt:inputNumber] stringValue], [[NSNumber numberWithInt:inputNumber + 1] stringValue]] forState:UIControlStateNormal];
      }
      else {
        if (audioController == 3) { // headphones
          [channelNameButton setTitle:@"Headphones" forState:UIControlStateNormal];
        }
        else {
          [channelNameButton setTitle:[NSString stringWithFormat:@"%@ %@ & %@",@"Analog", [[NSNumber numberWithInt:outputNumber] stringValue], [[NSNumber numberWithInt:outputNumber + 1] stringValue]] forState:UIControlStateNormal];
        }
      }
    }
    else {
      if (mixerType == in) {
        [channelNameButton setTitle:[NSString stringWithFormat:@"%@ %@",@"Analog", [[NSNumber numberWithInt:inputNumber] stringValue]] forState:UIControlStateNormal];
      }
      else {
        if (audioController == 3) { // headphones
          if (outputNumber == 1) {
            [channelNameButton setTitle:@"Headphones L" forState:UIControlStateNormal];
          }
          else {
            [channelNameButton setTitle:@"Headphones R" forState:UIControlStateNormal];
          }
        }
        else {
          [channelNameButton setTitle:[NSString stringWithFormat:@"%@ %@",@"Analog", [[NSNumber numberWithInt:outputNumber] stringValue]] forState:UIControlStateNormal];
        }
      }
    }
  }
  else {
    if (mixerType == in) {
      NSMutableArray *allAvailableInChannels = [[NSMutableArray alloc] init];
      NSMutableArray *portNames = [[NSMutableArray alloc] init];

      const auto &audioGlobalParm = device->get<AudioGlobalParm>();

      for (Word apid = 1; apid <= audioGlobalParm.numAudioPorts(); ++apid) {
        const auto &portParm = device->get<AudioPortParm>(apid);
        // TODO: Should be real names.
        switch(apid) {
          case 1:
            [portNames addObject:(@"U1")];
            break;
          case 2:
            [portNames addObject:(@"U2")];
            break;
          case 3:
            [portNames addObject:(@"A")];
            break;
          default:
            [portNames addObject:(@"")];
            break;
        }

        for (Byte inChannelID = 1; inChannelID <= portParm.numOutputChannels(); ++ inChannelID) {
          [allAvailableInChannels addObject:[NSArray arrayWithObjects:[NSNumber numberWithInt:apid], [NSNumber numberWithInt:inChannelID], nil]];
        }
      }

      bool isAssigned = false;
      bool isAssigned2 = false;
      NSString *title = @"";
      NSString *title2 = @"";
      for (int i=0; i< [allAvailableInChannels count]; i++) {
        if (mixerInterface->audioPortIDForInput(audioPortID, inputNumber) ==
            [[[allAvailableInChannels objectAtIndex:i] objectAtIndex:0] intValue] &&
            mixerInterface->channelIDForInput(audioPortID, inputNumber) ==
            [[[allAvailableInChannels objectAtIndex:i] objectAtIndex:1] intValue]) {
          title = [NSString stringWithFormat:@"%@%@%@",
                   [portNames objectAtIndex:([[[allAvailableInChannels objectAtIndex:i] objectAtIndex:0] intValue] - 1)],
                                                        @":",
                                                        [[[allAvailableInChannels objectAtIndex:i] objectAtIndex:1] stringValue]];
          isAssigned = true;
        }
        if (stereoLinked) {
          if (mixerInterface->audioPortIDForInput(audioPortID, inputNumber + 1) ==
              [[[allAvailableInChannels objectAtIndex:i] objectAtIndex:0] intValue] &&
              mixerInterface->channelIDForInput(audioPortID, inputNumber + 1) ==
              [[[allAvailableInChannels objectAtIndex:i] objectAtIndex:1] intValue]) {
            title2 = [NSString stringWithFormat:@" | %@%@%@",
                     [portNames objectAtIndex:([[[allAvailableInChannels objectAtIndex:i] objectAtIndex:0] intValue] - 1)],
                     @":",
                     [[[allAvailableInChannels objectAtIndex:i] objectAtIndex:1] stringValue]];
            isAssigned2 = true;
          }
        }
      }
      if (!isAssigned) {
        title = @"No SRC";
      }
      if (!isAssigned2) {
        title2 = @" | No SRC";
      }
      [channelNameButton setTitle:[NSString stringWithFormat:@"%@%@",title,title2] forState:UIControlStateNormal];
    }
    else {
      NSMutableArray *allAvailableOutChannels = [[NSMutableArray alloc] init];
      NSMutableArray *portNames = [[NSMutableArray alloc] init];

      const auto &audioGlobalParm = device->get<AudioGlobalParm>();

      for (Word apid = 1; apid <= audioGlobalParm.numAudioPorts(); ++apid) {
        const auto &portParm = device->get<AudioPortParm>(apid);
        // TODO: Should be real names.
        switch(apid) {
          case 1:
            [portNames addObject:(@"U1")];
            break;
          case 2:
            [portNames addObject:(@"U2")];
            break;
          case 3:
            [portNames addObject:(@"A")];
            break;
          default:
            [portNames addObject:(@"")];
            break;
        }

        for (Byte outChannelID = 1; outChannelID <= portParm.numInputChannels(); ++ outChannelID) {
          [allAvailableOutChannels addObject:[NSArray arrayWithObjects:[NSNumber numberWithInt:apid], [NSNumber numberWithInt:outChannelID], nil]];
        }
      }

      bool isAssigned = false;
      bool isAssigned2 = false;
      NSString *title = @"";
      NSString *title2 = @"";
      for (int i=0; i< [allAvailableOutChannels count]; i++) {
        if (mixerInterface->audioPortIDForInput(audioPortID, outputNumber) ==
            [[[allAvailableOutChannels objectAtIndex:i] objectAtIndex:0] intValue] &&
            mixerInterface->channelIDForInput(audioPortID, outputNumber) ==
            [[[allAvailableOutChannels objectAtIndex:i] objectAtIndex:1] intValue]) {
          title = [NSString stringWithFormat:@"%@%@%@",
                   [portNames objectAtIndex:([[[allAvailableOutChannels objectAtIndex:i] objectAtIndex:0] intValue] - 1)],
                   @":",
                   [[[allAvailableOutChannels objectAtIndex:i] objectAtIndex:1] stringValue]];
          isAssigned = true;
        }
        if (stereoLinked) {
          if (mixerInterface->audioPortIDForInput(audioPortID, outputNumber + 1) ==
              [[[allAvailableOutChannels objectAtIndex:i] objectAtIndex:0] intValue] &&
              mixerInterface->channelIDForInput(audioPortID, outputNumber + 1) ==
              [[[allAvailableOutChannels objectAtIndex:i] objectAtIndex:1] intValue]) {
            title2 = [NSString stringWithFormat:@" | %@%@%@",
                      [portNames objectAtIndex:([[[allAvailableOutChannels objectAtIndex:i] objectAtIndex:0] intValue] - 1)],
                      @":",
                      [[[allAvailableOutChannels objectAtIndex:i] objectAtIndex:1] stringValue]];
            isAssigned2 = true;
          }
        }
      }
      if (!isAssigned) {
        title = @"No SRC";
      }
      if (!isAssigned2) {
        title2 = @" | No SRC";
      }
      [channelNameButton setTitle:[NSString stringWithFormat:@"%@%@",title,title2] forState:UIControlStateNormal];
    }
  }
}

/*- (void) updateChannelName {

}*/

-(void) buildPanDials {
  panSoloContainer = [UIView new];
  panSoloContainer.translatesAutoresizingMaskIntoConstraints = NO;

  if (stereoLinked) {
    panDial = [[IOSKnobControl alloc] initWithFrame:CGRectMake(0,0,60,60) imageNamed:@"blue-knob"];

    panDial.translatesAutoresizingMaskIntoConstraints = NO;

    panDial.mode = IKCModeContinuous;

    panDial.clockwise = YES;
    panDial.enabled = YES;
    panDial.circular = YES;
    panDial.normalized = YES;
    panDial.clipsToBounds = NO;
    panDial.masksImage = YES;
    [panDial setUserInteractionEnabled:YES];
    // panDial.fontName = @"TrebuchetMS-Bold";

    [panDial removeConstraints:panDial.constraints];

    [panDial addTarget:self
                action:@selector(panValueChanged:)
      forControlEvents:UIControlEventValueChanged];

    if ((mixerType == out) && (isAudioStrip == NO)) {
      panDialContainer = [UIView new];
      panDialContainer.translatesAutoresizingMaskIntoConstraints = NO;
      [panDialContainer addSubview:panDial];
      [panDial autoAlignAxisToSuperviewAxis:ALAxisVertical];
      [panDial autoAlignAxisToSuperviewAxis:ALAxisHorizontal];

      NSArray *constraint_POS_HP = [NSLayoutConstraint constraintsWithVisualFormat:@"H:[panDial(60)]"
                                                                           options:0
                                                                           metrics:nil
                                                                             views:NSDictionaryOfVariableBindings(panDial)];
      NSArray *constraint_POS_VP = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[panDial(60)]"
                                                                           options:0
                                                                           metrics:nil
                                                                             views:NSDictionaryOfVariableBindings(panDial)];

      [panDialContainer addConstraints:constraint_POS_HP];
      [panDialContainer addConstraints:constraint_POS_VP];

      NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[panDialContainer(60)]"
                                                                          options:0
                                                                          metrics:nil
                                                                            views:NSDictionaryOfVariableBindings(panDialContainer)];
      [panSoloContainer addSubview:panDialContainer];
      [panDialContainer autoAlignAxisToSuperviewAxis:ALAxisHorizontal];
      [panSoloContainer addConstraints:constraint_POS_V];

      soloDial = [[IOSKnobControl alloc] initWithFrame:CGRectMake(0,0,60,60) imageNamed:@"orange-knob"];

      soloDial.translatesAutoresizingMaskIntoConstraints = NO;

      soloDial.mode = IKCModeContinuous;

      soloDial.clockwise = YES;
      soloDial.enabled = YES;
      soloDial.circular = YES;
      soloDial.normalized = YES;
      soloDial.clipsToBounds = NO;
      soloDial.masksImage = YES;
      [soloDial setUserInteractionEnabled:YES];
      // panDial.fontName = @"TrebuchetMS-Bold";

      [soloDial removeConstraints:panDial.constraints];

      soloDialContainer = [UIView new];
      soloDialContainer.translatesAutoresizingMaskIntoConstraints = NO;
      [soloDialContainer addSubview:soloDial];
      [soloDial autoAlignAxisToSuperviewAxis:ALAxisVertical];
      [soloDial autoAlignAxisToSuperviewAxis:ALAxisHorizontal];


      NSArray *constraint_POS_HPS = [NSLayoutConstraint constraintsWithVisualFormat:@"H:[soloDial(60)]"
                                                                            options:0
                                                                            metrics:nil
                                                                              views:NSDictionaryOfVariableBindings(soloDial)];
      NSArray *constraint_POS_VPS = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[soloDial(60)]"
                                                                            options:0
                                                                            metrics:nil
                                                                              views:NSDictionaryOfVariableBindings(soloDial)];

      [soloDialContainer addConstraints:constraint_POS_HPS];
      [soloDialContainer addConstraints:constraint_POS_VPS];

      NSArray *constraint_POS_VS = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[soloDialContainer(60)]"
                                                                           options:0
                                                                           metrics:nil
                                                                             views:NSDictionaryOfVariableBindings(soloDialContainer)];
      [panSoloContainer addSubview:soloDialContainer];
      [soloDialContainer autoAlignAxisToSuperviewAxis:ALAxisHorizontal];
      [panSoloContainer addConstraints:constraint_POS_VS];

      [[NSArray arrayWithObjects:panDialContainer,soloDialContainer,nil] autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeTop withFixedSpacing:4];

      [self addSubview:panSoloContainer];

      [soloDial addTarget:self
                   action:@selector(soloValueChanged:)
         forControlEvents:UIControlEventValueChanged];
    }
    else {
      [panSoloContainer addSubview:panDial];
      [panDial autoAlignAxisToSuperviewAxis:ALAxisVertical];
      [panDial autoAlignAxisToSuperviewAxis:ALAxisHorizontal];

      NSArray *constraint_POS_HP = [NSLayoutConstraint constraintsWithVisualFormat:@"H:[panDial(60)]"
                                                                           options:0
                                                                           metrics:nil
                                                                             views:NSDictionaryOfVariableBindings(panDial)];
      NSArray *constraint_POS_VP = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[panDial(60)]"
                                                                           options:0
                                                                           metrics:nil
                                                                             views:NSDictionaryOfVariableBindings(panDial)];
      
      [panSoloContainer addConstraints:constraint_POS_HP];
      [panSoloContainer addConstraints:constraint_POS_VP];
      
      [self addSubview:panSoloContainer];
      [panSoloContainer autoAlignAxisToSuperviewAxis:ALAxisVertical];
    }
    //  [panSoloContainer autoAlignAxisToSuperviewAxis:ALAxisVertical];
  }
  else {
    [self addSubview:panSoloContainer];
    [panSoloContainer autoAlignAxisToSuperviewAxis:ALAxisVertical];
  }

  NSArray *constraint_POS_HPS = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-2-[panSoloContainer]-2-|"
                                                                        options:0
                                                                        metrics:nil
                                                                          views:NSDictionaryOfVariableBindings(panSoloContainer)];
  NSArray *constraint_POS_VPS = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[panSoloContainer(60)]"
                                                                       options:0
                                                                       metrics:nil
                                                                         views:NSDictionaryOfVariableBindings(panSoloContainer)];
  [self addConstraints:constraint_POS_HPS];
  [self addConstraints:constraint_POS_VPS];
}

- (void) panValueChanged:(IOSKnobControl*)dial {
  if (!changingPan) {
    changingPan = YES;
    int16_t value, leftVal, rightVal;

    if (isAudioStrip) {
      if (mixerType == in) {
        value = (panDial.position / 3.1415926535 * analogInterface->trimMin(inputNumber) * (1.0 / 256.0));

        if (value > 0) {
          leftVal = 0; rightVal = value;
        }
        else if (value < 0) {
          rightVal = 0;
          leftVal = 0 - value;
        }
        else {
          rightVal = leftVal = 0;
        }

        analogInterface->trimCurrent(inputNumber, (int16_t)(leftVal * 256));
        analogInterface->trimCurrent(inputNumber + 1, (int16_t)(rightVal * 256));
      }
      else {
        value = (panDial.position / 3.1415926535 * analogInterface->trimMin(outputNumber) * (1.0 / 256.0));

        if (value > 0) {
          leftVal = 0;
          rightVal = 0 - value;
        }
        else if (value < 0) {
          rightVal = 0;
          leftVal = value;
        }
        else {
          rightVal = leftVal = 0;
        }

        //NSLog(@"panDial value: %d", value);
        analogInterface->trimCurrent(outputNumber, (int16_t)(leftVal * 256));
        analogInterface->trimCurrent(outputNumber + 1, (int16_t)(rightVal * 256));
      }
    }
    else {
      if (mixerType == in) {
        value = panDial.position / 3.1415926535 * mixerInputInterface->panMax(audioPortID);
        mixerInputInterface->panCurrent(audioPortID, outputNumber, inputNumber, value);
      }
      else {
        value = panDial.position / 3.1415926535 * mixerOutputInterface->panMax(audioPortID);
        mixerOutputInterface->panCurrent(audioPortID, outputNumber, value);
      }
    }
    
    [self updatePanLabels];
    changingPan = NO;
  }
}

- (void) soloValueChanged:(IOSKnobControl*)dial {
  if (!changingSolo) {
    changingSolo = YES;
    int16_t value;
    int soloMax = mixerOutputInterface->volumeMax(audioPortID);
    int soloMin = mixerOutputInterface->volumeMin(audioPortID);
    int soloRes = mixerOutputInterface->volumeResolution(audioPortID);
    value = soloMin + ((3.1415926535 + soloDial.position) / (2 * 3.1415926535)) * (soloMax - soloMin);
    value = round(value * 1.0 / soloRes);
    value = value * soloRes;

    mixerOutputInterface->solo(audioPortID, outputNumber, value);
    [self updateSoloLabel:value];
    changingSolo = NO;
  }
}

- (void) buildPanLabel {


  panSoloLabelContainer = [UIView new];
  panSoloLabelContainer.translatesAutoresizingMaskIntoConstraints = NO;

  if (stereoLinked) {
    panLabel = [[UIButton alloc] init];
    panLabel.translatesAutoresizingMaskIntoConstraints = NO;
    [panLabel setTitle:@"C" forState:UIControlStateNormal];
    [panLabel setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [panLabel.titleLabel setFont:[UIFont boldSystemFontOfSize:8]];
    panLabel.selected = NO;
    [panSoloLabelContainer addSubview:panLabel];

    if ((mixerType == out) && (isAudioStrip == NO)) {
      soloLabel = [[UIButton alloc] init];
      soloLabel.translatesAutoresizingMaskIntoConstraints = NO;
      [soloLabel setTitle:@"0.0 dB" forState:UIControlStateNormal];
      [soloLabel setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
      [soloLabel.titleLabel setFont:[UIFont boldSystemFontOfSize:8]];
      soloLabel.selected = NO;

      [panSoloLabelContainer addSubview:soloLabel];

      [soloLabel autoAlignAxisToSuperviewAxis:ALAxisHorizontal];
      [panLabel autoAlignAxisToSuperviewAxis:ALAxisHorizontal];

      NSArray *viewsToAlign2 = [NSArray arrayWithObjects: panLabel, soloLabel, nil];
      [viewsToAlign2 autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeHorizontal withFixedSpacing:2];
    }
    else {
      NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-2-[panLabel]-2-|"
                                                                          options:0
                                                                          metrics:nil
                                                                            views:NSDictionaryOfVariableBindings(panLabel)];
      NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[panLabel(25)]"
                                                                          options:0
                                                                          metrics:nil
                                                                            views:NSDictionaryOfVariableBindings(panLabel)];
      [panSoloLabelContainer addConstraints:constraint_POS_H];
      [panSoloLabelContainer addConstraints:constraint_POS_V];
      [panLabel autoAlignAxisToSuperviewAxis:ALAxisHorizontal];
    }
  }

  [self addSubview:panSoloLabelContainer];
  NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-2-[panSoloLabelContainer]-2-|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(panSoloLabelContainer)];
  NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[panSoloLabelContainer(25)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(panSoloLabelContainer)];
  [self addConstraints:constraint_POS_H];
  [self addConstraints:constraint_POS_V];
}

- (void) setColorOfMeter:(UIProgressView*)meter toRepresent:(int)value {
  [meter setTrackTintColor:UIColorFromRGB(0xd3d3d3)];
  if (value < -6) {
//    [meter setProgressTintColor:[UIColor colorWithRed:((CGFloat)0x11)/255.0 green:((CGFloat)0xe0) blue:((CGFloat)0x00) alpha:1.0]];
      [meter setProgressTintColor:[UIColor colorWithRed:0.066 green:1.0 blue:0.0 alpha:1.0]];
    //  [meter setProgressTintColor:[UIColor redColor]];
  }
  else if (value < -1) {
//    [meter setProgressTintColor:[UIColor colorWithRed:((CGFloat)0xff)/255.0 green:((CGFloat)0xcc)/255.0 blue:((CGFloat)0x00) alpha:1.0]];
      [meter setProgressTintColor:[UIColor colorWithRed:1.0 green:0.5 blue:0.0 alpha:1.0]];
    //  [meter setProgressTintColor:[UIColor greenColor]];

  }
  else{
    //[meter setProgressTintColor:[UIColor colorWithRed:((CGFloat)0xff)/255.0 green:((CGFloat)0x00) blue:((CGFloat)0x33)/255.0 alpha:1.0]];
    [meter setProgressTintColor:[UIColor colorWithRed:1.0 green:0.0 blue:0.2 alpha:1.0]];
    //  [meter setProgressTintColor:[UIColor yellowColor]];
  }
}

-(void) updatePanDials {
//  int16_t value;
  if (isAudioStrip) {
    if (mixerType == in) {
      int16_t value = analogInterface->trimCurrent(inputNumber);
      int16_t value2 = analogInterface->trimCurrent(inputNumber + 1);

      if (value < 0) {
        panDial.position = 1.0 * (value) / analogInterface->trimMin(inputNumber) * 3.1415926535;
      }
      else if (value2 < 0) {
        panDial.position = 0.0 - 1.0 * (value2) / analogInterface->trimMin(inputNumber) * 3.1415926535;
      }
      else {
        panDial.position = 0.0;
      }
    }
    else {
      int16_t value = analogInterface->trimCurrent(outputNumber);
      int16_t value2 = analogInterface->trimCurrent(outputNumber + 1);

      if (value < 0) {
        panDial.position = 1.0 * (value) / analogInterface->trimMin(outputNumber) * 3.1415926535;
      }
      else if (value2 < 0) {
        panDial.position = 0.0 - 1.0 * (value2) / analogInterface->trimMin(outputNumber) * 3.1415926535;
      }
      else {
        panDial.position = 0.0;
      }
    }
  }
  else {
    if (mixerType == in) {
      int16_t value = mixerInputInterface->panCurrent(audioPortID, outputNumber, inputNumber);

      panDial.position = 1.0 * value / mixerInputInterface->panMax(audioPortID) * 3.1415926535;
    }
    else {
      int16_t value = mixerOutputInterface->panCurrent(audioPortID, outputNumber);
      panDial.position = 1.0 * value / mixerOutputInterface->panMax(audioPortID) * 3.1415926535;

      value = mixerOutputInterface->solo(audioPortID, outputNumber);
      soloDial.position = -3.1415926535 + (2 * 3.1415926535) * (value - mixerOutputInterface->volumeMin(audioPortID))
            / (mixerOutputInterface->volumeMax(audioPortID) - mixerOutputInterface->volumeMin(audioPortID));
    }
  }
}

-(void) updatePanLabels {
  int16_t value, value2;
  if (isAudioStrip) {
    if (mixerType == in) {
      value = analogInterface->trimCurrent(inputNumber);
      value2 = analogInterface->trimCurrent(inputNumber + 1);
    }
    else {
      value = analogInterface->trimCurrent(outputNumber);
      value2 = analogInterface->trimCurrent(outputNumber + 1);
    }

    if (value < 0) {
      value = 0 - (value);
      [panLabel setTitle:[NSString stringWithFormat:@"R %0.1f", value / 256.0] forState:UIControlStateNormal];
    }
    else if (value2 < 0) {
      value = 0 - (value2);
      [panLabel setTitle:[NSString stringWithFormat:@"L %0.1f", value / 256.0] forState:UIControlStateNormal];
    }
    else {
      [panLabel setTitle:@"C" forState:UIControlStateNormal];
    }

  }
  else {
    if (mixerType == in) {
      value = mixerInputInterface->panCurrent(audioPortID, outputNumber, inputNumber);
      if (value < 0) {
        value = 0 - (value);
        [panLabel setTitle:[NSString stringWithFormat:@"L %d", value] forState:UIControlStateNormal];
      }
      else if (value > 0) {
        [panLabel setTitle:[NSString stringWithFormat:@"R %d", value] forState:UIControlStateNormal];
      }
      else {
        [panLabel setTitle:@"C" forState:UIControlStateNormal];
      }
    }
    else {
      value = mixerOutputInterface->panCurrent(audioPortID, outputNumber);
      if (value < 0) {
        value = 0 - (value);
        [panLabel setTitle:[NSString stringWithFormat:@"L %d", value] forState:UIControlStateNormal];
      }
      else if (value > 0) {
        [panLabel setTitle:[NSString stringWithFormat:@"R %d", value] forState:UIControlStateNormal];
      }
      else {
        [panLabel setTitle:@"C" forState:UIControlStateNormal];
      }

      [self updateSoloLabel:mixerOutputInterface->solo(audioPortID, outputNumber)];
    }
  }
}

-(void) buildVolumeSlider {
  volumeSlider = [[UISlider alloc] init];
  [volumeSlider setTranslatesAutoresizingMaskIntoConstraints:NO];
  [volumeSlider removeConstraints:volumeSlider.constraints];
  CGAffineTransform trans = CGAffineTransformMakeRotation(M_PI * -0.5);
  volumeSlider.transform = trans; //CGAffineTransformScale(trans,1.0f,4.0f);
  [volumeSlider setThumbImage:[volumeSlider thumbImageForState:UIControlStateNormal] forState:UIControlStateNormal];
  [volumeSlider setThumbTintColor:UIColorFromRGB(0xd3d3d3)];


  meterLeft = [[UIProgressView alloc] init];
  [meterLeft setTranslatesAutoresizingMaskIntoConstraints:NO];
  [meterLeft removeConstraints:meterLeft.constraints];
  meterLeft.transform = CGAffineTransformScale(trans,1.0f,3.5f);

  if (stereoLinked) {
    meterRight = [[UIProgressView alloc] init];
    [meterRight setTranslatesAutoresizingMaskIntoConstraints:NO];
    [meterRight removeConstraints:meterRight.constraints];
    meterRight.transform = CGAffineTransformScale(trans,1.0f,3.5f);
  }
  else {
    meterRight = nil;
  }

  totalVolumeSliderContainer = [UIView new];
  totalVolumeSliderContainer.translatesAutoresizingMaskIntoConstraints = NO;

  volumeSliderContainer = [UIView new];
  volumeSliderContainer.translatesAutoresizingMaskIntoConstraints = NO;
  [volumeSliderContainer addSubview:volumeSlider];

  meterLeftContainer = [UIView new];
  meterLeftContainer.translatesAutoresizingMaskIntoConstraints = NO;
  [meterLeftContainer addSubview:meterLeft];

  meterRightContainer = [UIView new];
  meterRightContainer.translatesAutoresizingMaskIntoConstraints = NO;

  if (meterRight != nil) {
    [meterRightContainer addSubview:meterRight];
  }

  meterContainer = [UIView new];
  meterContainer.translatesAutoresizingMaskIntoConstraints = NO;

  [volumeSliderContainer addConstraint:[NSLayoutConstraint constraintWithItem:volumeSlider
                                                                    attribute:NSLayoutAttributeWidth                                                relatedBy:NSLayoutRelationEqual
                                                                       toItem:volumeSliderContainer
                                                                    attribute:NSLayoutAttributeHeight
                                                                   multiplier:1.0
                                                                     constant:0]];

  [volumeSliderContainer addConstraint:[NSLayoutConstraint constraintWithItem:volumeSlider
                                                                    attribute:NSLayoutAttributeCenterX
                                                                    relatedBy:NSLayoutRelationEqual
                                                                       toItem:volumeSliderContainer
                                                                    attribute:NSLayoutAttributeCenterX
                                                                   multiplier:1
                                                                     constant:0]];

  [volumeSliderContainer addConstraint:[NSLayoutConstraint constraintWithItem:volumeSlider
                                                                    attribute:NSLayoutAttributeCenterY
                                                                    relatedBy:NSLayoutRelationEqual
                                                                       toItem:volumeSliderContainer
                                                                    attribute:NSLayoutAttributeCenterY
                                                                   multiplier:1
                                                                     constant:0]];

  [meterLeftContainer addConstraint:[NSLayoutConstraint constraintWithItem:meterLeft
                                                                    attribute:NSLayoutAttributeWidth                                                relatedBy:NSLayoutRelationEqual
                                                                       toItem:meterLeftContainer
                                                                    attribute:NSLayoutAttributeHeight
                                                                   multiplier:1.0
                                                                     constant:0]];

  [meterLeftContainer addConstraint:[NSLayoutConstraint constraintWithItem:meterLeft
                                                                    attribute:NSLayoutAttributeCenterX
                                                                    relatedBy:NSLayoutRelationEqual
                                                                       toItem:meterLeftContainer
                                                                    attribute:NSLayoutAttributeCenterX
                                                                   multiplier:1
                                                                     constant:0]];

  [meterLeftContainer addConstraint:[NSLayoutConstraint constraintWithItem:meterLeft
                                                                    attribute:NSLayoutAttributeCenterY
                                                                    relatedBy:NSLayoutRelationEqual
                                                                       toItem:meterLeftContainer
                                                                    attribute:NSLayoutAttributeCenterY
                                                                   multiplier:1
                                                                     constant:0]];
  if (meterRight != nil) {
    [meterRightContainer addConstraint:[NSLayoutConstraint constraintWithItem:meterRight
                                                                    attribute:NSLayoutAttributeWidth                                                relatedBy:NSLayoutRelationEqual
                                                                       toItem:meterRightContainer
                                                                    attribute:NSLayoutAttributeHeight
                                                                   multiplier:1.0
                                                                     constant:0]];

    [meterRightContainer addConstraint:[NSLayoutConstraint constraintWithItem:meterRight
                                                                    attribute:NSLayoutAttributeCenterX
                                                                    relatedBy:NSLayoutRelationEqual
                                                                       toItem:meterRightContainer
                                                                    attribute:NSLayoutAttributeCenterX
                                                                   multiplier:1
                                                                     constant:0]];

    [meterRightContainer addConstraint:[NSLayoutConstraint constraintWithItem:meterRight
                                                                    attribute:NSLayoutAttributeCenterY
                                                                    relatedBy:NSLayoutRelationEqual
                                                                       toItem:meterRightContainer
                                                                    attribute:NSLayoutAttributeCenterY
                                                                   multiplier:1
                                                                     constant:0]];
  }

  if (isAudioStrip) {
    if (mixerType == out) {
      [volumeSlider setMaximumValue:analogInterface->volumeMax(outputNumber)*
       (1.0 /  256.0)/
       (analogInterface->volumeResolution(outputNumber)*
        (1.0 /  256.0))];
      [volumeSlider setMinimumValue:analogInterface->volumeMin(outputNumber)*
         (1.0 /  256.0)/
       (analogInterface->volumeResolution(outputNumber)*
        (1.0 /  256.0))];
    }
    else {
      [volumeSlider setMaximumValue:analogInterface->volumeMax(inputNumber)*
       (1.0 /  256.0)/
       (analogInterface->volumeResolution(inputNumber)*
        (1.0 /  256.0))];
      [volumeSlider setMinimumValue:analogInterface->volumeMin(inputNumber)*
       (1.0 /  256.0)/
       (analogInterface->volumeResolution(inputNumber)*
        (1.0 /  256.0))];
    }
  }
  else {
    if (mixerType == out) {
      [volumeSlider setMaximumValue:299.0];
      [volumeSlider setMinimumValue:0.0];
    }
    else {
      [volumeSlider setMaximumValue:299.0];
      [volumeSlider setMinimumValue:0.0];
    }
  }

  [volumeSlider setUserInteractionEnabled:YES];

  [meterLeft setProgress:0.0];
  [self setColorOfMeter:meterLeft toRepresent:-20];

  if (meterRight != nil) {
    [meterRight setProgress:0.0];
    [self setColorOfMeter:meterRight toRepresent:-20];
  }

  meterMax = 299;

  [meterContainer addSubview:meterLeftContainer];
  [meterContainer addSubview:meterRightContainer];

  [volumeSliderContainer setUserInteractionEnabled:YES];
  [totalVolumeSliderContainer addSubview:volumeSliderContainer];
  [totalVolumeSliderContainer addSubview:meterContainer];

  [self addSubview:totalVolumeSliderContainer];

  NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[volumeSliderContainer(300)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(volumeSliderContainer)];
  NSArray *constraint_POS_V2 = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[meterLeftContainer(300)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(meterLeftContainer)];
  NSArray *constraint_POS_V3 = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[meterRightContainer(300)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(meterRightContainer)];
  NSArray *constraint_POS_V4 = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[meterContainer(300)]"
                                                                       options:0
                                                                       metrics:nil
                                                                         views:NSDictionaryOfVariableBindings(meterContainer)];

  NSArray *constraint_POS_V5 = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[totalVolumeSliderContainer(300)]"
                                                                       options:0
                                                                       metrics:nil
                                                                         views:NSDictionaryOfVariableBindings(totalVolumeSliderContainer)];

  NSArray *constraint_POS_H5 = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-2-[totalVolumeSliderContainer]-2-|"
                                                                       options:0
                                                                       metrics:nil
                                                                         views:NSDictionaryOfVariableBindings(totalVolumeSliderContainer)];

  [meterContainer addConstraints:constraint_POS_V2];
  [meterContainer addConstraints:constraint_POS_V3];

  [totalVolumeSliderContainer addConstraints:constraint_POS_V];
  [totalVolumeSliderContainer addConstraints:constraint_POS_V4];
  [self addConstraints:constraint_POS_V5];
  [self addConstraints:constraint_POS_H5];

  NSArray *viewsToAlign2 = [NSArray arrayWithObjects: meterLeftContainer, meterRightContainer, nil];
  [viewsToAlign2 autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeHorizontal withFixedSpacing:2];
  [meterLeftContainer autoAlignAxisToSuperviewAxis:ALAxisHorizontal];
  [meterRightContainer autoAlignAxisToSuperviewAxis:ALAxisHorizontal];

//  NSArray *viewsToAlign3 = [NSArray arrayWithObjects: meterLeftContainer, meterRightContainer, nil];

  NSArray *viewsToAlign = [NSArray arrayWithObjects: volumeSliderContainer, meterContainer, nil];
  [viewsToAlign autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeHorizontal withFixedSpacing:4];
  [viewsToAlign autoAlignViewsToAxis:ALAxisHorizontal];
  [meterContainer autoAlignAxisToSuperviewAxis:ALAxisHorizontal];
  [volumeSliderContainer autoAlignAxisToSuperviewAxis:ALAxisHorizontal];

  [volumeSlider addTarget:self
                   action:@selector(volumeValueChanged:)
         forControlEvents:UIControlEventValueChanged];

/*  [volumeSlider addTarget:self
                   action:@selector(volumeValueChangeEnded:)
         forControlEvents:UIControlEventTouchUpInside];*/
}

/*-(void) volumeValueChangeEnded:(UISlider*) slider {
  if (isAudioStrip) {
    slider.value = round(slider.value);

    int16_t value;
    if (mixerType == in) {
      value = (int)(round(slider.value) / (1.0 / 256.0) *
                           (analogInterface->volumeResolution(inputNumber)*
                            (1.0 / 256.0))) & 0xFFFF;
      analogInterface->volumeCurrent(inputNumber, value);
    }
    else {
      value = (int)(round(slider.value) / (1.0 / 256.0) *
                           (analogInterface->volumeResolution(outputNumber)*
                            (1.0 / 256.0))) & 0xFFFF;
      analogInterface->volumeCurrent(outputNumber, value);
    }
    [self updateVolumeLabel:value];
  }
}*/

-(void) volumeValueChanged:(UISlider*) slider {
  if (!changingVolume) {
    changingVolume = YES;
    int16_t value;
    if (isAudioStrip) {
      if (mixerType == in) {
        value = (int)(round(slider.value) / (1.0 / 256.0) *
                      (analogInterface->volumeResolution(inputNumber)*
                       (1.0 / 256.0))) & 0xFFFF;
        if (analogInterface->volumeCurrent(inputNumber) != value)
          analogInterface->volumeCurrent(inputNumber, value);
      }
      else {
        if (round(slider.value) == round(slider.minimumValue)) {
          value = 0x8000;
        }
        else {
          value = (int)(round(slider.value) / (1.0 / 256.0) *
                        (analogInterface->volumeResolution(outputNumber)*
                         (1.0 / 256.0))) & 0xFFFF;
        }

        if (analogInterface->volumeCurrent(outputNumber) != value)
          analogInterface->volumeCurrent(outputNumber, value);
      }
    }
    else {
      if (round(slider.value) == round(slider.minimumValue)) {
        value = 0x8000;
      }
      else {
        value = [self pixelsToIntForICA:(int)slider.value];
      }
      if (mixerType == in) {
        mixerInputInterface->volumeCurrent(audioPortID, outputNumber, inputNumber, value);
      }
      else {
        mixerOutputInterface->volumeCurrent(audioPortID, outputNumber, value);
      }
    }
    [self updateVolumeLabel:value];
    changingVolume = NO;
  }
}

-(void) updateVolumeSlider {
  int16_t volCurrent = 0;
  if (isAudioStrip) {
    if (mixerType == out) {
      volCurrent = analogInterface->volumeCurrent(outputNumber);
      if (volumeSlider.value != analogInterface->volumeCurrent(outputNumber) * (1.0 / 256.0))
        [volumeSlider setValue:analogInterface->volumeCurrent(outputNumber) * (1.0 / 256.0)];
    }
    else {
      volCurrent = analogInterface->volumeCurrent(inputNumber);
      if (volumeSlider.value != analogInterface->volumeCurrent(inputNumber) * (1.0 / 256.0))
        [volumeSlider setValue:analogInterface->volumeCurrent(inputNumber) * (1.0 / 256.0)];
    }
    [self updateVolumeLabel:volCurrent];
  }
  else {
    if (mixerType == out) {
      volCurrent = mixerOutputInterface->volumeCurrent(audioPortID, outputNumber);
    }
    else {
      volCurrent = mixerInputInterface->volumeCurrent(audioPortID, outputNumber, inputNumber);
    }
    if (volumeSlider.value != [self toPixelsFromICA:volCurrent])
      [volumeSlider setValue:[self toPixelsFromICA:volCurrent]];
    [self updateVolumeLabel:volCurrent];
  }
}

-(void) updateVolumeLabel:(int16_t)volCurrent {
  if (volCurrent == (int16_t)0x8000) {
    [volumeLabel setTitle:@"-\u221E dB" forState:UIControlStateNormal];
  }
  else {
    if (isAudioStrip) {
      [volumeLabel setTitle:[NSString stringWithFormat:@"%0.1f dB",volCurrent * (1.0 / 256.0)] forState:UIControlStateNormal];
    }
    else {
      [volumeLabel setTitle:[NSString stringWithFormat:@"%0.1f dB",volCurrent * (1.0 / 256.0)] forState:UIControlStateNormal];
    }
  }
}

-(void) updateSoloLabel:(int16_t)soloCurrent {
  if (soloCurrent == (int16_t)0x8000) {
    [soloLabel setTitle:@"-\u221E dB" forState:UIControlStateNormal];
  }
  else {
    if (isAudioStrip) {
      [soloLabel setTitle:[NSString stringWithFormat:@"%0.1f dB",soloCurrent * (1.0 / 256.0)] forState:UIControlStateNormal];
    }
    else {
      [soloLabel setTitle:[NSString stringWithFormat:@"%0.1f dB",soloCurrent * (1.0 / 256.0)] forState:UIControlStateNormal];
    }
  }
}

-(void) updateMeters {
  if (isAudioStrip) {
    int16_t meterLeftVal = 0, meterRightVal = 0;
    if (mixerType == out) {
      meterLeftVal = analogInterface->meterCurrent(outputNumber);
      if (stereoLinked && (outputNumber % 2)) {
        meterRightVal = analogInterface->meterCurrent(outputNumber + 1);
      }
    }
    else {
      meterLeftVal = analogInterface->meterCurrent(inputNumber);
      if (stereoLinked && (outputNumber % 2)) {
        meterRightVal = analogInterface->meterCurrent(inputNumber + 1);
      }
    }
    CGFloat meterLeftFraction = [self toPixelsFromICA:(((double)20 * log10((double)meterLeftVal / 8192.0)) * 256.0)]; // convert to mixer ICA style
    CGFloat meterRightFraction = [self toPixelsFromICA:(((double)20 * log10((double)meterRightVal / 8192.0)) * 256.0)];

    int minus40pixels = [self toPixelsFromICA:( -40 * 256 )];

    if (((double)20 * log10((double)meterLeftVal / 8192.0)) < -40)
      meterLeftFraction = 0;
    if (((double)20 * log10((double)meterRightVal / 8192.0)) < -40)
      meterRightFraction = 0;

    meterLeftFraction = ((double)(meterLeftFraction - minus40pixels) / (300 - 75 - minus40pixels)); // max = 0 instead of 6
    meterRightFraction = ((double)(meterRightFraction - minus40pixels) / (300 - 75 - minus40pixels)); // and ignore under -40

    [meterLeft setProgress:meterLeftFraction];
    [self setColorOfMeter:meterLeft toRepresent:(((double)20 * log10((double)meterLeftVal / 8192.0)) * 256.0)];
    if (meterRight != nil) {
      [meterRight setProgress:meterRightFraction];
      [self setColorOfMeter:meterRight toRepresent:(((double)20 * log10((double)meterRightVal / 8192.0)) * 256.0)];
    }
    //NSLog(@"meterLeftVal, meterRightVal: %d, %d", meterLeftVal, meterRightVal);
  }
  else {
    int16_t meterLeftVal = 0, meterRightVal = 0;
    if (mixerType == out) {
      meterLeftVal = mixerOutputInterface->meterCurrent(audioPortID, outputNumber);
      if (stereoLinked && (outputNumber % 2)) {
        meterRightVal = mixerOutputInterface->meterCurrent(audioPortID, outputNumber + 1);
      }
    }
    else {
      meterLeftVal = mixerInputInterface->meterCurrent(audioPortID, outputNumber, inputNumber);
      if (stereoLinked && (inputNumber % 2)) {
        meterRightVal = mixerInputInterface->meterCurrent(audioPortID, outputNumber, inputNumber + 1);
      }
    }

    CGFloat meterLeftFraction = [self toPixelsFromICA:(((double)20 * log10((double)meterLeftVal / 8192.0)) * 256.0)] / 299.0;
    CGFloat meterRightFraction = [self toPixelsFromICA:(((double)20 * log10((double)meterRightVal / 8192.0)) * 256.0)] / 299.0;

    if (meterLeftVal == 0) {
      [meterLeft setProgress:0.0];
    }
    else {
      [meterLeft setProgress:meterLeftFraction];
      [self setColorOfMeter:meterLeft toRepresent:(((double)20 * log10((double)meterLeftVal / 8192.0)) * 256.0)];
    }
    if (meterRight != nil) {
      if (meterRightVal == 0) {
        [meterRight setProgress:0.0];
      }
      else {
        [meterRight setProgress:meterRightFraction];
        [self setColorOfMeter:meterRight toRepresent:(((double)20 * log10((double)meterRightVal / 8192.0)) * 256.0)];
      }
    }
  }
}

-(int16_t) pixelsToIntForICA:(Word) pixels
{
  pixels = 299 - pixels;
  if (pixels < 75) {
    if (pixels == 0)
      return 6*256;
    else {
      return ((int)(6*256 - ((double)6*(double)256/(double)75 * pixels))) / 16 * 16;
    }
  }
  else if (pixels < 150) {
    if (pixels == 75)
      return 0;
    else {
      return ((int)(0*256 - ((double)6*(double)256/(double)75 * (pixels-75)))) / 16 * 16;
    }
  }
  else if (pixels < 188) {
    if (pixels == 150)
      return -6 * 256;
    else {
      return ((int)(-6*256 - ((double)6*(double)256/(double)38 * (pixels-150)))) / 16 * 16;
    }
  }
  else if (pixels < 206) {
    if (pixels == 188)
      return -12 * 256;
    else {
      return ((int)(-12*256 - ((double)6*(double)256/(double)18 * (pixels-188)))) / 16 * 16;
    }

  }
  else if (pixels < 224) {
    if (pixels == 206)
      return -18 * 256;
    else {
      return ((int)(-18*256 - ((double)6*(double)256/(double)18 * (pixels-206)))) / 16 * 16;
    }

  }
  else if (pixels < 243) {
    if (pixels == 224)
      return -24 * 256;
    else {
      return ((int)(-24*256 - ((double)12*(double)256/(double)19 * (pixels-224)))) / 16 * 16;
    }

  }
  else if (pixels < 263) {
    if (pixels == 243)
      return -36 * 256;
    else {
      return ((int)(-36*256 - ((double)12*(double)256/(double)19 * (pixels-243)))) / 16 * 16;
    }

  }
  else if (pixels < 282) {
    if (pixels == 263)
      return -48 * 256;
    else {
      return ((int)(-48*256 - ((double)12*(double)256/(double)19 * (pixels-263)))) / 16 * 16;
    }

  }
  else if (pixels < 300) {
    if (pixels == 282)
      return -60 * 256;
    else if (pixels == 299)
      return 0x8000;
    else {
      return ((int)(-60*256 - ((double)20*(double)256/(double)18 * (pixels-282)))) / 16 * 16;
    }
  }
  else return 0;
}

- (Word) toPixelsFromICA:(int16_t) theInt
{
  if (theInt <= -60*256) {
    if (theInt <= -80*256)
      return (299 - 299);
    else {
      return ((int)((299 - 299) + ((double)(theInt - -80*256)/((double)(256*20) / 18))))+1;
    }
  }
  else if (theInt < -48*256) {
    if (theInt == -60*256)
      return (299 - 282);
    else {
      return ((int)((299 - 282) + ((double)(theInt - -60*256)/((double)(256*12) / 19))))+1;
    }
  }
  else if (theInt < -36*256) {
    if (theInt == -48*256)
      return (299 - 263);
    else {
      return ((int)((299 - 263) + ((double)(theInt - -48*256)/((double)(256*12) / 19))))+1;
    }
  }
  else if (theInt < -24*256) {
    if (theInt == -36*256)
      return (299 - 243);
    else {
      return ((int)((299 - 243) + ((double)(theInt - -36*256)/((double)(256*12) / 19))))+1;
    }
  }
  else if (theInt < -18*256) {
    if (theInt == -24*256)
      return (299 - 224);
    else {
      return ((int)((299 - 224) + ((double)(theInt - -24*256)/((double)(256*6) / 18))))+1;
    }
  }
  else if (theInt < -12*256) {
    if (theInt == -18*256)
      return (299 - 206);
    else {
      return ((int)((299 - 206) + ((double)(theInt - -18*256)/((double)(256*6) / 18))))+1;
    }
  }
  else if (theInt < -6*256) {
    if (theInt == -12*256)
      return (299 - 188);
    else {
      return ((int)((299 - 188) + ((double)(theInt - -12*256)/((double)(256*6) / 38))))+1;
    }
  }
  else if (theInt < 0*256) {
    if (theInt == -6*256)
      return (299 - 150);
    else {
      return ((int)((299 - 150) + ((double)(theInt - -6*256)/((double)(256*6) / 75))))+1;
    }
  }
  else if (theInt < 6*256) {
    if (theInt == 0*256)
      return (299 - 75);
    else {
      return ((int)((299 - 75) + ((double)(theInt - 0*256)/((double)(256*6) / 75))))+1;
    }
  }
  else if (theInt >= 6*256) {
    return 299;
  }
  else return 0;
}

- (void) buildVolumeLabel {
  volumeLabel = [[UIButton alloc] init];
  volumeLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [volumeLabel setTitle:@"0.0 dB" forState:UIControlStateNormal];
  [volumeLabel setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
  [volumeLabel.titleLabel setFont:[UIFont boldSystemFontOfSize:8]];
  volumeLabel.selected = NO;

  [self addSubview:volumeLabel];

  NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-2-[volumeLabel]-2-|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(volumeLabel)];
  NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[volumeLabel(25)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(volumeLabel)];
  [self addConstraints:constraint_POS_H];
  [self addConstraints:constraint_POS_V];
}

-(void) buildMuteButton {
  muteButton = [[UIButton alloc] init];
  muteButton.translatesAutoresizingMaskIntoConstraints = NO;
  [muteButton setTitle:@"M" forState:UIControlStateNormal];
  [muteButton.titleLabel setFont:[UIFont boldSystemFontOfSize:8]];
  [muteButton setBackgroundColor:[UIColor grayColor]];
  muteButton.selected = NO;

  [self addSubview:muteButton];

  NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-2-[muteButton]-2-|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(muteButton)];
  NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[muteButton(25)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(muteButton)];
  [self addConstraints:constraint_POS_H];
  [self addConstraints:constraint_POS_V];

  [muteButton addTarget:self
                        action:@selector(mutePressed)
              forControlEvents:UIControlEventTouchUpInside];
}

- (void) updateMuteButton {
  if (isAudioStrip) {
    if (mixerType == in) {
      muteButton.selected = analogInterface->mute(inputNumber);
    }
    else {
      muteButton.selected = analogInterface->mute(outputNumber);
    }
  }
  else {
    if (mixerType == in) {
      muteButton.selected = mixerInputInterface->mute(audioPortID, outputNumber, inputNumber);
    }
    else {
      muteButton.selected = mixerOutputInterface->mute(audioPortID, outputNumber);
    }
  }

  [self updateButtonColor:muteButton];
}

- (void) mutePressed {
  muteButton.selected = !muteButton.selected;
  [self updateButtonColor:muteButton];

  if (isAudioStrip) {
    if (mixerType == in) {
      analogInterface->mute(inputNumber, muteButton.selected);
    }
    else {
      analogInterface->mute(outputNumber, muteButton.selected);
    }
  }
  else {
    if (mixerType == in) {
      mixerInputInterface->mute(audioPortID, outputNumber, inputNumber, muteButton.selected);
    }
    else {
      mixerOutputInterface->mute(audioPortID, outputNumber, muteButton.selected);
    }
  }
}

- (void) updateButtonColor:(UIButton *)button {
  if (button.selected)
    [button setBackgroundColor:[UIColor blueColor]];
  else
    [button setBackgroundColor:[UIColor grayColor]];
}

-(void) buildSoloButton {
  soloButton = [[UIButton alloc] init];
  soloButton.translatesAutoresizingMaskIntoConstraints = NO;
  [soloButton setTitle:@"S" forState:UIControlStateNormal];
  [soloButton.titleLabel setFont:[UIFont boldSystemFontOfSize:8]];
  [soloButton setBackgroundColor:[UIColor grayColor]];
  soloButton.selected = NO;

  [self addSubview:soloButton];

  NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-2-[soloButton]-2-|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(soloButton)];
  NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[soloButton(25)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(soloButton)];
  [self addConstraints:constraint_POS_H];
  [self addConstraints:constraint_POS_V];

  [soloButton addTarget:self
                 action:@selector(soloPressed)
       forControlEvents:UIControlEventTouchUpInside];
}

- (void) updateSoloButton {
  if (mixerType == in) {
    soloButton.selected = mixerInputInterface->solo(audioPortID, outputNumber, inputNumber);
  }

  [self updateButtonColor:soloButton];
}

- (void) soloPressed {
  soloButton.selected = !soloButton.selected;
  [self updateButtonColor:soloButton];

  if (mixerType == in) {
    mixerInputInterface->solo(audioPortID, outputNumber, inputNumber, soloButton.selected);
  }
}


-(void) buildInvertButtons {
  invertButtonContainer = [UIView new];

  invertButton = [[UIButton alloc] init];
  invertButton.translatesAutoresizingMaskIntoConstraints = NO;
  [invertButton setTitle:@"I" forState:UIControlStateNormal];
  [invertButton.titleLabel setFont:[UIFont boldSystemFontOfSize:8]];
  [invertButton setBackgroundColor:[UIColor grayColor]];
  invertButton.selected = NO;

  [invertButtonContainer addSubview:invertButton];
  [invertButton autoAlignAxisToSuperviewAxis:ALAxisHorizontal];

  NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[invertButton(25)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(invertButton)];
  [invertButtonContainer addConstraints:constraint_POS_V];

  [invertButton addTarget:self
                    action:@selector(invertPressed)
          forControlEvents:UIControlEventTouchUpInside];
  if (stereoLinked || outputStereoLinked) {
    invertRightButton = [[UIButton alloc] init];
    invertRightButton.translatesAutoresizingMaskIntoConstraints = NO;
    [invertRightButton setTitle:@"I" forState:UIControlStateNormal];
    [invertRightButton.titleLabel setFont:[UIFont boldSystemFontOfSize:8]];
    [invertRightButton setBackgroundColor:[UIColor grayColor]];
    invertRightButton.selected = NO;

    [invertButtonContainer addSubview:invertRightButton];
    [invertRightButton autoAlignAxisToSuperviewAxis:ALAxisHorizontal];


    NSArray *constraint_POS_V2 = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[invertRightButton(25)]"
                                                                        options:0
                                                                        metrics:nil
                                                                          views:NSDictionaryOfVariableBindings(invertRightButton)];
    [invertButtonContainer addConstraints:constraint_POS_V2];

    [invertRightButton addTarget:self
                      action:@selector(invertRightPressed)
            forControlEvents:UIControlEventTouchUpInside];

    [invertRightButton autoConstrainAttribute:ALAttributeWidth toAttribute:ALAttributeWidth ofView:invertButton];
  }
  NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-2-[invertButton]-2-[invertRightButton]-2-|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(invertButton,
                                                                                                             invertRightButton)];

  [invertButtonContainer addConstraints:constraint_POS_H];

  [self addSubview:invertButtonContainer];

  NSArray *constraint_POS_H3 = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[invertButtonContainer]|"
                                                                       options:0
                                                                       metrics:nil
                                                                         views:NSDictionaryOfVariableBindings(invertButtonContainer)];
  NSArray *constraint_POS_V3 = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[invertButtonContainer(25)]"
                                                                       options:0
                                                                       metrics:nil
                                                                         views:NSDictionaryOfVariableBindings(invertButtonContainer)];

  [self addConstraints:constraint_POS_H3];
  [self addConstraints:constraint_POS_V3];
}

- (void) updateInvertButtons {
  bool invert = false;
  bool invertRight = false;

  if (mixerType == in) {
    invert = mixerInputInterface->invert(audioPortID,outputNumber, inputNumber);

    if (!stereoLinked && outputStereoLinked)
      invertRight = mixerInputInterface->invert(audioPortID,outputNumber + 1,inputNumber);
    else if (stereoLinked && !outputStereoLinked)
      invertRight = mixerInputInterface->invert(audioPortID,outputNumber,inputNumber + 1);
    else if (stereoLinked && outputStereoLinked) {
      invertRight = mixerInputInterface->invert(audioPortID,outputNumber+1,inputNumber + 1);
    }
  }
  else {
    invert = mixerOutputInterface->invert(audioPortID, outputNumber);
    invertRight = mixerOutputInterface->invert(audioPortID, outputNumber + 1);
  }

  invertButton.selected = invert;
  invertRightButton.selected = invertRight;
  [self updateButtonColor:invertButton];
  [self updateButtonColor:invertRightButton];
}

- (void) invertPressed {
  invertButton.selected = !invertButton.selected;
  [self updateButtonColor:invertButton];

  if (mixerType == in) {
    if (mixerInputInterface->invert(audioPortID, outputNumber,inputNumber) != invertButton.selected)
      mixerInputInterface->invert(audioPortID, outputNumber,inputNumber, invertButton.selected);
  }
  else {
    if (mixerOutputInterface->invert(audioPortID, outputNumber) != invertButton.selected)
      mixerOutputInterface->invert(audioPortID, outputNumber, invertButton.selected);
  }
}

- (void) invertRightPressed {
  invertRightButton.selected = !invertRightButton.selected;
  [self updateButtonColor:invertRightButton];

  if (mixerType == in) {
    if (!stereoLinked && outputStereoLinked) {
      if (mixerInputInterface->invert(audioPortID, outputNumber+1,inputNumber) != invertRightButton.selected) {
        mixerInputInterface->invert(audioPortID, outputNumber+1,inputNumber, invertRightButton.selected);
      }
    }
    else if (stereoLinked && !outputStereoLinked) {
      if (mixerInputInterface->invert(audioPortID, outputNumber,inputNumber + 1) != invertRightButton.selected) {
        mixerInputInterface->invert(audioPortID, outputNumber,inputNumber + 1, invertRightButton.selected);
      }
    }
    else if (stereoLinked && outputStereoLinked) {
      if (mixerInputInterface->invert(audioPortID, outputNumber+1,inputNumber + 1) != invertRightButton.selected) {
        mixerInputInterface->invert(audioPortID, outputNumber+1,inputNumber + 1, invertRightButton.selected);
      }
    }
  }
  else {
    if (mixerOutputInterface->invert(audioPortID, outputNumber + 1) != invertRightButton.selected)
      mixerOutputInterface->invert(audioPortID, outputNumber + 1, invertRightButton.selected);
  }
}

-(void) buildSoloPFLButton {
  soloPFLButton = [[UIButton alloc] init];
  soloPFLButton.translatesAutoresizingMaskIntoConstraints = NO;
  [soloPFLButton setTitle:@"PFL" forState:UIControlStateNormal];
  [soloPFLButton.titleLabel setFont:[UIFont boldSystemFontOfSize:8]];
  [soloPFLButton setBackgroundColor:[UIColor grayColor]];
  soloPFLButton.selected = NO;

  [self addSubview:soloPFLButton];

  NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-2-[soloPFLButton]-2-|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(soloPFLButton)];
  NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[soloPFLButton(25)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(soloPFLButton)];
  [self addConstraints:constraint_POS_H];
  [self addConstraints:constraint_POS_V];

  [soloPFLButton addTarget:self
                 action:@selector(soloPFLPressed)
       forControlEvents:UIControlEventTouchUpInside];
}

- (void) updateSoloPFLButton {
  if (mixerType == in) {
    soloPFLButton.selected = mixerInputInterface->soloPFL(audioPortID, outputNumber, inputNumber);
  }
  else {
    soloPFLButton.selected = mixerOutputInterface->soloPFL(audioPortID, outputNumber);
  }

  [self updateButtonColor:soloPFLButton];
}

- (void) soloPFLPressed {
  soloPFLButton.selected = !soloPFLButton.selected;
  [self updateButtonColor:soloPFLButton];

  if (mixerType == in) {
    if (mixerInputInterface->soloPFL(audioPortID, outputNumber,inputNumber) != soloPFLButton.selected)
      mixerInputInterface->soloPFL(audioPortID, outputNumber,inputNumber, soloPFLButton.selected);
  }
  else {
    if (mixerOutputInterface->soloPFL(audioPortID, outputNumber) != soloPFLButton.selected)
      mixerOutputInterface->soloPFL(audioPortID, outputNumber, soloPFLButton.selected);
  }
}

-(void) buildPhantomPowerButton {
  phantomPowerButton = [[UIButton alloc] init];
  phantomPowerButton.translatesAutoresizingMaskIntoConstraints = NO;
  [phantomPowerButton setTitle:@"48v" forState:UIControlStateNormal];
  [phantomPowerButton.titleLabel setFont:[UIFont boldSystemFontOfSize:8]];
  [phantomPowerButton setBackgroundColor:[UIColor grayColor]];
  phantomPowerButton.selected = NO;

  [self addSubview:phantomPowerButton];

  NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-2-[phantomPowerButton]-2-|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(phantomPowerButton)];
  NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[phantomPowerButton(25)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(phantomPowerButton)];
  [self addConstraints:constraint_POS_H];
  [self addConstraints:constraint_POS_V];

  [phantomPowerButton addTarget:self
                 action:@selector(phantomPowerPressed)
       forControlEvents:UIControlEventTouchUpInside];
}

- (void) updatePhantomPowerButton {
  if (mixerType == in) {
    phantomPowerButton.selected = analogInterface->phantomPower(inputNumber);
  }
  else {
    phantomPowerButton.selected = analogInterface->phantomPower(outputNumber);
  }

  [self updateButtonColor:phantomPowerButton];
}

- (void) phantomPowerPressed {
  phantomPowerButton.selected = !phantomPowerButton.selected;
  [self updateButtonColor:phantomPowerButton];

  if (mixerType == in) {
    analogInterface->phantomPower(inputNumber, phantomPowerButton.selected);
  }
  else {
    analogInterface->phantomPower(outputNumber, phantomPowerButton.selected);
  }
}

-(void) buildHighImpedenceButton {
  highImpedenceButton = [[UIButton alloc] init];
  highImpedenceButton.translatesAutoresizingMaskIntoConstraints = NO;
  [highImpedenceButton.titleLabel setFont:[UIFont boldSystemFontOfSize:8]];

  if ( self->device->getDeviceID().pid() == DevicePID::iConnect4Audio ) {
    [highImpedenceButton setTitleColor:[UIColor grayColor] forState:UIControlStateNormal];
  }

  if (analogInterface->highImpedance(inputNumber)) {
    [highImpedenceButton setTitle:@"Inst" forState:UIControlStateNormal];
    if ( self->device->getDeviceID().pid() == DevicePID::iConnect2Audio )
      [highImpedenceButton setBackgroundColor:[UIColor blueColor]];
  }
  else {
    [highImpedenceButton setTitle:@"Line" forState:UIControlStateNormal];
    if ( self->device->getDeviceID().pid() == DevicePID::iConnect2Audio )
      [highImpedenceButton setBackgroundColor:[UIColor grayColor]];
  }

  [self addSubview:highImpedenceButton];

  NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-2-[highImpedenceButton]-2-|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(highImpedenceButton)];
  NSArray *constraint_POS_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[highImpedenceButton(25)]"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(highImpedenceButton)];
  [self addConstraints:constraint_POS_H];
  [self addConstraints:constraint_POS_V];

  [highImpedenceButton addTarget:self
                         action:@selector(highImpedencePressed)
               forControlEvents:UIControlEventTouchUpInside];
}

-(void) updateHighImpedenceButton {
  if ( self->device->getDeviceID().pid() == DevicePID::iConnect4Audio )
    return;

  if (analogInterface->highImpedance(inputNumber)) {
    [highImpedenceButton setTitle:@"Inst" forState:UIControlStateNormal];
    [highImpedenceButton setBackgroundColor:[UIColor blueColor]];
  }
  else {
    [highImpedenceButton setTitle:@"Line" forState:UIControlStateNormal];
    [highImpedenceButton setBackgroundColor:[UIColor grayColor]];
  }
}

- (void) highImpedencePressed {
  highImpedenceButton.selected = !highImpedenceButton.selected;

  if ( self->device->getDeviceID().pid() == DevicePID::iConnect4Audio )
    return;

  if (mixerType == in) {
    analogInterface->highImpedance(inputNumber, highImpedenceButton.selected);
  }
  else {
    analogInterface->highImpedance(outputNumber, highImpedenceButton.selected);
  }

  if (analogInterface->highImpedance(inputNumber)) {
    [highImpedenceButton setTitle:@"Inst" forState:UIControlStateNormal];
    [highImpedenceButton setBackgroundColor:[UIColor blueColor]];
  }
  else {
    [highImpedenceButton setTitle:@"Line" forState:UIControlStateNormal];
    [highImpedenceButton setBackgroundColor:[UIColor grayColor]];
  }
}
/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

@end
