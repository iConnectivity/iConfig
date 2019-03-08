/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICDefaultProvider.h"
#import "AudioPortPatchbay.h"
#import "AudioPortCfgInfo.h"

#import <map>
#import <utility>

using std::map;
using std::pair;

@interface ICAudioPatchbayOutputV1Selection : ICDefaultProvider {
  NSArray *buttonNames;
  NSString *title;

  GeneSysLib::AudioPortPatchbay audioPortPatchbay;
  GeneSysLib::AudioPortCfgInfo audioPortCfgInfo;
  size_t configBlockIndex;

  map<NSInteger, pair<Byte, Byte> > indexToOutputPair;
}

- (id)initWithPatchbay:(GeneSysLib::AudioPortPatchbay)audioPortPatchbayData
            blockIndex:(size_t)configBlockIndex
         selectedTitle:(NSString *)title;

@end
