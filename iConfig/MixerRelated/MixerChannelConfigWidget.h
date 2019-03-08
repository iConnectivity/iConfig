/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MixerChannelConfigWidget_H
#define MixerChannelConfigWidget_H

#include <vector>

#include "MixerChannelWidget.h"
#include "MixerInterface.h"
#include "AudioGlobalParm.h"
#include "AudioPortParm.h"
#include "DeviceInfo.h"

#include <QWidget>
#include <QList>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QMessageBox>
#include "property.h"

class MixerChannelConfigWidget : public QWidget
{
    Q_OBJECT
public:
    MixerChannelConfigWidget(MixerChannelWidget* mixer, DeviceInfoPtr device,
                             Word audioPortID, Byte mixerOutputNumber, Byte mixerInputNumber, QWidget* parent = 0);
    ~MixerChannelConfigWidget();
public slots:
    void handleDone();
    void handleCheckBox(bool checked);
signals:
private:
    DeviceInfoPtr device;
    MixerChannelWidget* mixer;
    QWidget* parent;
    Word audioPortID;
    Byte mixerOutputNumber;
    Byte mixerInputNumber;
    bool stereoLinked;

    MixerInterface* mixerInterface;

    QPushButton* doneButton;

    QList<QPair<int,int>> allAvailableChannels;
    QList<QPair<int,int>> currentChannels;
    QList<QPair<int,int>> currentChannelsPaired;
    QList<QCheckBox*> checkBoxes;
    QList<QString> portNames;
};

#endif // MixerChannelConfigWidget_H
