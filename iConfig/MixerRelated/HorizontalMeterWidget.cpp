/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "HorizontalMeterWidget.h"
#include "MixerChannelWidget.h"

HorizontalMeterWidget::HorizontalMeterWidget(DeviceInfoPtr device, Word audioPortID, Byte mixerOutputNumber, QWidget *parent) :
  device(device),
  audioPortID(audioPortID),
  mixerOutputNumber(mixerOutputNumber),
  QWidget(parent)
{
  mixerOutputInterface = new MixerOutputInterface(device);

  meterBar1 = new QProgressBar();
  setProgressBarColor(meterBar1, -20);

  meterBar1->setTextVisible(false);
  meterBar1->setMinimumHeight(4);
  meterBar1->setMaximumHeight(4);

  meterBar1->setOrientation(Qt::Horizontal);

  meterBar1->setMaximum(299);
  meterBar1->setMinimum(0);

  auto meterBar1spacer1 = new QWidget();
  meterBar1spacer1->setMaximumWidth(4);
  meterBar1spacer1->setMinimumWidth(4);
  meterBar1spacer1->setContentsMargins(0,0,0,0);

  auto meterBar1spacer1layout = new QHBoxLayout();
  meterBar1spacer1layout->setContentsMargins(0,0,0,0);
  meterBar1spacer1layout->setSpacing(0);

  clippingPushButton1 = new QPushButton();
  clippingPushButton1->setEnabled(false);
  clippingPushButton1->setMaximumWidth(2);
  clippingPushButton1->setMinimumHeight(2);
  clippingPushButton1->setMaximumHeight(2);

  clippingPushButton1->setCheckable(true);
  clippingPushButton1->setChecked(false);
  clippingPushButton1->setStyleSheet(QString("QPushButton {border: 0px; border-radius: 0px; background-image: url(:/mixer_graphics/Images/mixer_graphics/clip_horz_off.png);} \
                                        QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/clip_horz_on.png); }"));
  meterBar1spacer1layout->addWidget(clippingPushButton1);
  meterBar1spacer1layout->setAlignment(clippingPushButton1, Qt::AlignCenter);
  meterBar1spacer1->setLayout(meterBar1spacer1layout);

  auto meterBar1spacer2 = new QWidget();
  meterBar1spacer2->setMaximumWidth(4);
  meterBar1spacer2->setMaximumWidth(4);

  QHBoxLayout* meterBar1BoxLayout = new QHBoxLayout();
  meterBar1BoxLayout->setSpacing(0);
  meterBar1BoxLayout->setContentsMargins(0,0,0,0);
  meterBar1BoxLayout->addWidget(meterBar1spacer2);
  meterBar1BoxLayout->addWidget(meterBar1);
  meterBar1BoxLayout->addWidget(meterBar1spacer1);
  QWidget* meterBar1Box = new QWidget();
  meterBar1Box->setContentsMargins(5,0,5,0);
  meterBar1Box->setLayout(meterBar1BoxLayout);

  QWidget* meterBar2Box = 0;

  meterBar2 = new QProgressBar();
  setProgressBarColor(meterBar2, -20);
  meterBar2->setTextVisible(false);
  meterBar2->setMinimumHeight(4);
  meterBar2->setMaximumHeight(4);

  meterBar2->setOrientation(Qt::Horizontal);

  meterBar2->setMaximum(299);
  meterBar2->setMinimum(0);

  auto meterBar2spacer1 = new QWidget();
  meterBar2spacer1->setMaximumWidth(4);
  meterBar2spacer1->setMinimumWidth(4);
  meterBar2spacer1->setContentsMargins(0,0,0,0);

  auto meterBar2spacer1layout = new QHBoxLayout();
  meterBar2spacer1layout->setContentsMargins(0,0,0,0);
  meterBar2spacer1layout->setSpacing(0);

  clippingPushButton2 = new QPushButton();
  clippingPushButton2->setEnabled(false);
  clippingPushButton2->setMaximumWidth(2);
  clippingPushButton2->setMinimumHeight(2);
  clippingPushButton2->setMaximumHeight(2);

  clippingPushButton2->setCheckable(true);
  clippingPushButton2->setChecked(false);
  clippingPushButton2->setStyleSheet(QString("QPushButton {border: 0px; border-radius: 0px; background-image: url(:/mixer_graphics/Images/mixer_graphics/clip_horz_off.png);} \
                                        QPushButton:checked { background-image: url(:/mixer_graphics/Images/mixer_graphics/clip_horz_on.png); }"));
  meterBar2spacer1layout->addWidget(clippingPushButton2);
  meterBar2spacer1layout->setAlignment(clippingPushButton2, Qt::AlignCenter);
  meterBar2spacer1->setLayout(meterBar2spacer1layout);

  auto meterBar2spacer2 = new QWidget();
  meterBar2spacer2->setMaximumWidth(4);
  meterBar2spacer2->setMaximumWidth(4);

  QHBoxLayout* meterBar2BoxLayout = new QHBoxLayout();
  meterBar2BoxLayout->setSpacing(0);
  meterBar2BoxLayout->setContentsMargins(0,0,0,0);
  meterBar2BoxLayout->addWidget(meterBar2spacer2);
  meterBar2BoxLayout->addWidget(meterBar2);
  meterBar2BoxLayout->addWidget(meterBar2spacer1);
  meterBar2Box = new QWidget();
  meterBar2Box->setContentsMargins(5,0,5,0);
  meterBar2Box->setLayout(meterBar2BoxLayout);

  clippingTimer1 = new QTimer();
  clippingTimer2 = new QTimer();
  connect(clippingTimer1,SIGNAL(timeout()), this, SLOT(turnOffClipping1()));
  connect(clippingTimer2,SIGNAL(timeout()), this, SLOT(turnOffClipping2()));

  QVBoxLayout *topLayout = new QVBoxLayout();
  topLayout->setSpacing(0);
  topLayout->setContentsMargins(0,0,0,0);

  topLayout->addWidget(meterBar1Box);
  topLayout->addWidget(meterBar2Box);

  setLayout(topLayout);
  setContentsMargins(20,0,20,0);
}

HorizontalMeterWidget::~HorizontalMeterWidget()
{

}

void HorizontalMeterWidget::refreshWidget()
{
  bool volAvailable = false;
  int meterCurrent1 = 0;
  int meterCurrent2 = 0;

  volAvailable = mixerOutputInterface->isVolumeAvailable(audioPortID);
  meterCurrent1 = mixerOutputInterface->meterCurrent(audioPortID, mixerOutputNumber);
  meterCurrent2 = mixerOutputInterface->meterCurrent(audioPortID, mixerOutputNumber + 1);
  if (volAvailable) {
    int meterPng1 = MixerChannelWidget::toPixelsFromICA(((double)20 * log10((double)meterCurrent1 / 8192.0)) * 256.0);
    int meterPng2 = MixerChannelWidget::toPixelsFromICA(((double)20 * log10((double)meterCurrent2 / 8192.0)) * 256.0);

    if ((((double)20 * log10((double)meterCurrent1 / 8192.0)) >= 0.0)) {
      turnOnClipping1();
    }
    if ((((double)20 * log10((double)meterCurrent2 / 8192.0)) >= 0.0)) {
      turnOnClipping2();
    }

    if (meterCurrent1 == 0) {
      meterBar1->setValue(0);
      setProgressBarColor(meterBar1, -20);
    }
    else {
      meterBar1->setValue(meterPng1);
      setProgressBarColor(meterBar1, ((double)20 * log10((double)meterCurrent1 / 8192.0)));
    }
    if (meterCurrent2 == 0) {
      meterBar2->setValue(0);
      setProgressBarColor(meterBar2, -20);
    }
    else {
      meterBar2->setValue(meterPng2);
      setProgressBarColor(meterBar2, ((double)20 * log10((double)meterCurrent2 / 8192.0)));
    }
  }
}
void HorizontalMeterWidget::setProgressBarColor(QProgressBar* pb, double value) {
  if (value < 0) {
    pb->setStyleSheet("QProgressBar{ margin-top: 1px; margin-bottom: 1px; border: 0px; background-color: #545454; border-radius: 0px; text-align: center} \
    QProgressBar::chunk { background-color: #11e000; }");
  }
  else if (value < 5.9) {
    pb->setStyleSheet("QProgressBar{ margin-top: 1px; margin-bottom: 1px; border: 0px; background-color: #545454; border-radius: 0px; text-align: center} \
    QProgressBar::chunk { background-color: #FFCC00; }");
  }
  else{
    pb->setStyleSheet("QProgressBar{ margin-top: 1px; margin-bottom: 1px; border: 0px; background-color: #545454; border-radius: 0px; text-align: center} \
    QProgressBar::chunk { background-color: #ff0033; }");
  }
}

void HorizontalMeterWidget::turnOnClipping1()
{
  clippingPushButton1->setChecked(true);
  clippingTimer1->stop();
  clippingTimer1->setSingleShot(true);
  clippingTimer1->setInterval(1000);
  clippingTimer1->start();
}

void HorizontalMeterWidget::turnOffClipping1()
{
  clippingPushButton1->setChecked(false);
}

void HorizontalMeterWidget::turnOnClipping2()
{
  clippingPushButton2->setChecked(true);
  clippingTimer2->stop();
  clippingTimer2->setSingleShot(true);
  clippingTimer2->setInterval(1000);
  clippingTimer2->start();
}

void HorizontalMeterWidget::turnOffClipping2()
{
  clippingPushButton2->setChecked(false);
}
