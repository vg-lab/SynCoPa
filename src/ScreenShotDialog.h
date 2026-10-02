/*
 Copyright (c) 2026. VG-Lab/URJC.

 Authors: Felix de las Pozas Alvarez <felix.delaspozas@urjc.es>

 This library is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License version 3.0 as published
 by the Free Software Foundation.

 This library is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 details.

 You should have received a copy of the GNU Lesser General Public License
 along with this library; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _SCREENSHOT_DIALOG_H_
#define _SCREENSHOT_DIALOG_H_

// Qt
#include <QDialog>

// Project
#include "ui_ScreenShotDialog.h"

/** \class ScreenShotDialog
 * \brief Implements a dialog to select the screenshot resoluction and filename.
 */
class ScreenShotDialog : public QDialog, private Ui::ScreenShotDialog
{
    Q_OBJECT
  public:
    /** \brief ScreenShotDialog class constructor.
     * \param[in] currentSize Size of the 3D view widget.
     * \param[in] parent pointer to the widget parent of this one.
     * \param[in] flags window flags.
     */
    explicit ScreenShotDialog(const QSize& currentSize, QWidget* parent = nullptr,
                              Qt::WindowFlags f = Qt::WindowFlags());

    /** \brief ScreenShotDialog class virtual destructor. 
     */
    virtual ~ScreenShotDialog() = default;

    /** \brief Returns the selected image size.
     */
    QSize imageSize() const;

    /** \brief Returns the selected filename name and path.
     */
    QString filename() const;

  protected:
    virtual void showEvent(QShowEvent* e) override;

  private slots:
    /** \brief Opens the directory selection dialog.
     */
    void onFolderSelectionClicked();

    /** \brief Modifies the custom height value depending on the width selected by the user.
     */
    void onCustomWidthChanged();

  private:
    /** \brief Helper method to connect signals to slots.
     */
    void connectSignals();

    const QSize m_currentSize; /** current 3d widget size. */
};

#endif
