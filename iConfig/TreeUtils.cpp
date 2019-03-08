/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "TreeUtils.h"

#include "MyLineEdit.h"

#include <QHeaderView>
#include <QLayout>
#include <QTableWidget>
#include <QTreeWidgetItem>

namespace TreeUtils {

QTableWidget *createInfoTableWidget(QStringList labels, int rowCount) {
  auto *tableWidget = new QTableWidget();

  tableWidget->setColumnCount(labels.size());
  tableWidget->setHorizontalHeaderLabels(labels);
  tableWidget->setRowCount(rowCount);
  tableWidget->horizontalHeaderItem(0)
      ->setSizeHint(QSize(10, kDefaultHeaderSize));
  tableWidget->verticalHeader()->hide();
  tableWidget->setMaximumHeight(kDefaultRowSize * rowCount +
                                kDefaultHeaderSize + 2);
  tableWidget->setMinimumHeight(kDefaultRowSize * rowCount +
                                kDefaultHeaderSize + 2);
  tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
  tableWidget->setStyleSheet(
      "QLabel { padding-left: 4px; padding-right: 4px } *{font-size:12px }");

  return tableWidget;
}

MyCheckBox *addCheckBox(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                        QString title, bool checked) {
  Q_ASSERT(treeWidget != nullptr);

  auto *const treeItem = new QTreeWidgetItem(parent);
  treeItem->setText(0, title);

  auto *const checkBox = new MyCheckBox(treeWidget);
  checkBox->setChecked(checked);

  treeWidget->setItemWidget(treeItem, 1, checkBox);

  return checkBox;
}

MyCheckBox *addCheckBox(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                        QString title, const GetBoolFunctor &getFunc,
                        const SetBoolFunctor &setFunc) {
  Q_ASSERT(treeWidget != nullptr);

  auto *const treeItem = new QTreeWidgetItem(parent);
  treeItem->setText(0, title);

  auto *const checkBox = new MyCheckBox(getFunc, setFunc, treeWidget);

  treeWidget->setItemWidget(treeItem, 1, checkBox);

  return checkBox;
}

MyLabel *addLabelItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                      QString title, QString text) {
  Q_ASSERT(treeWidget != nullptr);

  QTreeWidgetItem *treeItem = new QTreeWidgetItem(parent);
  treeItem->setText(0, title);

  auto *const label = new MyLabel(treeWidget);
  label->setText(text);

  treeWidget->setItemWidget(treeItem, 1, label);

  return label;
}

MyLabel *addLabelItem(QTreeWidget *_treeWidget, QTreeWidgetItem *_parent,
                      QString _title, GetQStringFunctor _getFunc) {
  Q_ASSERT(_treeWidget != nullptr);

  auto *const treeItem = new QTreeWidgetItem(_parent);
  treeItem->setText(0, _title);

  auto *const label = new MyLabel(_getFunc, _treeWidget);

  _treeWidget->setItemWidget(treeItem, 1, label);

  return label;
}

MyLabel *addLabelItem(QTreeWidgetItem *_parent, QString _title,
                      GetQStringFunctor _getFunc) {
  Q_ASSERT(_parent != nullptr);
  Q_ASSERT(_parent->treeWidget() != nullptr);

  auto *const treeItem = new QTreeWidgetItem(_parent);
  treeItem->setText(0, _title);

  auto *const label = new MyLabel(_getFunc, _parent->treeWidget());

  _parent->treeWidget()->setItemWidget(treeItem, 1, label);

  return label;
}

MyLineEdit *addLineEditItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                            QString title, QString text, int maxLength) {
  Q_ASSERT(treeWidget != nullptr);

  QTreeWidgetItem *treeItem = new QTreeWidgetItem(parent);
  treeItem->setText(0, title);

  auto *const widget = new QWidget(treeWidget);
  auto *const layout = new QHBoxLayout(widget);
  layout->setContentsMargins(0, 2, 6, 2);

  auto *const lineEdit = new MyLineEdit(widget);

  layout->addWidget(lineEdit);

  lineEdit->setText(text);
  lineEdit->setMaxLength(maxLength);

  treeWidget->setItemWidget(treeItem, 1, widget);

  return lineEdit;
}

MyLineEdit *addLineEditItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                            QString title, int maxLength) {
  Q_ASSERT(treeWidget != nullptr);

  QTreeWidgetItem *treeItem = new QTreeWidgetItem(parent);
  treeItem->setText(0, title);

  auto *const widget = new QWidget(treeWidget);
  auto *const layout = new QHBoxLayout(widget);
  layout->setContentsMargins(0, 2, 6, 2);

  auto *const lineEdit = new MyLineEdit(widget);

  layout->addWidget(lineEdit);

  lineEdit->setMaxLength(maxLength);

  treeWidget->setItemWidget(treeItem, 1, widget);

  return lineEdit;
}

MyLineEdit *addLineEditItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                            QString title, int maxLength,
                            GetQStringFunctor getFunc,
                            SetQStringFunctor setFunc) {
  Q_ASSERT(treeWidget != nullptr);

  QTreeWidgetItem *treeItem = new QTreeWidgetItem(parent);
  treeItem->setText(0, title);

  auto *const widget = new QWidget(treeWidget);
  auto *const layout = new QHBoxLayout(widget);
  layout->setContentsMargins(0, 2, 6, 2);

  auto *const lineEdit = new MyLineEdit(getFunc, setFunc, widget);

  layout->addWidget(lineEdit);

  lineEdit->setMaxLength(maxLength);

  treeWidget->setItemWidget(treeItem, 1, widget);

  return lineEdit;
}

QSpinBox *addSpinBoxItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                         QString title, int min, int max, int value) {
  Q_ASSERT(treeWidget != nullptr);

  auto *const treeItem = new QTreeWidgetItem(parent);
  treeItem->setText(0, title);

  auto *const spinBox = new MySpinBox(treeWidget);
  spinBox->setMinimum(min);
  spinBox->setMaximum(max);
  spinBox->setValue(value);

  treeWidget->setItemWidget(treeItem, 1, spinBox);

  return spinBox;
}

MySpinBox *addSpinBoxItem(QTreeWidget *_treeWidget, QTreeWidgetItem *_parent,
                          QString _title, int _min, int _max,
                          const GetIntFunctor &_getFunc,
                          const SetIntFunctor &_setFunc) {
  Q_ASSERT(_treeWidget != nullptr);

  auto *const treeItem = new QTreeWidgetItem(_parent);
  treeItem->setText(0, _title);

  auto *const spinBox = new MySpinBox(_getFunc, _setFunc, _treeWidget);
  spinBox->setMinimum(_min);
  spinBox->setMaximum(_max);

  _treeWidget->setItemWidget(treeItem, 1, spinBox);

  return spinBox;
}

QComboBox *addComboBoxItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                           QString title, QStringList items,
                           int selectedIndex) {
  Q_ASSERT(treeWidget != nullptr);

  QTreeWidgetItem *treeItem = new QTreeWidgetItem(parent);
  treeItem->setText(0, title);

  QComboBox *comboBox = new QComboBox(treeWidget);
  comboBox->addItems(items);
  comboBox->setCurrentIndex(selectedIndex);

  treeWidget->setItemWidget(treeItem, 1, comboBox);

  return comboBox;
}

MyComboBox *addComboBoxItem(QTreeWidgetItem *parent, QString title,
                            const GetComboBoxOptions &options,
                            const GetComboBoxSelectionFunctor &getFunc,
                            const SetComboBoxSelectionFunctor &setFunc) {
  assert(parent != nullptr);

  auto *const treeWidget = parent->treeWidget();

  assert(treeWidget != nullptr);

  auto *const treeItem = new QTreeWidgetItem(parent);
  treeItem->setText(0, title);

  auto *const cbox = new MyComboBox(options, getFunc, setFunc, treeWidget);
  treeWidget->setItemWidget(treeItem, 1, cbox);

  return cbox;
}

QTreeWidgetItem *addTextItem(QTreeWidgetItem *parent, QString title,
                             QString value) {
  /*
  auto *const widget = new QWidget(parent->treeWidget());
  auto *const layout = new QHBoxLayout(widget);
  auto *const label = new QLabel(value, widget);
  label->setMargin(2);

  layout->addWidget(label);

  */
  QTreeWidgetItem *treeItem = new QTreeWidgetItem(parent);
  treeItem->setText(0, title);
  //treeItem->treeWidget()->setItemWidget(treeItem, 1, label);
  treeItem->setText(1, value);

  return treeItem;
}

QTreeWidgetItem *addTextItem(QTreeWidget *parent, QString title,
                             QString value) {
  /*
  auto *const widget = new QWidget(parent);
  auto *const layout = new QHBoxLayout(widget);
  auto *const label = new QLabel(value, widget);
  label->setMargin(2);

  layout->addWidget(label);
  */

  QTreeWidgetItem *treeItem = new QTreeWidgetItem(parent);
  treeItem->setText(0, title);
  //treeItem->treeWidget()->setItemWidget(treeItem, 1, label);
  treeItem->setText(1, value);

  return treeItem;
}

QTreeWidgetItem *addTreeWidgetItem(QTreeWidget *parent, QString title) {
  QTreeWidgetItem *treeItem = new QTreeWidgetItem(parent);
  treeItem->setText(0, title);
  return treeItem;
}

}  // namespace TreeUtils
