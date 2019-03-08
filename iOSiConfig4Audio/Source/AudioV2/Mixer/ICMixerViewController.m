/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "DeviceInfo.h"
#import "ICMixerViewController.h"
#import "AudioPortParm.h"
#import "ICAnalogInterface.h"
#import "PureLayout.h"

using namespace GeneSysLib;

@implementation ICMixerViewController

- (id)initWithCommunicator:(CommPtr)m_comm device:(DeviceInfoPtr)m_device {
  self = [super init];
  if (self) {
    self->device = m_device;
    self->comm = m_comm;

    self->inputGroup = [[ICMixerAnalogGroup alloc] initWithCommunicator:m_comm device:m_device isInput:true mvc:self];
    self->outputGroup = [[ICMixerAnalogGroup alloc] initWithCommunicator:m_comm device:m_device isInput:false mvc:self];
    self->mixerSelector = [[ICMixerSelector alloc] initWithCommunicator:m_comm device:m_device currentAudioPortID:3 currentOutputNumber:1 mvc:self];
  }
  return self;
}

- (void)resetView {
  for (UIView *view in [self.view subviews]) {
    [view removeFromSuperview];
  }

  self->inputGroup = [[ICMixerAnalogGroup alloc] initWithCommunicator:self->comm device:self->device isInput:true mvc:self];
  self->outputGroup = [[ICMixerAnalogGroup alloc] initWithCommunicator:self->comm device:self->device isInput:false mvc:self];
  self->mixerSelector = [[ICMixerSelector alloc] initWithCommunicator:self->comm device:self->device currentAudioPortID:3 currentOutputNumber:1 mvc:self];

  [self.view addSubview:self->mixerSelector];

  [self finishViewSetup];
}

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view, typically from a nib.

  [self.view setBackgroundColor:[UIColor whiteColor]];

  self.view.multipleTouchEnabled = NO;
  [self.view setUserInteractionEnabled:YES];
  self.view.translatesAutoresizingMaskIntoConstraints = NO;

//  [self.view addSubview:self->inputGroup];
  [self.view addSubview:self->mixerSelector];
//  [self.view addSubview:self->outputGroup];

}

- (void) setupInputView {
  inputView = [UIView new];
  inputView.translatesAutoresizingMaskIntoConstraints = NO;
  inputLabel = [[UILabel alloc] init];
  [inputLabel setTextColor:[UIColor blackColor]];
  [inputLabel setText:@"Analog Inputs"];
  [inputLabel setFont:[UIFont boldSystemFontOfSize:22.0]];
  inputSpacer = [UIView new];
  inputSpacer.translatesAutoresizingMaskIntoConstraints = NO;

  [inputView addSubview:inputLabel];
  [inputView addSubview:inputSpacer];
  [inputView addSubview:inputGroup];

  [[NSArray arrayWithObjects:inputLabel, inputSpacer, inputGroup, nil] autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeLeft withFixedSpacing:4 insetSpacing:YES matchedSizes:NO];

  NSArray *inputConstraints_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[inputLabel(25)]"
                                                                        options:0
                                                                        metrics:nil
                                                                          views:NSDictionaryOfVariableBindings(inputLabel)];

  NSArray *inputConstraints_V2 = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[inputSpacer(30)]"
                                                                          options:0
                                                                          metrics:nil
                                                                            views:NSDictionaryOfVariableBindings(inputSpacer)];


  NSArray *inputConstraints_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[inputGroup]|"
                                                                        options:0
                                                                        metrics:nil
                                                                          views:NSDictionaryOfVariableBindings(inputGroup)];

  [inputView addConstraints:inputConstraints_H];
  [inputView addConstraints:inputConstraints_V];
  [inputView addConstraints:inputConstraints_V2];

  [self.view addSubview:inputView];
}

- (void) updateMeters {
  if (self->inputGroup)
    [self->inputGroup updateMeters];
  if (self->mixerSelector)
    [self->mixerSelector updateMeters];
  if (self->outputGroup)
    [self->outputGroup updateMeters];
}

- (void) setupOutputView {
  outputView = [UIView new];
  outputView.translatesAutoresizingMaskIntoConstraints = NO;
  outputLabel = [[UILabel alloc] init];
  [outputLabel setTextColor:[UIColor blackColor]];
  [outputLabel setText:@"Analog outputs"];
  [outputLabel setFont:[UIFont boldSystemFontOfSize:22.0]];
  outputSpacer = [UIView new];
  outputSpacer.translatesAutoresizingMaskIntoConstraints = NO;

  [outputView addSubview:outputLabel];
  [outputView addSubview:outputSpacer];
  [outputView addSubview:outputGroup];

  [[NSArray arrayWithObjects:outputLabel, outputSpacer, outputGroup, nil] autoDistributeViewsAlongAxis:ALAxisVertical alignedTo:ALAttributeLeft withFixedSpacing:4 insetSpacing:YES matchedSizes:NO];

  NSArray *outputConstraints_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[outputLabel(25)]"
                                                                        options:0
                                                                        metrics:nil
                                                                          views:NSDictionaryOfVariableBindings(outputLabel)];

  NSArray *outputConstraints_V2 = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[outputSpacer(30)]"
                                                                         options:0
                                                                         metrics:nil
                                                                           views:NSDictionaryOfVariableBindings(outputSpacer)];


  NSArray *outputConstraints_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[outputGroup]|"
                                                                        options:0
                                                                        metrics:nil
                                                                          views:NSDictionaryOfVariableBindings(outputGroup)];

  [outputView addConstraints:outputConstraints_H];
  [outputView addConstraints:outputConstraints_V];
  [outputView addConstraints:outputConstraints_V2];

  [self.view addSubview:outputView];
}

- (void) finishViewSetup {
  int numIn = [inputGroup postReadyInit];
  int numMixer = [mixerSelector postReadyInit];
  int numOut = [outputGroup postReadyInit];

  [self setupInputView];
  [self setupOutputView];

  int mixerWidth = floor((self.view.bounds.size.width - 16 - 132) / (double)(numIn + numMixer + numOut));
  //NSLog(@"mixerWidth is: %d", mixerWidth);

  NSArray *constraint_MIXER_V1 = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[inputView]-|"
                                                                         options:0
                                                                         metrics:nil
                                                                           views:NSDictionaryOfVariableBindings(mixerSelector,
                                                                                                                inputView,
                                                                                                                outputGroup)];
  NSArray *constraint_MIXER_V2 = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[outputView]-|"
                                                                         options:0
                                                                         metrics:nil
                                                                           views:NSDictionaryOfVariableBindings(mixerSelector,
                                                                                                                outputView,
                                                                                                                outputGroup)];
  NSArray *constraint_MIXER_V3 = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[mixerSelector]-|"
                                                                         options:0
                                                                         metrics:nil
                                                                           views:NSDictionaryOfVariableBindings(mixerSelector,
                                                                                                                inputGroup,
                                                                                                                outputGroup)];

  NSArray *constraint_MIXER_H = [NSLayoutConstraint constraintsWithVisualFormat:
                                 @"H:|[inputView(inputSize)]-8-[mixerSelector(mixerSize)]-8-[outputView(outputSize)]|"
                                                                        options:0
                                                                        metrics:@{@"inputSize": [NSNumber numberWithInt:numIn*mixerWidth],
                                                                                  @"outputSize": [NSNumber numberWithInt:numOut*mixerWidth],
                                                                                  @"mixerSize": [NSNumber numberWithInt:numMixer*mixerWidth+132]}
                                                                          views:NSDictionaryOfVariableBindings(mixerSelector,
                                                                                                               inputView,
                                                                                                               outputView)];


  [self.view addConstraints:constraint_MIXER_H];
  [self.view addConstraints:constraint_MIXER_V1];
  [self.view addConstraints:constraint_MIXER_V2];
  [self.view addConstraints:constraint_MIXER_V3];
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [self.navigationController setNavigationBarHidden:NO animated:YES];

  [self finishViewSetup];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
