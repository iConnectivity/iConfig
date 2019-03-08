/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOCONTROLWIDGET_H__
#define __AUDIOCONTROLWIDGET_H__

#include "AudioChannelsControlWidget.h"
#include "AudioControlFeatureSource.h"
#include "DeviceInfo.h"
#include "CommandQList.h"
#include "refreshobject.h"

#include <QWidget>
#include <vector>

struct IRefreshWidget;

/*  +-------------- containing widget (this)
 *  |    +--------- AudioChannelsControlWidget
 *  V    |    +---- AudioFeatureControlWidget
 * +-----V----|------------------------------+
 * |+--[ACCW]-V------------++--[ACCW]-------+|
 * ||+------+--//--+------+||+------+--//--+||
 * |||      |      |      ||||      |      |||
 * ||| AFCW | AFCW | AFCW |||| AFCW | AFCW |||
 * |||      |      |      ||||      |      |||
 * ||+------+--//--+------+||+------+--//--+||
 * |+----------------------++---------------+|
 * +-----------------------------------------+
 */

class AudioControlWidget : public RefreshObject {
  Q_OBJECT
 public:

  static const QString DefaultWindowTitle;
  static CommandQList Query();

  explicit AudioControlWidget(DeviceInfoPtr device, QWidget *parent = 0);

private:
  void buildFeatureGroups();

  void refreshWidget();

  DeviceInfoPtr device;
  std::vector<AudioControlFeatureSourcePtr> audioControlFeatures;

  std::vector<IRefreshWidget*> refreshList;
};

#endif  // __AUDIOCONTROLWIDGET_H__
