#pragma once

#include "QPlainTextEdit"

class CConsoleView : public QPlainTextEdit
{
    Q_OBJECT

    typedef QPlainTextEdit tParent;

public:
    explicit CConsoleView(QWidget *parent = nullptr);

protected:
    virtual void keyPressEvent ( QKeyEvent* pEvent ) override;
};
