/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "ICSwitchCellDelegate.h"

@interface ICSwitchCellProvider : NSObject

@property(nonatomic, strong) id<ICSwitchCellDelegate> switchDelegate;

+ (ICSwitchCellProvider *)providerWithDelegate:
        (id<ICSwitchCellDelegate>)switchDelegate;

- (id)initWithDelegate:(id<ICSwitchCellDelegate>)switchDelegate;

- (UITableViewCell *)cellForTableView:(UITableView *)tableView;

#pragma mark - update switch
- (void)updateSwitch:(UISwitch *)sender;

@end
