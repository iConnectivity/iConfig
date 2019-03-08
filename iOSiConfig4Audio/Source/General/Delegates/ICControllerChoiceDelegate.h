/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "ICChoiceSelectionViewController.h"

@interface ICControllerChoiceDelegate : NSObject<ICChoiceDelegate> {
}

+ (NSArray *)contollerList;

- (id)initWithSelectedController:(NSInteger)selected;

- (NSString *)title;
- (NSArray *)options;
- (NSInteger)optionCount;
- (NSInteger)getChoice;
- (void)setChoice:(NSInteger)value;

@end
