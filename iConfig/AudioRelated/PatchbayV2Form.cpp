/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "PatchbayV2Form.h"
#include "ui_patchbayV2Form.h"

#include <QPaintEvent>
#include <QPainter>
#include <QDebug>

#include <algorithm>
#include <numeric>
#include <string>
#include <cmath>

#ifndef Q_MOC_RUN
#include <boost/tuple/tuple.hpp>
#endif

static const float textHeight = 20.0f;
static const float digitWidth = 20.0f;
static const float arrow_width = 5.0f;
static const float arrow_height = arrow_width * 1.6666f;
static const float arrowXOffset = 5.0f;
static const float arrowYOffset = 2.0f;

using namespace std;

const QString PatchbayV2Form::DefaultWindowTitle = "Audio Patchbay";

PatchbayV2Form::PatchbayV2Form(boost::shared_ptr<IAudioPatchbaySource> source,
                               QWidget *parent)
    : RefreshObject(parent),
      ui(new Ui::PatchbayV2Form),
      isLeftMouseDown(false),
      mouseLocation(0, 0),
      dashOffset(0),
      m_source(source) {
  ui->setupUi(this);
  setMouseTracking(true);

  setWindowTitle(PatchbayV2Form::DefaultWindowTitle);

  setMinimumSize(800, 600);

  dashTimer = new QTimer(this);
  connect(dashTimer, SIGNAL(timeout()), this, SLOT(increaseDashOffset()));

  Q_ASSERT(m_source);
  m_source->init([ = ]() {
    this->repaint();
  });
}

void PatchbayV2Form::paintEvent(QPaintEvent *event) {
  QRect gridRect = event->rect();
  draw(gridRect);
}

void PatchbayV2Form::increaseDashOffset() {
  dashOffset -= 1.5f;
  repaint();
}

void PatchbayV2Form::draw(const QRect &srcRect) {
  const auto grid = gridRect();
  const auto &vHeaderRect = verticalHeaderRect();
  const auto &hHeaderRect = horizontalHeaderRect();

  const auto dx = (float) grid.width() / (float) m_source->numOutputs();
  const auto dy = (float) grid.height() / (float) m_source->numInputs();
  const auto dx_2 = dx * 0.5f;
  const auto dy_2 = dy * 0.5f;

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::black);
  painter.setPen(Qt::DotLine);

  int index = 0;

  auto x = (float)(grid.left());
  int count1 = 1;
  for (int section = 1; section <= m_source->numSections(); ++section) {
    const QSizeF sectionSize =
        QSizeF(dx * m_source->numOutputsPerSection(section), rect().height());
    if ((count1 & 0x01) == 0x00) {
      painter.fillRect(QRectF(QPointF(x, srcRect.top()), sectionSize),
                       //QBrush(QColor(150, 200, 255, 128)));
                       QBrush(QColor(180, 220, 255, 128)));
      //QBrush(QColor(180, 220, 255, 128), Qt::Dense3Pattern));
    }

    const auto bounds =
        QRectF(QPointF(x, hHeaderRect.top()),
               QSizeF(sectionSize.width(), hHeaderRect.height()));
    painter.drawText(bounds, Qt::AlignCenter | Qt::TextWordWrap,
                     m_source->nameForSection(section));

    if (m_source->numOutputsPerSection(section)) {
      count1++;
    }
    for (int output = 1; output <= m_source->numOutputsPerSection(section);
         ++output) {
      QPointF mid_p = QPointF(x + dx_2, verticalArrowOffset());

      drawDownTriangle(painter, mid_p, QBrush(Qt::black));

      painter.setPen(Qt::DotLine);
      painter.drawLine(QPointF(x + dx_2, verticalArrowOffset()),
                       QPointF(x + dx_2, rect().bottom()));


      const auto bounds = QRectF(QPointF(x, verticalChannelNumOffset()),
                                 QSizeF(dx, textHeight));
      if (!m_source->isCollapsedInput(section)) {
        painter.drawText(bounds, Qt::AlignCenter | Qt::TextWordWrap,
                       QString::number(output));
      }

      ++index;
      x += dx;
    }
  }

  index = 0;
  auto y = (float)(grid.top());
  int count2 = 1;
  for (int section = 1; section <= m_source->numSections(); ++section) {
    const auto sectionSize =
        QSizeF(rect().width(), dy * m_source->numInputsPerSection(section));

    if ((count2 & 0x01) == 0x00) {
      painter.setCompositionMode(QPainter::CompositionMode_Multiply);
      painter.fillRect(QRectF(QPointF(grid.left(), y), sectionSize),
                       //QBrush(QColor(150, 200, 255, 128)));
                       QBrush(QColor(180, 220, 255, 128)));
      //QBrush(QColor(180, 220, 255, 128), Qt::Dense2Pattern));
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    const auto bounds =
        QRectF(QPointF(vHeaderRect.left(), y),
               QSizeF(vHeaderRect.width(), sectionSize.height()));
    painter.drawText(bounds, Qt::AlignCenter | Qt::TextWordWrap,
                     m_source->nameForSection(section));

    if (m_source->numInputsPerSection(section)) {
      count2++;
    }
    //PlayAudio Bugfixing, zx-02-22
    int nOutputChannelCount =  m_source->numInputsPerSection(section);
    for (int output = 1; output <= nOutputChannelCount; ++output) {
      QPointF mid_p;
      mid_p = QPointF(horizontalArrowOffset(), y + dy_2);
      drawRightTriangle(painter, mid_p, QBrush(Qt::black));

      painter.setPen(Qt::DotLine);
      painter.drawLine(QPointF(grid.left(), y + dy_2),
                       QPointF(horizontalArrowOffset(), y + dy_2));

      const auto bounds = QRectF(QPointF(horizontalChannelNumOffset(), y),
                                 QSizeF(digitWidth, dy));

      // TODO: Hack to make L/R show up for headphones on ICA4
      if (!m_source->isCollapsedOutput(section)) {
        if (m_source->isMixerToo() && (section == 5 && output > (nOutputChannelCount-2))) {
          QString letters;
          if (output == nOutputChannelCount-1)
            letters = "HL";
          else
            letters = "HR";

          painter.drawText(bounds, Qt::AlignCenter | Qt::TextWordWrap,
                           letters);
        }
        else {
          painter.drawText(bounds, Qt::AlignCenter | Qt::TextWordWrap,
                         QString::number(output));
        }
      }
      y += dy;
      ++index;
    }
  }

  m_source->for_each_mixer([&](const device_pair_t & m) {
    int outPort, outCh, inPort, inCh;
    boost::tie(outPort, outCh) = m.first;
    boost::tie(inPort, inCh) = m.second;
    if ((outPort > 0) && (inPort > 0) && (outCh > 0) && (inCh > 0) &&
        (outPort <= m_source->numSections()) &&
        (outCh <= m_source->numOutputsPerSection(outPort)) &&
        (inPort <= m_source->numSections()) &&
        (inCh <= m_source->numInputsPerSection(inPort))
        ) {
      QPointF a, b, c;

      const auto totalOuts = m_source->outputIndexToTotal(m.first);
      const auto totalIns = m_source->inputIndexToTotal(m.second);

      float horX = (float)(totalOuts) * dx - dx_2;
      float horY = (float)(totalIns) * dy - dy_2;

      a = QPointF(grid.left() + horX, verticalArrowOffset());
      b = QPointF(grid.left() + horX, grid.top() + horY);
      c = QPointF(horizontalArrowOffset(), grid.top() + horY);

      QPen pen;
      if (m_source->isCollapsedInput(outPort) || m_source->isCollapsedOutput(inPort))
        pen = QPen(QBrush(Qt::blue), 1.2f);
      else
        pen = QPen(QBrush(Qt::black), 1.2f);
      painter.setPen(pen);
      painter.drawLine(a, b);
      painter.drawLine(b, c);
      if (m_source->isCollapsedInput(outPort) || m_source->isCollapsedOutput(inPort))
        painter.setBrush(Qt::blue);
      else
        painter.setBrush(Qt::black);
      painter.drawEllipse(b, arrowSize(), arrowSize());
    }
  });

  m_source->for_each([&](const device_pair_t & m) {
    int outPort, outCh, inPort, inCh;
    boost::tie(outPort, outCh) = m.first;
    boost::tie(inPort, inCh) = m.second;
    if ((outPort > 0) && (inPort > 0) && (outCh > 0) && (inCh > 0) &&
        (outPort <= m_source->numSections()) &&
        (outCh <= m_source->numOutputsPerSection(outPort)) &&
        (inPort <= m_source->numSections()) &&
        (inCh <= m_source->numInputsPerSection(inPort))
        ) {
      QPointF a, b, c;

      const auto totalOuts = m_source->outputIndexToTotal(m.first);
      const auto totalIns = m_source->inputIndexToTotal(m.second);

      float horX = (float)(totalOuts) * dx - dx_2;
      float horY = (float)(totalIns) * dy - dy_2;

      a = QPointF(grid.left() + horX, verticalArrowOffset());
      b = QPointF(grid.left() + horX, grid.top() + horY);
      c = QPointF(horizontalArrowOffset(), grid.top() + horY);

      QPen pen;
      if (m_source->isCollapsedInput(outPort) || m_source->isCollapsedOutput(inPort))
        pen = QPen(QBrush(Qt::blue), 1.2f);
      else
        pen = QPen(QBrush(Qt::black), 1.2f);
      painter.setPen(pen);
      painter.drawLine(a, b);
      painter.drawLine(b, c);
      if (m_source->isCollapsedInput(outPort) || m_source->isCollapsedOutput(inPort))
        painter.setBrush(Qt::blue);
      else
        painter.setBrush(Qt::black);
      painter.drawEllipse(b, arrowSize(), arrowSize());
    }
  });

  // draw the legend
  drawLegend(painter);

  // draw the proposed line
  if ((isLeftMouseDown) && (grid.contains(mouseLocation))) {
    QPointF a, b, c;

    int from = int(floor((mouseLocation.x() - grid.left())) / dx);
    int to = int(floor((mouseLocation.y() - grid.top()) / dy));

    fromDevice = m_source->outputTotalToIndex(from + 1);
    toDevice = m_source->inputTotalToIndex(to + 1);

    if ((from >= 0) && (to >= 0)) {
      float fx, fy;
      fx = from * dx + dx_2;
      fy = to * dy + dy_2;

      bool addChannelMap = !(m_source->isPatched(fromDevice, toDevice));
      auto color = ((addChannelMap) ? (Qt::blue) : (Qt::red));

      a = QPointF(grid.left() + fx, verticalArrowOffset());
      b = QPointF(grid.left() + fx, grid.top() + fy);
      c = QPointF(horizontalArrowOffset(), grid.top() + fy);
      QPen pen = QPen(QBrush(color), 3.0);
      if (addChannelMap) {
        pen.setStyle(Qt::DashDotDotLine);
        pen.setDashOffset(dashOffset);
      }
      painter.setPen(pen);

      painter.drawLine(a, b);
      painter.drawLine(b, c);

      pen.setStyle(Qt::SolidLine);

      const auto &ds = arrowSize() * 1.25f;
      if (addChannelMap) {
        painter.setPen(color);
        painter.setBrush(color);
        painter.drawEllipse(b, ds, ds);
      } else {
        painter.setPen(QPen(QBrush(color), 2.0f));
        painter.drawLine(b.x() - ds, b.y() - ds, b.x() + ds, b.y() + ds);
        painter.drawLine(b.x() - ds, b.y() + ds, b.x() + ds, b.y() - ds);
      }

      //
      QPointF mid_p;
      mid_p = QPointF(grid.left() + fx, verticalArrowOffset());
      drawDownTriangle(painter, mid_p, QBrush(color));

      mid_p = QPointF(horizontalArrowOffset(), fy + grid.top());
      drawRightTriangle(painter, mid_p, QBrush(color));
    }
  }
  else if ((isLeftMouseDown) && (horizontalHeaderRect().contains(mouseLocation))) {
    int from = int(floor((mouseLocation.x() - grid.left())) / dx);

    fromDevice = m_source->outputTotalToIndex(from + 1);
  }
  else if ((isLeftMouseDown) && (verticalHeaderRect().contains(mouseLocation))) {
    int to = int(floor((mouseLocation.y() - grid.top()) / dy));

    toDevice = m_source->inputTotalToIndex(to + 1);
  }
  painter.setPen(Qt::black);
  painter.drawLine(srcRect.topLeft(), srcRect.topRight());
  painter.drawLine(srcRect.bottomRight(), srcRect.topRight());
  painter.drawLine(srcRect.bottomLeft(), srcRect.bottomRight());
  painter.drawLine(srcRect.topLeft(), srcRect.bottomLeft());
}

void PatchbayV2Form::drawLegend(QPainter &painter) {
  const auto &legendRect = topRightRect();
  const auto &vHeaderRect = verticalHeaderRect();
  const auto &hHeaderRect = horizontalHeaderRect();
  const auto &vGutterRect = verticalGutterRect();
  const auto &hGutterRect = horizontalGutterRect();

  auto headerBrush = QBrush(QColor(200, 200, 200, 200), Qt::Dense4Pattern);
  //QBrush(QColor(200, 200, 200, 200));
  painter.setCompositionMode(QPainter::CompositionMode_Multiply);

  painter.fillRect(hHeaderRect, headerBrush);
  painter.fillRect(verticalGutterRect(), headerBrush);

  QPainterPath path = QPainterPath(legendRect.bottomLeft());
  path.lineTo(legendRect.topRight());
  path.lineTo(legendRect.bottomRight());

  painter.fillPath(path, headerBrush);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

  painter.save();

  painter.setPen(Qt::black);

  // draw vertial header lines and vertial gutter line
  painter.drawLine(vHeaderRect.topLeft(), vHeaderRect.topRight());
  painter.drawLine(vHeaderRect.topLeft(), vHeaderRect.bottomLeft());
  painter.drawLine(vGutterRect.topLeft(), vGutterRect.bottomLeft());

  // draw '/' line in top right
  painter.drawLine(legendRect.bottomLeft(), legendRect.topRight());

  // draw horizontal lines and horizontal gutter lines
  painter.drawLine(hHeaderRect.bottomLeft(), hHeaderRect.bottomRight());
  painter.drawLine(hHeaderRect.topRight(), hHeaderRect.bottomRight());
  painter.drawLine(hGutterRect.bottomLeft(), hGutterRect.bottomRight());

  QSizeF size = QSizeF(legendRect.width() * 0.5f, legendRect.height() * 0.5f);

  QRectF drawRect =
      QRectF(QPointF(size.width() * -0.5f, size.height() * -0.5f), size);

  QPointF v = legendRect.topRight() - legendRect.bottomLeft();
  float angle = atan2f(v.y(), v.x()) * 180.0f /3.14159265359f;

  painter.translate((legendRect.topLeft() + legendRect.center()) * 0.5f);
  painter.rotate(angle);
  painter.drawText(drawRect,
                   Qt::AlignHCenter | Qt::AlignBottom | Qt::TextWordWrap,
                   QString("Sources"));

  painter.resetTransform();

  painter.translate((legendRect.center() + legendRect.bottomRight()) * 0.5f);
  painter.rotate(angle);
  painter.drawText(drawRect, Qt::AlignHCenter | Qt::TextWordWrap,
                   QString("Destinations"));

  painter.restore();
}

void PatchbayV2Form::drawTriangle(QPainter &painter, const QPointF &a,
                                  const QPointF &b, const QPointF &c,
                                  const QBrush &brush) {
  QPainterPath path = QPainterPath(a);
  path.lineTo(b);
  path.lineTo(c);

  painter.fillPath(path, brush);
}

void PatchbayV2Form::drawDownTriangle(QPainter &painter, const QPointF &point,
                                      const QBrush &brush) {
  QPointF mid_p, top_l, top_r;
  mid_p = point;
  top_l = mid_p - QPointF(-arrow_width, arrow_height);
  top_r = mid_p - QPointF(arrow_width, arrow_height);
  drawTriangle(painter, mid_p, top_l, top_r, brush);
}

void PatchbayV2Form::drawRightTriangle(QPainter &painter, const QPointF &point,
                                       const QBrush &brush) {
  QPointF mid_p, top_l, top_r;
  mid_p = point + QPointF(arrowSize(), 0);
  top_l = mid_p - QPointF(arrow_height, -arrow_width);
  top_r = mid_p - QPointF(arrow_height, arrow_width);
  drawTriangle(painter, mid_p, top_l, top_r, brush);
}

void PatchbayV2Form::mousePressEvent(QMouseEvent *event) {
  isLeftMouseDown = event->buttons().testFlag(Qt::LeftButton);
  mouseLocation = event->posF();
  dashTimer->start(100);
  repaint();
}

void PatchbayV2Form::mouseReleaseEvent(QMouseEvent *event) {
  using namespace std;
  isLeftMouseDown = event->buttons().testFlag(Qt::LeftButton);
  mouseLocation = event->posF();

  if (gridRect().contains(mouseLocation)) {
    if (m_source->isPatched(fromDevice, toDevice)) {
      m_source->setPatch(make_pair(0, 0), toDevice, fromDevice);
    } else {
      m_source->setPatch(fromDevice, toDevice);
    }
  }
  else if (horizontalHeaderRect().contains(mouseLocation)) {
    m_source->toggleCollapseInput(fromDevice.first);
  }
  else if (verticalHeaderRect().contains(mouseLocation)) {
    m_source->toggleCollapseOutput(toDevice.first);
  }
  dashTimer->stop();
  repaint();
}

void PatchbayV2Form::mouseMoveEvent(QMouseEvent *event) {
  mouseLocation = event->posF();
  repaint();
}

float PatchbayV2Form::horizontalHeaderWidth() const {
  return (float) rect().width() - verticalHeaderWidth() - gutterSize();
}

float PatchbayV2Form::horizontalHeaderHeight() const {
  return 60.0f;
}  // 120.0f; }

float PatchbayV2Form::verticalHeaderWidth() const { return 140.0f; }

float PatchbayV2Form::verticalHeaderHeight() const {
  return (float) rect().height() - horizontalHeaderHeight() - gutterSize();
}

float PatchbayV2Form::gridWidth() const { return horizontalHeaderWidth(); }

float PatchbayV2Form::gridHeight() const { return verticalHeaderHeight(); }

float PatchbayV2Form::gutterSize() const { return 40.0f; }

float PatchbayV2Form::arrowSize() const { return 5.0f; }

float PatchbayV2Form::horizontalChannelNumOffset() const {
  return horizontalArrowOffset() + 5.0f;
}

float PatchbayV2Form::horizontalArrowOffset() const {
  return gridWidth() + arrowSize() + arrowXOffset;
}

float PatchbayV2Form::verticalChannelNumOffset() const {
  return verticalArrowOffset() - arrowSize() - textHeight - 5.0f;
}

float PatchbayV2Form::verticalArrowOffset() const {
  return horizontalHeaderHeight() + gutterSize() - arrowYOffset;
}

QRectF PatchbayV2Form::horizontalHeaderRect() const {
  const QRect &r = rect();
  // (x, y, w, h)
  return QRectF((float) r.left(), (float) r.top(), horizontalHeaderWidth(),
                horizontalHeaderHeight());
}

QRectF PatchbayV2Form::verticalHeaderRect() const {
  const QRect &r = rect();
  // (x, y, w, h)
  return QRectF((float) r.left() + horizontalHeaderWidth() + gutterSize(),
                (float) r.top() + horizontalHeaderHeight() + gutterSize(),
                verticalHeaderWidth(), verticalHeaderHeight());
}

QRectF PatchbayV2Form::horizontalGutterRect() const {
  const QRect &r = rect();
  // (x, y, w, h)
  return QRectF((float) r.left(), (float) r.top() + horizontalHeaderHeight(),
                horizontalHeaderWidth(), gutterSize());
}

QRectF PatchbayV2Form::verticalGutterRect() const {
  const QRect &r = rect();
  // (x, y, w, h)
  return QRectF((float) r.left() + horizontalHeaderWidth(),
                (float) r.top() + horizontalHeaderHeight() + gutterSize(),
                gutterSize(), verticalHeaderHeight());
}

QRectF PatchbayV2Form::topRightRect() const {
  const QRect &r = rect();
  // (x, y, w, h)
  return QRectF((float) r.left() + horizontalHeaderWidth(), (float) r.top(),
                verticalHeaderWidth() + gutterSize(),
                horizontalHeaderHeight() + gutterSize());
}

QRectF PatchbayV2Form::gridRect() const {
  const QRect &r = rect();
  // (x, y, w, h)
  return QRectF((float) r.left(),
                (float) r.top() + horizontalHeaderHeight() + gutterSize(),
                gridWidth(), gridHeight());

}

void PatchbayV2Form::refreshWidget() { repaint(); }
