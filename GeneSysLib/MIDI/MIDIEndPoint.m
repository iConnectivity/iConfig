/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "MIDIEndPoint.h"

@implementation MIDIEndPoint

@synthesize endPoint;

+ (MIDIEndPoint *)endPoint:(MIDIEndpointRef)_endPoint {
  MIDIEndPoint *midiEndPoint = [[MIDIEndPoint alloc] init];
  midiEndPoint.endPoint = _endPoint;
  return midiEndPoint;
}

@end
