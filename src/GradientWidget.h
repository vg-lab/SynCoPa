/*
 * @file  Gradient.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef GRADIENT_H
#define GRADIENT_H

#include <QFrame>
#include <QMouseEvent>

class GradientWidget : public QFrame
{
  Q_OBJECT;

public:

  enum Direction { HORIZONTAL, VERTICAL };

  GradientWidget( QWidget *parent = 0 );
  virtual ~GradientWidget( );

  void setDirection( Direction direction );

  void setGradientStops( const QGradientStops& stops );
  const QGradientStops& getGradientStops() const;

  void plot( const QPolygonF& plot );
  QPolygonF plot( void );

  void clearPlot( void );

signals:

  void clicked( void );

protected:

  virtual void paintEvent( QPaintEvent* event );
  virtual void mousePressEvent( QMouseEvent* event );

  Direction _direction;
  QGradientStops _stops;

  QPolygonF _plot;

private:

  float xPos( float x_ );
  float yPos( float y_ );

};

#endif
