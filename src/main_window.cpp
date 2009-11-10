// -*-c++-*-

/*!
  \file main_window.cpp
  \brief main application window class Source File.
*/

/*
 *Copyright:

 Copyright (C) The RoboCup Soccer Server Maintenance Group.
 Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtGui>

#include "main_window.h"

#include "config_dialog.h"
#include "field_canvas.h"
#include "monitor_client.h"
#include "options.h"

#include <string>
#include <iostream>
#include <cstring>
#include <cstdio>

#include "icons/rcss.xpm"

// #ifndef PACKAGE_STRING
// #define PACKAGE_STRING "rcssmonitor x.x.x"
// #endif

/*-------------------------------------------------------------------*/
/*!

 */
MainWindow::MainWindow()
    : QMainWindow( /* parent, flags */ ),
      M_window_style( "plastique" ),
      M_config_dialog( static_cast< ConfigDialog * >( 0 ) ),
      M_field_canvas( static_cast< FieldCanvas * >( 0 ) ),
      M_monitor_client( static_cast< MonitorClient * >( 0 ) )
{
    this->setWindowIcon( QIcon( QPixmap( rcss_xpm ) ) );
    this->setWindowTitle( tr( PACKAGE_STRING ) );
    this->setMinimumSize( 280, 220 );

    readSettings();

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createFieldCanvas();
    createConfigDialog();

    connect( M_field_canvas, SIGNAL( focusChanged( const QPoint & ) ),
             this, SLOT( setFocusPoint( const QPoint & ) ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
MainWindow::~MainWindow()
{
    writeSettings();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::init()
{
    if ( Options::instance().connect() )
    {
        connectMonitor();
    }

    if ( Options::instance().fullScreen() )
    {
        this->showFullScreen();
    }
    else if ( Options::instance().maximize() )
    {
        this->showMaximized();
    }
    else if ( Options::instance().canvasWidth() > 0
              && Options::instance().canvasHeight() > 0 )
    {
        resizeCanvas( QSize( Options::instance().canvasWidth(),
                             Options::instance().canvasHeight() ) );
    }
    else
    {
        this->resize( 640, 480 );
    }

    if ( QApplication::setStyle( M_window_style ) )
    {
        Q_FOREACH( QAction * action, M_style_act_group->actions() )
        {
            if ( action->data().toString().toLower() == QApplication::style()->objectName().toLower() )
            {
                M_window_style = QApplication::style()->objectName().toLower();
                action->setChecked( true );
                break;
            }
        }
    }

    toggleMenuBar( ! Options::instance().hideMenuBar() );
    toggleStatusBar( ! Options::instance().hideStatusBar() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::readSettings()
{
    QSettings settings( Options::CONF_FILE,
                        QSettings::IniFormat );

    settings.beginGroup( "MainWindow" );

    QVariant val;

    M_window_style = settings.value( "window_style", "plastique" ).toString();

    settings.endGroup();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::writeSettings()
{
    QSettings settings( Options::CONF_FILE,
                        QSettings::IniFormat );

    settings.beginGroup( "MainWindow" );

    settings.setValue( "window_style", M_window_style );

//         settings.setValue( "window_width", this->width() );
//         settings.setValue( "window_height", this->height() );
//         settings.setValue( "window_x", this->pos().x() );
//         settings.setValue( "window_y", this->pos().y() );
//         settings.setValue( "maximize", this->isMaximized() );
//         settings.setValue( "full_screen", this->isFullScreen() );
//         settings.setValue( "hide_menu_bar", this->menuBar()->isHidden() );
//         settings.setValue( "hide_tool_bar", ( M_log_player_tool_bar->isHidden()
//                                               && M_log_slider_tool_bar->isHidden() ) );
//         settings.setValue( "hide_status_bar", this->statusBar()->isHidden() );

    settings.endGroup();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createActions()
{
    createActionsFile();
    createActionsMonitor();
    createActionsReferee();
    createActionsView();
    createActionsHelp();

}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createActionsFile()
{
    //
    M_exit_act = new QAction( tr( "&Quit" ), this );
#ifdef Q_WS_MAC
    M_exit_act->setShortcut( Qt::META + Qt::Key_Q );
#else
    M_exit_act->setShortcut( Qt::CTRL + Qt::Key_Q );
#endif
    M_exit_act->setStatusTip( tr( "Exit the application." ) );
    connect( M_exit_act, SIGNAL( triggered() ),
             this, SLOT( close() ) );
    this->addAction( M_exit_act );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createActionsMonitor()
{
    //
    M_connect_monitor_act = new QAction( tr( "&Connect" ), this );
#ifdef Q_WS_MAC
    M_connect_monitor_act->setShortcut( Qt::META + Qt::Key_C );
#else
    M_connect_monitor_act->setShortcut( Qt::CTRL + Qt::Key_C );
#endif
    M_connect_monitor_act->setStatusTip( "Connect to the rcssserver on localhost" );
    M_connect_monitor_act->setEnabled( true );
    connect( M_connect_monitor_act, SIGNAL( triggered() ),
             this, SLOT( connectMonitor() ) );
    this->addAction( M_connect_monitor_act );
    //
    M_connect_monitor_to_act = new QAction( tr( "Connect &to ..." ), this );
    M_connect_monitor_to_act->setStatusTip( tr( "Connect to the rcssserver on the remote host." ) );
    M_connect_monitor_to_act->setEnabled( true );
    connect( M_connect_monitor_to_act, SIGNAL( triggered() ),
             this, SLOT( connectMonitorTo() ) );
    this->addAction( M_connect_monitor_to_act );
    //
    M_disconnect_monitor_act = new QAction( tr( "&Disconnect" ), this );
    M_disconnect_monitor_act->setStatusTip( tr( "Disonnect from rcssserver." ) );
    M_disconnect_monitor_act->setEnabled( false );
    connect( M_disconnect_monitor_act, SIGNAL( triggered() ),
             this, SLOT( disconnectMonitor() ) );
    this->addAction( M_disconnect_monitor_act );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createActionsReferee()
{
    // kick off
    M_kick_off_act = new QAction( tr( "&KickOff" ), this );
#ifdef Q_WS_MAC
    M_kick_off_act->setShortcut( Qt::META + Qt::Key_K );
#else
    M_kick_off_act->setShortcut( Qt::CTRL + Qt::Key_K );
#endif
    M_kick_off_act->setStatusTip( tr( "Send kick-off command." ) );
    connect( M_kick_off_act, SIGNAL( triggered() ),
             this, SLOT( kickOff() ) );
    this->addAction( M_kick_off_act );

    // yellow card
    M_yellow_card_act = new QAction( tr( "Yellow Card" ), this );
    M_yellow_card_act->setStatusTip( tr( "Call yellow card." ) );
    connect( M_yellow_card_act, SIGNAL( triggered() ),
             this, SLOT( yellowCard() ) );
    this->addAction( M_yellow_card_act );
    // red card
    M_red_card_act = new QAction( tr( "Red Card" ), this );
    M_red_card_act->setStatusTip( tr( "Call red card." ) );
    connect( M_red_card_act, SIGNAL( triggered() ),
             this, SLOT( yellowCard() ) );
    this->addAction( M_red_card_act );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createActionsView()
{
    // menu bar
    M_toggle_menu_bar_act = new QAction( tr( "Menu Bar" ), this );
#ifdef Q_WS_MAC
    M_toggle_menu_bar_act->setShortcut( Qt::META + Qt::Key_M );
#else
    M_toggle_menu_bar_act->setShortcut( Qt::CTRL + Qt::Key_M );
#endif
    M_toggle_menu_bar_act->setCheckable( true );
    M_toggle_menu_bar_act->setChecked( ! Options::instance().hideMenuBar() );
    M_toggle_menu_bar_act->setStatusTip( tr( "Show/Hide menu bar." ) );
    connect( M_toggle_menu_bar_act, SIGNAL( toggled( bool ) ),
             this, SLOT( toggleMenuBar( bool ) ) );
    this->addAction( M_toggle_menu_bar_act );

    // status bar
    M_toggle_status_bar_act = new QAction( tr( "Status Bar" ), this );
    M_toggle_status_bar_act->setCheckable( true );
    M_toggle_status_bar_act->setChecked( ! Options::instance().hideStatusBar() );
    M_toggle_status_bar_act->setStatusTip( tr( "Show/Hide status bar." ) );
    connect( M_toggle_status_bar_act, SIGNAL( toggled( bool ) ),
             this, SLOT( toggleStatusBar( bool ) ) );
    this->addAction( M_toggle_status_bar_act );

    // qt style menu group
    M_style_act_group = new QActionGroup( this );
    Q_FOREACH ( QString style_name, QStyleFactory::keys() )
    {
        QAction * subaction = new QAction( M_style_act_group );
        subaction->setText( style_name );
        subaction->setData( style_name );
        subaction->setCheckable( true );
        if ( style_name.toLower() == QApplication::style()->objectName().toLower() )
        {
            subaction->setChecked( true );
        }
        connect( subaction, SIGNAL( triggered( bool ) ),
                 this, SLOT( changeStyle( bool ) ) );
    }

    // show/hide config dialog
    M_show_config_dialog_act = new QAction( tr( "Config" ), this );
#ifdef Q_WS_MAC
    M_show_config_dialog_act->setShortcut( tr( "Meta+V" ) );
#else
    M_show_config_dialog_act->setShortcut( tr( "Ctrl+V" ) );
#endif
    M_show_config_dialog_act->setStatusTip( tr( "Show config dialog." ) );
    connect( M_show_config_dialog_act, SIGNAL( triggered() ),
             this, SLOT( showConfigDialog() ) );
    this->addAction( M_show_config_dialog_act );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createActionsHelp()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createMenus()
{
    createMenuFile();
    createMenuMonitor();
    createMenuReferee();
    createMenuView();
    createMenuHelp();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createMenuFile()
{
    //QMenu * menu = menuBar()->addMenu( tr( "&File" ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createMenuMonitor()
{
    QMenu * menu = menuBar()->addMenu( tr( "&Monitor" ) );

    menu->addAction( M_connect_monitor_act );
    menu->addAction( M_connect_monitor_to_act );
    menu->addAction( M_disconnect_monitor_act );

    menu->addSeparator();

    menu->addAction( M_exit_act );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createMenuReferee()
{
    QMenu * menu = menuBar()->addMenu( tr( "&Referee" ) );

    menu->addAction( M_kick_off_act );
    menu->addAction( M_yellow_card_act );
    menu->addAction( M_red_card_act );


//     menu->addAction( M_set_live_mode_act );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createMenuView()
{
    QMenu * menu = menuBar()->addMenu( tr( "&View" ) );

    menu->addAction( M_toggle_menu_bar_act );
    menu->addAction( M_toggle_status_bar_act );

    menu->addSeparator();

    {
        QMenu * submenu = menu->addMenu( tr( "Window &Style" ) );
        Q_FOREACH ( QAction * action, M_style_act_group->actions() )
        {
            submenu->addAction( action );
        }
    }

    menu->addSeparator();

    menu->addAction( M_show_config_dialog_act );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createMenuHelp()
{
    QMenu * menu = menuBar()->addMenu( tr( "&Help" ) );
    //menu->addAction( M_about_act );
    {
        QAction * act = menu->addAction( tr( "About" ), this, SLOT( about() ) );
        act->setStatusTip( tr( "Show the about dialog." ) );
    }

    menu->addAction( tr( "About Qt" ), qApp, SLOT( aboutQt() ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createToolBars()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createStatusBar()
{
    this->statusBar()->showMessage( tr( "Ready" ) );

    M_position_label = new QLabel( tr( "(0.0, 0.0)" ) );

    int min_width
        = M_position_label->fontMetrics().width(  tr( "(-60.0, -30.0)" ) )
        + 16;
    M_position_label->setMinimumWidth( min_width );
    M_position_label->setAlignment( Qt::AlignRight );

    this->statusBar()->addPermanentWidget( M_position_label );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createFieldCanvas()
{
    M_field_canvas = new FieldCanvas( M_disp_holder );
    this->setCentralWidget( M_field_canvas );

    M_field_canvas->setFocus();

    connect( this, SIGNAL( viewUpdated() ),
             M_field_canvas, SLOT( update() ) );

    connect( M_field_canvas, SIGNAL( mouseMoved( const QPoint & ) ),
             this, SLOT( updatePositionLabel( const QPoint & ) ) );

    connect( M_field_canvas, SIGNAL( dropBall( const QPoint & ) ),
             this, SLOT( dropBall( const QPoint & ) ) );
    connect( M_field_canvas, SIGNAL( freeKickLeft( const QPoint & ) ),
             this, SLOT( freeKickLeft( const QPoint & ) ) );
    connect( M_field_canvas, SIGNAL( freeKickRight( const QPoint & ) ),
             this, SLOT( freeKickRight( const QPoint & ) ) );

}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::createConfigDialog()
{
    if ( M_config_dialog )
    {
        return;
    }

    M_config_dialog = new ConfigDialog( this, M_disp_holder );

    M_config_dialog->hide();

    connect( M_config_dialog, SIGNAL( configured() ),
             this, SIGNAL( viewUpdated() ) );

    connect( M_config_dialog, SIGNAL( canvasResized( const QSize & ) ),
             this, SLOT( resizeCanvas( const QSize & ) ) );

    // register short cut keys
    {
        // z
        QAction * act = new QAction( tr( "ZoomIn" ), this );
        act->setShortcut( Qt::Key_Z );
        act->setStatusTip( tr( "Zoom in." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( zoomIn() ) );
    }
    {
        // x
        QAction * act = new QAction( tr( "ZoomOut" ), this );
        act->setShortcut( Qt::Key_X );
        act->setStatusTip( tr( "Zoom out." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( zoomOut() ) );
    }
    {
        // Ctrl + z
        QAction * act = new QAction( tr( "ZoomOut" ), this );
#ifdef Q_WS_MAC
        act->setShortcut( Qt::META + Qt::Key_Z );
#else
        act->setShortcut( Qt::CTRL + Qt::Key_Z );
#endif
        act->setStatusTip( tr( "Zoom out." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( zoomOut() ) );
    }
    {
        // i
        QAction * act = new QAction( tr( "Fit Field Size" ), this );
        act->setShortcut( Qt::Key_I );
        act->setStatusTip( tr( "Fit field size to the screen." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( fitToScreen() ) );
    }

    // field style
//     {
//         QAction * act = new QAction( tr( "Show Keepaway Area" ), this );
//         act->setStatusTip( tr( "Show keepaway area." ) );
//         this->addAction( act );
//         connect( act, SIGNAL( triggered() ),
//                  M_config_dialog, SLOT( toggleShowKeepawayArea() ) );
//     }

    // player detail
    {
        // n
        QAction * act = new QAction( tr( "Show Player Number" ), this );
        act->setShortcut( Qt::Key_N );
        act->setStatusTip( tr( "Show/Hide player number." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowPlayerNumber() ) );
    }
    {
        // h
        QAction * act = new QAction( tr( "Show Player Type Id" ), this );
        act->setShortcut( Qt::Key_H );
        act->setStatusTip( tr( "Show/Hide player type id." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowPlayerType() ) );
    }
    {
        // s
        QAction * act = new QAction( tr( "Show Stamina" ), this );
        act->setShortcut( Qt::Key_S );
        act->setStatusTip( tr( "Show/Hide player's stamina." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowStamina() ) );
    }
    {
        // Ctrl + s
        QAction * act = new QAction( tr( "Show Stamina Capacity" ), this );
#ifdef Q_WS_MAC
        act->setShortcut( Qt::META + Qt::Key_S );
#else
        act->setShortcut( Qt::CTRL + Qt::Key_S );
#endif
        act->setStatusTip( tr( "Show/Hide player's stamina capacity." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowStaminaCapacity() ) );
    }
    {
        // v
        QAction * act = new QAction( tr( "Show View Area" ), this );
        act->setShortcut( Qt::Key_V );
        act->setStatusTip( tr( "Show/Hide player's view area." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowViewArea() ) );
    }
    {
        // c
        QAction * act = new QAction( tr( "Show Catch Area" ), this );
        act->setShortcut( Qt::Key_C );
        act->setStatusTip( tr( "Show/Hide goalie's catchable area." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowCatchArea() ) );
    }
    {
        // k
        QAction * act = new QAction( tr( "Show Tackle Area" ), this );
        act->setShortcut( Qt::Key_T );
        act->setStatusTip( tr( "Show/Hide player's tackle area if player can tackle the ball." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowTackleArea() ) );
    }
    {
        // k
        QAction * act = new QAction( tr( "Show Kick Accel Area" ), this );
        act->setShortcut( Qt::Key_K );
        act->setStatusTip( tr( "Show/Hide player's kick accel area if player can kick the ball." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowKickAccelArea() ) );
    }
    {
        // Ctrl + p
        QAction * act = new QAction( tr( "Show Pointto Point" ), this );
#ifdef Q_WS_MAC
        act->setShortcut( Qt::META + Qt::Key_P );
#else
        act->setShortcut( Qt::CTRL + Qt::Key_P );
#endif
        act->setStatusTip( tr( "Show/Hide player's pointing position." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowPointto() ) );
    }

    // show/hide
//     {
//         // Ctrl + s
//         QAction * act = new QAction( tr( "Show Score Board" ), this );
//  #ifdef Q_WS_MAC
//         act->setShortcut( Qt::META + Qt::Key_S );
// #else
//         act->setShortcut( Qt::CTRL + Qt::Key_S );
// #endif
//         this->addAction( act );
//         connect( act, SIGNAL( triggered() ),
//                  M_config_dialog, SLOT( toggleShowScoreBoard() ) );
//     }
    {
        // Ctrl + b
        QAction * act = new QAction( tr( "Show Ball" ), this );
#ifdef Q_WS_MAC
        act->setShortcut( Qt::META + Qt::Key_B );
#else
        act->setShortcut( Qt::CTRL + Qt::Key_B );
#endif
        act->setStatusTip( tr( "Show/Hide ball." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowBall() ) );
    }
    {
        // Ctrl + p
        QAction * act = new QAction( tr( "Show Players" ), this );
#ifdef Q_WS_MAC
        act->setShortcut( Qt::META + Qt::Key_P );
#else
        act->setShortcut( Qt::CTRL + Qt::Key_P );
#endif
        act->setStatusTip( tr( "Show/Hide players." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowPlayer() ) );
    }
    {
        // Ctrl + f
        QAction * act = new QAction( tr( "Show Flags" ), this );
#ifdef Q_WS_MAC
        act->setShortcut( Qt::META + Qt::Key_F );
#else
        act->setShortcut( Qt::CTRL + Qt::Key_F );
#endif
        act->setStatusTip( tr( "Show/Hide flags." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowFlag() ) );
    }
    {
        // o
        QAction * act = new QAction( tr( "Show Offside Line" ), this );
        act->setShortcut( Qt::Key_O );
        act->setStatusTip( tr( "Show/Hide offside lines." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleShowOffsideLine() ) );
    }

    // number 1-10
    for ( int i = 0; i < 10; ++i )
    {
        {
            QAction * act = new QAction( QString( "Select Left %1" ).arg( i ), this );
            act->setShortcut( Qt::Key_0 + i );
            act->setStatusTip( QString( "Select left player %1" ).arg( i ) );
            this->addAction( act );
            connect( act, SIGNAL( triggered() ),
                     M_config_dialog, SLOT( selectPlayerWithKey() ) );
        }
        {
            QAction * act = new QAction( QString( "Selct Right %1" ).arg( i ), this );
#ifdef Q_WS_MAC
            act->setShortcut( Qt::META + Qt::Key_0 + i );
#else
            act->setShortcut( Qt::CTRL + Qt::Key_0 + i );
#endif
            act->setStatusTip( QString( "Select right player %1" ).arg( i ) );
            this->addAction( act );
            connect( act, SIGNAL( triggered() ),
                     M_config_dialog, SLOT( selectPlayerWithKey() ) );
        }
    }
    // number 11
    {
        {
            QAction * act = new QAction( tr( "Select Left 11" ), this );
            act->setShortcut( Qt::Key_Minus );
            act->setStatusTip( tr( "Select left player 11" ) );
            this->addAction( act );
            connect( act, SIGNAL( triggered() ),
                     M_config_dialog, SLOT( selectPlayerWithKey() ) );
        }
        {
            QAction * act = new QAction( tr( "Select Right 11" ), this );
#ifdef Q_WS_MAC
            act->setShortcut( Qt::META + Qt::Key_Minus );
#else
            act->setShortcut( Qt::CTRL + Qt::Key_Minus );
#endif
            act->setStatusTip( tr( "Select right player 11" ) );
            this->addAction( act );
            connect( act, SIGNAL( triggered() ),
                     M_config_dialog, SLOT( selectPlayerWithKey() ) );
        }
    }
    // b
    {
        QAction * act = new QAction( tr( "Focus Ball" ), this );
        act->setShortcut( Qt::Key_B );
        act->setStatusTip( tr( "Toggle automatic ball focus mode." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleFocusBall() ) );
    }
    // p
    {
        QAction * act = new QAction( tr( "Focus Player" ), this );
        act->setShortcut( Qt::Key_P );
        act->setStatusTip( tr( "Toggle automatic player focus mode." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleFocusPlayer() ) );
    }
    // a
    {
        QAction * act = new QAction( tr( "Select auto all" ), this );
        act->setShortcut( Qt::Key_A );
        act->setStatusTip( tr( "Toggle automatic player selection from all players." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleSelectAutoAll() ) );
    }
    // l
    {
        QAction * act = new QAction( tr( "Select auto left" ), this );
        act->setShortcut( Qt::Key_L );
        act->setStatusTip( tr( "Toggle automatic player selection from left team." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleSelectAutoLeft() ) );
    }
    // r
    {
        QAction * act = new QAction( tr( "Select auto right" ), this );
        act->setShortcut( Qt::Key_R );
        act->setStatusTip( tr( "Toggle automatic player selection from right team." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( toggleSelectAutoRight() ) );
    }
    // u
    {
        QAction * act = new QAction( tr( "Unselect" ), this );
        act->setShortcut( Qt::Key_U );
        act->setStatusTip( tr( "Unselect the player." ) );
        this->addAction( act );
        connect( act, SIGNAL( triggered() ),
                 M_config_dialog, SLOT( setUnselect() ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::closeEvent( QCloseEvent * event )
{
    event->ignore();

    qApp->quit();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::connectMonitorTo( const char * hostname )
{
    if ( std::strlen( hostname ) == 0 )
    {
        std::cerr << "Empty host name! Connection failed!" << std::endl;
        return;
    }

    std::cerr << "Connect to [" << hostname << "] ..." << std::endl;

    M_monitor_client = new MonitorClient( this,
                                          M_disp_holder,
                                          hostname,
                                          Options::instance().serverPort(),
                                          Options::instance().clientVersion() );

    if ( ! M_monitor_client->isConnected() )
    {
        std::cerr << "Conenction failed." << std::endl;
        delete M_monitor_client;
        M_monitor_client = static_cast< MonitorClient * >( 0 );
        return;
    }

    // reset all data
    M_disp_holder.clear();

//     if ( M_player_type_dialog )
//     {
//         M_player_type_dialog->hide();
//     }

    if ( M_config_dialog )
    {
        M_config_dialog->fitToScreen();
    }

    //Options::instance().setMonitorClientMode( true );
    Options::instance().setServerHost( hostname );

//     M_save_image_act->setEnabled( false );
//     M_open_output_act->setEnabled( true );

//     M_set_live_mode_act->setEnabled( true );
//     M_connect_monitor_act->setEnabled( false );
//     M_connect_monitor_to_act->setEnabled( false );
//     M_disconnect_monitor_act->setEnabled( true );

    connect( M_monitor_client, SIGNAL( received() ),
             this, SLOT( receiveMonitorPacket() ) );
//     connect( M_monitor_client, SIGNAL( timeout() ),
//              this, SLOT( disconnectMonitor() ) );

//     M_log_player->setLiveMode();

    M_monitor_client->sendDispInit();

    if ( QApplication::overrideCursor() )
    {
        QApplication::restoreOverrideCursor();
    }

}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::about()
{
    QString msg( tr( PACKAGE_NAME"-"VERSION"\n\n" ) );
    msg += tr( "The RoboCup Soccer Simulator Monitor (rcssmonitor) is used to view\n"
               "the simulation as it takes place by connecting to the rcssserver or\n"
               "to view the playback of a simulation by connecting to the rcsslogplayer.\n"
               "\n"
               "The RoboCup Soccer Simulator Official Web Page:\n"
               "  http://sserver.sourceforge.net/\n"
               "Author:\n"
               "  The RoboCup Soccer Simulator Maintenance Committee.\n"
               "  <sserver-admin@lists.sourceforgenet>" );

    QMessageBox::about( this,
                        tr( "About "PACKAGE_NAME ),
                        msg );

    // from Qt 4.1 documents
    /*
      about() looks for a suitable icon in four locations:

      1. It prefers parent->icon() if that exists.
      2. If not, it tries the top-level widget containing parent.
      3. If that fails, it tries the active window.
      4. As a last resort it uses the Information icon.

      The about box has a single button labelled "OK".
    */
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::kickOff()
{
    if ( M_monitor_client
         && M_monitor_client->isConnected() )
    {
        M_monitor_client->sendKickOff();
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::connectMonitor()
{
    std::string host = Options::instance().serverHost();
    if ( host.empty() )
    {
        host = "127.0.0.1";
    }

    connectMonitorTo( host.c_str() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::connectMonitorTo()
{
    std::string host = Options::instance().serverHost();
    if ( host.empty() )
    {
        host = "127.0.0.1";
    }

    bool ok = true;
    QString text = QInputDialog::getText( this,
                                          tr( "Input sserver host name" ),
                                          tr( "Host name: "),
                                          QLineEdit::Normal,
                                          QString::fromStdString( host ),
                                          & ok );
    if ( ok
         && ! text.isEmpty() )
    {
        connectMonitorTo( text.toStdString().c_str() );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::disconnectMonitor()
{
    //std::cerr << "MainWindow::disconnectMonitor()" << std::endl;
    if ( M_monitor_client )
    {
        M_monitor_client->disconnect();

        disconnect( M_monitor_client, SIGNAL( received() ),
                    this, SLOT( receiveMonitorPacket() ) );

        disconnect( M_monitor_client, SIGNAL( timeout() ),
                    this, SLOT( disconnectMonitor() ) );

        delete M_monitor_client;
        M_monitor_client = static_cast< MonitorClient * >( 0 );
    }

    //Options::instance().setMonitorClientMode( false );

//     M_set_live_mode_act->setEnabled( false );
    M_connect_monitor_act->setEnabled( true );
    M_connect_monitor_to_act->setEnabled( true );
    M_disconnect_monitor_act->setEnabled( false );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::toggleMenuBar( bool checked )
{
    this->menuBar()->setVisible( checked );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::toggleStatusBar( bool checked )
{
    this->statusBar()->setVisible( checked );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::changeStyle( bool checked )
{
    if ( ! checked )
    {
        return;
    }

    QAction * action = qobject_cast< QAction * >( sender() );
    QStyle * style = QStyleFactory::create( action->data().toString() );
    Q_ASSERT( style );

    QApplication::setStyle( style );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::showConfigDialog()
{
    M_config_dialog->setVisible( ! M_config_dialog->isVisible() );

    if ( M_config_dialog->isVisible() )
    {
        M_config_dialog->setFixedSize( M_config_dialog->size() );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::setFocusPoint( const QPoint & point )
{
    Options::instance().setFocusPoint( point.x(), point.y() );

    emit viewUpdated();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::dropBall( const QPoint & point )
{
    {
        double x = Options::instance().fieldX( point.x() );
        double y = Options::instance().fieldY( point.y() );

        std::cerr << "drop ball to ("
                  << x << ", " << y << ")"
                  << std::endl;
    }

    if ( M_monitor_client
         && M_monitor_client->isConnected() )
    {
        double x = Options::instance().fieldX( point.x() );
        double y = Options::instance().fieldY( point.y() );

        std::cerr << "drop ball to ("
                  << x << ", " << y << ")"
                  << std::endl;
        M_monitor_client->sendDropBall( x, y );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::freeKickLeft( const QPoint & point )
{
    if ( M_monitor_client
         && M_monitor_client->isConnected() )
    {
        double x = Options::instance().fieldX( point.x() );
        double y = Options::instance().fieldY( point.y() );

        std::cerr << "free kick left at ("
                  << x << ", " << y << ")"
                  << std::endl;
        M_monitor_client->sendFreeKickLeft( x, y );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::freeKickRight( const QPoint & point )
{
    if ( M_monitor_client
         && M_monitor_client->isConnected() )
    {
        double x = Options::instance().fieldX( point.x() );
        double y = Options::instance().fieldY( point.y() );

        std::cerr << "free kick right at ("
                  << x << ", " << y << ")"
                  << std::endl;
        M_monitor_client->sendFreeKickRight( x, y );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::yellowCard( const char side,
                        const int unum )
{
    if ( M_monitor_client
         && M_monitor_client->isConnected() )
    {
        std::cerr << "yellow_card : "
                  << side << ' ' << unum
                  << std::endl;

        rcss::rcg::Side s = ( side == 'l'
                              ? rcss::rcg::LEFT
                              : side == 'r'
                              ? rcss::rcg::RIGHT
                              : rcss::rcg::NEUTRAL );
        M_monitor_client->sendYellowCard( s, unum );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::redCard( const char side,
                     const int unum )
{
    if ( M_monitor_client
         && M_monitor_client->isConnected() )
    {
        std::cerr << "red_card : "
                  << side << ' ' << unum
                  << std::endl;
        rcss::rcg::Side s = ( side == 'l'
                              ? rcss::rcg::LEFT
                              : side == 'r'
                              ? rcss::rcg::RIGHT
                              : rcss::rcg::NEUTRAL );
        M_monitor_client->sendRedCard( s, unum );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::yellowCard()
{
    QStringList players;
    players << tr( "" );
    for ( int i = 1; i <= 11; ++i )
    {
        players << tr( "Left %1" ).arg( i );
    }
    for ( int i = 1; i <= 11; ++i )
    {
        players << tr( "Right %1" ).arg( i );
    }

    bool ok = false;
    QString str = QInputDialog::getItem( this,
                                         tr( "Yellow Card" ),
                                         tr( "Select Player" ),
                                         players,
                                         0, // current
                                         false, // editable
                                         &ok );
    if ( ! ok )
    {
        return;
    }

    char side_str[8];
    int unum = 0;
    if ( std::sscanf( str.toAscii().data(), " %s %d ", side_str, &unum ) != 2 )
    {
        return;
    }

    yellowCard( ( side_str[0] == 'L'
                  ?  'l'
                  : side_str[0] == 'R'
                  ? 'r'
                  : '?' ),
                unum );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::redCard()
{
    QStringList players;
    players << tr( "" );
    for ( int i = 1; i <= 11; ++i )
    {
        players << tr( "Left %1" ).arg( i );
    }
    for ( int i = 1; i <= 11; ++i )
    {
        players << tr( "Right %1" ).arg( i );
    }

    bool ok = false;
    QString str = QInputDialog::getItem( this,
                                         tr( "Red Card" ),
                                         tr( "Select Player" ),
                                         players,
                                         0, // current
                                         false, // editable
                                         &ok );
    if ( ! ok )
    {
        return;
    }

    char side_str[8];
    int unum = 0;
    if ( std::sscanf( str.toAscii().data(), " %s %d ", side_str, &unum ) != 2 )
    {
        return;
    }

    redCard( ( side_str[0] == 'L'
               ?  'l'
               : side_str[0] == 'R'
               ? 'r'
               : '?' ),
             unum );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::receiveMonitorPacket()
{
    if ( M_disp_holder.playmode() == rcss::rcg::PM_TimeOver )
    {
        if ( Options::instance().autoQuitMode() )
        {
            int wait_msec = ( Options::instance().autoQuitWait() > 0
                              ? Options::instance().autoQuitWait() * 1000
                              : 100 );
            QTimer::singleShot( wait_msec,
                                qApp, SLOT( quit() ) );
        }
    }

    emit viewUpdated();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::resizeCanvas( const QSize & size )
{
    if ( centralWidget() )
    {
        if ( this->isMaximized()
             || this->isFullScreen() )
        {
            this->showNormal();
        }

        QRect rect = this->geometry();

        int width_diff = rect.width() - this->centralWidget()->width();
        int height_diff = rect.height() - this->centralWidget()->height();

        if ( width_diff + size.width() < this->minimumWidth() )
        {
            std::cerr << "Too small canvas width "
                      << size.width() << " "
                      << " minimum=" << this->minimumWidth()
                      << std::endl;
            return;
        }

        if ( height_diff + size.height() < this->minimumHeight() )
        {
            std::cerr << "Too small canvas height "
                      << size.height() << " "
                      << " minimum=" << this->minimumHeight()
                      << std::endl;
            return;
        }

        rect.setWidth( size.width() + width_diff );
        rect.setHeight( size.height() + height_diff );

        this->setGeometry( rect );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
MainWindow::updatePositionLabel( const QPoint & point )
{
    if ( this->statusBar()->isVisible() )
    {
        double x = Options::instance().fieldX( point.x() );
        double y = Options::instance().fieldY( point.y() );

        char buf[32];
        snprintf( buf, 32,
                  "(%.2f, %.2f)",
                  x, y );

        M_position_label->setText( QString::fromAscii( buf ) );
    }
}