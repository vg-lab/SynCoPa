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

#ifndef _CONFIGURATION_DIALOG_H_
#define _CONFIGURATION_DIALOG_H_

// Qt
#include <QDialog>

// Project
#include "ui_ConfigurationDialog.h"


class ConfigurationDialog: public QDialog, private Ui::ConfigurationDialog
{
    Q_OBJECT
  public:
    /** \brief ConfigurationDialog class constructor. 
     * \param[in] parent Raw pointer of the widget parent of this one. 
     * \param[in] f Dialog flags. 
     */
    explicit ConfigurationDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /** \brief ConfigurationDialog class virtual destructor. 
     */
    virtual ~ConfigurationDialog() = default;

  protected:
    virtual void showEvent(QShowEvent* e) override;

  private slots:
    /** \brief Helper method to change the associated samples spinbox value and send a signal.
     * \param[ni] newValue New sampling value.
     */
    void samplesValueChanged(int newValue);
    
    /** \brief Helper method to change the associated alpha spinbox value and send a signal.
     * \param[ni] newValue New sampling value.
     */
    void alphaValueChanged(int newValue);

    /** \brief Helper method to change the associated lod spinbox value and send a signal.
     * \param[in] newValue New lod value.
     */
    void lodValueChanged(int newValue);

  public slots:
    virtual void reject() override;

  private:
    /** \brief Helper method to connect signals to slots.
     */
    void connectSignals();

    float m_initialAlpha;          /** initial apha value */
    unsigned int m_initialSamples; /** initial samples value. */
    float m_initialLOD;            /** initial lod value */
};

#endif