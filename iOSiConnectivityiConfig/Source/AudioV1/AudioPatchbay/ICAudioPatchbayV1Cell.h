/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>

@interface ICAudioPatchbayV1Cell : UITableViewCell

@property(nonatomic, strong) IBOutlet UILabel *sourceLabel;
@property(nonatomic, strong) IBOutlet UIPickerView *sourcePicker;
@property(nonatomic, strong) IBOutlet UILabel *destLabel;

@end
