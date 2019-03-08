/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __MIXERWIDGET_H__
#define __MIXERWIDGET_H__

#include "MixerPortWidget.h"
#include "MixerInterface.h"
#include "DeviceInfo.h"
#include "CommandQList.h"
#include "refreshobject.h"
#include "../AudioRelated/AudioControlWidget.h"
#include "HorizontalMeterWidget.h"

#include <QWidget>
#include <QComboBox>
#include <vector>
#include <QScrollArea>

struct IRefreshWidget;

/*  +-------------- containing widget (this)
 *  |    +--------- MixerPortWidget
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

class MyTabWidget:public QTabWidget
{
public:
    MyTabWidget(QWidget* parent = 0)
    {
      setParent(parent);
    }

    //Overridden method from QTabWidget
    QTabBar* tabBar()
    {
      return QTabWidget::tabBar();
    }
};

class MixerWidget : public RefreshObject {
  Q_OBJECT
public:

  static const QString DefaultWindowTitle;
  static CommandQList Query();

  explicit MixerWidget(DeviceInfoPtr device, QWidget *parent = 0);
  virtual ~MixerWidget();

public slots:
  void mixerTabChanged(int which);
  void handleEditButton();

//  void notifyStereoLinkHappened();
//  void redrawAll();

  void doAdjustSize();

  void refreshWidget();
  void refreshMeters();

signals:
  void doneAudioRefresh();
  void doneMetersRefresh();
private:
  void buildMixerPortWidget();
  void buildMixerTabs();
  void buildMixerHeader();
  void buildPortComboBox();
  void buildMixerEditButton();
  void buildInputControls();
  void buildOutputControls();
  void readdAnalogControlsToRefreshList();

  void remove(QLayout* layout);



  DeviceInfoPtr device;
  MixerInterface* mixerInterface;
  MixerInputInterface* mixerInputInterface;
  MixerOutputInterface* mixerOutputInterface;

  MixerPortWidget* mixerPortWidget;

  QList<QPair<Word,Byte>> outputLookup;

  Byte mixerOutputNumber;
  Word audioPortID;

  //QComboBox* portComboBox;
  QPushButton *editButton;
  MyTabWidget* mixerTabWidget;
  QVBoxLayout* tabLayout;
  QWidget* tabHolderTop;
  QHBoxLayout* mixerScrollLayout;
  QWidget* mixerScrollWidget;
  QScrollArea *mixerScroll;

  QTabWidget* inputTabWidget;
  QTabWidget* outputTabWidget;

  QWidget* inputTabHolder;
  QWidget* outputTabHolder;

  int minimumSize;

  std::vector<IRefreshWidget*> refreshList;
  std::vector<HorizontalMeterWidget*> headerMeters;
  std::vector<AudioChannelsControlWidget*> analogList;
  std::vector<AudioChannelsControlWidget*> outputList;
  std::vector<AudioChannelsControlWidget*> inputList;
};

#endif  // __MIXERWIDGET_H__
