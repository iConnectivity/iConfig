/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __MIXERPORTWIDGET_H__
#define __MIXERPORTWIDGET_H__

#include "DeviceInfo.h"
#include <QGroupBox>
#include <QList>
#include <QSignalMapper>
#include "IRefreshWidget.h"
#include "MixerInterface.h"
#include "MixerInputInterface.h"
#include "MixerOutputInterface.h"
#include "MixerChannelWidget.h"

struct IRefreshWidget;

/*  +-------------- MixerWidget
 *  |    +--------- containing widget (this)
 *  V    |    +---- MixerChannelWidget
 * +-----V----|------------------------------+
 * |+--[MPW]--V------------++--[MPW]--------+|
 * ||+------+--//--+------+||+------+--//--+||
 * |||      |      |      ||||      |      |||
 * ||| MCW  | MCW  | MCW  |||| MCW  | MCW  |||
 * |||      |      |      ||||      |      |||
 * ||+------+--//--+------+||+------+--//--+||
 * |+----------------------++---------------+|
 * +-----------------------------------------+
 */

class MixerWidget;

class MixerPortWidget : public QWidget, public IRefreshWidget {
  Q_OBJECT
 public:
  explicit MixerPortWidget(DeviceInfoPtr device, Word audioPortID, Byte mixerOutputNumber, MixerWidget *parent = 0);
  virtual ~MixerPortWidget();

  void refreshWidget();
  void refreshMeters();

 signals:

 public slots:
  //void handleConfigButton();
  void handleDoneButton();

  void notifyStereoLinkHappened();
  void notifyChannelChange();

protected:
  void paintEvent(QPaintEvent *);
private slots:
   void callDoAdjustSize();
   void linkButtonClicked(int whichButton);

 private:
  void buildAll();
  void buildChannelWidgets();

  void clear();

  void remove(QLayout* layout);

  DeviceInfoPtr device;
  Word audioPortID;
  Byte mixerOutputNumber;
  MixerWidget *parent;

  QHBoxLayout *topLayout;

  QVector<int> savedInputs;
  int savedNumberOfOutputs;

  MixerInterface* mixerInterface;
  MixerInputInterface* mixerInputInterface;
  MixerOutputInterface* mixerOutputInterface;

  bool refreshable;
  bool stereoLinkHappened;
  bool doneBuilding;

  std::vector<IRefreshWidget*> refreshList;
  std::vector<MixerChannelWidget*> channelWidgets;

  QSignalMapper* signalMapper;
};

#endif  // __MIXERPORTWIDGET_H__
