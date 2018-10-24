/*
 * @file  Gradient.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include <QPainter>
#include <QBrush>

#include <iostream>

#include "GradientWidget.h"

GradientWidget::GradientWidget(QWidget *parent_) :
    QFrame(parent_),
    _direction(VERTICAL)
{
     QPixmap pixmap(20, 20);
     QPainter painter(&pixmap);
     painter.fillRect(0, 0, 10, 10, Qt::lightGray);
     painter.fillRect(10, 10, 10, 10, Qt::lightGray);
     painter.fillRect(0, 10, 10, 10, Qt::darkGray);
     painter.fillRect(10, 0, 10, 10, Qt::darkGray);
     painter.end();
     QPalette pal = palette();
     pal.setBrush(backgroundRole(), QBrush(pixmap));
     setPalette(pal);
     setAutoFillBackground(true);
}

GradientWidget::~GradientWidget( )
{ }

void GradientWidget::setDirection( Direction direction )
{
  _direction = direction;
  update();
}

void GradientWidget::setGradientStops(const QGradientStops &stops)
{
  _stops = stops;
  update();
}

const QGradientStops& GradientWidget::getGradientStops() const
{
    return _stops;
}

void GradientWidget::plot( const QPolygonF& plot_ )
{
  _plot = plot_;
}

QPolygonF GradientWidget::plot( void )
{
  return _plot;
}

void GradientWidget::clearPlot( void )
{
  _plot.clear( );
}

float GradientWidget::xPos( float x_ )
{
  return x_ * width( );// + x( );
}

float GradientWidget::yPos( float y_ )
{
  return (1.0f - y_) * height( );// + y( );
}


void GradientWidget::mousePressEvent( QMouseEvent* event_ )
{
  QFrame::mousePressEvent( event_ );

  if( event_->button( ) == Qt::LeftButton )
  {
    emit clicked( );
  }
}


void GradientWidget::paintEvent(QPaintEvent* /*e*/)
{
    QPainter painter(this);
    QLinearGradient gradient(0, 0, _direction == HORIZONTAL ? width() : 0,
                             _direction == VERTICAL ? height() : 0);
    gradient.setStops(_stops);
    QBrush brush(gradient); 
    painter.fillRect(rect(), brush);

    if( _plot.size( ) > 0)
    {
      painter.setBrush( QBrush( QColor( 0, 0, 0 )));
      auto prev = _plot.begin( );
      QPointF prevPoint( xPos( prev->x( )), yPos( prev->y( )));
      for( auto current = prev + 1; current != _plot.end( ); current++ )
      {
        QPointF point( xPos( current->x( )), yPos( current->y( ) ) );

        painter.drawLine( prevPoint, point );

        prevPoint = point;
      }
    }
}
