/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import <Foundation/Foundation.h>
#import "DeviceInfo.h"

#import "LibTypes.h"
#import "FilterID.h"
#import "ChannelFilterStatusBit.h"
#import "ChannelBitmapBit.h"
#import "FilterStatusBit.h"

namespace EventRows {
  typedef enum Enum {
    PitchBendEvents = 0,
    ChannelPressureEvents,
    ProgramChangeEvents,
    ControlChangeEvents,
    PolyKeyPressureEvents,
    NoteEvents,

    ResetEvents,
    ActiveSensingEvents,
    RealtimeEvents,
    TuneRequestEvents,
    SongSelectEvents,
    SongPositionPointerEvents,
    TimeCodeEvents,
    SystemExclusiveEvents

  } Enum;
}

typedef struct midi_port_t {
  UInt8 devicePort;
  UInt8 port;
} midi_port_t;

typedef struct midi_pair_t {
  midi_port_t source;
  midi_port_t destination;
} midi_pair_t;

inline bool operator<(const midi_port_t &a, const midi_port_t &b) {
  return ((a.devicePort < b.devicePort) || ((a.devicePort == b.devicePort) && (a.port < b.port)));
}

inline bool operator==(const midi_port_t &a, const midi_port_t &b) {
  return ((a.devicePort == b.devicePort) && (a.port == b.port));
}

typedef void (^RefreshCallback)(void);
typedef void (^MIDIPairOperator)(midi_pair_t *);

@interface ICMIDIPatchbayDelegate : NSObject

- (id)initWithDevice:(DeviceInfoPtr)device;

- (void)setCallback:(RefreshCallback)refreshCallback;

- (int)numSourceSections;
- (int)numDestinationSections;

- (int)numSourcesPerSection:(int)section;
- (int)numDestinationsPerSection:(int)section;

- (int)realNumSourcesPerSection:(int)section;
- (int)realNumDestinationsPerSection:(int)section;

- (int)totalSources;
- (int)totalDestinations;

- (int)sourceIndexToTotal:(midi_port_t)targetIndex;
- (midi_port_t)sourceTotalToIndex:(int)totalInput;

- (int)destinationIndexToTotal:(midi_port_t)targetIndex;
- (midi_port_t)destinationTotalToIndex:(int)totalOutput;

- (NSString *)nameForSection:(int)section;

- (bool)isPatchedFrom:(midi_port_t)src to:(midi_port_t)dst;
- (void)setPatchedFrom:(midi_port_t)src to:(midi_port_t)dst andRemove:(midi_port_t)toRemove;

- (void)toggleCollapseInput:(int)section;
- (bool)isCollapsedInput:(int)section;

- (void)toggleCollapseOutput:(int)section;
- (bool)isCollapsedOutput:(int)section;

- (void)foreach:(MIDIPairOperator)midiOperator;

- (BOOL)isSourceDeviceInBounds:(midi_port_t)device;
- (BOOL)isDestinationDeviceInBounds:(midi_port_t)device;

-(NSString*)nameForDevicePort:(Byte)popupDevicePort andPort:(Byte)popupPort;

- (bool) isPortFilterCellSetAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut;
- (void) setPortFilterAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(bool)state;

- (NSArray*) getChannelRemapsForDevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut;
- (bool) isChannelRemapSetAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut;
- (void) setChannelRemapAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(bool)state;
- (void) setChannelRemapAtCol:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(int)state;

- (NSArray*) getControllerFilterIDsForDevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut;
- (bool) isControllerFilterSetAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut;
- (void) setControllerFilterAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(bool)state;
- (void) setControllerFilterIDAtRow:(int)row DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(int)state;

- (NSArray*) getControllerRemapIDsForDevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut;
- (bool) isControllerRemapSetAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut;
- (void) setControllerRemapAtRow:(int)row Col:(int)col DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(bool)state;
- (void) setControllerRemapIDAtRow:(int)row DevicePort:(Byte)popupDevicePort Port:(Byte)popupPort Put:(int)popupPut ToState:(int)state isSource:(bool)isSource;

@end
