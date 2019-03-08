/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "CCList.h"

const QStringList &CCList()
{
    static QStringList cclist;
    static bool initialized = false;

    if (!initialized) {
        initialized = true;
        cclist << QString("0 - Bank Select (coarse)");
        cclist << QString("1 - Modulation Wheel (coarse)");
        cclist << QString("2 - Breath Control (coarse)");
        cclist << QString("3 - Continuous controller #3");
        cclist << QString("4 - Foot Controller (coarse)");
        cclist << QString("5 - Portamento Time (coarse)");
        cclist << QString("6 - Data Entry Slider (coarse)");
        cclist << QString("7 - Main Volume (coarse)");
        cclist << QString("8 - Stereo Balance (coarse)");
        cclist << QString("9 - Continuous controller #9");
        cclist << QString("10 - Pan (coarse)");
        cclist << QString("11 - Expression (sub-Volume) (coarse)");
        cclist << QString("12 - Effect Control 1 (coarse)");
        cclist << QString("13 - Effect Control 2 (coarse)");
        cclist << QString("14 - Continuous controller #14");
        cclist << QString("15 - Continuous controller #15");
        cclist << QString("16 - General Purpose Slider 1");
        cclist << QString("17 - General Purpose Slider 2");
        cclist << QString("18 - General Purpose Slider 3");
        cclist << QString("19 - General Purpose Slider 4");
        cclist << QString("20 - Continuous controller #20");
        cclist << QString("21 - Continuous controller #21");
        cclist << QString("22 - Continuous controller #22");
        cclist << QString("23 - Continuous controller #23");
        cclist << QString("24 - Continuous controller #24");
        cclist << QString("25 - Continuous controller #25");
        cclist << QString("26 - Continuous controller #26");
        cclist << QString("27 - Continuous controller #27");
        cclist << QString("28 - Continuous controller #28");
        cclist << QString("29 - Continuous controller #29");
        cclist << QString("30 - Continuous controller #30");
        cclist << QString("31 - Continuous controller #31");
        cclist << QString("32 - Bank Select (fine)");
        cclist << QString("33 - Modulation Wheel (fine)");
        cclist << QString("34 - Breath Control (fine)");
        cclist << QString("35 - Continuous controller #3 (fine)");
        cclist << QString("36 - Foot Controller (fine)");
        cclist << QString("37 - Portamento Time (fine)");
        cclist << QString("38 - Data Entry Slider (fine)");
        cclist << QString("39 - Main Volume (fine)");
        cclist << QString("40 - Stereo Balance (fine)");
        cclist << QString("41 - Continuous controller #9 (fine)");
        cclist << QString("42 - Pan (fine)");
        cclist << QString("43 - Expression (sub-Volume) (fine)");
        cclist << QString("44 - Effect Control 1 (fine)");
        cclist << QString("45 - Effect Control 2 (fine)");
        cclist << QString("46 - Continuous controller #14 (fine)");
        cclist << QString("47 - Continuous controller #15 (fine)");
        cclist << QString("48 - Continuous controller #16");
        cclist << QString("49 - Continuous controller #17");
        cclist << QString("50 - Continuous controller #18");
        cclist << QString("51 - Continuous controller #19");
        cclist << QString("52 - Continuous controller #20 (fine)");
        cclist << QString("53 - Continuous controller #21 (fine)");
        cclist << QString("54 - Continuous controller #22 (fine)");
        cclist << QString("55 - Continuous controller #23 (fine)");
        cclist << QString("56 - Continuous controller #24 (fine)");
        cclist << QString("57 - Continuous controller #25 (fine)");
        cclist << QString("58 - Continuous controller #26 (fine)");
        cclist << QString("59 - Continuous controller #27 (fine)");
        cclist << QString("60 - Continuous controller #28 (fine)");
        cclist << QString("61 - Continuous controller #29 (fine)");
        cclist << QString("62 - Continuous controller #30 (fine)");
        cclist << QString("63 - Continuous controller #31 (fine)");
        cclist << QString("64 - Hold pedal (Sustain) on/off");
        cclist << QString("65 - Portamento on/off");
        cclist << QString("66 - Sustenuto Pedal on/off");
        cclist << QString("67 - Soft Pedal on/off");
        cclist << QString("68 - Legato Pedal on/off");
        cclist << QString("69 - Hold Pedal 2 on/off");
        cclist << QString("70 - Sound Variation");
        cclist << QString("71 - Sound Timbre");
        cclist << QString("72 - Sound Release Time");
        cclist << QString("73 - Sound Attack Time");
        cclist << QString("74 - Sound Brightness");
        cclist << QString("75 - Sound Control 6");
        cclist << QString("76 - Sound Control 7");
        cclist << QString("77 - Sound Control 8");
        cclist << QString("78 - Sound Control 9");
        cclist << QString("79 - Sound Control 10");
        cclist << QString("80 - General Purpose Button");
        cclist << QString("81 - General Purpose Button");
        cclist << QString("82 - General Purpose Button");
        cclist << QString("83 - General Purpose Button");
        cclist << QString("84 - Undefined on/off");
        cclist << QString("85 - Undefined on/off");
        cclist << QString("86 - Undefined on/off");
        cclist << QString("87 - Undefined on/off");
        cclist << QString("88 - Undefined on/off");
        cclist << QString("89 - Undefined on/off");
        cclist << QString("90 - Undefined on/off");
        cclist << QString("91 - Effects Level");
        cclist << QString("92 - Tremulo Level");
        cclist << QString("93 - Chorus Level");
        cclist << QString("94 - Celeste (Detune) Level");
        cclist << QString("95 - Phaser Level");
        cclist << QString("96 - Data entry +1");
        cclist << QString("97 - Data entry -1");
        cclist << QString("98 - Non-Registered Parameter Number (coarse)");
        cclist << QString("99 - Non-Registered Parameter Number (fine)");
        cclist << QString("100 - Registered Parameter Number (coarse)");
        cclist << QString("101 - Registered Parameter Number (fine)");
        cclist << QString("102 - Undefined");
        cclist << QString("103 - Undefined");
        cclist << QString("104 - Undefined");
        cclist << QString("105 - Undefined");
        cclist << QString("106 - Undefined");
        cclist << QString("107 - Undefined");
        cclist << QString("108 - Undefined");
        cclist << QString("109 - Undefined");
        cclist << QString("110 - Undefined");
        cclist << QString("111 - Undefined");
        cclist << QString("112 - Undefined");
        cclist << QString("113 - Undefined");
        cclist << QString("114 - Undefined");
        cclist << QString("115 - Undefined");
        cclist << QString("116 - Undefined");
        cclist << QString("117 - Undefined");
        cclist << QString("118 - Undefined");
        cclist << QString("119 - Undefined");
        cclist << QString("120 - All Sound Off");
        cclist << QString("121 - All Controllers Off");
        cclist << QString("122 - Local Keyboard On/Off");
        cclist << QString("123 - All Notes Off");
        cclist << QString("124 - Omni Mode Off");
        cclist << QString("125 - Omni Mode On");
        cclist << QString("126 - Monophonic Mode On");
        cclist << QString("127 - Polyphonic Mode On (mono=off)");
    }

    return cclist;
}
