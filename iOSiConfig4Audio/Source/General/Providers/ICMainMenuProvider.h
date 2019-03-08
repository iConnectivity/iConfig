/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "Screen.h"
#import "CommandDefines.h"
#import "ICViewController.h"

#import <list>

typedef void (^ButtonActionBlock)(ICViewController *);
typedef void (^ActionBlock)(ICViewController *);

@interface ICMainMenuProvider : NSObject  {
  NSArray *buttonNames;
  NSArray *actionArray;
  id queryNotificationHandler;
}

- (NSString *)providerName;

- (ButtonActionBlock)createActionForQuery:(std::list<GeneSysLib::CmdEnum>)query
                                   screen:(Screen)screen
                                   action:(ActionBlock)action;

- (void)initializeProviderButtons:(ICViewController *)sender;
- (void)onButtonDown:(ICViewController *)sender index:(NSInteger)buttonIndex;

@end
