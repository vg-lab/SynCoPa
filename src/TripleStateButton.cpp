//
// Created by gaeqs on 5/05/22.
//

#include <QDebug>
#include <QMouseEvent>

#include "TripleStateButton.h"

void TripleStateButton::refreshIcon( )
{
  switch ( _state )
  {
    case Off:
      setIcon( QIcon( _offIcon ));
      setToolTip( _offTooltip );
      break;
    case On:
      setIcon( QIcon( _onIcon ));
      setToolTip( _onTooltip );
      break;
    case Focus:
      setIcon( QIcon( _focusIcon ));
      setToolTip( _focusTooltip );
      break;
  }
}


TripleStateButton::TripleStateButton( TripleStateButton::State visibility ,
                                      QWidget* parent ,
                                      const QString& offIcon ,
                                      const QString& onIcon ,
                                      const QString& focusIcon ,
                                      const QString& offTooltip ,
                                      const QString& onTooltip ,
                                      const QString& focusTooltip )
  : QPushButton( parent )
  , _state( visibility )
  , _offIcon( offIcon )
  , _onIcon( onIcon )
  , _focusIcon( focusIcon )
  , _offTooltip( offTooltip )
  , _onTooltip( onTooltip )
  , _focusTooltip( focusTooltip )
{
  setFlat( true );
  setFixedSize( 20 , 20 );
  setIconSize( QSize( 18 , 18 ));
  refreshIcon( );
}

TripleStateButton::State TripleStateButton::getState( ) const
{
  return _state;
}

void TripleStateButton::setState( TripleStateButton::State state )
{
  _state = state;
  refreshIcon( );
  emit onStateChange( _state );
}

void TripleStateButton::toggleState( bool inverse )
{
  switch ( _state )
  {
    case Off:
      _state = inverse ? Focus : On;
      break;
    case On:
      _state = inverse ? Off : Focus;
      break;
    case Focus:
      _state = inverse ? On : Off;
      break;
  }
  refreshIcon( );
  emit onStateChange( _state );
}

void TripleStateButton::mouseReleaseEvent( QMouseEvent* event )
{
  if ( hitButton( event->pos( )))
  {
    if ( event->button( ) == Qt::LeftButton )
    {
      toggleState( false );
      event->accept();
    }
    else if ( event->button( ) == Qt::RightButton )
    {
      toggleState( true );
      event->accept();
    }
  }
  setDown( false );
  QPushButton::mouseReleaseEvent( event );
}

void TripleStateButton::mousePressEvent( QMouseEvent* event )
{
  if ( event->button( ) == Qt::RightButton )
  {
    setDown( true );
    event->accept();
  }
  QAbstractButton::mousePressEvent( event );
}

void TripleStateButton::mouseMoveEvent( QMouseEvent* event )
{
  if ( hitButton( event->pos()) != isDown()) {
    setDown(!isDown());
    repaint();
    event->accept();
  } else if (!hitButton( event->pos())) {
    event->ignore();
  }
}
