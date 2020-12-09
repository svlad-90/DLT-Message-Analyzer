#include <QAction>
#include <QMenu>

#include "../api/CConsoleView.hpp"

#include "DMA_Plantuml.hpp"

namespace NShortcuts
{
    static bool isClearShortcut( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->modifiers() & Qt::ControlModifier && pEvent->modifiers() & Qt::ShiftModifier && pEvent->key() == Qt::Key_Space;
    }
}

CConsoleView::CConsoleView(QWidget *parent):
tParent(parent)
{
    auto showContextMenu = [this](const QPoint &pos)
    {
        QMenu* pContextMenu = createStandardContextMenu();

        pContextMenu->addSeparator();

        {
            QAction* pAction = new QAction("Clear", this);
            pAction->setShortcut(QKeySequence(tr("Ctrl+Shift+Space")));
            connect(pAction, &QAction::triggered, [this]()
            {
                clear();
            });
            pContextMenu->addAction(pAction);
        }

        pContextMenu->exec(mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );
}

void CConsoleView::keyPressEvent ( QKeyEvent* pEvent )
{
    if(true == NShortcuts::isClearShortcut(pEvent))
    {
        clear();
    }
    else
    {
        tParent::keyPressEvent(pEvent);
    }
}

PUML_PACKAGE_BEGIN(DMA_Log_API)
    PUML_CLASS_BEGIN_CHECKED(CConsoleView)
        PUML_INHERITANCE_CHECKED(QPlainTextEdit, extends)
    PUML_CLASS_END()
PUML_PACKAGE_END()
