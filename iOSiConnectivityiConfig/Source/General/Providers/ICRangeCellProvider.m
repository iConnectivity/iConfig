/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICRangeCellProvider.h"

@implementation ICRangeCellProvider

+ (ICRangeCellProvider *)providerWithDelegate:
                             (id<ICRangeChoiceDelegate>)rangeDelegate {
  return [[ICRangeCellProvider alloc] initWithDelegate:rangeDelegate];
}

- (id)initWithDelegate:(id<ICRangeChoiceDelegate>)rangeDelegate {
  self = [super init];

  if (self) {
    NSParameterAssert(rangeDelegate);
    self.rangeDelegate = rangeDelegate;
  }

  return self;
}

- (UITableViewCell *)cellForTableView:(UITableView *)tableView {
  static NSString *CellIdentifier = @"RangeCell";

  UITableViewCell *cell =
      [tableView dequeueReusableCellWithIdentifier:CellIdentifier];

  if (cell == nil) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                  reuseIdentifier:CellIdentifier];

    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
  }

  [cell.textLabel setText:[self.rangeDelegate title]];
  [cell.detailTextLabel
      setText:[NSString
                  stringWithFormat:@"%ld", (long)[self.rangeDelegate getValue]]];

  return cell;
}

@end
