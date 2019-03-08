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
#import "ICButtonCellProvider.h"
#import "ICAppDelegate.h"

@implementation ICBaseInfoViewController

- (id)init {
  self = [super initWithNibName:@"ICBaseInfoViewController" bundle:nil];
  if (self) {
    // Custom initialization
    sectionArrays = [NSMutableArray array];
    sectionTitles = [NSMutableArray array];
  }

  self.definesPresentationContext = YES;

  return self;
}

- (void)viewDidLoad {
  //NSLog(@"viewDidLoad");
  [super viewDidLoad];

  super.tableView = self.myTableView;
}

- (void)viewWillAppear:(BOOL)animated {
  //NSLog(@"viewWillAppear");
  [super viewWillAppear:animated];
  [self.navigationController setNavigationBarHidden:FALSE animated:true];
  if ([self.tableView
          respondsToSelector:@selector(setEstimatedSectionHeaderHeight:)]) {
    [self.tableView setEstimatedSectionHeaderHeight:0];
  }
  NSLog(@"BaseInfoView: viewWillAppear");
}

- (void) refreshInfo {
  NSLog(@"refreshInfo");
  [self.myTableView reloadData];
  [self.myTableView deselectRowAtIndexPath:[self.myTableView indexPathForSelectedRow] animated:NO];

}

#pragma mark - Table View Data Source
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
  //NSLog(@"numberOfSectionsInTableView");
  return [sectionArrays count];
}

- (NSInteger)tableView:(UITableView *)tableView
    numberOfRowsInSection:(NSInteger)section {
  //NSLog(@"numberOfRowsInSection");
  return [sectionArrays[section] count];
}

- (NSString *)tableView:(UITableView *)tableView
    titleForHeaderInSection:(NSInteger)section {
  //NSLog(@"titleForHeaderInSection");
  return sectionTitles[section];
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  //NSLog(@"cellForRowAtIndexPath");
  NSArray *const sectionArray = sectionArrays[indexPath.section];

  assert([sectionArray[indexPath.row]
      respondsToSelector:@selector(cellForTableView:)]);

  return [sectionArray[indexPath.row] cellForTableView:tableView];
}

#pragma mark - Table View Delegate

- (void)tableView:(UITableView *)tableView
    didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  ICAppDelegate *appDelegate = (ICAppDelegate *)[[UIApplication sharedApplication] delegate];

  //NSLog(@"didSelectRowAtIndexPath");
  NSArray *const sectionArray = sectionArrays[indexPath.section];

  if ([sectionArray[indexPath.row] isKindOfClass:[ICRangeCellProvider class]]) {
    ICRangeCellProvider *cellProvider =
        (ICRangeCellProvider *)sectionArray[indexPath.row];

    ICRangeSelectionViewController *rangeSelectionViewController =
        [[ICRangeSelectionViewController alloc]
            initWithDelegate:cellProvider.rangeDelegate];

    [[appDelegate getNavigationController] pushViewController:rangeSelectionViewController
                                         animated:YES];

  } else if ([sectionArray[indexPath.row]
                 isKindOfClass:[ICChoiceCellProvider class]]) {
    ICChoiceCellProvider *choiceProvider =
        (ICChoiceCellProvider *)(sectionArray[indexPath.row]);

    ICChoiceSelectionViewController *choiceSelectionView =
        [[ICChoiceSelectionViewController alloc]
            initWithDelegate:choiceProvider.choiceDelegate];

    [[appDelegate getNavigationController] pushViewController:choiceSelectionView
                                         animated:YES];
  }
  else if ([sectionArray[indexPath.row] isKindOfClass:[ICButtonCellProvider class]]) {
    ICButtonCellProvider *buttonProvider =
      (ICButtonCellProvider *)(sectionArray[indexPath.row]);

    buttonProvider.buttonDelegate.func();
  }
}

- (CGFloat)tableView:(UITableView *)tableView
    heightForFooterInSection:(NSInteger)section {
  //NSLog(@"tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section");
  return 1.0;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:
            (UIInterfaceOrientation)interfaceOrientation {
  //NSLog(@"shouldAutorotateToInterfaceOrientation");
  const auto &isiPad = ([[UIDevice currentDevice] userInterfaceIdiom] ==
                        UIUserInterfaceIdiomPad);
  const auto &isPortait =
      UIInterfaceOrientationIsPortrait(interfaceOrientation);

  // should only rotate with an iPad or if the orientation is portrait
  return (isiPad || isPortait);
}

@end
