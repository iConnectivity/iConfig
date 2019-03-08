/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICMenuViewController.h"

@interface ICMenuViewController ()

@end

@implementation ICMenuViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  itemsArray = [[NSMutableArray alloc] initWithObjects:@"Device Functions", @"Help", @"Disconnect", nil];

  [_menuTableView setDataSource:self];
  [_menuTableView setDelegate:self];

  [_menuTableView setContentInset:UIEdgeInsetsMake(50,0,0,0)];

    // Do any additional setup after loading the view from its nib.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)setICViewController:(ICViewController*)ic {
  self->ICvc = ic;
  [_menuTableView reloadData];
}

- (void)tableView:(UITableView *)aTableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  int which = indexPath.row;
  switch (which) {
    case 0:
      if (self->ICvc) {
        [self->ICvc deviceFunctionsPressed];
      }
      break;
    case 1:
      if (self->ICvc) {
        [self->ICvc helpButtonPressed:nil];
      }
      break;
    case 2:
      if (self->ICvc) {
        [self->ICvc onSelectDevice:nil];
      }
      break;
    default:
      break;
  }
  [aTableView deselectRowAtIndexPath:indexPath animated:NO];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
  // Return the number of sections.
  // If You have only one(1) section, return 1, otherwise you must handle sections
  return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
  // Return the number of rows in the section.
  return [itemsArray count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
  static NSString *CellIdentifier = @"Cell";

  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
  if (cell == nil) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier];
  }

  // Configure the cell...
  cell.textLabel.text = [NSString stringWithFormat:@"%@",[itemsArray objectAtIndex:indexPath.row]];


  return cell;
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
