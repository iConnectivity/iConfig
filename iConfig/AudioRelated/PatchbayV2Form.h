/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef PATCHBAYV2FORM_H
#define PATCHBAYV2FORM_H

#include "IAudioPatchbaySource.h"
#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#endif
#include "RefreshObject.h"

#include <QWidget>
#include <QTimer>
#include <QSharedPointer>

namespace Ui {
class PatchbayV2Form;
}

class PatchbayV2Form : public RefreshObject
{
  Q_OBJECT

public:
  explicit PatchbayV2Form(boost::shared_ptr<IAudioPatchbaySource> source,
                      QWidget *parent = 0);

  void paintEvent(QPaintEvent *event);

  void drawTriangle(QPointF mid_p, QPointF top_r, QPainter painter,
                    QPointF top_l);

  static const QString DefaultWindowTitle;

private slots:
  void increaseDashOffset();

private:
  QSharedPointer<Ui::PatchbayV2Form> ui;

  void draw(const QRect &gridRect);
  void drawLegend(QPainter &painter);
  void drawTriangle(QPainter &painter, const QPointF &a, const QPointF &b,
                    const QPointF &c, const QBrush &brush);
  void drawDownTriangle(QPainter &painter, const QPointF &point,
                        const QBrush &brush);
  void drawRightTriangle(QPainter &painter, const QPointF &point,
                         const QBrush &brush);

  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);

  float horizontalHeaderWidth() const;
  float horizontalHeaderHeight() const;
  float verticalHeaderWidth() const;
  float verticalHeaderHeight() const;
  float gridWidth() const;
  float gridHeight() const;
  float gutterSize() const;
  float arrowSize() const;
  float horizontalArrowOffset() const;
  float horizontalChannelNumOffset() const;
  float verticalArrowOffset() const;
  float verticalChannelNumOffset() const;

  QRectF horizontalHeaderRect() const;
  QRectF verticalHeaderRect() const;
  QRectF horizontalGutterRect() const;
  QRectF verticalGutterRect() const;
  QRectF topRightRect() const;
  QRectF gridRect() const;

  void refreshWidget();

  bool isLeftMouseDown;
  QPointF mouseLocation;

  device_port_t fromDevice;
  device_port_t toDevice;
  qreal dashOffset;
  QTimer *dashTimer;

  boost::shared_ptr<IAudioPatchbaySource> m_source;
};

#endif // PATCHBAYV2FORM_H
