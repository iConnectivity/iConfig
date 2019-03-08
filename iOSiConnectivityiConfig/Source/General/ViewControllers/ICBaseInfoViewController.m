/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICBaseInfoViewController.h"
#import "ICChoiceCellProvider.h"
#import "ICChoiceSelectionViewController.h"
#import "ICNormalCellProvider.h"
#import "ICRangeCellProvider.h"
#import "ICRangeSelectionViewController.h"
#import "ICTextEditCellProvider.h"

@implementation ICBaseInfoViewController

- (id)init {
  self = [super initWithNibName:@"ICBaseInfoViewController" bundle:nil];
  if (self) {
    // Custom initialization
    sectionArrays = [NSMutableArray array];
    sectionTitles = [NSMutableArray array];
  }

  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  super.tableView = self.myTableView;
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [self.navigationController setNavigationBarHidden:FALSE animated:true];
  if ([self.tableView
          respondsToSelector:@selector(setEstimatedSectionHeaderHeight:)]) {
    [self.tableView setEstimatedSectionHeaderHeight:0];
  }
}

#pragma mark - Table View Data Source
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
  return [sectionArrays count];
}

- (NSInteger)tableView:(UITableView *)tableView
    numberOfRowsInSection:(NSInteger)section {
  return [sectionArrays[section] count];
}

- (NSString *)tableView:(UITableView *)tableView
    titleForHeaderInSection:(NSInteger)section {
  return sectionTitles[section];
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  NSArray *const sectionArray = sectionArrays[indexPath.section];

  assert([sectionArray[indexPath.row]
      respondsToSelector:@selector(cellForTableView:)]);

  return [sectionArray[indexPath.row] cellForTableView:tableView];
}

#pragma mark - Table View Delegate

- (void)tableView:(UITableView *)tableView
    didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  NSArray *const sectionArray = sectionArrays[indexPath.section];

  if ([sectionArray[indexPath.row] isKindOfClass:[ICRangeCellProvider class]]) {
    ICRangeCellProvider *cellProvider =
        (ICRangeCellProvider *)sectionArray[indexPath.row];

    ICRangeSelectionViewController *rangeSelectionViewController =
        [[ICRangeSelectionViewController alloc]
            initWithDelegate:cellProvider.rangeDelegate];

    [self.navigationController pushViewController:rangeSelectionViewController
                                         animated:YES];

  } else if ([sectionArray[indexPath.row]
                 isKindOfClass:[ICChoiceCellProvider class]]) {
    ICChoiceCellProvider *choiceProvider =
        (ICChoiceCellProvider *)(sectionArray[indexPath.row]);

    ICChoiceSelectionViewController *choiceSelectionView =
        [[ICChoiceSelectionViewController alloc]
            initWithDelegate:choiceProvider.choiceDelegate];

    [self.navigationController pushViewController:choiceSelectionView
                                         animated:YES];
  }
}

- (CGFloat)tableView:(UITableView *)tableView
    heightForFooterInSection:(NSInteger)section {
  return 1.0;
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
