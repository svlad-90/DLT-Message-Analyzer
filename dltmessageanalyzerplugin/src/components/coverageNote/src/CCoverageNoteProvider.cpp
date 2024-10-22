#include "CCoverageNoteProvider.hpp"

#include "QMenu"
#include "QTextStream"
#include "QKeyEvent"
#include "QMessageBox"

#include "components/log/api/CLog.hpp"

CoverageNoteTableModel::CoverageNoteTableModel(tCoverageNote& coverageNote,QObject *parent)
    : QAbstractTableModel(parent), mCoverageNote(coverageNote)
{}

int CoverageNoteTableModel::rowCount(const QModelIndex &) const
{
    return mCoverageNote.coverageNoteItemVec.size();
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
            auto pElement = mCoverageNote.coverageNoteItemVec[index.row()];

            if(pElement)
            {
                if(static_cast<eColumn>(index.column()) == eColumn::INDEX)
                {
                    result = index.row();
                }
                else if(static_cast<eColumn>(index.column()) == eColumn::TIME)
                {
                    result = pElement->time;
                }
                else if(static_cast<eColumn>(index.column()) == eColumn::USERNAME)
                {
                    if(pElement->userName.pString)
                    {
                        result = *pElement->userName.pString;
                    }
                }
            }
        }
    }

    return result;
}

bool CoverageNoteTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool bResult = false;

    auto pElement = mCoverageNote.coverageNoteItemVec[index.row()];

    if(pElement)
    {
        if (role == Qt::EditRole)
        {
            if(static_cast<eColumn>(index.column()) == eColumn::USERNAME)
            {
                pElement->userName.pString = std::make_shared<QString>(value.toString());
                emit dataChanged(index, index);
                bResult = true;
            }
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

static const int FIRST_INDEX = 0;
static const int INVALID_INDEX = -1;

CCoverageNoteProvider::CCoverageNoteProvider(QTabWidget* pMainTabWidget,
                                             const tSettingsManagerPtr& pSettingsManager,
                                             QTextEdit* commentTextEdit,
                                             QTableView* itemsTableView,
                                             QTextEdit* messagesTextEdit,
                                             QPushButton* openButton,
                                             QTextEdit* regexTextEdit,
                                             QPushButton* useRegexButton,
                                             QObject *parent):
ICoverageNoteProvider(parent),
CSettingsManagerClient(pSettingsManager),
mpCommentTextEdit(commentTextEdit),
mpItemsTableView(itemsTableView),
mpMessagesTextEdit(messagesTextEdit),
mpOpenButton(openButton),
mpRegexTextEdit(regexTextEdit),
mpUseRegexButton(useRegexButton),
mpCoverageNoteTableModel(std::make_shared<CoverageNoteTableModel>(mCoverageNote)),
mpMainTabWidget(pMainTabWidget)
{
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
                            addCoverageNoteItem();
                        }
                    });
                    contextMenu.addAction(pAction);
                }

                {
                    auto pSelectionModel = mpItemsTableView->selectionModel();

                    if(pSelectionModel)
                    {
                        auto selectedRows = pSelectionModel->selectedRows();

                        if(selectedRows.size() == 1)
                        {
                            QAction* pAction = new QAction("Delete comment", mpItemsTableView);
                            pAction->setShortcut(QKeySequence("Del"));

                            connect(pAction, &QAction::triggered, [this]()
                            {
                                removeSelectedCoverageNote();
                            });
                            contextMenu.addAction(pAction);
                        }
                    }
                }
            }

            contextMenu.exec(mpItemsTableView->mapToGlobal(pos));
        };

        connect( mpItemsTableView, &QWidget::customContextMenuRequested, showContextMenu );

        mpItemsTableView->installEventFilter(this);

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
                        if(rowId >= 0 && rowId < static_cast<int>(mCoverageNote.coverageNoteItemVec.size()))
                        {
                            auto& pItem = mCoverageNote.coverageNoteItemVec[rowId];

                            if(pItem)
                            {
                                if(mpMessagesTextEdit)
                                {
                                    QString txt;

                                    if(pItem->noteMessage.pString)
                                    {
                                        txt.append(*pItem->noteMessage.pString);
                                    }

                                    mpMessagesTextEdit->setText(txt);
                                }

                                if(mpCommentTextEdit)
                                {
                                    QString txt;


                                    if(pItem->message.pString)
                                    {
                                        txt.append(*pItem->message.pString);
                                    }

                                    mpCommentTextEdit->setText(txt);
                                }

                                if(mpRegexTextEdit)
                                {
                                    QString txt;


                                    if(pItem->regex.pString)
                                    {
                                        txt.append(*pItem->regex.pString);
                                    }

                                    mpRegexTextEdit->setText(txt);
                                }
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
        connect( mpUseRegexButton, &QPushButton::pressed, [this]()
        {
            if(mpRegexTextEdit)
            {
                QString txt = mpRegexTextEdit->toPlainText();

                if(!txt.isEmpty())
                {
                    regexApplicationRequested(txt);
                    if(mpMainTabWidget)
                    {
                        mpMainTabWidget->setCurrentIndex(static_cast<int>(eTabIndexes::SEARCH_VIEW));
                    }
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

bool CCoverageNoteProvider::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == mpItemsTableView)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Delete)
            {
                removeSelectedCoverageNote();
                return true;  // Key event handled, stop propagation
            }
            else if((keyEvent->modifiers() & Qt::ControlModifier) != 0 &&
            (keyEvent->modifiers() & Qt::AltModifier) != 0 &&
            (keyEvent->key() == Qt::Key::Key_A))
            {
                if(mpCoverageNoteTableModel)
                {
                    addCoverageNoteItem();
                }
                return true;  // Key event handled, stop propagation
            }
        }
    }
    else if(obj == mpCommentTextEdit)
    {
        if (event->type() == QEvent::FocusOut)
        {
            if(mpCommentTextEdit && mpItemsTableView)
            {
                auto pSelectionModel = mpItemsTableView->selectionModel();

                if(pSelectionModel)
                {
                    auto selectedRows = pSelectionModel->selectedRows();

                    if(selectedRows.size() == 1)
                    {
                        auto rowId = selectedRows[0].row();

                        if(rowId >= 0 && rowId < static_cast<int>(mCoverageNote.coverageNoteItemVec.size()))
                        {
                            auto pItem = mCoverageNote.coverageNoteItemVec[rowId];
                            if(pItem)
                            {
                                pItem->message.pString = std::make_shared<QString>(mpCommentTextEdit->toHtml());
                            }
                        }
                    }
                }
            }
        }
    }
    else if(obj == mpMessagesTextEdit)
    {
        if (event->type() == QEvent::FocusOut)
        {
            if(mpMessagesTextEdit && mpItemsTableView)
            {
                auto pSelectionModel = mpItemsTableView->selectionModel();

                if(pSelectionModel)
                {
                    auto selectedRows = pSelectionModel->selectedRows();

                    if(selectedRows.size() == 1)
                    {
                        auto rowId = selectedRows[0].row();

                        if(rowId >= 0 && rowId < static_cast<int>(mCoverageNote.coverageNoteItemVec.size()))
                        {
                            auto pItem = mCoverageNote.coverageNoteItemVec[rowId];
                            if(pItem)
                            {
                                pItem->noteMessage.pString = std::make_shared<QString>(mpMessagesTextEdit->toHtml());
                            }
                        }
                    }
                }
            }
        }
    }
    else if(obj == mpRegexTextEdit)
    {
        if (event->type() == QEvent::FocusOut)
        {
            if(mpRegexTextEdit && mpItemsTableView)
            {
                auto pSelectionModel = mpItemsTableView->selectionModel();

                if(pSelectionModel)
                {
                    auto selectedRows = pSelectionModel->selectedRows();

                    if(selectedRows.size() == 1)
                    {
                        auto rowId = selectedRows[0].row();

                        if(rowId >= 0 && rowId < static_cast<int>(mCoverageNote.coverageNoteItemVec.size()))
                        {
                            auto pItem = mCoverageNote.coverageNoteItemVec[rowId];
                            if(pItem)
                            {
                                pItem->regex.pString = std::make_shared<QString>(mpRegexTextEdit->toHtml());
                            }
                        }
                    }
                }
            }
        }
    }

    return QObject::eventFilter(obj, event);  // Continue with default processing
}

bool CCoverageNoteProvider::loadCoverageNoteFile(const QString& filePath)
{
    bool bResult = false;

    QFile jsonFile(filePath);
    if(jsonFile.open(QFile::ReadWrite))
    {
        nlohmann::json jsonData;
        try
        {
            jsonData = nlohmann::json::parse(jsonFile.readAll());
            bResult = mCoverageNote.parseCoverageNote(jsonData);

            if(!bResult)
            {
                SEND_ERR(QString("Error during the parsing of the coverage note json! File path - '%1'").arg(filePath));
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

bool CCoverageNoteProvider::saveCoverageNoteFile(const QString& filePath)
{
    bool bResult = false;

    QFile jsonFile(filePath);
    if(jsonFile.open(QFile::ReadWrite))
    {
        try
        {
            auto jsonContent = mCoverageNote.serializeCoverageNote().dump();
            QTextStream out(&jsonFile);
            out << QString::fromStdString(jsonContent);
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

void CCoverageNoteProvider::clearCoverageNote()
{
    mCoverageNote.coverageNoteItemVec.clear();
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
            pSelectionModel->select(mpCoverageNoteTableModel->index(mCoverageNote.coverageNoteItemVec.size() - 1,
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

        result = mCoverageNote.coverageNoteItemVec.size();

        auto pItem = std::make_shared<tCoverageNoteItem>();
        pItem->time = QDateTime::currentDateTime();
        pItem->userName.pString = std::make_shared<QString>(getSettingsManager()->getUsername());
        mCoverageNote.coverageNoteItemVec.push_back(pItem);

        mpCoverageNoteTableModel->endInsertRows();
    }

    return result;
}

void CCoverageNoteProvider::removeCoverageNoteItem(const tCoverageNoteItemId& id)
{
    mpCoverageNoteTableModel->beginRemoveRows(QModelIndex(), id, id);

    if(id >= 0 && id < static_cast<int>(mCoverageNote.coverageNoteItemVec.size()))
    {
        mCoverageNote.coverageNoteItemVec.erase(mCoverageNote.coverageNoteItemVec.begin() + id);
    }

    mpCoverageNoteTableModel->endRemoveRows();
}

void CCoverageNoteProvider::moveCoverageNoteItem(const tCoverageNoteItemId& from,
                                                 const tCoverageNoteItemId& to,
                                                 bool after)
{
    auto& vec = mCoverageNote.coverageNoteItemVec;
    if(from >= 0 && from < static_cast<int>(mCoverageNote.coverageNoteItemVec.size()) &&
       to >= 0 && to < static_cast<int>(mCoverageNote.coverageNoteItemVec.size()))
    {
        auto value = vec[from];
        vec.erase(vec.begin() + from);

        auto insert_pos = vec.begin() + to;

        if(after)
            ++insert_pos;

        vec.insert(insert_pos, value);
    }
}

void CCoverageNoteProvider::setCoverageNoteItemRegex(const tCoverageNoteItemId& id, const QString& regex)
{
    if(id >= 0 && id < static_cast<int>(mCoverageNote.coverageNoteItemVec.size()))
    {
        mCoverageNote.coverageNoteItemVec[id]->regex.pString = std::make_shared<QString>(regex);
    }
}

void CCoverageNoteProvider::setCoverageNoteMessage(const tCoverageNoteItemId& id,
                                                   const QString& coverageNoteMessage)
{
    if(id >= 0 && id < static_cast<int>(mCoverageNote.coverageNoteItemVec.size()))
    {
        mCoverageNote.coverageNoteItemVec[id]->noteMessage = std::make_shared<QString>(coverageNoteMessage);
    }
}

tCoverageNoteItemPtr CCoverageNoteProvider::getCoverageNoteItem(const tCoverageNoteItemId& id) const
{
    tCoverageNoteItemPtr pResult = nullptr;

    if(id >= 0 && id < static_cast<int>(mCoverageNote.coverageNoteItemVec.size()))
    {
        pResult = mCoverageNote.coverageNoteItemVec[id];
    }

    return pResult;
}
