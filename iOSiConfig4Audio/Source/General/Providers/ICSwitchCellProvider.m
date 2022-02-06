/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICSwitchCellProvider.h"
#import <UIKit/UIKit.h>

@implementation ICSwitchCellProvider

+ (ICSwitchCellProvider *)providerWithDelegate:
                              (id<ICSwitchCellDelegate>)switchDelegate {
  return [[ICSwitchCellProvider alloc] initWithDelegate:switchDelegate];
}

- (id)initWithDelegate:(id<ICSwitchCellDelegate>)switchDelegate {
  self = [super init];

  if (self != nil) {
    NSParameterAssert(switchDelegate);

    self.switchDelegate = switchDelegate;
  }

  return self;
}

- (UITableViewCell *)cellForTableView:(UITableView *)tableView {
  static NSString *CellIdentifier = @"SwitchEditCell";

  UITableViewCell *cell =
      [tableView dequeueReusableCellWithIdentifier:CellIdentifier];

  if (cell == nil) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                  reuseIdentifier:CellIdentifier];

    auto *const switchView = [[UISwitch alloc] initWithFrame:CGRectZero];
    cell.accessoryView = switchView;

    cell.accessoryView = switchView;
    cell.accessoryType = UITableViewCellAccessoryNone;
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
  }

  NSString *title = [self.switchDelegate title];
  [cell.textLabel setText:title];

  // remove the previous action
  UISwitch *const currentSwitch = (UISwitch *)cell.accessoryView;
  if ([currentSwitch
          respondsToSelector:@selector(targetForAction:withSender:)]) {
    id target = [currentSwitch targetForAction:@selector(updateSwitch:)
                                    withSender:currentSwitch];
    [currentSwitch removeTarget:target
                         action:@selector(updateSwitch:)
               forControlEvents:UIControlEventValueChanged];
  } else {
    auto *const targets = [currentSwitch allTargets];

    for (id t in targets) {
      auto *const actions =
          [currentSwitch actionsForTarget:t
                          forControlEvent:UIControlEventValueChanged];
      for (NSString *const a in actions) {
        if ([a compare:NSStringFromSelector(@selector(updateSwitch:))] ==
            NSOrderedSame) {
          [currentSwitch removeTarget:t
                               action:@selector(updateSwitch:)
                     forControlEvents:UIControlEventValueChanged];
        }
      }
    }
  }

  [currentSwitch addTarget:self
                    action:@selector(updateSwitch:)
          forControlEvents:UIControlEventValueChanged];

  [currentSwitch setOn:[self.switchDelegate getValue] animated:NO];

  return cell;
}

#pragma mark - update switch
- (void)updateSwitch:(UISwitch *)aSwitch {

  // set the value
  [self.switchDelegate setValue:aSwitch.isOn];

  // reread the value
  [aSwitch setOn:[self.switchDelegate getValue] animated:YES];
}

@end
