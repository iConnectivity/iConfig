/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICAudioPatchbayV1ViewController.h"
#import "ICAudioPatchbayV1Cell.h"

#import "AudioCfgInfo.h"
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
#import <boost/range/adaptors.hpp>

using namespace GeneSysLib;
using namespace MyAlgorithms;
using namespace boost::adaptors;
using namespace std;

@interface ICAudioPatchbayV1ViewController () {
  bool isiPhone;
  vector<FlatAudioPortPatchbay> flatPatchbay;
  vector<int> dirtyIndices;
}
@end

@implementation ICAudioPatchbayV1ViewController

- (id)initWithCommunicator:(CommPtr)_comm device:(DeviceInfoPtr)_device {
  isiPhone = ([[UIDevice currentDevice] userInterfaceIdiom] ==
              UIUserInterfaceIdiomPhone);

  self = [super initWithNibName:@"ICAudioPatchbayV1ViewController" bundle:nil];
  if (self) {
    NSParameterAssert(_comm);
    NSParameterAssert(_device);

    // custom initialization
    comm = _comm;
    device = _device;

    device->for_each<AudioPortPatchbay>([&](const AudioPortPatchbay &patchbay) {
      const auto &flatPortPatchbay = patchbay.flatList();
      flatPatchbay.insert(flatPatchbay.end(), flatPortPatchbay.begin(),
                          flatPortPatchbay.end());
    });
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  assert(comm);
  assert(device);

  if (isiPhone) {
    UINib *nib = [UINib nibWithNibName:@"ICAudioPatchbayV1iPhoneCell"
                                bundle:[NSBundle mainBundle]];
    [self.tableView registerNib:nib
         forCellReuseIdentifier:@"ICAudioPatchbayCell"];
  } else {
    UINib *nib = [UINib nibWithNibName:@"ICAudioPatchbayV1iPadCell"
                                bundle:[NSBundle mainBundle]];
    [self.tableView registerNib:nib
         forCellReuseIdentifier:@"ICAudioPatchbayCell"];
  }
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [self.navigationController setNavigationBarHidden:FALSE animated:true];
  if ([self.tableView
          respondsToSelector:@selector(setEstimatedSectionHeaderHeight:)]) {
    [self.tableView setEstimatedSectionHeaderHeight:0];
  }

  [self.navigationItem setTitle:@"Audio Patchbay"];

  // update the dirty elements
  for (const auto &i : dirtyIndices) {
    auto &current = flatPatchbay.at(i);

    if (device->contains<AudioPortPatchbay>(current.inPortID)) {
      const auto &patchBay = device->get<AudioPortPatchbay>(current.inPortID);

      current = patchBay.flatPatchbay(current.inChannelNumber);
    }
  }
  dirtyIndices.clear();

  // redraw the table
  [self.tableView reloadData];
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
  return flatPatchbay.size();
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  static NSString *CellIdentifier = @"ICAudioPatchbayCell";

  UITableViewCell *cell =
      [tableView dequeueReusableCellWithIdentifier:CellIdentifier
                                      forIndexPath:indexPath];

  assert(cell);

  // configure the cell...
  if ([cell isKindOfClass:[ICAudioPatchbayV1Cell class]]) {
    auto *const audioCell = (ICAudioPatchbayV1Cell *)cell;
    const auto &flatPortPatchbay = flatPatchbay.at(indexPath.row);

    bool hasInPort = device->contains<AudioPortInfo>(flatPortPatchbay.inPortID);
    bool hasOutPort =
        device->contains<AudioPortInfo>(flatPortPatchbay.outPortID);

    auto inPortName =
        ((hasInPort) ? (device->get<AudioPortInfo>(flatPortPatchbay.inPortID)
                            .portName())
                     : (string("Nothing")));
    auto outPortName =
        ((hasOutPort) ? (device->get<AudioPortInfo>(flatPortPatchbay.outPortID)
                             .portName())
                      : (string("Nothing")));

    if ((inPortName.length() != 0) && (flatPortPatchbay.inChannelNumber != 0)) {
      audioCell.destLabel.text =
          [NSString stringWithFormat:@"%s Ch %d", inPortName.c_str(),
                                     flatPortPatchbay.inChannelNumber];
    } else {
      audioCell.destLabel.text = @"Nothing";
    }

    if ((outPortName.length() != 0) &&
        (flatPortPatchbay.outChannelNumber != 0)) {
      audioCell.sourceLabel.text =
          [NSString stringWithFormat:@"%s Ch %d", outPortName.c_str(),
                                     flatPortPatchbay.outChannelNumber];
    } else {
      audioCell.sourceLabel.text = @"Nothing";
    }
  }

  return cell;
}

- (UIView *)tableView:(UITableView *)tableView
    viewForHeaderInSection:(NSInteger)section {
  return self.headerView;
}

#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView
    didSelectRowAtIndexPath:(NSIndexPath *)indexPath {

  // add dirty index
  dirtyIndices.push_back(indexPath.row);

  // create the next view controller
  ICAudioPortPatchbayV1ViewController *portPatchbayViewController =
      [[ICAudioPortPatchbayV1ViewController alloc]
          initWithCommunicator:comm
                        device:device
                  flatPatchbay:flatPatchbay.at(indexPath.row)];

  // push the view controller
  [self.navigationController pushViewController:portPatchbayViewController
                                       animated:YES];
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
