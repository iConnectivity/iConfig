/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICRangeSelectionViewController.h"
#import "ICViewController.h"
#import "UINavigationController+CompletionHandler.h"

@interface ICRangeSelectionViewController ()
//@property(strong, nonatomic) id<ICRangeChoiceDelegate> rangeDelegate;
@property(strong, nonatomic) NSArray *rangeArray;
@end

@implementation ICRangeSelectionViewController

+ (ICRangeSelectionViewController *)
    rangeSelectionWithDelegate:(id<ICRangeChoiceDelegate>)rangeDelegate {
  return
      [[ICRangeSelectionViewController alloc] initWithDelegate:rangeDelegate];
}

- (id)initWithDelegate:(id<ICRangeChoiceDelegate>)rangeDelegate {
  //self = [super init];
  self = [super initWithNibName:@"ICRangeSelectionViewController" bundle:nil];

  if (self) {
    NSParameterAssert(rangeDelegate);
    self.rangeDelegate = rangeDelegate;

    auto *const array = [NSMutableArray array];

    for (NSInteger i = [rangeDelegate getMin]; i <= [rangeDelegate getMax];
         i += [rangeDelegate getStride]) {
      [array addObject:[NSString stringWithFormat:@"%ld", (long)i]];
    }

    self.rangeArray = array;
  }

  return self;
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [self.navigationController setNavigationBarHidden:NO animated:YES];
}

- (NSInteger)tableView:(UITableView *)tableView
    numberOfRowsInSection:(NSInteger)section {
  return [self.rangeArray count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  static NSString *const identifierCell = @"CellIdentifier";

  UITableViewCell *cell =
      [tableView dequeueReusableCellWithIdentifier:identifierCell];
  if (cell == NULL) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                  reuseIdentifier:identifierCell];
  }

  [cell.textLabel setText:self.rangeArray[indexPath.row]];

  auto selectedIndex =
      [self.rangeDelegate getValue] - [self.rangeDelegate getMin];

  cell.accessoryType =
      ((indexPath.row == selectedIndex) ? (UITableViewCellAccessoryCheckmark)
                                        : (UITableViewCellAccessoryNone));

  return cell;
}

- (void)tableView:(UITableView *)tableView
    didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  [self.rangeDelegate setValue:(indexPath.row + [self.rangeDelegate getMin])];

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
