#include <QImage>
#include <QImageReader>
#include <QDir>
#include <QPainter>

#include "CImageViewer.hpp"
#include "components/log/api/CLog.hpp"

#include "DMA_Plantuml.hpp"

CImageViewer::CImageViewer(QWidget *parent):
    QWidget(parent)
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

PUML_PACKAGE_BEGIN(DMA_PlantumlView)
    PUML_CLASS_BEGIN_CHECKED(CImageViewer)
        PUML_INHERITANCE_CHECKED(QWidget, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(QImage, 1, 1, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
