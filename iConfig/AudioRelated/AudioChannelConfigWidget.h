/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef AudioChannelConfigWidget_H
#define AudioChannelConfigWidget_H

#include <vector>

#include "AudioGlobalParm.h"
#include "AudioPortParm.h"
#include "AudioControlFeatureSource.h"
#include "AudioFeatureControlWidget.h"
#include "DeviceInfo.h"
#include "MixerRelated/MixerInterface.h"
#include "MixerRelated/MixerInputInterface.h"
#include "MixerRelated/MixerOutputInterface.h"

#include <QWidget>
#include <QList>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QMessageBox>
#include "property.h"

class AudioChannelConfigWidget : public QWidget
{
    Q_OBJECT
public:
    AudioChannelConfigWidget(AudioFeatureControlWidget* audioControl, DeviceInfoPtr device, IAudioControlFeatureSourcePtr audioFeatureSource,
                                                     Word audioPortID, Byte inChannelID, Byte outChannelID, QWidget* parent = 0, int totalOutputChannelInPort = 6);  //Bugfixing for PlayAudio, zx-03-02
    ~AudioChannelConfigWidget();
public slots:
    void handleDone();
    void handleCheckBox(bool checked);
signals:
private:
    DeviceInfoPtr device;
    AudioFeatureControlWidget* audioControl;
    QWidget* parent;
    Word audioPortID;
    Byte inChannelID;
    Byte outChannelID;
    Byte trueChannelID;
    bool stereoLinked;

    IAudioControlFeatureSourcePtr audioFeatureSource;
    MixerInterface* mixerInterface;
    MixerInputInterface* mixerInputInterface;
    MixerOutputInterface* mixerOutputInterface;

    QPushButton* doneButton;

    QList<QPair<int,int>> allAvailableChannels;
    QList<QPair<int,int>> currentChannels;
    QList<QPair<int,int>> currentChannelsPaired;
    QList<QCheckBox*> checkBoxes;
    QList<QString> portNames;

    int         m_TotalOutputChannelInPort;  //Bugfixing for PlayAudio, zx-03-02

};

#endif // AudioChannelConfigWidget_H
