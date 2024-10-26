#include "CCoverageNoteProvider.hpp"

#include "QMenu"
#include "QTextStream"
#include "QKeyEvent"
#include "QMessageBox"
#include "QFileDialog"
#include "QHeaderView"
#include "QLineEdit"

#include "components/log/api/CLog.hpp"

#include "DMA_Plantuml.hpp"

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

bool CoverageNoteTableModel::beginMoveRows(const QModelIndex &sourceParent,
                                           int sourceFirst,
                                           int sourceLast,
                                           const QModelIndex &destinationParent,
                                           int destinationRow)
{
    return QAbstractItemModel::beginMoveRows(sourceParent,
                                             sourceFirst,
                                             sourceLast,
                                             destinationParent,
                                             destinationRow);
}

void CoverageNoteTableModel::endMoveRows()
{
    QAbstractItemModel::endMoveRows();
}

////////////////////////////////////////////////////////////////////////////

static const int INVALID_INDEX = -1;
static const char* EMPTY_JSON_FILE_NAME = "none";

CCoverageNoteProvider::CCoverageNoteProvider(const tSettingsManagerPtr& pSettingsManager,
                                             QTextEdit* commentTextEdit,
                                             QTableView* itemsTableView,
                                             QTextEdit* messagesTextEdit,
                                             QTextEdit* regexTextEdit,
                                             QPushButton* useRegexButton,
                                             QLineEdit* pCurrentFileLineEdit,
                                             QTextEdit* pFilesViewTextEdit,
                                             QObject *parent):
ICoverageNoteProvider(parent),
CSettingsManagerClient(pSettingsManager),
mpCommentTextEdit(commentTextEdit),
mpItemsTableView(itemsTableView),
mpMessagesTextEdit(messagesTextEdit),
mpRegexTextEdit(regexTextEdit),
mpUseRegexButton(useRegexButton),
mpCoverageNoteTableModel(std::make_shared<CoverageNoteTableModel>(mCoverageNote)),
mLoadedJsonFilePath(),
mpCurrentFileLineEdit(pCurrentFileLineEdit),
mpFilesViewTextEdit(pFilesViewTextEdit),
mpMainTableView(nullptr)
{
    if(mpCurrentFileLineEdit)
    {
        mpCurrentFileLineEdit->setReadOnly(true);
    }

    setLoadedJsonFile("");

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
                    QAction* pAction = new QAction("Add comment from main table", mpItemsTableView);
                    pAction->setShortcut(QKeySequence(tr("Ctrl+Alt+M")));
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        addCommentFromMainTableRequested();
                    });
                    pAction->setEnabled(mpMainTableView && mpMainTableView->selectionModel() &&
                                        !mpMainTableView->selectionModel()->selectedRows().empty());
                    contextMenu.addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Add files data comment", mpItemsTableView);
                    pAction->setShortcut(QKeySequence(tr("Ctrl+Alt+F")));
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        addFilesDataComment();
                    });
                    contextMenu.addAction(pAction);
                }

                {
                    auto selectedRows = getSelectedRows();

                    QAction* pAction = new QAction("Delete comment", mpItemsTableView);
                    pAction->setShortcut(QKeySequence("Del"));

                    connect(pAction, &QAction::triggered, [this]()
                    {
                        removeSelectedCoverageNote();
                    });
                    pAction->setEnabled(selectedRows.size() == 1);
                    contextMenu.addAction(pAction);
                }

                contextMenu.addSeparator();

                {
                    QAction* pAction = new QAction("New ...", mpItemsTableView);
                    pAction->setShortcut(QKeySequence("Ctrl+N"));

                    connect(pAction, &QAction::triggered, [this]()
                    {
                        static_cast<void>(clearCoverageNote());
                    });
                    contextMenu.addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Open ...", mpItemsTableView);
                    pAction->setShortcut(QKeySequence("Ctrl+O"));

                    connect(pAction, &QAction::triggered, [this]()
                    {
                        loadCoverageNote();
                    });
                    contextMenu.addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Save ...", mpItemsTableView);
                    pAction->setShortcut(QKeySequence("Ctrl+S"));

                    connect(pAction, &QAction::triggered, [this]()
                    {
                        saveCoverageNote();
                    });
                    pAction->setEnabled(!mCoverageNote.empty());
                    contextMenu.addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Save as ...", mpItemsTableView);
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        saveCoverageNote(true);
                    });
                    pAction->setEnabled(!mCoverageNote.empty());
                    contextMenu.addAction(pAction);
                }

                contextMenu.addSeparator();

                {
                    auto selectedRows = getSelectedRows();

                    QAction* pAction = new QAction("Move up", mpItemsTableView);
                    pAction->setShortcut(QKeySequence("Ctrl+Up"));
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        moveSelectedCoverageNoteUp();
                    });
                    pAction->setEnabled(selectedRows.size() == 1u &&
                            selectedRows[0].row() > 0);
                    contextMenu.addAction(pAction);
                }

                {
                    auto selectedRows = getSelectedRows();

                    QAction* pAction = new QAction("Move down", mpItemsTableView);
                    pAction->setShortcut(QKeySequence("Ctrl+Down"));
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        moveSelectedCoverageNoteDown();
                    });
                    pAction->setEnabled(selectedRows.size() == 1u &&
                            selectedRows[0].row() < (static_cast<int>(mCoverageNote.size()) - 1));
                    contextMenu.addAction(pAction);
                }

                contextMenu.addSeparator();

                {
                    QAction* pAction = new QAction("Export to HTML ...", mpItemsTableView);
                    pAction->setShortcut(QKeySequence("Ctrl+E"));

                    connect(pAction, &QAction::triggered, [this]()
                    {
                        exportCoverageNoteAsHTML();
                    });
                    pAction->setEnabled(!mCoverageNote.empty());
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

void CCoverageNoteProvider::setMainTableView(QTableView* pMainTableView)
{
    mpMainTableView = pMainTableView;
}

void CCoverageNoteProvider::removeSelectedCoverageNote()
{
    auto selectedRows = getSelectedRows();

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

void CCoverageNoteProvider::setLoadedJsonFile(const QString& filePath)
{
    mLoadedJsonFilePath = filePath;

    if(mpCurrentFileLineEdit)
    {
        if(!filePath.isEmpty())
        {
            mpCurrentFileLineEdit->setText(filePath);
        }
        else
        {
            mpCurrentFileLineEdit->setText(EMPTY_JSON_FILE_NAME);
        }
    }
}

const QString& CCoverageNoteProvider::getLoadedJsonFile() const
{
    return mLoadedJsonFilePath;
}

void CCoverageNoteProvider::saveCoverageNote(bool saveAsMode)
{
    QString filePath;

    if(getLoadedJsonFile().isEmpty() || saveAsMode)
    {
        filePath = QFileDialog::getSaveFileName(
            nullptr,
            "Save coverage note",         // Dialog title
            QDir::homePath(),             // Default directory (home directory)
            "JSON Files (*.json)"         // Filter to only show JSON files
        );

        if (filePath.isEmpty())
        {
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
        filePath = getLoadedJsonFile();
    }

    if(saveCoverageNoteFile(filePath))
    {
        mCoverageNote.resetModified();
    }

    setLoadedJsonFile(filePath);
}

void CCoverageNoteProvider::loadCoverageNote()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Open coverage note",         // Dialog title
        QDir::homePath(),             // Default directory (home directory)
        "JSON Files (*.json)"         // Filter to only show JSON files
    );

    if (filePath.isEmpty())
    {
        SEND_WRN(QString("No File Selected. Please select or specify a valid file."));
        return;  // If no file is selected, exit the function
    }

    // Ensure the file name has the correct ".json" extension
    if (!filePath.endsWith(".json", Qt::CaseInsensitive)) {
        filePath += ".json";
    }

    if(loadCoverageNoteFile(filePath))
    {
        setLoadedJsonFile(filePath);
        mCoverageNote.resetModified();
    }

    scrollToFirstCoveageNoteItem();
}

void CCoverageNoteProvider::fetchTextEfitorDataToModel()
{
    if(mpItemsTableView &&
       mpCommentTextEdit)
    {
        auto selectedRows = getSelectedRows();

        if(selectedRows.size() == 1)
        {
            auto rowId = selectedRows[0].row();

            if(rowId >= 0 && rowId < static_cast<int>(mCoverageNote.size()))
            {
                mCoverageNote.setComment(rowId, mpCommentTextEdit->toPlainText());
            }
        }
    }
}

#define FETCH_TEXT_EDITOR_DATA_TO_MODEL(TEXT_EDIT, TABLE_VIEW, FIELD_NAME) \
if(TEXT_EDIT && TABLE_VIEW) \
{ \
    auto selectedRows = getSelectedRows(); \
\
    if(selectedRows.size() == 1) \
    { \
        auto rowId = selectedRows[0].row(); \
\
        if(rowId >= 0 && rowId < static_cast<int>(mCoverageNote.size())) \
        { \
            mCoverageNote.set##FIELD_NAME(rowId, TEXT_EDIT->toPlainText()); \
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
                bResult = true;
                event->accept();
            }
            else if((keyEvent->modifiers() & Qt::ControlModifier) != 0 &&
            (keyEvent->modifiers() & Qt::AltModifier) != 0 &&
            (keyEvent->key() == Qt::Key::Key_A))
            {
                addGeneralComment();
                event->accept();
                bResult = true;
            }
            else if((keyEvent->modifiers() & Qt::ControlModifier) != 0 &&
            (keyEvent->modifiers() & Qt::AltModifier) != 0 &&
            (keyEvent->key() == Qt::Key::Key_F))
            {
                addFilesDataComment();
                event->accept();
                bResult = true;
            }
            else if((keyEvent->modifiers() & Qt::ControlModifier) != 0 &&
            (keyEvent->modifiers() & Qt::AltModifier) != 0 &&
            (keyEvent->key() == Qt::Key::Key_M))
            {
                addCommentFromMainTableRequested();
                event->accept();
                bResult = true;
            }
            else if((keyEvent->modifiers() & Qt::ControlModifier) != 0 &&
            (keyEvent->key() == Qt::Key::Key_Up))
            {
                moveSelectedCoverageNoteUp();
                event->accept();
                bResult = true;
            }
            else if((keyEvent->modifiers() & Qt::ControlModifier) != 0 &&
            (keyEvent->key() == Qt::Key::Key_Down))
            {
                moveSelectedCoverageNoteDown();
                event->accept();
                bResult = true;
            }
        }
        else if(event->type() == QEvent::ShortcutOverride)
        {
            if((keyEvent->modifiers() & Qt::ControlModifier) != 0 &&
            (keyEvent->key() == Qt::Key::Key_S))
            {
                if(!mCoverageNote.empty())
                {
                    saveCoverageNote();
                }
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
                bResult = true;
            }
            else if((keyEvent->modifiers() & Qt::ControlModifier) != 0 &&
            (keyEvent->key() == Qt::Key::Key_E))
            {
                if(!mCoverageNote.empty())
                {
                    exportCoverageNoteAsHTML();
                }
                event->accept();
                bResult = true;
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
            FETCH_TEXT_EDITOR_DATA_TO_MODEL(mpCommentTextEdit, mpItemsTableView, Comment)
            bResult = true;
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

void CCoverageNoteProvider::shutdown()
{
    fetchTextEfitorDataToModel();

    if(mCoverageNote.isModified())
    {
        QString printFileName = !getLoadedJsonFile().isEmpty() ? getLoadedJsonFile() : EMPTY_JSON_FILE_NAME;

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, "Save Confirmation",
            QString("The '%1' coverage note was changed. Save it?").arg(printFileName),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        // Check user response
        if (reply == QMessageBox::Yes)
        {
            saveCoverageNote();
        }
    }
}

bool CCoverageNoteProvider::clearCoverageNote()
{
    bool bResult = true;

    auto handle_clear = [this]()
    {
        mpCoverageNoteTableModel->beginRemoveRows(QModelIndex(), 0, mCoverageNote.size() - 1);
        mCoverageNote.clear();
        mpCoverageNoteTableModel->endRemoveRows();
        setLoadedJsonFile("");
    };

    fetchTextEfitorDataToModel();

    if(mCoverageNote.isModified())
    {
        QString printFileName = !getLoadedJsonFile().isEmpty() ? getLoadedJsonFile() : EMPTY_JSON_FILE_NAME;

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, "Save Confirmation",
            QString("The '%1' coverage note was changed. Save it, continue without saving, or abort the operation?").arg(printFileName),
            QMessageBox::No | QMessageBox::Yes | QMessageBox::Abort, QMessageBox::Abort);

        // Check user response
        if (reply == QMessageBox::Yes)
        {
            saveCoverageNote();
            handle_clear();
        }
        if (reply == QMessageBox::No)
        {
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

void CCoverageNoteProvider::exportCoverageNoteAsHTML()
{
    QString filePath = QFileDialog::getSaveFileName(
        nullptr,
        "Save coverage note",         // Dialog title
        QDir::homePath(),             // Default directory (home directory)
        "HTML Files (*.html)"         // Filter to only show HTML files
    );

    if (filePath.isEmpty())
    {
        SEND_WRN(QString("No File Selected. Please select or specify a valid file."));
        return;  // If no file is selected, exit the function
    }

    if (!filePath.endsWith(".html", Qt::CaseInsensitive)) {
        filePath += ".html";
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        SEND_ERR(QString("Failed to open the target file for the HTML export. File path - '%1'").arg(filePath));
        return; // Failed to open the file
    }

    QTextStream out(&file);

    out << "<!DOCTYPE html>\n";
    out << "<html lang=\"en\">\n";
    out << "<head>\n";
    out << "<meta charset=\"UTF-8\">\n";
    out << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    out << "<title>Coverage Note</title>\n";
    out << "<style>\n"
        << "table { width: 100%; border-collapse: collapse; }\n"
        << "th, td { border: 1px solid black; padding: 8px; text-align: left; }\n"
        << "th { background-color: #f2f2f2; }\n"
        << "#details-container { display: flex; }\n"
        << "#general-info { width: 30%; border-right: 1px solid black; padding: 10px; }\n"
        << "#detailed-info { width: 70%; padding: 10px; }\n"
        << "tr.selected { background-color: orange; }\n"
        << "</style>\n";
    out << "</head>\n";
    out << "<body>\n";
    out << "<h2>Coverage Note</h2>\n";

    out << "<div id=\"details-container\">\n";

    out << "<div id=\"general-info\">\n";
    out << "<table id=\"coverage-table\">\n";
    out << "<tr>\n"
        << "<th>Index</th>\n"
        << "<th>Time</th>\n"
        << "<th>Username</th>\n"
        << "</tr>\n";

    int index = 0;
    for (const auto& item : mCoverageNote.getCoverageNoteItemVec())
    {
        out << "<tr onclick=\"selectRow(this, " << index << "); showDetails(" << index << ")\" id=\"row-" << index << "\">\n";
        out << "<td>" << index++ << "</td>\n";
        out << "<td>" << item->dateTime.toString("yyyy/MM/dd hh:mm:ss AP") << "</td>\n";
        out << "<td>" << (item->userName.pString ? item->userName.pString->toHtmlEscaped().replace("'", "\\'") : "") << "</td>\n";
        out << "</tr>\n";
    }

    out << "</table>\n";
    out << "</div>\n";

    out << "<div id=\"detailed-info\">\n";
    out << "<h3>Details</h3>\n";
    out << "<div id=\"details-content\"></div>\n";
    out << "</div>\n";

    out << "</div>\n";

    out << "<script>\n"
        << "const data = [\n";

    for (const auto& item : mCoverageNote.getCoverageNoteItemVec())
    {
        out << "{\n"
            << "  messages: '" << (item->message.pString ? item->message.pString->toHtmlEscaped().
            replace("'", "\\'") : "") << "',\n"
            << "  comment: '" << (item->comment.pString ? item->comment.pString->toHtmlEscaped().
            replace("'", "\\'").replace("\n", "<br/>").toHtmlEscaped() : "") << "',\n"
            << "  regex: '" << (item->regex.pString ? item->regex.pString->toHtmlEscaped().
            replace("'", "\\'") : "") << "'\n"
            << "},\n";
    }
    out << "];\n"
        << "function unescapeHTML(html) {\n"
        << "  var doc = new DOMParser().parseFromString(html, 'text/html');\n"
        << "  return doc.documentElement.textContent;\n"
        << "}\n"
        << "function showDetails(index) {\n"
        << "  const details = data[index];\n"
        << "  document.getElementById('details-content').innerHTML = `<p><strong>Messages:</strong> ${unescapeHTML(details.messages)}</p>` +\n"
        << "    `<p><strong>Comment:</strong> ${unescapeHTML(details.comment)}</p>` +\n"
        << "    `<p><strong>Used Regex:</strong> ${unescapeHTML(details.regex)}</p>`;\n"
        << "}\n"
        << "function selectRow(row, index) {\n"
        << "  const rows = document.querySelectorAll('#coverage-table tr');\n"
        << "  rows.forEach(r => r.classList.remove('selected'));\n"
        << "  row.classList.add('selected');\n"
        << "}\n"
        << "document.addEventListener('DOMContentLoaded', (event) => {\n"
        << "  const firstRow = document.getElementById('row-0');\n"
        << "  if (firstRow) {\n"
        << "    firstRow.classList.add('selected');\n"
        << "    showDetails(0);\n"
        << "  }\n"
        << "});\n"
        << "</script>\n";

    out << "</body>\n";
    out << "</html>\n";

    file.close();
}


void CCoverageNoteProvider::scrollToLastCoveageNoteItem()
{
    if(mpItemsTableView)
    {
        if(mCoverageNote.size() > 0)
        {
            auto targetIndex = mpCoverageNoteTableModel->index(mCoverageNote.size() - 1,
                static_cast<int>(CoverageNoteTableModel::eColumn::USERNAME));
            mpItemsTableView->scrollTo( targetIndex, QAbstractItemView::ScrollHint::PositionAtCenter );
            mpItemsTableView->setCurrentIndex(targetIndex);
        }
    }
}

void CCoverageNoteProvider::scrollToFirstCoveageNoteItem()
{
    if(mpItemsTableView)
    {
        if(mCoverageNote.size() > 0)
        {
            auto targetIndex = mpCoverageNoteTableModel->index(0,
                static_cast<int>(CoverageNoteTableModel::eColumn::USERNAME));
            mpItemsTableView->scrollTo( targetIndex, QAbstractItemView::ScrollHint::PositionAtCenter );
            mpItemsTableView->setCurrentIndex(targetIndex);
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
    scrollToLastCoveageNoteItem();
}

void CCoverageNoteProvider::addFilesDataComment()
{
    if(mpFilesViewTextEdit)
    {
        auto itemId = addCoverageNoteItem();
        QString message = "The following files were used for the analysis:<br/><br/>";
        message += mpFilesViewTextEdit->toPlainText().toHtmlEscaped().replace("\n", "<br/>");
        setCoverageNoteItemMessage(itemId, message);
        scrollToLastCoveageNoteItem();
    }
}

QModelIndexList CCoverageNoteProvider::getSelectedRows() const
{
    QModelIndexList result;

    if(mpItemsTableView)
    {
        auto pSelectionModel = mpItemsTableView->selectionModel();

        if(pSelectionModel)
        {
            result = pSelectionModel->selectedRows();
        }
    }

    return result;
}

void CCoverageNoteProvider::moveSelectedCoverageNoteUp()
{
    if(mpItemsTableView && mpCoverageNoteTableModel)
    {
        auto selectedRows = getSelectedRows();

        if(selectedRows.size() == 1)
        {
            auto sourceRow = selectedRows[0].row();

            if(sourceRow > 0)
            {
                auto targetRow = sourceRow - 1;
                auto bMoveStartResult = mpCoverageNoteTableModel->beginMoveRows(QModelIndex(),
                                            sourceRow,
                                            sourceRow,
                                            QModelIndex(),
                                            targetRow);
                if(bMoveStartResult)
                {
                    mCoverageNote.swap(sourceRow, targetRow);
                    mpCoverageNoteTableModel->endMoveRows();

                    // Ensure the new row is selected after the move
                    auto pSelectionModel = mpItemsTableView->selectionModel();
                    if(pSelectionModel)
                    {
                        QModelIndex targetIndex = mpCoverageNoteTableModel->index(targetRow, 0);  // Assuming column 0
                        pSelectionModel->clearSelection();  // Clear previous selection
                        pSelectionModel->select(targetIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                    }
                }
            }
        }
    }
}

void CCoverageNoteProvider::moveSelectedCoverageNoteDown()
{
    if(mpItemsTableView && mpCoverageNoteTableModel)
    {
        auto selectedRows = getSelectedRows();

        if(selectedRows.size() == 1)
        {
            auto sourceRow = selectedRows[0].row();

            if(sourceRow < (static_cast<int>(mCoverageNote.size()) - 1))
            {
                auto targetRow = sourceRow + 1;
                auto bMoveStartResult = mpCoverageNoteTableModel->beginMoveRows(QModelIndex(),
                                            targetRow,
                                            targetRow,
                                            QModelIndex(),
                                            sourceRow);
                if(bMoveStartResult)
                {
                    mCoverageNote.swap(sourceRow, targetRow);
                    mpCoverageNoteTableModel->endMoveRows();

                    // Ensure the new row is selected after the move
                    auto pSelectionModel = mpItemsTableView->selectionModel();
                    if(pSelectionModel)
                    {
                        QModelIndex targetIndex = mpCoverageNoteTableModel->index(targetRow, 0);  // Assuming column 0
                        pSelectionModel->clearSelection();  // Clear previous selection
                        pSelectionModel->select(targetIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                    }
                }
            }
        }
    }
}

PUML_PACKAGE_BEGIN(DMA_CoverageNote)
    PUML_CLASS_BEGIN_CHECKED(CoverageNoteTableModel)
        PUML_INHERITANCE_CHECKED(QAbstractTableModel, implements)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(tCoverageNote, 1, 1, uses)
    PUML_CLASS_END()

    PUML_CLASS_BEGIN_CHECKED(CCoverageNoteProvider)
        PUML_INHERITANCE_CHECKED(ICoverageNoteProvider, implements)
        PUML_USE_DEPENDENCY_CHECKED(ISettingsManager, 1, 1, uses)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(tCoverageNote, 1, 1, contains)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTextEdit, 1, 4, passes to nested entities)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTableView, 1, 2, passes to nested entities)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QPushButton, 1, 1, passes to nested entities)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QLineEdit, 1, 1, passes to nested entities)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CoverageNoteTableModel, 1, 1, uses)
        PUML_USE_DEPENDENCY_CHECKED(nlohmann::json, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
