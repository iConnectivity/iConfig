/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "QClickyDbLabel.h"

#include <QLayout>
#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>

QClickyDbLabel::QClickyDbLabel(IQClickyDbLabelAcceptor *parent) :
  parent(parent)
{
  label = new QLabel();
  label->setAlignment(Qt::AlignHCenter);
#ifdef _WIN32
  label->setStyleSheet("margin: 0px; font-size: 7pt; font-family:Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;");
#else
  label->setStyleSheet("margin: 0px; font-size: 9pt; font-family:Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;");
#endif
  label->setMinimumWidth(30);
  label->setMaximumWidth(30);
  label->setMinimumHeight(15);
  label->setMaximumHeight(15);

  textEdit = new QTextEdit();
  textEdit->setAlignment(Qt::AlignHCenter);
#ifdef _WIN32
  textEdit->setStyleSheet("margin:0px; padding: 0px; spacing: 0px; font-size: 7pt; font-family:Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;");
#else
  textEdit->setStyleSheet("margin:0px; padding: 0px; spacing: 0px; font-size: 9pt; font-family:Consolas,Monaco,Lucida Console,Liberation Mono,DejaVu Sans Mono,Bitstream Vera Sans Mono,Courier New, monospace;");
#endif
  textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  textEdit->setTabChangesFocus(true);
  textEdit->setMinimumWidth(30);
  textEdit->setMaximumWidth(30);
  textEdit->setMinimumHeight(15);
  textEdit->setMaximumHeight(15);
  textEdit->installEventFilter(this);

  dbValue = 0;

  auto lay = new QHBoxLayout();
  lay->setMargin(0);
  lay->setContentsMargins(0,0,0,0);
  setLayout(lay);
  setContentsMargins(0,0,0,0);
  setMaximumWidth(30);
  setMinimumWidth(30);
  setMinimumHeight(15);
  setMaximumHeight(15);
  textEdit->setLineWrapMode(QTextEdit::NoWrap);
  textEdit->setWordWrapMode(QTextOption::NoWrap);

  layout()->addWidget(label);
  layout()->addWidget(textEdit);
  textEdit->hide();
}

void QClickyDbLabel::setValue(double newValue) {
  dbValue = newValue;
  if (newValue == -100000) {
    label->setText("<b>" + QString("- &infin;").rightJustified(5, ' ').replace(" ", "&nbsp;") + "</b>");
  }
  else if (newValue == -100001) {
    label->setText("<b>" + QString("N/A") + "</b>");
  }
  else {
    label->setText("<b>" + QString::number(dbValue,'f',1).rightJustified(5, ' ').replace(" ", "&nbsp;") + "</b>");
  }
}

void QClickyDbLabel::mousePressEvent(QMouseEvent *mouseEvent) {
  textEdit->setText(QString::number(dbValue,'f',1));
  textEdit->setMaximumHeight(label->height() + 10);
  label->hide();
  textEdit->show();
  textEdit->setFocus();
  textEdit->selectAll();
}

void QClickyDbLabel::mouseReleaseEvent(QMouseEvent *mouseEvent) {

}

bool QClickyDbLabel::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Return) {
      textEdit->clearFocus();
      bool valid;
      double converted = textEdit->toPlainText().toDouble(&valid);
      if (valid) {
        parent->labelDone(converted);
        setValue(converted);
      }

      finishEditing();
      return true;
    }
    else if (keyEvent->key() == Qt::Key_Escape) {
      textEdit->clearFocus();
      finishEditing();
      return true;
    }
    else {
      return QObject::eventFilter(obj, event);
    }
  }
  else if (event->type() == QEvent::FocusOut) {
    finishEditing();
    return true;
  }
  else {
    // standard event processing
    return QObject::eventFilter(obj, event);
  }
}

void QClickyDbLabel::finishEditing() {
  textEdit->hide();
  label->show();
}
