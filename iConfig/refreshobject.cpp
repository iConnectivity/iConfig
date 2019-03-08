/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "RefreshObject.h"

RefreshObject::RefreshObject(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_DeleteOnClose);
}

RefreshObject::~RefreshObject() { this->disconnect(); }
