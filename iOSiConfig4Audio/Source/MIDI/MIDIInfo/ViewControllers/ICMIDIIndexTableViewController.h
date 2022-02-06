/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "MIDIInfo.h"

@protocol SidebarDelegate<NSObject>

- (void)indexPathSelected:(NSIndexPath *)indexPath;

@end

@interface ICMIDIIndexTableViewController : UITableViewController

@property(strong, nonatomic) id<SidebarDelegate> sidebarDelegate;

- (void)rebuildWithMIDIInfo:(const GeneSysLib::MIDIInfo &)midiInfo;
- (void)clear;

@end
