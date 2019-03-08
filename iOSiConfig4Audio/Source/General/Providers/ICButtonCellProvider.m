/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICButtonCellProvider.h"
#import "ICButtonBlockDelegate.h"

@implementation ICButtonCellProvider
+ (ICButtonCellProvider *)providerWithTitle:(NSString *)title
                                      value:(NSString *)value {
  ICButtonBlockDelegate *buttonDelegate =
    [[ICButtonBlockDelegate alloc] initWithTitle:title value:value func:nil];
  return [[ICButtonCellProvider alloc] initWithDelegate:buttonDelegate];
}

+ (ICButtonCellProvider *)providerWithTitle:(NSString *)title
                                      value:(NSString*)value
                                       func:(ButtonCellFunc)func {

  ICButtonBlockDelegate *buttonDelegate =
    [[ICButtonBlockDelegate alloc] initWithTitle:title value:value func:func];
  return [[ICButtonCellProvider alloc] initWithDelegate:buttonDelegate];
}


- (id)initWithDelegate:(id<ButtonCellDelegate>)buttonDelegate {
  self = [super init];

  if (self) {
    NSParameterAssert(buttonDelegate);
    self.buttonDelegate = buttonDelegate;
  }

  return self;
}

- (UITableViewCell *)cellForTableView:(UITableView *)tableView {
  static NSString *CellIdentifier = @"ButtonCell";

  UITableViewCell *cell =
      [tableView dequeueReusableCellWithIdentifier:CellIdentifier];

  if (cell == nil) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                  reuseIdentifier:CellIdentifier];

    cell.accessoryType = UITableViewCellAccessoryNone;
  }

  [cell.textLabel setText:[self.buttonDelegate title]];
  [cell.detailTextLabel setText:[self.buttonDelegate value]];

  return cell;
}

@end
