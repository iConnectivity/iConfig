/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef TABLELISTENER_H
#define TABLELISTENER_H

#include "BlockState.h"

#ifndef Q_MOC_RUN
#include <boost/function.hpp>
#endif

#include <QObject>
#include <QTableWidget>
#include <QString>
#include <QLabel>

typedef boost::function<BlockState::Enum(int, int)> BlockStateFunction;
typedef boost::function<QString(int, int)> CellTextFunction;
typedef boost::function<void(QTableWidget *)> SetSpanningFunction;

class TableListener : public QObject {
  Q_OBJECT
 public:
  explicit TableListener(QTableWidget *tableWidget, QObject *parent = 0);

  void addCornerLabel(QString cornerText);
  void updateWidgets(BlockStateFunction blockStateFunction);

  bool isRowFull(int row);
  bool isColFull(int col);
  bool isAllFull();

  BlockState::Enum cellState(int row, int col);
  BlockState::Enum cellState(QLabel *label);

  QLabel *labelAt(int row, int col);

  void addIgnoreRow(int row);
  void addIgnoreRows(QSet<int> rows);
  void addIgnoreCol(int col);
  void addIgnoreCols(QSet<int> cols);

 signals:
  void cellStateChanged(int row, int col, BlockState::Enum state);

 public slots:
  void cellEntered(int row, int col);
  void cellMouseDown(int row, int col);
  void clickAll();
  void rowClicked(int row);
  void colClicked(int col);

 private:
  void changeCell(int row, int col, BlockState::Enum state);
  void setCellImage(int row, int col, BlockState::Enum state);

  BlockState::Enum fillType;

  QPixmap fullBlockPix;
  QPixmap halfBlockPix;

  QSet<int> ignoreColumns;
  QSet<int> ignoreRows;

  QTableWidget *tableWidget;
};

#endif  // TABLELISTENER_H
