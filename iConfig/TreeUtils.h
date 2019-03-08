/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef TREEUTILS_H
#define TREEUTILS_H

#include "MyCheckBox.h"
#include "MyComboBox.h"
#include "MyLabel.h"
#include "MySpinBox.h"

#include <QSpinBox>
#include <QTreeWidget>

class MyLineEdit;
class QTableWidget;

namespace TreeUtils {

#define kDefaultColumnSize (50)
#define kDefaultRowSize (45)
#define kDefaultHeaderSize (45)

QTableWidget* createInfoTableWidget(QStringList labels, int rowCount);

MyCheckBox *addCheckBox(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                        QString title, bool checked);
MyCheckBox *addCheckBox(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                        QString title, const GetBoolFunctor &getFunc,
                        const SetBoolFunctor &setFunc);

MyLabel *addLabelItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                      QString title, QString text);
MyLabel *addLabelItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                      QString title, GetQStringFunctor getFunc);
MyLabel *addLabelItem(QTreeWidgetItem *_parent, QString _title,
                      GetQStringFunctor _getFunc);

MyLineEdit *addLineEditItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                            QString title, QString text, int maxLength);
MyLineEdit *addLineEditItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                            QString title, int maxLength);
MyLineEdit *addLineEditItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                            QString title, int maxLength,
                            GetQStringFunctor getFunc,
                            SetQStringFunctor setFunc);

QSpinBox *addSpinBoxItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                         QString title, int min, int max, int value);
MySpinBox *addSpinBoxItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                          QString title, int min, int max,
                          const GetIntFunctor &getFunc,
                          const SetIntFunctor &setFunc);

QComboBox *addComboBoxItem(QTreeWidget *treeWidget, QTreeWidgetItem *parent,
                           QString title, QStringList items, int selectedItem);
MyComboBox *addComboBoxItem(QTreeWidgetItem *parent, QString title,
                            const GetComboBoxOptions &options,
                            const GetComboBoxSelectionFunctor &getFunc,
                            const SetComboBoxSelectionFunctor &setFunc);

QTreeWidgetItem *addTextItem(QTreeWidgetItem *parent, QString title,
                             QString value);

QTreeWidgetItem *addTextItem(QTreeWidget *parent, QString title, QString value);

QTreeWidgetItem *addTreeWidgetItem(QTreeWidget *parent, QString title);

}  // namespace TreeUtils

#endif  // TREEUTILS_H
