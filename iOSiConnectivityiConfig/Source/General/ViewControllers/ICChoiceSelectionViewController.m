/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICChoiceSelectionViewController.h"

@interface ICChoiceSelectionViewController ()

@end

@implementation ICChoiceSelectionViewController

- (id)initWithDelegate:(id<ICChoiceDelegate>)rangeDelegate {
  self = [super initWithNibName:@"ICChoiceSelectionViewController" bundle:nil];

  if (self) {
    NSParameterAssert(rangeDelegate);
    self.rangeDelegate = rangeDelegate;
  }

  return self;
}

- (void)viewDidUnload {
  [self setMyTable:nil];
  [super viewDidUnload];
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  [self.navigationController setNavigationBarHidden:NO animated:YES];
}

- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];

  NSInteger choice = [self.rangeDelegate getChoice];
  NSIndexPath *chosenIndex = [NSIndexPath indexPathForRow:choice inSection:0];

  [self.myTable scrollToRowAtIndexPath:chosenIndex
                      atScrollPosition:UITableViewScrollPositionMiddle
                              animated:animated];
}

- (NSInteger)tableView:(UITableView *)tableView
    numberOfRowsInSection:(NSInteger)section {
  return [self.rangeDelegate optionCount];
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

  NSString *option = (NSString *)([self.rangeDelegate options][indexPath.row]);
  [cell.textLabel setText:option];

  NSInteger choice = [self.rangeDelegate getChoice];

  cell.accessoryType =
      ((indexPath.row == choice) ? (UITableViewCellAccessoryCheckmark)
                                 : (UITableViewCellAccessoryNone));

  return cell;
}

- (void)tableView:(UITableView *)tableView
    didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  static auto handling = false;
  if (!handling) {
    handling = true;

    [self.rangeDelegate setChoice:indexPath.row];

    [self.navigationController popViewControllerAnimated:YES];
  }
  handling = false;
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
