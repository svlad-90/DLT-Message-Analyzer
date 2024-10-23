#include "CCoverageNoteProvider.hpp"

#include "QMenu"
#include "QTextStream"
#include "QKeyEvent"
#include "QMessageBox"
#include "QFileDialog"
#include "QHeaderView"

#include "components/log/api/CLog.hpp"

CoverageNoteTableModel::CoverageNoteTableModel(tCoverageNote& coverageNote,QObject *parent)
    : QAbstractTableModel(parent),
      mCoverageNote(coverageNote)
{}

int CoverageNoteTableModel::rowCount(const QModelIndex &) const
{
    return mCoverageNote.size();
}

int CoverageNoteTableModel::columnCount(const QModelIndex &) const
{
    return static_cast<int>(eColumn::AMOUNT);
}

QVariant CoverageNoteTableModel::data(const QModelIndex &index, int role) const
{
    QVariant result;

    if(index.isValid())
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            if(static_cast<eColumn>(index.column()) == eColumn::INDEX)
            {
                result = index.row();
            }
            else if(static_cast<eColumn>(index.column()) == eColumn::TIME)
            {
                result = mCoverageNote.getDateTime(index.row());
            }
            else if(static_cast<eColumn>(index.column()) == eColumn::USERNAME)
            {
                result = *mCoverageNote.getUsername(index.row());
            }
        }
    }

    return result;
}

bool CoverageNoteTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool bResult = false;

    if (role == Qt::EditRole)
    {
        if(static_cast<eColumn>(index.column()) == eColumn::USERNAME)
        {
            mCoverageNote.setUsername(index.row(), value.toString());
            emit dataChanged(index, index);
            bResult = true;
        }
    }

    return bResult;
}

Qt::ItemFlags CoverageNoteTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = Qt::NoItemFlags;
    if(index.isValid())
    {
        if(static_cast<eColumn>(index.column()) == eColumn::USERNAME)
        {
            result = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
        }
        else
        {
            result = Qt::NoItemFlags;
        }
    }

    return result;;
}

void CoverageNoteTableModel::beginRemoveRows(const QModelIndex &parent, int first, int last)
{
    QAbstractItemModel::beginRemoveRows(parent, first, last);
}

void CoverageNoteTableModel::endRemoveRows()
{
    QAbstractItemModel::endRemoveRows();
}

void CoverageNoteTableModel::beginInsertRows(const QModelIndex &parent, int first, int last)
{
    QAbstractItemModel::beginInsertRows(parent, first, last);
}

void CoverageNoteTableModel::endInsertRows()
{
    QAbstractItemModel::endInsertRows();
}

QVariant CoverageNoteTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if(static_cast<eColumn>(section) == eColumn::INDEX)
        {
            result = "Index";
        }
        else if(static_cast<eColumn>(section) == eColumn::TIME)
        {
            result = "Time";
        }
        else if(static_cast<eColumn>(section) == eColumn::USERNAME)
        {
            result = "Username";
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////

static const int INVALID_INDEX = -1;

CCoverageNoteProvider::CCoverageNoteProvider(const tSettingsManagerPtr& pSettingsManager,
                                             QTextEdit* commentTextEdit,
                                             QTableView* itemsTableView,
                                             QTextEdit* messagesTextEdit,
                                             QTextEdit* regexTextEdit,
                                             QPushButton* useRegexButton,
                                             QObject *parent):
ICoverageNoteProvider(parent),
CSettingsManagerClient(pSettingsManager),
mpCommentTextEdit(commentTextEdit),
mpItemsTableView(itemsTableView),
mpMessagesTextEdit(messagesTextEdit),
mpRegexTextEdit(regexTextEdit),
mpUseRegexButton(useRegexButton),
mpCoverageNoteTableModel(std::make_shared<CoverageNoteTableModel>(mCoverageNote)),
mLoadedJsonFilePath()
{
    if(mpMessagesTextEdit)
    {
        mpMessagesTextEdit->setReadOnly(true);
    }

    if(mpRegexTextEdit)
    {
        mpRegexTextEdit->setReadOnly(true);
    }

    if(mpItemsTableView)
    {
        mpItemsTableView->setModel(mpCoverageNoteTableModel.get());

        auto showContextMenu = [this](const QPoint &pos)
        {
            QMenu contextMenu("Context menu", mpItemsTableView);

            {
                {
                    QAction* pAction = new QAction("------", mpItemsTableView);
                    connect(pAction, &QAction::triggered, []()
                    {});
                    pAction->setEnabled(false);
                    contextMenu.addAction(pAction);
                }

                contextMenu.addSeparator();

                {
                    QAction* pAction = new QAction("Add comment", mpItemsTableView);
                    pAction->setShortcut(QKeySequence(tr("Ctrl+Alt+A")));
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        if(mpCoverageNoteTableModel)
                        {
                            addGeneralComment();
                        }
                    });
                    contextMenu.addAction(pAction);
                }

                {
                    auto pSelectionModel = mpItemsTableView->selectionModel();

                    if(pSelectionModel)
                    {
                        auto selectedRows = pSelectionModel->selectedRows();

                        QAction* pAction = new QAction("Delete comment", mpItemsTableView);
                        pAction->setShortcut(QKeySequence("Del"));

                        connect(pAction, &QAction::triggered, [this]()
                        {
                            removeSelectedCoverageNote();
                        });
                        pAction->setEnabled(selectedRows.size() == 1);
                        contextMenu.addAction(pAction);
                    }
                }

                {
                    QAction* pAction = new QAction("Save coverage note...", mpItemsTableView);
                        pAction->setShortcut(QKeySequence("Ctrl+S"));

                        connect(pAction, &QAction::triggered, [this]()
                        {
                            saveCoverageNote();
                        });
                        pAction->setEnabled(!mCoverageNote.empty());
                        contextMenu.addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Save coverage note as...", mpItemsTableView);

                        connect(pAction, &QAction::triggered, [this]()
                        {
                            saveCoverageNote(true);
                        });
                        pAction->setEnabled(!mCoverageNote.empty());
                        contextMenu.addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Open coverage note...", mpItemsTableView);
                        pAction->setShortcut(QKeySequence("Ctrl+O"));

                        connect(pAction, &QAction::triggered, [this]()
                        {
                            loadCoverageNote();
                        });
                        contextMenu.addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Create new coverage note...", mpItemsTableView);
                        pAction->setShortcut(QKeySequence("Ctrl+N"));

                        connect(pAction, &QAction::triggered, [this]()
                        {
                            static_cast<void>(clearCoverageNote());
                        });
                        contextMenu.addAction(pAction);
                }
            }

            contextMenu.exec(mpItemsTableView->mapToGlobal(pos));
        };

        connect( mpItemsTableView, &QWidget::customContextMenuRequested, showContextMenu );

        mpItemsTableView->installEventFilter(this);

        if(mpCommentTextEdit)
        {
            mpCommentTextEdit->installEventFilter(this);
        }

        if(mpMessagesTextEdit)
        {
            mpMessagesTextEdit->installEventFilter(this);
        }

        if(mpRegexTextEdit)
        {
            mpRegexTextEdit->installEventFilter(this);
        }

        auto pSelectionModel = mpItemsTableView->selectionModel();
        if(pSelectionModel)
        {
            QObject::connect(pSelectionModel, &QItemSelectionModel::selectionChanged,
            [this](const QItemSelection &selected, const QItemSelection &)
            {
                if (!selected.indexes().isEmpty())
                {
                    QModelIndex index = selected.indexes().first();

                    if(index.isValid())
                    {
                        auto rowId = index.row();
                        if(rowId >= 0 && rowId < static_cast<int>(mCoverageNote.size()))
                        {
                            if(mpMessagesTextEdit)
                            {
                                mpMessagesTextEdit->setText(*mCoverageNote.getMessage(rowId));
                            }

                            if(mpCommentTextEdit)
                            {
                                mpCommentTextEdit->setText(*mCoverageNote.getComment(rowId));
                            }

                            if(mpRegexTextEdit)
                            {
                                mpRegexTextEdit->setText(*mCoverageNote.getRegex(rowId));
                            }
                        }
                    }
                }
                else
                {
                    if(mpMessagesTextEdit)
                    {
                        mpMessagesTextEdit->clear();
                    }

                    if(mpCommentTextEdit)
                    {
                        mpCommentTextEdit->clear();
                    }

                    if(mpRegexTextEdit)
                    {
                        mpRegexTextEdit->clear();
                    }
                }
            });
        }
    }

    if(mpCommentTextEdit)
    {
        mpCommentTextEdit->installEventFilter(this);
    }

    if(mpMessagesTextEdit)
    {
        mpMessagesTextEdit->installEventFilter(this);
    }

    if(mpRegexTextEdit)
    {
        mpRegexTextEdit->installEventFilter(this);
    }

    if(mpUseRegexButton)
    {
        connect( mpUseRegexButton, &QPushButton::clicked, [this](bool)
        {
            if(mpRegexTextEdit)
            {
                QString txt = mpRegexTextEdit->toPlainText();

                if(!txt.isEmpty())
                {
                    regexApplicationRequested(txt);
                }
            }
        });
    }
}

void CCoverageNoteProvider::removeSelectedCoverageNote()
{
    if(mpCoverageNoteTableModel)
    {
        auto pSelectionModel = mpItemsTableView->selectionModel();

        if(pSelectionModel)
        {
            auto selectedRows = pSelectionModel->selectedRows();

            if(selectedRows.size() == 1)
            {
                // Create a confirmation dialog
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(nullptr, "Delete Confirmation",
                    QString("Are you sure you want to delete the coverage note item '%1'?").arg(selectedRows[0].row()),
                    QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);

                // Check user response
                if (reply == QMessageBox::Yes)
                {
                    removeCoverageNoteItem(selectedRows[0].row());
                }
            }
        }
    }
}

void CCoverageNoteProvider::saveCoverageNote(bool saveAsMode)
{
    QString filePath;

    if(mLoadedJsonFilePath.isEmpty() || saveAsMode)
    {
        filePath = QFileDialog::getSaveFileName(
            nullptr,
            "Save coverage note",         // Dialog title
            QDir::homePath(),             // Default directory (home directory)
            "JSON Files (*.json)"         // Filter to only show JSON files
        );

        if (filePath.isEmpty()) {
            SEND_WRN(QString("No File Selected. Please select or specify a valid file."));
            return;  // If no file is selected, exit the function
        }

        // Ensure the file name has the correct ".json" extension
        if (!filePath.endsWith(".json", Qt::CaseInsensitive)) {
            filePath += ".json";
        }
    }
    else
    {
        filePath = mLoadedJsonFilePath;
    }

    if(saveCoverageNoteFile(filePath))
    {
        mCoverageNote.resetModified();
    }
}

void CCoverageNoteProvider::loadCoverageNote()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Open coverage note",         // Dialog title
        QDir::homePath(),             // Default directory (home directory)
        "JSON Files (*.json)"         // Filter to only show JSON files
    );

    if (filePath.isEmpty()) {
        SEND_WRN(QString("No File Selected. Please select or specify a valid file."));
        return;  // If no file is selected, exit the function
    }

    // Ensure the file name has the correct ".json" extension
    if (!filePath.endsWith(".json", Qt::CaseInsensitive)) {
        filePath += ".json";
    }

    if(loadCoverageNoteFile(filePath))
    {
        mLoadedJsonFilePath = filePath;
        mCoverageNote.resetModified();
    }
}

#define HANDLE_FOCUS_OUT_EVENT(TEXT_EDIT, TABLE_VIEW, FIELD_NAME) \
if(TEXT_EDIT && TABLE_VIEW) \
{ \
    auto pSelectionModel = TABLE_VIEW->selectionModel(); \
 \
    if(pSelectionModel) \
    { \
        auto selectedRows = pSelectionModel->selectedRows(); \
 \
        if(selectedRows.size() == 1) \
        { \
            auto rowId = selectedRows[0].row(); \
 \
            if(rowId >= 0 && rowId < static_cast<int>(mCoverageNote.size())) \
            { \
                mCoverageNote.set##FIELD_NAME(rowId, TEXT_EDIT->toHtml()); \
            } \
        } \
    } \
 \
    bResult = true; \
}

bool CCoverageNoteProvider::eventFilter(QObject *obj, QEvent *event)
{
    if(!mpItemsTableView || !mpMessagesTextEdit || !mpRegexTextEdit || !mpCommentTextEdit)
        return QObject::eventFilter(obj, event);

    auto handleGenericShortcuts = [this, &event]() -> bool
    {
        bool bResult = false;

        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (event->type() == QEvent::KeyPress)
        {
            if (keyEvent->key() == Qt::Key_Delete)
            {
                removeSelectedCoverageNote();
                bResult = true;  // Key event handled, stop propagation
                event->accept();
            }
            else if((keyEvent->modifiers() & Qt::ControlModifier) != 0 &&
            (keyEvent->modifiers() & Qt::AltModifier) != 0 &&
            (keyEvent->key() == Qt::Key::Key_A))
            {
                addGeneralComment();
                event->accept();
                bResult = true;  // Key event handled, stop propagation
            }
        }
        else if(event->type() == QEvent::ShortcutOverride)
        {
            if((keyEvent->modifiers() & Qt::ControlModifier) != 0 &&
            (keyEvent->key() == Qt::Key::Key_S))
            {
                saveCoverageNote();
                event->accept();
                bResult = true;
            }
            else if((keyEvent->modifiers() & Qt::ControlModifier) != 0 &&
            (keyEvent->key() == Qt::Key::Key_O))
            {
                loadCoverageNote();
                event->accept();
                bResult = true;
            }
            else if((keyEvent->modifiers() & Qt::ControlModifier) != 0 &&
            (keyEvent->key() == Qt::Key::Key_N))
            {
                static_cast<void>(clearCoverageNote());
                event->accept();
                bResult = true;  // Key event handled, stop propagation
            }
        }

        return bResult;
    };

    bool bResult = false;

    if (event->type() == QEvent::KeyPress ||
        event->type() == QEvent::ShortcutOverride)
    {
        bResult = handleGenericShortcuts();

        if(!bResult)
        {
            bResult = QObject::eventFilter(obj, event);
        }
    }
    else if (event->type() == QEvent::FocusOut)
    {
        if(obj == mpCommentTextEdit)
        {
            HANDLE_FOCUS_OUT_EVENT(mpCommentTextEdit, mpItemsTableView, Comment)
        }
        else if(obj == mpMessagesTextEdit)
        {
            HANDLE_FOCUS_OUT_EVENT(mpMessagesTextEdit, mpItemsTableView, Message)
        }
        else if(obj == mpRegexTextEdit)
        {
            HANDLE_FOCUS_OUT_EVENT(mpRegexTextEdit, mpItemsTableView, Regex)
        }
        else
        {
            bResult = QObject::eventFilter(obj, event);
        }
    }
    else
    {
        bResult = QObject::eventFilter(obj, event);
    }

    return bResult;
}

bool CCoverageNoteProvider::loadCoverageNoteFile(const QString& filePath)
{
    bool bResult = false;

    QFile jsonFile(filePath);
    if(jsonFile.open(QFile::ReadOnly))
    {
        nlohmann::json jsonData;
        try
        {
            bResult = clearCoverageNote();

            if(bResult)
            {
                jsonData = nlohmann::json::parse(jsonFile.readAll());
    
                bResult = mCoverageNote.parseCoverageNote(jsonData);
    
                mpCoverageNoteTableModel->beginInsertRows(QModelIndex(), 0, mCoverageNote.size() - 1);
                if(!bResult)
                {
                    SEND_ERR(QString("Error during the parsing of the coverage note json! File path - '%1'").arg(filePath));
                }
                else
                {
                    SEND_MSG(QString("Successfully loaded the coverage note - '%1'").arg(filePath));
                }

                mpCoverageNoteTableModel->endInsertRows();

                normalizeColumnsSize();
            }
        }
        catch (const std::exception & e)
        {
            SEND_ERR(QString("Error during the parsing of the coverage note json: '%1'! File path - %2").arg(e.what()).arg(filePath));
        }
    }
    else
    {
        SEND_ERR(QString("Failed to open the coverage note json file. File path - '%1'").arg(filePath));
    }

    return bResult;
}

void CCoverageNoteProvider::normalizeColumnsSize()
{
    if(mpItemsTableView)
    {
       mpItemsTableView->resizeColumnsToContents();

       QHeaderView* pHeader = mpItemsTableView->horizontalHeader();

       if(pHeader)
       {
           int lastColumnIndex = pHeader->count() - 1;

           for (int i = 0; i < lastColumnIndex; ++i)
           {
               pHeader->setSectionResizeMode(i, QHeaderView::ResizeToContents);
           }

           pHeader->setSectionResizeMode(lastColumnIndex, QHeaderView::Stretch);
       }
    }
}

bool CCoverageNoteProvider::saveCoverageNoteFile(const QString& filePath)
{
    bool bResult = false;

    QFile jsonFile(filePath);
    if(jsonFile.open(QFile::Truncate|QFile::WriteOnly))
    {
        try
        {
            auto jsonContent = mCoverageNote.serializeCoverageNote().dump();
            QTextStream out(&jsonFile);
            out << QString::fromStdString(jsonContent);
            bResult = true;
            SEND_MSG(QString("Successfully saved the coverage note - '%1'").arg(filePath));
        }
        catch (const std::exception & e)
        {
            SEND_ERR(QString("Error during saving the coverage note json: '%1'! File path - %2").arg(e.what()).arg(filePath));
        }
    }
    else
    {
        SEND_ERR(QString("Failed to open the file. File path - %1").arg(filePath));
    }

    return bResult;
}

bool CCoverageNoteProvider::clearCoverageNote()
{
    bool bResult = true;

    auto handle_clear = [this]()
    {
        mpCoverageNoteTableModel->beginRemoveRows(QModelIndex(), 0, mCoverageNote.size() - 1);
        mCoverageNote.clear();
        mpCoverageNoteTableModel->endRemoveRows();
        mLoadedJsonFilePath.clear();
    };

    if(mCoverageNote.isModified() && !mLoadedJsonFilePath.isEmpty())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, "Save Confirmation",
            QString("The '%1' coverage note was changed. Save it or abort the operation?").arg(mLoadedJsonFilePath),
            QMessageBox::Abort | QMessageBox::Yes, QMessageBox::Abort);

        // Check user response
        if (reply == QMessageBox::Yes)
        {
            saveCoverageNote();
            handle_clear();
        }
        else
        {
            bResult = false;
        }
    }
    else
    {
        handle_clear();
    }

    return bResult;
}

bool CCoverageNoteProvider::exportCoverageNoteAsHTML(const QString& /*targetPath*/)
{
    // TODO
    return true;
}

void CCoverageNoteProvider::scrollToLastCoveageNoteItem()
{
    if(mpItemsTableView)
    {
        auto pSelectionModel = mpItemsTableView->selectionModel();

        if(pSelectionModel)
        {
            pSelectionModel->clearSelection();
            pSelectionModel->select(mpCoverageNoteTableModel->index(mCoverageNote.size() - 1,
                static_cast<int>(CoverageNoteTableModel::eColumn::USERNAME)), QItemSelectionModel::ClearAndSelect);
        }
    }
}

tCoverageNoteItemId CCoverageNoteProvider::addCoverageNoteItem()
{
    tCoverageNoteItemId result = INVALID_INDEX;

    if(mpCoverageNoteTableModel)
    {
        mpCoverageNoteTableModel->beginInsertRows(QModelIndex(), mpCoverageNoteTableModel->rowCount(), mpCoverageNoteTableModel->rowCount());

        result = mCoverageNote.size();

        auto id = mCoverageNote.addCoverageNoteItem();
        mCoverageNote.setDateTime(id, QDateTime::currentDateTime());
        mCoverageNote.setUsername(id, getSettingsManager()->getUsername());

        mpCoverageNoteTableModel->endInsertRows();
    }

    normalizeColumnsSize();

    return result;
}

void CCoverageNoteProvider::removeCoverageNoteItem(const tCoverageNoteItemId& id)
{
    mpCoverageNoteTableModel->beginRemoveRows(QModelIndex(), id, id);

    if(id >= 0 && id < static_cast<int>(mCoverageNote.size()))
    {
        mCoverageNote.removeCoverageNoteItem(id);
    }

    mpCoverageNoteTableModel->endRemoveRows();

    normalizeColumnsSize();
}

void CCoverageNoteProvider::setCoverageNoteItemRegex(const tCoverageNoteItemId& id, const QString& regex)
{
    if(id >= 0 && id < static_cast<int>(mCoverageNote.size()))
    {
        mCoverageNote.setRegex(id, regex);
    }
}

void CCoverageNoteProvider::setCoverageNoteItemMessage(const tCoverageNoteItemId& id,
                                                   const QString& comment)
{
    if(id >= 0 && id < static_cast<int>(mCoverageNote.size()))
    {
        mCoverageNote.setMessage(id, comment);
    }
}

void CCoverageNoteProvider::addGeneralComment()
{
    auto itemId = addCoverageNoteItem();
    setCoverageNoteItemMessage(itemId, "General comment");
    mCoverageNote.setRegex(itemId, "----");
    scrollToLastCoveageNoteItem();
}
