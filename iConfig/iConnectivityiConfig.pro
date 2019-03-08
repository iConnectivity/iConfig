#-------------------------------------------------
#
# Project created by QtCreator 2013-03-07T15:25:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6

TARGET = "iConnectivity iConfig"
TEMPLATE = app
include(./qtsinglapplication/qtsingleapplication.pri)


#mac: LIBS += -F -isystem/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/System/Library/Frameworks/ -framework CoreMIDI
#mac: LIBS += -F -isystem/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/System/Library/Frameworks/ -framework CoreFoundation
#mac: LIBS += -F -isystem/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/System/Library/Frameworks/ -framework CoreAudio

#mac: LIBS += -F -isystem/Library/Frameworks/ -framework CoreMIDI
#mac: LIBS += -F -isystem/Library/Frameworks/ -framework CoreFoundation
#mac: LIBS += -F -isystem/Library/Frameworks/ -framework CoreAudio

SOURCES +=                                                        \
    ../rtmidi-2.1.1/RtMidi.cpp                                    \
    ./AudioRelated/AudioInfoForm.cpp                              \
    ./AudioRelated/AudioPatchbayForm.cpp                          \
    ./MIDIRelated/MIDIChannelRemapForm.cpp                        \
    ./MIDIRelated/MIDIControllerFilterForm.cpp                    \
    ./MIDIRelated/MIDIControllerRemapForm.cpp                     \
    ./MIDIRelated/MIDIInfoForm.cpp                                \
    ./MIDIRelated/MIDIPortFiltersForm.cpp                         \
    ./MIDIRelated/MIDIPortRoutingForm.cpp                         \
    ./MIDIRelated/MIDIPortSelectionForm.cpp                       \
    ./FirmwareRelated/FirmwareUpgradeDialog.cpp                   \
    ./CCList.cpp                                                  \
    ./DeviceInfo.cpp                                              \
    ./DeviceInfoForm.cpp                                          \
    ./DeviceRebooter.cpp                                          \
    ./DeviceSelectionDialog.cpp                                   \
    ./Main.cpp                                                    \
    ./MainWindow.cpp                                              \
    ./MySleep.cpp                                                 \
    ./RefreshObject.cpp                                           \
    ./RotatedLabel.cpp                                            \
    ./TableListener.cpp                                           \
    ./TimerThread.cpp                                             \
    ./TreeUtils.cpp                                               \
    ./MyCheckBox.cpp                                              \
    ./MyLineEdit.cpp                                              \
    ./MyComboBox.cpp                                              \
    ./MyLabel.cpp                                                 \
    ./MyConverters.cpp                                            \
    ./MySpinBox.cpp                                               \
    ./CenteredContainer.cpp                                       \
    ./AudioRelated/IAudioPatchbaySource.cpp                            \
    ./AudioRelated/PatchbayV2Form.cpp                             \
    ./AudioRelated/AudioPatchbaySourceV2.cpp                      \
    ./AudioRelated/AudioPatchbaySourceV1.cpp \
    AudioRelated/AudioControlFeatureSource.cpp \
    AudioRelated/AudioControlWidget.cpp \
    AudioRelated/AudioFeatureControlWidget.cpp \
    AudioRelated/AudioChannelsControlWidget.cpp \
    MixerRelated/MixerWidget.cpp \
    MixerRelated/MixerPortWidget.cpp \
    MixerRelated/MixerInputInterface.cpp \
    MixerRelated/MixerOutputInterface.cpp \
    MixerRelated/MixerInterface.cpp \
    MixerRelated/MixerChannelWidget.cpp \
    QtSingleApplicationWrapper.cpp \
    QClickyDial.cpp \
    QClickySlider.cpp \
    QClickyDbLabel.cpp \
    MixerRelated/HorizontalMeterWidget.cpp \
    MixerRelated/MixerChannelConfigWidget.cpp \
    AudioRelated/AudioChannelConfigWidget.cpp \
    DeviceInformationDialog.cpp \
    Presets/ICSaveDialog.cpp \
    Presets/ICRestoreDialog.cpp \
    FirmwareRelated/FirmwareCheckDialog.cpp

HEADERS +=                                                        \
#    ../rtmidi-2.0.1/RtError.h                                     \
#    ../rtmidi-2.0.1/RtMidi.h                                      \
    ../rtmidi-2.1.1/RtMidi.h                                      \
    ./AudioRelated/AudioInfoForm.h                                \
    ./AudioRelated/AudioPatchbayForm.h                            \
    ./MIDIRelated/MIDIChannelRemapForm.h                          \
    ./MIDIRelated/MIDIControllerFilterForm.h                      \
    ./MIDIRelated/MIDIControllerRemapForm.h                       \
    ./MIDIRelated/MIDIInfoForm.h                                  \
    ./MIDIRelated/MIDIPortFiltersForm.h                           \
    ./MIDIRelated/MIDIPortRoutingForm.h                           \
    ./MIDIRelated/MIDIPortSelectionForm.h                         \
    ./FirmwareRelated/FirmwareUpgradeDialog.h                     \
    ./BlockState.h                                                \
    ./CCList.h                                                    \
    ./CommandQList.h                                              \
    ./DeviceInfo.h                                                \
    ./DeviceInfoForm.h                                            \
    ./DeviceRebooter.h                                            \
    ./DeviceSelectionDialog.h                                     \
    ./MainWindow.h                                                \
    ./MySleep.h                                                   \
    ./PortIDVector.h                                              \
    ./RefreshObject.h                                             \
    ./RotatedLabel.h                                              \
    ./Screen.h                                                    \
    ./TableListener.h                                             \
    ./TimerThread.h                                               \
    ./TreeUtils.h                                                 \
    ./MyCheckBox.h                                                \
    ./MyLineEdit.h                                                \
    ./GetSetFunc.h                                                \
    ./MyComboBox.h                                                \
    ./IRefreshData.h                                              \
    ./MyLabel.h                                                   \
    ./MyConverters.h                                              \
    ./MySpinBox.h                                                 \
    ./CenteredContainer.h                                         \
    ./AudioRelated/IAudioPatchbaySource.h                              \
    ./AudioRelated/PatchbayV2Form.h                               \
    ./AudioRelated/AudioPatchbaySourceV2.h                        \
    ./AudioRelated/AudioPatchbaySourceV1.h \
    AudioRelated/IAudioControlSelectorSource.h \
    AudioRelated/IAudioControlFeatureSource.h \
    AudioRelated/AudioControlFeatureSource.h \
    AudioRelated/AudioControlWidget.h \
    AudioRelated/AudioFeatureControlWidget.h \
    AudioRelated/AudioChannelsControlWidget.h \
    MixerRelated/MixerPortWidget.h \
    MixerRelated/MixerInputInterface.h \
    MixerRelated/MixerOutputInterface.h \
    MixerRelated/MixerInterface.h \
    MixerRelated/MixerWidget.h \
    MixerRelated/MixerChannelWidget.h \
    QtSingleApplicationWrapper.h \
    QClickyDial.h \
    QClickySlider.h \
    QClickyDbLabel.h \
    IQClickyDbLabelAcceptor.h \
    MixerRelated/HorizontalMeterWidget.h \
    MixerRelated/MixerChannelConfigWidget.h \
    AudioRelated/AudioChannelConfigWidget.h \
    DeviceInformationDialog.h \
    Presets/ICSaveDialog.h \
    Presets/ICRestoreDialog.h \
    FirmwareRelated/FirmwareCheckDialog.h

FORMS +=                                                          \
    ./AudioRelated/AudioInfoForm.ui                               \
    ./AudioRelated/AudioPatchbayForm.ui                           \
    ./MIDIRelated/MIDIChannelRemapForm.ui                         \
    ./MIDIRelated/MIDIControllerFilterForm.ui                     \
    ./MIDIRelated/MIDIControllerRemapForm.ui                      \
    ./MIDIRelated/MIDIInfoForm.ui                                 \
    ./MIDIRelated/MIDIPortFiltersForm.ui                          \
    ./MIDIRelated/MIDIPortRoutingForm.ui                          \
    ./MIDIRelated/MIDIPortSelectionForm.ui                        \
    ./FirmwareRelated/FirmwareUpgradeDialog.ui                    \
    ./DeviceInfoForm.ui                                           \
    ./DeviceSelectionDialog.ui                                    \
    ./MainWindow.ui                                               \
    AudioRelated/PatchbayV2Form.ui \
    DeviceInformationDialog.ui \
    Presets/ICSaveDialog.ui \
    Presets/ICRestoreDialog.ui \
    FirmwareRelated/FirmwareCheckDialog.ui

win32: QMAKE_CXXFLAGS += /F 32000000
win32: QMAKE_LFLAGS += /STACK:32000000

DEFINES += BOOST_RESULT_OF_USE_DECLTYPE

#mac: DEFINES        += __MACOSX_CORE__
win32: DEFINES      += __WINDOWS_MM__
win32: DEFINES      += __RTMIDI_DEBUG__
win32: LIBS         += -L"C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/" -L"C:/Program Files (x86)/Microsoft Visual C++ Compiler Nov 2013 CTP/lib" -lWinMM

#mac: CONFIG        += precompile_header
#mac: PRECOMPILED_HEADER = stdafx.h



mac: QMAKE_CXXFLAGS = -std=c++11 -stdlib=libstdc++ -Wno-unused-parameter -Wno-deprecated-register -O2 -mmacosx-version-min=10.6
mac: QMAKE_LFLAGS = -std=c++11 -stdlib=libstdc++ -Wno-unused-parameter -Wno-deprecated-register -O2 -mmacosx-version-min=10.6
mac: QMAKE_CXXFLAGS += -isystem /opt/local/include

mac: LIBS           += -framework CoreMIDI
mac: LIBS           += -framework CoreFoundation
mac: LIBS           += -framework CoreAudio

mac: DEPENDPATH     += $$PWD/../../rtmidi-2.1.1
mac: INCLUDEPATH    += $$PWD/../../rtmidi-2.1.1
mac: DEPENDPATH     += $$PWD/../rtmidi-2.1.1
mac: INCLUDEPATH    += $$PWD/../rtmidi-2.1.1

win32: DEPENDPATH   += $$PWD/../rtmidi-2.1.1
win32: INCLUDEPATH  += $$PWD/../rtmidi-2.1.1

DEPENDPATH          += $$PWD/../GeneSysLib/
INCLUDEPATH         += $$PWD/../GeneSysLib/
INCLUDEPATH         += \
    $$PWD/../GeneSysLib/Audio \
    $$PWD/../GeneSysLib/Audio/Mixer \
    $$PWD/../GeneSysLib/Audio/AudioV1 \
    $$PWD/../GeneSysLib/Audio/AudioV2 \
    $$PWD/../GeneSysLib/Base \
    $$PWD/../GeneSysLib/Device \
    $$PWD/../GeneSysLib/MIDI


mac: QMAKE_CFLAGS   += -gdwarf-2
mac: QMAKE_CXXFLAGS += -gdwarf-2



RESOURCES +=                                                    \
    Resources.qrc

OTHER_FILES +=                                                  \
    ./Images/EmptyBlock.png                                     \
    ./Images/scale.png                                     \
    ./Images/FullBlock.png                                      \
    ./Images/HalfBlock.png                                      \
    ./Images/block_icm1box.png                                  \
    ./Images/block_icm1device.png                               \
    ./Images/block_icm2plusbox.png                              \
    ./Images/block_icm2plusdevice.png                           \
    ./Images/block_icm4plusbox.png                              \
    ./Images/block_icm4plusdevice.png                           \
    ./Images/block_ica4plusdevice.png                           \
    ./Images/block_miobox.png                                   \
    ./Images/block_miodevice.png                                \
    ./Icons.ico                                                 \
    ./iconResource.rc                                           \
    ./Icon.icns

win32: INCLUDEPATH  += C:/boost_1_57_0/
win32: DEPENDPATH   += C:/boost_1_57_0/

mac: INCLUDEPATH += /opt/local/include/
mac: DEPENDPATH += /opt/local/include/
mac: INCLUDEPATH += -isystem /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include/
mac: INCLUDEPATH += -isystem /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include/c++/4.2.1/
mac: DEPENDPATH += -isystem /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include/

ICON = ./Icon.icns
win32: RC_FILE += iconResource.rc

win32: LIBS += -LC:/boost_1_57_0/lib32-msvc-12.0/

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../GeneSysLib/build-GeneSysLib-Desktop-Release/release/ -lGeneSysLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../GeneSysLib/build-GeneSysLib-Desktop-Debug/debug/ -lGeneSysLib
else:macx: LIBS += -L$$PWD/../Software/GeneSysLib/build-GeneSysLib-Default-Release/ -lGeneSysLib

INCLUDEPATH += $$PWD/../GeneSysLib/build-GeneSysLib-Desktop-Release
DEPENDPATH += $$PWD/../GeneSysLib/build-GeneSysLib-Desktop-Release
mac:INCLUDEPATH += $$PWD/../Software/GeneSysLib/build-GeneSysLib-Desktop-Release
mac:DEPENDPATH += $$PWD/../Software/GeneSysLib/build-GeneSysLib-Desktop-Release
win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../GeneSysLib/build-GeneSysLib-Default-Release/libGeneSysLib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../GeneSysLib/build-GeneSysLib-Default-Debug/libGeneSysLib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../GeneSysLib/build-GeneSysLib-Desktop-Release/release/GeneSysLib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../GeneSysLib/build-GeneSysLib-Desktop-Debug/debug/GeneSysLib.lib
else:macx: PRE_TARGETDEPS += $$PWD/../Software/GeneSysLib/build-GeneSysLib-Default-Release/libGeneSysLib.a

DISTFILES += \
    Images/block_pa12device.png

