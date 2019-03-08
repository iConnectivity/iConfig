/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __PATCHBAY_DELEGATE_H__
#define __PATCHBAY_DELEGATE_H__

typedef struct device_channel_t {
  UInt16 port;
  UInt8 channel;
} device_channel_t;

typedef struct device_pair_t {
  device_channel_t source;
  device_channel_t destination;
} device_pair_t;

inline bool operator<(const device_channel_t &a, const device_channel_t &b) {
  return ((a.port < b.port) || ((a.port == b.port) && (a.channel < b.channel)));
}

inline bool operator==(const device_channel_t &a, const device_channel_t &b) {
  return ((a.port == b.port) && (a.channel == b.channel));
}

typedef void (^RefreshCallback)(void);
typedef void (^DevicePairOperator)(device_pair_t *);

@protocol ICPatchbayDelegate

- (void)setCallback:(RefreshCallback)refreshCallback;

- (int)numSourceSections;
- (int)numDestinationSections;

- (int)numSourcesPerSection:(int)section;
- (int)numDestinationsPerSection:(int)section;

- (int)totalSources;
- (int)totalDestinations;

- (int)sourceIndexToTotal:(device_channel_t)targetIndex;
- (device_channel_t)sourceTotalToIndex:(int)totalInput;

- (int)destinationIndexToTotal:(device_channel_t)targetIndex;
- (device_channel_t)destinationTotalToIndex:(int)totalOutput;

- (NSString *)nameForSourceSection:(int)section;
- (NSString *)nameForDestinationSection:(int)section;

- (bool)isPatchedFrom:(device_channel_t)src to:(device_channel_t)dst;
- (void)setPatchedFrom:(device_channel_t)src to:(device_channel_t)dst andRemove:(device_channel_t)toRemove;

- (void)toggleCollapseInput:(int)section;
- (bool)isCollapsedInput:(int)section;

- (void)toggleCollapseOutput:(int)section;
- (bool)isCollapsedOutput:(int)section;

- (bool)isMixerToo;

- (void)foreach:(DevicePairOperator)deviceOperator;
- (void)foreachMixer:(DevicePairOperator)deviceOperator;

- (BOOL)isSourceDeviceInBounds:(device_channel_t)device;
- (BOOL)isDestinationDeviceInBounds:(device_channel_t)device;

@end

#endif  // __PATCHBAY_SOURCE_H__
