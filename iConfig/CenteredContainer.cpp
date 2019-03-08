/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "CenteredContainer.h"

#include <QLayout>
#include <QWidget>

namespace CenteredContainer {

QWidget *centeredContainer(QWidget *child) {
  auto* w = new QWidget();
  auto* l = new QHBoxLayout();
  l->setAlignment(Qt::AlignCenter);
  l->addWidget(child);
  l->setContentsMargins(0, 0, 0, 0);
  w->setLayout(l);
  return w;
}

}  // namespace CenteredContainer
