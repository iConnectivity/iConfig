/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef HORIZONTALMETERWIDGET_H
#define HORIZONTALMETERWIDGET_H

#include "MixerInterface.h"
#include "AudioGlobalParm.h"
#include "AudioPortParm.h"
#include "DeviceInfo.h"
#include "property.h"
#include "MixerOutputInterface.h"

#include <QWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <IRefreshWidget.h>

class HorizontalMeterWidget : public QWidget, public IRefreshWidget
{
  Q_OBJECT
public:
  HorizontalMeterWidget(DeviceInfoPtr device, Word audioPortID, Byte mixerOutputNumber, QWidget *parent = 0);
  ~HorizontalMeterWidget();

  void refreshWidget();
  void refreshMeters() { refreshWidget(); }

public slots:
  void turnOnClipping1();
  void turnOffClipping1();
  void turnOnClipping2();
  void turnOffClipping2();

private:
  void setProgressBarColor(QProgressBar *pb, double value);

  QTimer *clippingTimer1;
  QTimer *clippingTimer2;

  QProgressBar* meterBar1;
  QProgressBar* meterBar2;

  QPushButton *clippingPushButton1;
  QPushButton *clippingPushButton2;

  DeviceInfoPtr device;
  QWidget* parent;
  Word audioPortID;
  Byte mixerOutputNumber;

  MixerOutputInterface* mixerOutputInterface;
};

#endif // HORIZONTALMETERWIDGET_H
