/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#import "ICControllerChoiceDelegate.h"
#import "CommandList.h"

@interface ICControllerChoiceDelegate ()
@property(nonatomic) NSInteger selected;

@end

@implementation ICControllerChoiceDelegate

+ (NSArray *)contollerList {
  static NSArray *array = nil;

  if (array == nil) {
    array = @[
      @"0 - Bank Select (coarse)",
      @"1 - Modulation Wheel (coarse)",
      @"2 - Breath Control (coarse)",
      @"3 - Continuous controller #3",
      @"4 - Foot Controller (coarse)",
      @"5 - Portamento Time (coarse)",
      @"6 - Data Entry Slider (coarse)",
      @"7 - Main Volume (coarse)",
      @"8 - Stereo Balance (coarse)",
      @"9 - Continuous controller #9",
      @"10 - Pan (coarse)",
      @"11 - Expression (sub-Volume) (coarse)",
      @"12 - Effect Control 1 (coarse)",
      @"13 - Effect Control 2 (coarse)",
      @"14 - Continuous controller #14",
      @"15 - Continuous controller #15",
      @"16 - General Purpose Slider 1",
      @"17 - General Purpose Slider 2",
      @"18 - General Purpose Slider 3",
      @"19 - General Purpose Slider 4",
      @"20 - Continuous controller #20",
      @"21 - Continuous controller #21",
      @"22 - Continuous controller #22",
      @"23 - Continuous controller #23",
      @"24 - Continuous controller #24",
      @"25 - Continuous controller #25",
      @"26 - Continuous controller #26",
      @"27 - Continuous controller #27",
      @"28 - Continuous controller #28",
      @"29 - Continuous controller #29",
      @"30 - Continuous controller #30",
      @"31 - Continuous controller #31",
      @"32 - Bank Select (fine)",
      @"33 - Modulation Wheel (fine)",
      @"34 - Breath Control (fine)",
      @"35 - Continuous controller #3 (fine)",
      @"36 - Foot Controller (fine)",
      @"37 - Portamento Time (fine)",
      @"38 - Data Entry Slider (fine)",
      @"39 - Main Volume (fine)",
      @"40 - Stereo Balance (fine)",
      @"41 - Continuous controller #9 (fine)",
      @"42 - Pan (fine)",
      @"43 - Expression (sub-Volume) (fine)",
      @"44 - Effect Control 1 (fine)",
      @"45 - Effect Control 2 (fine)",
      @"46 - Continuous controller #14 (fine)",
      @"47 - Continuous controller #15 (fine)",
      @"48 - Continuous controller #16",
      @"49 - Continuous controller #17",
      @"50 - Continuous controller #18",
      @"51 - Continuous controller #19",
      @"52 - Continuous controller #20 (fine)",
      @"53 - Continuous controller #21 (fine)",
      @"54 - Continuous controller #22 (fine)",
      @"55 - Continuous controller #23 (fine)",
      @"56 - Continuous controller #24 (fine)",
      @"57 - Continuous controller #25 (fine)",
      @"58 - Continuous controller #26 (fine)",
      @"59 - Continuous controller #27 (fine)",
      @"60 - Continuous controller #28 (fine)",
      @"61 - Continuous controller #29 (fine)",
      @"62 - Continuous controller #30 (fine)",
      @"63 - Continuous controller #31 (fine)",
      @"64 - Hold pedal (Sustain) on/off",
      @"65 - Portamento on/off",
      @"66 - Sustenuto Pedal on/off",
      @"67 - Soft Pedal on/off",
      @"68 - Legato Pedal on/off",
      @"69 - Hold Pedal 2 on/off",
      @"70 - Sound Variation",
      @"71 - Sound Timbre",
      @"72 - Sound Release Time",
      @"73 - Sound Attack Time",
      @"74 - Sound Brighness",
      @"75 - Sound Control 6",
      @"76 - Sound Control 7",
      @"77 - Sound Control 8",
      @"78 - Sound Control 9",
      @"79 - Sound Control 10",
      @"80 - General Purpose Button",
      @"81 - General Purpose Button",
      @"82 - General Purpose Button",
      @"83 - General Purpose Button",
      @"84 - Undefined on/off",
      @"85 - Undefined on/off",
      @"86 - Undefined on/off",
      @"87 - Undefined on/off",
      @"88 - Undefined on/off",
      @"89 - Undefined on/off",
      @"90 - Undefined on/off",
      @"91 - Effects Level",
      @"92 - Tremulo Level",
      @"93 - Chorus Level",
      @"94 - Celeste (Detune) Level",
      @"95 - Phaser Level",
      @"96 - Data entry +1",
      @"97 - Data entry -1",
      @"98 - Non-Registered Parameter Number (coarse)",
      @"99 - Non-Registered Parameter Number (fine)",
      @"100 - Registered Parameter Number (coarse)",
      @"101 - Registered Parameter Number (fine)",
      @"102 - Undefined",
      @"103 - Undefined",
      @"104 - Undefined",
      @"105 - Undefined",
      @"106 - Undefined",
      @"107 - Undefined",
      @"108 - Undefined",
      @"109 - Undefined",
      @"110 - Undefined",
      @"111 - Undefined",
      @"112 - Undefined",
      @"113 - Undefined",
      @"114 - Undefined",
      @"115 - Undefined",
      @"116 - Undefined",
      @"117 - Undefined",
      @"118 - Undefined",
      @"119 - Undefined",
      @"120 - All Sound Off",
      @"121 - All Controllers Off",
      @"122 - Local Keyboard On/Off",
      @"123 - All Notes Off",
      @"124 - Omni Mode Off",
      @"125 - Omni Mode On",
      @"126 - Monophonic Mode On",
      @"127 - Polyphonic Mode On (mono=off)"
    ];
  }
  return array;
}

- (id)initWithSelectedController:(NSInteger)selected {
  self = [super init];

  if (self) {
    // Custom initialization
    self.selected = selected;
  }
  return self;
}

- (NSString *)title {
  return @"";
}

- (NSArray *)options {
  return [ICControllerChoiceDelegate contollerList];
}

- (NSInteger)optionCount {
  return [[ICControllerChoiceDelegate contollerList] count];
}

- (NSInteger)getChoice {
  return self.selected;
}

- (void)setChoice:(NSInteger)value {
  self.selected = value;
}

@end
