#pragma once

#include <QWidget>
#include <QImage>

class CImageViewer : public QWidget
{
    Q_OBJECT

public:
    CImageViewer(QWidget *parent = nullptr);
    bool loadFile(const QString &);
    void clear();

private:
    void setImage(const QImage &newImage);
    virtual void paintEvent(QPaintEvent* pEv) override;

private:
    QImage mImage;
};
