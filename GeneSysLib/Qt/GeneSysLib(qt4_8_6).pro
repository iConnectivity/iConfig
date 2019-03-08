#-------------------------------------------------
#
# Project created by QtCreator 2014-06-27T11:20:36
#
#-------------------------------------------------

QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GeneSysLib
TEMPLATE = lib
CONFIG += staticlib
CONFIG -= PRECOMPILED_HEADER

DEFINES += BOOST_RESULT_OF_USE_DECLTYPE
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6

#mac: DEFINES        += __MACOSX_CORE__
win32: DEFINES      += __WINDOWS_MM__
win32: LIBS         += -L"C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/" -lWinMM

mac: QMAKE_CXXFLAGS = -std=c++11 -stdlib=libstdc++ -Wno-unused-parameter -mmacosx-version-min=10.6
mac: QMAKE_LFLAGS = -std=c++11 -stdlib=libstdc++ -Wno-unused-parameter -mmacosx-version-min=10.6
mac: QMAKE_CXXFLAGS += -isystem /opt/local/include

win32: INCLUDEPATH  += C:/boost_1_57_0/
win32: DEPENDPATH   += C:/boost_1_57_0/

mac: INCLUDEPATH += /opt/local/include/
mac: DEPENDPATH += /opt/local/include/

mac: INCLUDEPATH += -isystem /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include/
mac: INCLUDEPATH += -isystem /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include/c++/4.2.1/
mac: DEPENDPATH += -isystem /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include/

INCLUDEPATH += $$PWD/../../rtmidi-2.1.1
DEPENDPATH += $$PWD/../../rtmidi-2.1.1

mac: QMAKE_CFLAGS   += -gdwarf-2
mac: QMAKE_CXXFLAGS += -gdwarf-2

mac: LIBS           += -framework CoreMIDI
mac: LIBS           += -framework CoreFoundation
mac: LIBS           += -framework CoreAudio

SOURCES += \
    ../Audio/AudioV1/AudioCfgInfo.cpp \
    ../Audio/AudioV1/AudioClockInfo.cpp \
    ../Audio/AudioV1/AudioInfo.cpp \
    ../Audio/AudioV1/AudioPortCfgInfo.cpp \
    ../Audio/AudioV1/AudioPortInfo.cpp \
    ../Audio/AudioV1/AudioPortPatchbay.cpp \
    ../Audio/AudioV1/ClockSourceType.cpp \
    ../Audio/AudioV2/AudioClockParm.cpp \
    ../Audio/AudioV2/AudioControlDetail.cpp \
    ../Audio/AudioV2/AudioControlParm.cpp \
    ../Audio/AudioV2/AudioControlTypes.cpp \
    ../Audio/AudioV2/AudioDeviceParm.cpp \
    ../Audio/AudioV2/AudioDeviceTypes.cpp \
    ../Audio/AudioV2/AudioGlobalParm.cpp \
    ../Audio/AudioV2/AudioPatchbayParm.cpp \
    ../Audio/AudioV2/AudioPortParm.cpp \
    ../Audio/AudioV2/AudioPortParmTypes.cpp \
    ../Audio/BitDepthCode.cpp \
    ../Audio/SampleRateCode.cpp \
    ../Base/ACK.cpp \
#    ../Base/ByteCommandData.cpp \
#    ../Base/BytesCommandData.cpp \
#    ../Base/CommandData.cpp \
    ../Base/CommandDefines.cpp \
    ../Base/CommandList.cpp \
    ../Base/Communicator.cpp \
    ../Base/Generator.cpp \
    ../Base/Lookup.cpp \
    ../Base/MyAlgorithms.cpp \
    ../Base/stdafx.cpp \
    ../Base/SysexParser.cpp \
    ../Base/TimerThread.cpp \
    ../Device/Device.cpp \
    ../Device/DeviceID.cpp \
    ../Device/EthernetPortInfo.cpp \
    ../Device/GizmoCount.cpp \
    ../Device/GizmoInfo.cpp \
    ../Device/Info.cpp \
    ../Device/InfoList.cpp \
    ../Device/SaveRestoreList.cpp \
    ../MIDI/MIDIInfo.cpp \
    ../MIDI/MIDIPortDetail.cpp \
    ../MIDI/MIDIPortDetailTypes.cpp \
    ../MIDI/MIDIPortFilter.cpp \
    ../MIDI/MIDIPortInfo.cpp \
    ../MIDI/MIDIPortRemap.cpp \
    ../MIDI/MIDIPortRoute.cpp \
    ../MIDI/RTPMIDIConnectionDetail.cpp \
    ../MIDI/USBHostMIDIDeviceDetail.cpp \
    ../Audio/AudioV2/AudioControlDetailValue.cpp \
    ../Audio/AudioV2/AudioControlDetailValueTypes.cpp \
    ../Audio/AudioV2/AudioControlDetailTypes.cpp \
    ../Audio/AudioV2/AudioPortMeterValue.cpp \
    ../Audio/Mixer/MixerParm.cpp \
    ../Audio/Mixer/MixerInputControl.cpp \
    ../Audio/Mixer/MixerInputControlValue.cpp \
    ../Audio/Mixer/MixerInputParm.cpp \
    ../Audio/Mixer/MixerOutputControl.cpp \
    ../Audio/Mixer/MixerOutputControlValue.cpp \
    ../Audio/Mixer/MixerOutputParm.cpp \
    ../Audio/Mixer/MixerPortParm.cpp \
    ../Audio/Mixer/MixerMeterValue.cpp

HEADERS += \
    ../Audio/AudioV1/AudioCfgInfo.h \
    ../Audio/AudioV1/AudioClockInfo.h \
    ../Audio/AudioV1/AudioInfo.h \
    ../Audio/AudioV1/AudioPortCfgInfo.h \
    ../Audio/AudioV1/AudioPortInfo.h \
    ../Audio/AudioV1/AudioPortPatchbay.h \
    ../Audio/AudioV1/ClockSourceType.h \
    ../Audio/AudioV2/AudioClockParm.h \
    ../Audio/AudioV2/AudioControlDetail.h \
    ../Audio/AudioV2/AudioControlParm.h \
    ../Audio/AudioV2/AudioControlTypes.h \
    ../Audio/AudioV2/AudioDeviceParm.h \
    ../Audio/AudioV2/AudioDeviceTypes.h \
    ../Audio/AudioV2/AudioGlobalParm.h \
    ../Audio/AudioV2/AudioPatchbayParm.h \
    ../Audio/AudioV2/AudioPortParm.h \
    ../Audio/AudioV2/AudioPortParmTypes.h \
    ../Audio/AudioV2/ControllerType.h \
    ../Audio/BitDepthCode.h \
    ../Audio/ChannelBitmapBit.h \
    ../Audio/ChannelFilterStatusBit.h \
    ../Audio/HostType.h \
    ../Audio/PortSpecificOptionsBit.h \
    ../Audio/SampleRateCode.h \
    ../Base/ACK.h \
    ../Base/blockNotFound.h \
    ../Base/ByteCommandData.h \
    ../Base/BytesCommandData.h \
    ../Base/CommandData.h \
    ../Base/CommandDataKey.h \
    ../Base/CommandDefines.h \
    ../Base/CommandList.h \
    ../Base/Communicator.h \
    ../Base/ErrorCode.h \
    ../Base/Generator.h \
    ../Base/ICRunOnMain.h \
    ../Base/LibTypes.h \
    ../Base/Lookup.h \
    ../Base/MyAlgorithms.h \
    ../Base/PortType.h \
    ../Base/property.h \
    ../Base/stdafx.h \
    ../Base/StreamHelpers.h \
    ../Base/SysexCommand.h \
    ../Base/SysexParser.h \
    ../Base/TimerThread.h \
    ../Device/BootMode.h \
    ../Device/Device.h \
    ../Device/DeviceID.h \
    ../Device/DevicePID.h \
    ../Device/EthernetPortInfo.h \
    ../Device/GizmoCount.h \
    ../Device/GizmoInfo.h \
    ../Device/Info.h \
    ../Device/InfoID.h \
    ../Device/InfoList.h \
    ../Device/IPMode.h \
    ../Device/Reset.h \
    ../Device/ResetList.h \
    ../Device/SaveRestore.h \
    ../Device/SaveRestoreID.h \
    ../Device/SaveRestoreList.h \
    ../MIDI/FilterID.h \
    ../MIDI/FilterStatusBit.h \
#    ../MIDI/ICMMIDITools.h \
    ../MIDI/MIDIEndPoint.h \
    ../MIDI/MIDIInfo.h \
    ../MIDI/MIDIPortDetail.h \
    ../MIDI/MIDIPortDetailTypes.h \
    ../MIDI/MIDIPortFilter.h \
    ../MIDI/MIDIPortInfo.h \
    ../MIDI/MIDIPortRemap.h \
    ../MIDI/MIDIPortRoute.h \
    ../MIDI/PortMIDIFlags.h \
    ../MIDI/RemapID.h \
    ../MIDI/RTPMIDIConnectionDetail.h \
    ../MIDI/USBHostMIDIDeviceDetail.h \
    ../Audio/AudioV2/AudioControlDetailValue.h \
    ../Audio/AudioV2/AudioControlDetailValueTypes.h \
    ../Audio/AudioV2/AudioControlDetailTypes.h \
    ../Audio/AudioV2/AudioPortMeterValue.h \
    ../Audio/Mixer/MixerParm.h \
    ../Audio/Mixer/MixerInputControl.h \
    ../Audio/Mixer/MixerInputControlValue.h \
    ../Audio/Mixer/MixerInputParm.h \
    ../Audio/Mixer/MixerOutputControl.h \
    ../Audio/Mixer/MixerOutputControlValue.h \
    ../Audio/Mixer/MixerOutputParm.h \
    ../Audio/Mixer/MixerPortParm.h \
    ../Audio/Mixer/MixerMeterValue.h

INCLUDEPATH += \
    ../Audio \
    ../Audio/AudioV1 \
    ../Audio/AudioV2 \
    ../Audio/Mixer \
    ../Base \
    ../Device \
    ../MIDI
