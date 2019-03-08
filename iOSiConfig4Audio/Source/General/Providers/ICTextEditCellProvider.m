/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICTextEditCellProvider.h"

@implementation ICTextEditCellProvider

+ (ICTextEditCellProvider *)providerWithDelegate:
                                (id<ICTextEditCellDelegate>)textEditDelegate {
  return [[ICTextEditCellProvider alloc] initWithDelegate:textEditDelegate];
}

- (id)initWithDelegate:(id<ICTextEditCellDelegate>)textEditDelegate {
  self = [super init];

  if (self) {
    NSParameterAssert(textEditDelegate);
    self.textEditDelegate = textEditDelegate;
  }

  return self;
}

- (UITableViewCell *)cellForTableView:(UITableView *)tableView {
  static NSString *CellIdentifier = @"TextEditCell";

  // try to get a cell from the pool
  UITableViewCell *cell =
      [tableView dequeueReusableCellWithIdentifier:CellIdentifier];

  // if no cell exists then create one
  if (cell == nil) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                  reuseIdentifier:CellIdentifier];

    // create a default text field to add to the newly created cell
    const auto &cellSize = cell.frame.size;
    auto *const textField = [[UITextField alloc]
        initWithFrame:CGRectMake(cellSize.width * 0.55, cellSize.height * 0.15,
                                 cellSize.width * 0.45, cellSize.height * 0.7)];
    [textField setBorderStyle:UITextBorderStyleBezel];
    [textField setAutoresizingMask:UIViewAutoresizingFlexibleWidth |
                                   UIViewAutoresizingFlexibleHeight |
                                   UIViewAutoresizingFlexibleLeftMargin |
                                   UIViewAutoresizingFlexibleRightMargin];

    // add the text field to the cell
    cell.accessoryView = textField;

    // make sure that the cell cannot be selected
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
  }

  // update the cells title
  [cell.textLabel setText:[self.textEditDelegate title]];

  // get a pointer to the current textField
  UITextField *const textField = (UITextField *)cell.accessoryView;

  // being paranoid to make sure there is a text field
  assert(textField);

  // update the delegate of the text field to self so that changes are mapped
  // correctly
  [textField setDelegate:self];

  // update the value of the text field to match the value from the getter
  [textField setText:[self.textEditDelegate getValue]];

  return cell;
}

#pragma mark - Text Field Delgate

- (void)textFieldDidEndEditing:(UITextField *)textField {
  // check to see if there is a change in data if so then change the data
  if ([[self.textEditDelegate getValue] compare:textField.text] !=
      NSOrderedSame) {
    [self.textEditDelegate setValue:textField.text];
  }
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
  // check to see if there is a change in data if so then change the data
  if ([[self.textEditDelegate getValue] compare:textField.text] !=
      NSOrderedSame) {
    [self.textEditDelegate setValue:textField.text];
  }

  return YES;
}

- (BOOL)textField:(UITextField *)textField
    shouldChangeCharactersInRange:(NSRange)range
                replacementString:(NSString *)string {
  BOOL result = YES;

  if (result) {
    const auto &oldLength = [textField.text length];
    const auto &replacementLength = [string length];
    const auto &rangeLength = range.length;

    // determine the length of the changed text
    const auto &newLength = oldLength - rangeLength + replacementLength;
    result = newLength <= [self.textEditDelegate maxLength];
  }

  // determine if the character set is correct
  if ((result) &&
      ([self.textEditDelegate respondsToSelector:@selector(charSet)])) {
    NSCharacterSet *const charSet = [NSCharacterSet
        characterSetWithCharactersInString:[self.textEditDelegate charSet]];

    const auto &charRange =
        [string rangeOfCharacterFromSet:[charSet invertedSet]];

    result = (charRange.location == NSNotFound);
  }

  // determine if the new text matches the regular expression
  if ((result) &&
      ([self.textEditDelegate respondsToSelector:@selector(validator)])) {
    auto *const test =
        [NSPredicate predicateWithFormat:@"SELF MATCHES %@",
                                         [self.textEditDelegate validator]];
    result = [test
        evaluateWithObject:[textField.text
                               stringByReplacingCharactersInRange:range
                                                       withString:string]];
  }

  // verify that there are no new line characters
  const auto &returnKey = [string rangeOfString:@"\n"].location != NSNotFound;

  // return the result
  return result || returnKey;
}

@end
