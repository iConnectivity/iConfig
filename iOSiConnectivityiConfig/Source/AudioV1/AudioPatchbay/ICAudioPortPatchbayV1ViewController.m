/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "AudioCfgInfo.h"
#import "AudioInfo.h"
#import "AudioInfo.h"
#import "AudioPortCfgInfo.h"
#import "AudioPortInfo.h"
#import "AudioPortPatchbay.h"
#import "ICAudioPatchbayV1InputSelection.h"
#import "ICAudioPatchbayV1OutputSelection.h"
#import "ICAudioPortPatchbayV1ViewController.h"
#import "ICRunOnMain.h"
#import "MyAlgorithms.h"

#import <vector>
#import <tuple>
#import <boost/range/adaptors.hpp>

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace boost::adaptors;
using namespace std;

enum SourceChannelsEnum {
  SourceChannelTitle,
  SourceChannelPortID,
  SourceChannelChannel
};

@interface ICAudioPortPatchbayV1ViewController () {
  // vector of (title, portID, ch)
  vector<tuple<NSString *, Word, Byte> > sourceChannels;
}

@end
@implementation ICAudioPortPatchbayV1ViewController
- (id)initWithCommunicator:(GeneSysLib::CommPtr)_comm
                    device:(DeviceInfoPtr)_device
              flatPatchbay:(GeneSysLib::FlatAudioPortPatchbay)_flatPatchbay {
  self =
      [super initWithNibName:@"ICAudioPortPatchbayV1ViewController" bundle:nil];

  if (self) {
    comm = _comm;
    device = _device;
    selectedFlatPortPatchbay = _flatPatchbay;

    // generate the source channels vector
    // add the Nothing to the begining
    sourceChannels.push_back(make_tuple(@"Nothing", 0, 0));

    device->for_each<AudioPortCfgInfo>([&](
        const AudioPortCfgInfo &portCfgInfo) {
      Word portID = portCfgInfo.portID();

      if (device->contains<AudioPortInfo>(portID)) {
        const auto *const portName =
            device->get<AudioPortInfo>(portID).portName().c_str();

        for (Word ch = 1; ch <= portCfgInfo.numOutputChannels(); ++ch) {
          sourceChannels.push_back(
              make_tuple([NSString stringWithFormat:@"%s Ch %d", portName, ch],
                         portCfgInfo.portID(), ch));
        }
      }
    });
  }

  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  assert(comm);
  assert(device);
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [self.navigationController setNavigationBarHidden:FALSE animated:true];
  if ([self.tableView
          respondsToSelector:@selector(setEstimatedSectionHeaderHeight:)]) {
    [self.tableView setEstimatedSectionHeaderHeight:0];
  }
  [self.navigationItem setTitle:@"Patch Selection"];

  NSString *nameStr =
      @(device->get<AudioPortInfo>(selectedFlatPortPatchbay.inPortID)
            .portName()
            .c_str());
  NSString *labelText =
      [NSString stringWithFormat:@"%@ Ch %d", nameStr,
                                 selectedFlatPortPatchbay.inChannelNumber];
  [self.destLabel setText:labelText];
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
  // dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
  return 1;
}

- (NSInteger)tableView:(UITableView *)tableView
    numberOfRowsInSection:(NSInteger)section {
  // Return the number of rows in the section.
  return sourceChannels.size();
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  static NSString *CellIdentifier = @"Cell";

  const auto &sourceCh = sourceChannels.at(indexPath.row);
  auto *const title = std::get<SourceChannelTitle>(sourceCh);
  const auto &outPort = std::get<SourceChannelPortID>(sourceCh);
  const auto &outCh = std::get<SourceChannelChannel>(sourceCh);

  UITableViewCell *cell =
      [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
  if (!cell) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                  reuseIdentifier:CellIdentifier];
  }

  const auto isSelected =
      ((selectedFlatPortPatchbay.outPortID == outPort) &&
       (selectedFlatPortPatchbay.outChannelNumber == outCh));

  cell.textLabel.text = title;
  cell.accessoryType = ((isSelected) ? (UITableViewCellAccessoryCheckmark)
                                     : (UITableViewCellAccessoryNone));
  return cell;
}

#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView
    didSelectRowAtIndexPath:(NSIndexPath *)indexPath {

  // set the source of the flat map to the new selection
  const auto &sourceCh = sourceChannels.at(indexPath.row);
  const auto &outPort = std::get<SourceChannelPortID>(sourceCh);
  const auto &outCh = std::get<SourceChannelChannel>(sourceCh);

  selectedFlatPortPatchbay.outPortID = outPort;
  selectedFlatPortPatchbay.outChannelNumber = outCh;

  assert(
      device->contains<AudioPortPatchbay>(selectedFlatPortPatchbay.inPortID));

  auto &audioPatchbay =
      device->get<AudioPortPatchbay>(selectedFlatPortPatchbay.inPortID);
  try {
    auto &block =
        audioPatchbay.findInputBlock(selectedFlatPortPatchbay.inChannelNumber);

    block.portIDOfOutput(selectedFlatPortPatchbay.outPortID);
    block.outputChannelNumber(selectedFlatPortPatchbay.outChannelNumber);

    device->send<SetAudioPortPatchbayCommand>(audioPatchbay);
  }
  catch (blockNotFoundException e) {
    // could not find the data
  }
  [self.navigationController popViewControllerAnimated:YES];
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

@end
