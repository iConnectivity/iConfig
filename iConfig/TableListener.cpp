/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "TableListener.h"

#include <QAbstractButton>
#include <QApplication>
#include <QGridLayout>
#include <QHeaderView>
#include <QObject>
#include <QDebug>

TableListener::TableListener(QTableWidget *_tableWidget, QObject *_parent)
    : QObject(_parent),
      fillType(BlockState::Empty),
      fullBlockPix(":/Blocks/Images/FullBlock.png"),
      halfBlockPix(":/Blocks/Images/HalfBlock.png"),
      tableWidget(_tableWidget) {
  Q_ASSERT(tableWidget);

  QAbstractButton *cornerButton = tableWidget->findChild<QAbstractButton *>();
  if (cornerButton) {
    // Connect the cornerButton
    connect(cornerButton, SIGNAL(clicked()), this, SLOT(clickAll()));
  }

  connect(tableWidget, SIGNAL(cellEntered(int, int)), this,
          SLOT(cellEntered(int, int)));
  connect(tableWidget, SIGNAL(cellPressed(int, int)), this,
          SLOT(cellMouseDown(int, int)));
  connect(tableWidget->verticalHeader(), SIGNAL(sectionClicked(int)), this,
          SLOT(rowClicked(int)));
  connect(tableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)), this,
          SLOT(colClicked(int)));
}

void TableListener::addCornerLabel(QString cornerText) {
  // Get the corner button
  QAbstractButton *cornerButton = tableWidget->findChild<QAbstractButton *>();
  if (cornerButton) {
    // If you need to change the text
    if (cornerText.count() > 0) {
      // Create a label
      QLabel *cornerLabel = new QLabel(cornerText);

      // Change the font size
      QFont font = cornerLabel->font();
      font.setPointSize(11);
      cornerLabel->setFont(font);

      QGridLayout *gridLayout = new QGridLayout();
      gridLayout->setMargin(3);
      gridLayout->addWidget(cornerLabel);

      // Add Label widget to button
      cornerButton->setLayout(gridLayout);
    }
  }
}

void TableListener::updateWidgets(BlockStateFunction blockStateFunction) {
  for (auto row = 0; row < tableWidget->rowCount();
       row += tableWidget->rowSpan(row, 0)) {
    if (ignoreRows.contains(row)) {
      continue;
    }
    for (auto col = 0; col < tableWidget->columnCount();
         col += tableWidget->columnSpan(row, col)) {
      if (ignoreColumns.contains(col)) {
        continue;
      }
      const auto &state = blockStateFunction(row, col);
      auto label = labelAt(row, col);
      if (label) {
        label->setProperty(kBlockState, state);
      }

      setCellImage(row, col, state);
    }
  }

  tableWidget->update();
}

bool TableListener::isRowFull(int row) {
  float setCount = 0.0f;
  float emptyCount = 0.0f;

  for (auto col = 0; col < tableWidget->columnCount();
       col += tableWidget->columnSpan(row, col)) {
    if (ignoreColumns.contains(col)) {
      continue;
    }
    QLabel *item = qobject_cast<QLabel *>(tableWidget->cellWidget(row, col));
    if (item) {
      auto result = (BlockState::Enum)(item->property(kBlockState).toInt());

      switch (result) {
        case BlockState::Full:
          setCount += 1.0f;
          break;

        case BlockState::Half:
          setCount += 0.5f;
          break;

        case BlockState::Empty:
          emptyCount += 1.0f;
          break;

        default:
          break;
      }
    }
  }

  return (emptyCount < setCount);
}

bool TableListener::isColFull(int col) {
  float setCount = 0.0f;
  float emptyCount = 0.0f;
  for (auto row = 0; row < tableWidget->rowCount();
       row += tableWidget->rowSpan(row, col)) {
    if (ignoreRows.contains(row)) {
      continue;
    }
    QLabel *item = qobject_cast<QLabel *>(tableWidget->cellWidget(row, col));
    if (item) {
      auto result = (BlockState::Enum)(item->property(kBlockState).toInt());

      switch (result) {
        case BlockState::Full:
          setCount += 1.0f;
          break;

        case BlockState::Half:
          setCount += 0.5f;
          break;

        case BlockState::Empty:
          emptyCount += 1.0f;
          break;

        default:
          break;
      }
    }
  }

  return (emptyCount < setCount);
}

bool TableListener::isAllFull() {
  float setCount = 0.0f;
  float emptyCount = 0.0f;
  for (auto row = 0; row < tableWidget->rowCount();
       row += tableWidget->rowSpan(row, 0)) {
    if (ignoreRows.contains(row)) {
      continue;
    }
    for (auto col = 0; col < tableWidget->columnCount();
         col += tableWidget->columnSpan(row, col)) {
      if (ignoreColumns.contains(col)) {
        continue;
      }
      QLabel *item = qobject_cast<QLabel *>(tableWidget->cellWidget(row, col));
      if (item) {
        auto result = (BlockState::Enum)(item->property(kBlockState).toInt());

        switch (result) {
          case BlockState::Full:
            setCount += 1.0f;
            break;

          case BlockState::Half:
            setCount += 0.5f;
            break;

          case BlockState::Empty:
            emptyCount += 1.0f;
            break;

          default:
            break;
        }
      }
    }
  }

  return (emptyCount < setCount);
}

void TableListener::cellEntered(int row, int col) {
  if ((!ignoreRows.contains(row)) && (!ignoreColumns.contains(col)) &&
      ((QApplication::mouseButtons() & Qt::LeftButton))) {
    QLabel *item = qobject_cast<QLabel *>(tableWidget->cellWidget(row, col));
    if (item) {
      changeCell(row, col, fillType);
    }
  }
}

void TableListener::cellMouseDown(int row, int col) {
  if ((!ignoreRows.contains(row)) && (!ignoreColumns.contains(col))) {
    QLabel *label = qobject_cast<QLabel *>(tableWidget->cellWidget(row, col));
    if (label) {
      auto state = cellState(label);
      switch (state) {
        case BlockState::Full:
        case BlockState::Half:
          fillType = BlockState::Empty;
          break;
        case BlockState::Empty:
          fillType = BlockState::Full;
          break;
        default:
          break;
      }
    }
    changeCell(row, col, fillType);
  }
}

void TableListener::clickAll() {
  auto nextState = ((isAllFull()) ? (BlockState::Empty) : (BlockState::Full));

  for (auto row = 0; row < tableWidget->rowCount();
       row += tableWidget->rowSpan(row, 0)) {
    if (ignoreRows.contains(row)) {
      continue;
    }
    for (auto col = 0; col < tableWidget->columnCount();
         col += tableWidget->columnSpan(row, col)) {
      if (ignoreColumns.contains(col)) {
        continue;
      }
      changeCell(row, col, nextState);
    }
  }
}

void TableListener::rowClicked(int row) {
  auto nextState =
      ((isRowFull(row)) ? (BlockState::Empty) : (BlockState::Full));

  for (auto col = 0; col < tableWidget->columnCount();
       col += tableWidget->columnSpan(row, col)) {
    if (ignoreColumns.contains(col)) {
      continue;
    }
    changeCell(row, col, nextState);
  }
}

void TableListener::colClicked(int col) {
  auto nextState =
      ((isColFull(col)) ? (BlockState::Empty) : (BlockState::Full));

  for (auto row = 0; row < tableWidget->rowCount();
       row += tableWidget->rowSpan(row, col)) {
    if (ignoreRows.contains(row)) {
      continue;
    }
    changeCell(row, col, nextState);
  }
}

void TableListener::changeCell(int row, int col, BlockState::Enum state) {
  if ((!ignoreRows.contains(row)) && (!ignoreColumns.contains(col))) {
    auto label = labelAt(row, col);
    if (label) {
      label->setProperty(kBlockState, state);
      setCellImage(row, col, state);
      emit cellStateChanged(row, col, state);
    }
  }
}

void TableListener::setCellImage(int row, int col, BlockState::Enum state) {
  if ((!ignoreRows.contains(row)) && (!ignoreColumns.contains(col))) {

    QLabel *label = labelAt(row, col);
    if (label) {
      switch (state) {
        case BlockState::Full:
          label->setPixmap(fullBlockPix);
          break;
        case BlockState::Half:
          label->setPixmap(halfBlockPix);
          break;
        default:
          label->setPixmap(nullptr);
          break;
      }
    }
  }
}

BlockState::Enum TableListener::cellState(int row, int col) {
  if ((ignoreRows.contains(row)) || (ignoreColumns.contains(col))) {
    return BlockState::UnknownBlockState;
  }
  QLabel *label = labelAt(row, col);
  return ((label != nullptr) ? (cellState(label))
                             : (BlockState::UnknownBlockState));
}

BlockState::Enum TableListener::cellState(QLabel *label) {
  auto state = BlockState::UnknownBlockState;
  if (label != nullptr) {
    state = (BlockState::Enum)(label->property(kBlockState).toInt());
  }
  return state;
}

QLabel *TableListener::labelAt(int row, int col) {
  QLabel *label = nullptr;

  auto rowCount = tableWidget->rowCount();
  auto colCount = tableWidget->columnCount();

  if ((!ignoreRows.contains(row)) && (!ignoreColumns.contains(col)) &&
      (row >= 0) && (row < rowCount) && (col >= 0) && (col < colCount)) {
    label = qobject_cast<QLabel *>(tableWidget->cellWidget(row, col));
  }

  return label;
}

void TableListener::addIgnoreRow(int row) { ignoreRows.insert(row); }

void TableListener::addIgnoreRows(QSet<int> rows) { ignoreRows.unite(rows); }

void TableListener::addIgnoreCol(int col) { ignoreColumns.insert(col); }

void TableListener::addIgnoreCols(QSet<int> cols) { ignoreColumns.unite(cols); }
