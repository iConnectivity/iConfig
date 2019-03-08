/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICMIDIIndexTableViewController.h"
#import "MMSideDrawerTableViewCell.h"
#import "MMSideDrawerSectionHeaderView.h"
#import "UIViewController+MMDrawerController.h"

using namespace GeneSysLib;

@interface ICMIDIIndexTableViewController () {
  NSArray *sections;
}

@end

@implementation ICMIDIIndexTableViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  sections = @[];

  [self.tableView setAutoresizingMask:UIViewAutoresizingFlexibleWidth |
                                      UIViewAutoresizingFlexibleHeight];
  [self.tableView setSeparatorColor:[UIColor colorWithRed:49.0 / 255.0
                                                    green:54.0 / 255.0
                                                     blue:57.0 / 255.0
                                                    alpha:1.0]];

  [self.tableView setBackgroundColor:[UIColor colorWithRed:77.0 / 255.0
                                                     green:79.0 / 255.0
                                                      blue:80.0 / 255.0
                                                     alpha:1.0]];

  [self.view setBackgroundColor:[UIColor colorWithRed:66.0 / 255.0
                                                green:69.0 / 255.0
                                                 blue:71.0 / 255.0
                                                alpha:1.0]];

  [self.view setBackgroundColor:[UIColor clearColor]];
}

- (void)rebuildWithMIDIInfo:(const MIDIInfo &)midiInfo {
  const auto &containsDINS = (midiInfo.numDINPairs() > 0);
  const auto &containsUSBDevice = (midiInfo.numUSBDeviceJacks() > 0);
  const auto &containsUSBHost = (midiInfo.numUSBHostJacks() > 0);
  const auto &containsEthernet = (midiInfo.numEthernetJacks() > 0);

  auto *const tempSections = [NSMutableArray array];

  [tempSections addObject:@{
                            @"title" : @"MIDI Info",
                            @"array" : @[ @"General Information" ]
                          }];

  if (containsDINS) {
    auto *const array = [NSMutableArray array];
    for (int i = 1; i <= midiInfo.numDINPairs(); ++i) {
      [array addObject:[NSString stringWithFormat:@"DIN %d", i]];
    }

    [tempSections addObject:@{@"title" : @"DIN Ports", @"array" : array}];
  }

  if (containsUSBDevice) {
    auto *const array = [NSMutableArray array];
    for (int jack = 1; jack <= midiInfo.numUSBDeviceJacks(); ++jack) {
      for (int port = 1; port <= midiInfo.numUSBMIDIPortPerDeviceJack();
           ++port) {
        [array
            addObject:[NSString stringWithFormat:@"USB Device Jack %d Port %d",
                                                 jack, port]];
      }
    }
    [tempSections
        addObject:@{@"title" : @"USB Device Ports", @"array" : array}];
  }

  if (containsUSBHost) {
    auto *const array = [NSMutableArray array];
    for (int jack = 1; jack <= midiInfo.numUSBHostJacks(); ++jack) {
      for (int port = 1; port <= midiInfo.numUSBMIDIPortPerHostJack(); ++port) {
        [array addObject:[NSString stringWithFormat:@"USB Host Jack %d Port %d",
                                                    jack, port]];
      }
    }
    [tempSections addObject:@{@"title" : @"USB Host Ports", @"array" : array}];
  }

  if (containsEthernet) {
    auto *const array = [NSMutableArray array];
    for (int jack = 1; jack <= midiInfo.numEthernetJacks(); ++jack) {
      for (int session = 1;
           session <= midiInfo.numRTPMIDISessionsPerEthernetJack(); ++session) {
        [array
            addObject:[NSString stringWithFormat:@"Ethernet Jack %d Session %d",
                                                 jack, session]];
      }
    }

    [tempSections addObject:@{@"title" : @"Ethernet Ports", @"array" : array}];
  }

  sections = tempSections;
  [self.tableView reloadData];
}

- (void)clear {
  sections = @[];
  [self.tableView reloadData];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
  return [sections count];
}

- (NSInteger)tableView:(UITableView *)tableView
    numberOfRowsInSection:(NSInteger)section {
  NSDictionary *const dictionary = [sections objectAtIndex:section];
  return ((dictionary != nil) ? ([[dictionary objectForKey:@"array"] count])
                              : (0));
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  static NSString *const CellIdentifier = @"Cell";

  UITableViewCell *cell = (UITableViewCell *)
      [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
  if (cell == nil) {
    cell = [[MMSideDrawerTableViewCell alloc]
          initWithStyle:UITableViewCellStyleDefault
        reuseIdentifier:CellIdentifier];
    [cell setSelectionStyle:UITableViewCellSelectionStyleBlue];
  }

  NSDictionary *const dictionary = [sections objectAtIndex:indexPath.section];

  cell.textLabel.text = @"";

  if ((dictionary) && ([dictionary objectForKey:@"array"])) {
    NSArray *const array = [dictionary objectForKey:@"array"];
    if ((array) && (indexPath.row <= [array count])) {
      cell.textLabel.text = [array objectAtIndex:indexPath.row];
    }
  }

  return cell;
}

- (NSString *)tableView:(UITableView *)tableView
    titleForHeaderInSection:(NSInteger)section {
  const NSDictionary *const dictionary = [sections objectAtIndex:section];

  return [dictionary objectForKey:@"title"];
}

- (UIView *)tableView:(UITableView *)tableView
    viewForHeaderInSection:(NSInteger)section {
  auto *const headerView = [[MMSideDrawerSectionHeaderView alloc]
      initWithFrame:CGRectMake(0, 0, CGRectGetWidth(tableView.bounds), 20.0f)];
  [headerView setAutoresizingMask:UIViewAutoresizingFlexibleHeight |
                                  UIViewAutoresizingFlexibleWidth];
  [headerView setTitle:[tableView.dataSource tableView:tableView
                               titleForHeaderInSection:section]];
  return headerView;
}

- (CGFloat)tableView:(UITableView *)tableView
    heightForHeaderInSection:(NSInteger)section {
  return 23.0;
}

- (CGFloat)tableView:(UITableView *)tableView
    heightForRowAtIndexPath:(NSIndexPath *)indexPath {
  return 40.0;
}

- (void)tableView:(UITableView *)tableView
    didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  [tableView deselectRowAtIndexPath:indexPath animated:YES];

  if (self.sidebarDelegate) {
    [self.sidebarDelegate indexPathSelected:indexPath];
  }
  [self.mm_drawerController closeDrawerAnimated:YES completion:nil];
}

- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];
}

- (void)viewDidDisappear:(BOOL)animated {
  [super viewDidDisappear:animated];
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
