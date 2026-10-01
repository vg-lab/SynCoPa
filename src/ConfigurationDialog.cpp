/*
 Copyright (c) 2025. VG-Lab/URJC.

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

// Project
#include "ConfigurationDialog.h"
#include "OpenGLWidget.h"

// Glew
#include <GL/gl.h>
#include <iostream>

//-----------------------------------------------------------------------------
ConfigurationDialog::ConfigurationDialog(QWidget* parent, Qt::WindowFlags f)
: QDialog{parent, f}
{
    setupUi(this);

    GLint maxSamples;
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxSamples);
    unsigned int power = 0;
    while(maxSamples > 1)
    {
        ++power;
        maxSamples /= 2;
    }

    this->msSlider->setMaximum(power);
    this->msSlider->setMinimum(0);
    this->msSpinbox->setMaximum(1 << power);
    this->msSpinbox->setMinimum(1);
    this->alphaSlider->setMaximum(90);
    this->alphaSlider->setMinimum(10);
    this->alphaSpinbox->setMaximum(90);
    this->alphaSpinbox->setMinimum(10);
    this->lodSlider->setMaximum(1000);
    this->lodSlider->setMinimum(10);
    this->lodSpinbox->setMaximum(100);
    this->lodSpinbox->setMinimum(10);

    auto samples = OpenGLWidget::SAMPLES;
    power = 0;
    while(samples > 1)
    {
        ++power;
        samples /= 2;
    }

    msSlider->setValue(power);
    const auto alpha = OpenGLWidget::ALPHA * 100;
    const auto lod = OpenGLWidget::LOD;

    this->alphaSlider->setValue(alpha);
    this->alphaSpinbox->setValue(alpha);
    this->msSlider->setValue(power);
    this->msSpinbox->setValue(1 << power);
    this->lodSlider->setValue(lod*10);
    this->lodSpinbox->setValue(lod);

    m_initialSamples = OpenGLWidget::SAMPLES;
    m_initialLOD = OpenGLWidget::LOD;
    m_initialAlpha = OpenGLWidget::ALPHA;

    connectSignals();
}

//-----------------------------------------------------------------------------
void ConfigurationDialog::showEvent(QShowEvent* e)
{
    QDialog::showEvent(e);

    alphaSpinbox->setMinimumWidth(msSpinbox->width());

    setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    setMinimumSize(600,0);
      
    adjustSize();
    setFixedSize(size());
      
    auto winrect = QApplication::activeWindow()->geometry();
    move(winrect.center() - rect().center());
}

//-----------------------------------------------------------------------------
void ConfigurationDialog::samplesValueChanged(int newValue)
{
    const auto samplesNum = 1 << newValue;
    msSpinbox->setValue(samplesNum);

    OpenGLWidget::SAMPLES = samplesNum;
}

//-----------------------------------------------------------------------------
void ConfigurationDialog::alphaValueChanged(int newValue)
{
    const float alpha = newValue / 100.f;
    alphaSpinbox->setValue(newValue);

    OpenGLWidget::ALPHA = alpha;
}

//-----------------------------------------------------------------------------
void ConfigurationDialog::lodValueChanged(int newValue)
{
    const float lod = newValue / 10.f;
    lodSpinbox->setValue(lod);

    OpenGLWidget::LOD = lod;
}

//-----------------------------------------------------------------------------
void ConfigurationDialog::connectSignals()
{
    connect(this->alphaSlider, SIGNAL(valueChanged(int)), this, SLOT(alphaValueChanged(int)));
    connect(this->msSlider, SIGNAL(valueChanged(int)), this, SLOT(samplesValueChanged(int)));
    connect(this->lodSlider, SIGNAL(valueChanged(int)), this, SLOT(lodValueChanged(int)));
}

//-----------------------------------------------------------------------------
void ConfigurationDialog::reject()
{
    OpenGLWidget::SAMPLES = m_initialSamples;
    OpenGLWidget::ALPHA = m_initialAlpha;
    OpenGLWidget::LOD = m_initialLOD;

    QDialog::reject();
}

