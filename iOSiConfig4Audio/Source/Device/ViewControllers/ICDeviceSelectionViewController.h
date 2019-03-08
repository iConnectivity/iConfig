/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <UIKit/UIKit.h>
#import "Communicator.h"
#import "DeviceInfo.h"
#import "DeviceID.h"
#include <list>
#include <map>
#include <queue>
#include <utility>

using namespace std;

@interface ICDeviceSelectionViewController
    : UIViewController<UITableViewDataSource, UITableViewDelegate> {
  NSTimer *searchTimer;
  NSTimer *infoTimer;

  BOOL inHandler;
  BOOL foundDevice;

  list<DeviceInfoPtr> deviceList;
  map<GeneSysLib::CmdEnum, long> registeredHandlers;

  queue<pair<unsigned int, Bytes> > pendingGetDevice;
  queue<pair<unsigned int, Bytes> > pendingSysex;
}

@property(strong, nonatomic) IBOutlet UIView *searchingView;
@property(weak, nonatomic) IBOutlet UITableView *myTableView;
@property(assign, nonatomic) GeneSysLib::CommPtr comm;

- (id)initWithCommunicator:(GeneSysLib::CommPtr)communicator;

- (DeviceInfoPtr)deviceForIndex:(NSInteger)index;
- (DeviceInfoPtr)deviceForDeviceID:(GeneSysLib::DeviceID)deviceID;

- (IBAction)startSearch;

- (void)deviceTimerTick;
- (void)searchComplete;

- (void)sendNextSysex;
- (void)sendNextGetDevice;

- (void)startSearchTimer;
- (void)restartSearchTimer;
- (void)stopSearchTimer;

- (void)startInfoTimer;
- (void)restartInfoTimer;
- (void)stopInfoTimer;

@end
