/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICMixerSelector.h"
#import "DeviceInfo.h"
#import "AudioGlobalParm.h"
#import "PureLayout.h"
#import "ICMixerViewController.h"


//64bit conversion. zx, 2017-07-28
#define UIColorFromRGB(rgbValue) [UIColor colorWithRed:(CGFloat)(((CGFloat)((rgbValue & 0xFF0000) >> 16))/255.0) green:(CGFloat)(((CGFloat)((rgbValue & 0xFF00) >> 8))/255.0) blue:(CGFloat)(((CGFloat)(rgbValue & 0xFF))/255.0) alpha:1.0]
/*
#define UIColorFromRGB(rgbValue) [UIColor colorWithRed:((float)((rgbValue & 0xFF0000) >> 16))/255.0 green:((float)((rgbValue & 0xFF00) >> 8))/255.0 blue:((float)(rgbValue & 0xFF))/255.0 alpha:1.0]
*/
using namespace GeneSysLib;

@implementation ICMixerSelector

- (id)initWithCommunicator:(CommPtr)m_comm device:(DeviceInfoPtr)m_device currentAudioPortID:(Word)m_audioPortID currentOutputNumber:(Byte)m_outputNumber mvc:(ICMixerViewController*)mvc {
  self = [super init];
  if (self) {
    self->device = m_device;
    self->comm = m_comm;

    self->audioPortID = m_audioPortID;
    self->outputNumber = m_outputNumber;

    self->mixerInterface = new MixerInterface(device);
    self->mixerInputInterface = new MixerInputInterface(device);
    self->mixerOutputInterface = new MixerOutputInterface(device);

    self->mixerGroup = [[ICMixerMixerGroup alloc] initWithCommunicator:comm device:device mvc:mvc];

    self->topMVC = mvc;

    mixerGroup.translatesAutoresizingMaskIntoConstraints = NO;
    [mixerGroup setUserInteractionEnabled:YES];

    self.translatesAutoresizingMaskIntoConstraints = NO;

    mixerButtonsView = [UIView new];
    mixerButtonsView.translatesAutoresizingMaskIntoConstraints = NO;

    mixerLabel = [[UILabel alloc] init];
    [mixerLabel setTextColor:[UIColor blackColor]];
    [mixerLabel setText:@"Sub Mix / Bussing"];
    [mixerLabel setFont:[UIFont boldSystemFontOfSize:22.0]];

    mixerButtons = [[NSMutableArray alloc] init];

    const auto &globalParm = device->get<AudioGlobalParm>();

    BOOL mixerGroupInitialized = NO;

    int count = 0;
    for (int i = 1; i <= globalParm.numAudioPorts(); i++) {
      int numOutputs = mixerInterface->numberOutputs(i);
      for (int j = 1; j <= numOutputs; j+=2) {
        UIButton* button = [UIButton new];
        button.translatesAutoresizingMaskIntoConstraints = NO;
        NSString *name = [NSString stringWithCString:mixerInterface->mixName(i, j).c_str() encoding:NSUTF8StringEncoding];

        NSString *prefix = @"";
        if (i == 1) {
          prefix = @"U1";
        }
        else if (i == 2) {
          prefix = @"U2";
        }
        else {
          prefix = @"A";
        }

        name = [NSString stringWithFormat:@"%@: %@", prefix, name];

        [button setTitle:name forState:UIControlStateNormal];
        [button.titleLabel setFont:[UIFont systemFontOfSize:14.0]];
        button.tag = (i << 8) | j;

        int whichColour = (count % 5);
        UIColor *color;
        switch (whichColour) {
          case 0:
            color = UIColorFromRGB(0xda3b77);
            break;
          case 1:
            color = UIColorFromRGB(0xfffea2);
            break;
          case 2:
            color = UIColorFromRGB(0x21bda2);
            break;
          case 3:
            color = UIColorFromRGB(0xe13939);
            break;
          case 4:
            color = UIColorFromRGB(0x7bff65);
            break;
          default:
            color = UIColorFromRGB(0xffffff);
            break;
        }

        color = [UIColor grayColor];

        if (!mixerGroupInitialized) {
          [mixerGroup setMixerAudioPort:i outputNumber:j];
          mixerGroupInitialized = YES;
          [button.titleLabel setFont:[UIFont boldSystemFontOfSize:14.0]];
          color = [UIColor blueColor];
        }

        [button setBackgroundColor:color];
        [button setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];

        [button addTarget:self action:@selector(mixChanged:) forControlEvents:UIControlEventTouchUpInside];

        [mixerButtons addObject:button];
        [mixerButtonsView addSubview:button];
        [button autoAlignAxisToSuperviewAxis:ALAxisHorizontal];
        count++;
      }
    }
    [mixerButtons autoDistributeViewsAlongAxis:ALAxisHorizontal alignedTo:ALAttributeTop withFixedSpacing:2 insetSpacing:YES matchedSizes:YES];
  }
  return self;
}

- (void) mixChanged:(id)button {
  UIButton *but = (UIButton*)button;
  Word apid = but.tag >> 8;
  Byte outn = but.tag & 0xff;

  for (UIButton* but2 in mixerButtons) {
    [but2.titleLabel setFont:[UIFont systemFontOfSize:14.0]];
    [but2 setBackgroundColor:[UIColor grayColor]];
  }

  [but.titleLabel setFont:[UIFont boldSystemFontOfSize:14.0]];
  [but setBackgroundColor:[UIColor blueColor]];

  [mixerGroup setMixerAudioPort:apid outputNumber:outn];
  [mixerGroup redraw];
}

-(UIColor*) inverseColor:(UIColor*) color
{
  CGFloat r,g,b,a;
  [color getRed:&r green:&g blue:&b alpha:&a];
  return [UIColor colorWithRed:1.-r green:1.-g blue:1.-b alpha:a];
}

- (void) updateMeters {
  [self->mixerGroup updateMeters];
}

- (int) postReadyInit {
  [self addSubview:mixerLabel];
  [self addSubview:mixerButtonsView];
  [self addSubview:mixerGroup];

  [[NSArray arrayWithObjects:mixerLabel, mixerButtonsView, mixerGroup, nil] autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeLeft withFixedSpacing:4 insetSpacing:YES matchedSizes:NO];

  NSArray *constraint_POS_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[mixerGroup]|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(mixerGroup)];
  NSArray *constraint_POS_H2 = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[mixerButtonsView]|"
                                                                      options:0
                                                                      metrics:nil
                                                                        views:NSDictionaryOfVariableBindings(mixerButtonsView)];
  [self addConstraints:constraint_POS_H];
  [self addConstraints:constraint_POS_H2];

  NSArray *mixerConstraints_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[mixerLabel(25)]"
                                                                         options:0
                                                                         metrics:nil
                                                                           views:NSDictionaryOfVariableBindings(mixerLabel)];

  NSArray *mixerConstraints_V2 = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[mixerButtonsView(30)]"
                                                                          options:0
                                                                          metrics:nil
                                                                            views:NSDictionaryOfVariableBindings(mixerButtonsView)];
  [self addConstraints:mixerConstraints_V];
  [self addConstraints:mixerConstraints_V2];
  return [mixerGroup postReadyInit];
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

@end
