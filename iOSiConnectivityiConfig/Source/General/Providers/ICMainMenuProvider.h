/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICDefaultProvider.h"
#import "Screen.h"
#import "CommandDefines.h"

#import <list>

typedef void (^ButtonActionBlock)(ICViewController *);
typedef void (^ActionBlock)(ICViewController *);

@interface ICMainMenuProvider : ICDefaultProvider {
  NSArray *buttonNames;
  NSArray *actionArray;
  id queryNotificationHandler;
}

- (NSString *)providerName;

- (ButtonActionBlock)createActionForQuery:(std::list<GeneSysLib::CmdEnum>)query
                                   screen:(Screen)screen
                                   action:(ActionBlock)action;

@end
