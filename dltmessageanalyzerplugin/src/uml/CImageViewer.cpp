#include <QImage>
#include <QImageReader>
#include <QDir>
#include <QPainter>

#include "CImageViewer.hpp"
#include "../log/CConsoleCtrl.hpp"

CImageViewer::CImageViewer(QWidget *parent)
   : QWidget(parent)
{}

bool CImageViewer::loadFile(const QString &fileName)
{
    bool bResult = false;

    QImageReader reader(fileName);
    const QImage newImage = reader.read();

    if (false == newImage.isNull())
    {
        bResult = true;
        setImage(newImage);
    }
    else
    {
        SEND_WRN( QString("Cannot load %1: %2").arg(QDir::toNativeSeparators(fileName), reader.errorString()));
    }

    return bResult;
}

void CImageViewer::clear()
{
    setImage(QImage());
}

void CImageViewer::setImage(const QImage &newImage)
{
    mImage = newImage;
    resize(mImage.size());
}

void CImageViewer::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(0, 0, mImage);
}