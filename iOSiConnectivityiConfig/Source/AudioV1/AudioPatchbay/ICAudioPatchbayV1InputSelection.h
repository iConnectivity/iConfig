/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "AudioPortPatchbay.h"
#import "ICDefaultProvider.h"

#import <map>
#import <utility>

using std::map;
using std::pair;

@interface ICAudioPatchbayV1InputSelection : ICDefaultProvider {
  NSArray *buttonNames;
  map<NSInteger, pair<GeneSysLib::AudioPortPatchbay, size_t> >
      indexToAudioPatchbayBlockPair;
}

@end
