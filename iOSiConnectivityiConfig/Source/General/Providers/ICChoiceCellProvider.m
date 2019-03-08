/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICChoiceCellProvider.h"

@interface ICChoiceCellProvider ()

@end

@implementation ICChoiceCellProvider

+ (ICChoiceCellProvider *)providerWithDelegate:
                              (id<ICChoiceDelegate>)choiceDelegate {
  ICChoiceCellProvider *provider =
      [[ICChoiceCellProvider alloc] initWithDelegate:choiceDelegate];
  return provider;
}

- (id)initWithDelegate:(id<ICChoiceDelegate>)choiceDelegate {
  self = [super init];
  if (self != nil) {
    NSParameterAssert(choiceDelegate);
    self.choiceDelegate = choiceDelegate;
  }
  return self;
}

- (UITableViewCell *)cellForTableView:(UITableView *)tableView {
  static NSString *CellIdentifier = @"ChoiceCell";

  UITableViewCell *cell =
      [tableView dequeueReusableCellWithIdentifier:CellIdentifier];

  if (cell == nil) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                  reuseIdentifier:CellIdentifier];
    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
  }

  [cell.textLabel setText:[self.choiceDelegate title]];

  NSArray *const options = [self.choiceDelegate options];

  const auto &index = [self.choiceDelegate getChoice];
  if (index < [self.choiceDelegate optionCount]) {
    [cell.detailTextLabel setText:options[index]];
  } else {
    [cell.detailTextLabel setText:@"None"];
  }

  return cell;
}

@end
