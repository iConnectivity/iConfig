/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICNormalCellProvider.h"
#import "ICNormalBlockDelegate.h"
#import <UIKit/UIKit.h>


@implementation ICNormalCellProvider
+ (ICNormalCellProvider *)providerWithTitle:(NSString *)title
                                      value:(NSString *)value {
  ICNormalBlockDelegate *normalDelegate =
      [[ICNormalBlockDelegate alloc] initWithTitle:title value:value];
  return [[ICNormalCellProvider alloc] initWithDelegate:normalDelegate];
}

+ (ICNormalCellProvider *)providerWithTitle:(NSString *)title
                                   getValue:(NormalCellGetBlock)getValue {

  ICNormalBlockDelegate *normalDelegate =
      [[ICNormalBlockDelegate alloc] initWithTitle:title getValue:getValue];
  return [[ICNormalCellProvider alloc] initWithDelegate:normalDelegate];
}

+ (ICNormalCellProvider *)providerWithDelegate:
                              (id<NormalCellDelegate>)normalDelegate {
  return [[ICNormalCellProvider alloc] initWithDelegate:normalDelegate];
}

- (id)initWithDelegate:(id<NormalCellDelegate>)normalDelegate {
  self = [super init];

  if (self) {
    NSParameterAssert(normalDelegate);
    self.normalDelegate = normalDelegate;
  }

  return self;
}

- (UITableViewCell *)cellForTableView:(UITableView *)tableView {
  static NSString *CellIdentifier = @"NormalCell";

  UITableViewCell *cell =
      [tableView dequeueReusableCellWithIdentifier:CellIdentifier];

  if (cell == nil) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                  reuseIdentifier:CellIdentifier];

    cell.accessoryType = UITableViewCellAccessoryNone;
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
  }

  [cell.textLabel setText:[self.normalDelegate title]];
  [cell.detailTextLabel setText:[self.normalDelegate value]];

  return cell;
}

@end
