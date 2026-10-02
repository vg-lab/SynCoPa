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

// Project
#include "ScreenShotDialog.h"
#include "OpenGLWidget.h"

// Qt
#include <QStandardPaths>
#include <QDateTime>
#include <QFileDialog>
#include <QSettings>

//-----------------------------------------------------------------------------
ScreenShotDialog::ScreenShotDialog(const QSize &currentSize, QWidget * parent, Qt::WindowFlags f)
: QDialog{parent, f}
, m_currentSize{currentSize}
{
    setupUi(this);

    m_currentSizeText->setText(QString("(%1x%2)").arg(currentSize.width()).arg(currentSize.height()));
    m_current->setChecked(true);

    m_dir->setText(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first());
    const auto currentTime = QDateTime::currentDateTime();
    m_filename->setText(QString("Screenshot-%1.bmp").arg(currentTime.toString("dd-MM-yy-hh_mm_ss")));

    connectSignals();

    GLint maxTextureSize[2] = {0, 0};
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize[0]);
    m_customWidth->setMaximum(maxTextureSize[0]);

    m_customWidth->setValue(maxTextureSize[0]);
    m_GPULimit->setText(QString::number(maxTextureSize[0]));

    m_warning->setText(m_warning->text().arg(OpenGLWidget::SAMPLES));

    adjustSize();
}

//-----------------------------------------------------------------------------
QSize ScreenShotDialog::imageSize() const
{
    int width = m_currentSize.width(), height = m_currentSize.height();
    if(m_4k->isChecked())
    {
        width = 3840; 
        height = 2160;
    }
    else if(m_4kposter->isChecked())
    {
        width = 2160;
        height = 3840;
    }
    else if(m_8k->isChecked())
    {
        width = 7680;
        height = 4320;
    }
    else if(m_8kposter->isChecked())
    {
        width = 4320;
        height = 7680;
    }
    else if(m_custom->isChecked())
    {
        width = m_customWidth->value();
        height = m_customHeight->value();
    }

    return QSize{width, height};
}

//-----------------------------------------------------------------------------
QString ScreenShotDialog::filename() const
{
    return m_dir->text() + QDir::separator() + m_filename->text();
}

//-----------------------------------------------------------------------------
void ScreenShotDialog::showEvent(QShowEvent* e) 
{
    QDialog::showEvent(e);

    setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    setMinimumSize(500,0);
      
    adjustSize();
    setFixedSize(size());
      
    auto winrect = QApplication::activeWindow()->geometry();
    move(winrect.center() - rect().center());
}

//-----------------------------------------------------------------------------
void ScreenShotDialog::connectSignals()
{
    connect(m_dirButton, SIGNAL(clicked(bool)), this, SLOT(onFolderSelectionClicked()));
    connect(m_customWidth, SIGNAL(textChanged(const QString &)), this, SLOT(onCustomWidthChanged()));
}

//-----------------------------------------------------------------------------
void ScreenShotDialog::onFolderSelectionClicked()
{
    auto dir = QFileDialog::getExistingDirectory(this, "Directory to save Screenshot",
                                        QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first(),
                                        QFileDialog::Option::ShowDirsOnly|QFileDialog::Option::DontUseNativeDialog);

    if(!dir.isEmpty())
    {
        m_dir->setText(dir);
    }
}

//-----------------------------------------------------------------------------
void ScreenShotDialog::onCustomWidthChanged()
{
    const auto width = m_customWidth->value();
    m_customHeight->setValue(width * static_cast<float>(9.f/16.f));
}