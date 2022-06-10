//
// Created by gaeqs on 5/05/22.
//

#ifndef SYNCOPA_TRIPLESTATEBUTTON_H
#define SYNCOPA_TRIPLESTATEBUTTON_H


#include <QPushButton>

class TripleStateButton : public QPushButton
{

Q_OBJECT

public:

  enum State
  {
    Off ,
    On ,
    Focus
  };

private:

  State _state;

  QString _offIcon;
  QString _onIcon;
  QString _focusIcon;
  QString _offTooltip;
  QString _onTooltip;
  QString _focusTooltip;

  void refreshIcon( );

public:

  TripleStateButton( State visibility ,
                     QWidget* parent = nullptr ,
                     const QString& offIcon = ":/icons/eye_closed.svg" ,
                     const QString& onIcon = ":/icons/eye_open.svg" ,
                     const QString& focusIcon = ":/icons/eye_focus.svg" ,
                     const QString& offTooltip = "" ,
                     const QString& onTooltip = "" ,
                     const QString& focusTooltip = "" );

  State getState( ) const;

public slots:

  void setState( TripleStateButton::State state );

  void toggleState( bool inverse );

signals:

  void onStateChange( TripleStateButton::State state );

protected:

  void mousePressEvent( QMouseEvent* e ) override;

  void mouseReleaseEvent( QMouseEvent* event ) override;

  void mouseMoveEvent( QMouseEvent* event ) override;

};


#endif //SYNCOPA_TRIPLESTATEBUTTON_H
