#pragma once

#include "QTextEdit"
#include "QTableView"
#include "QPushButton"
#include "QFile"
#include "QEvent"

#include "../api/ICoverageNoteProvider.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

class CoverageNoteTableModel : public QAbstractTableModel {
    Q_OBJECT

public:

    enum class eColumn
    {
        INDEX = 0,
        TIME = 1,
        USERNAME = 2,
        AMOUNT = USERNAME + 1
    };

    CoverageNoteTableModel(tCoverageNote& coverageNote, QObject *parent = nullptr);
    int rowCount(const QModelIndex & = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void updateView(const QModelIndex& from, const QModelIndex& to);
    void beginRemoveRows(const QModelIndex &parent, int first, int last);
    void endRemoveRows();
    void beginInsertRows(const QModelIndex &parent, int first, int last);
    void endInsertRows();
    bool beginMoveRows(const QModelIndex &sourceParent,
                       int sourceFirst, int sourceLast,
                       const QModelIndex &destinationParent,
                       int destinationRow);
    void endMoveRows();
private:
    tCoverageNote& mCoverageNote;
};

class CCoverageNoteProvider : public ICoverageNoteProvider,
                              public CSettingsManagerClient
{
    Q_OBJECT
public:
    CCoverageNoteProvider(const tSettingsManagerPtr& pSettingsManager,
                          QTextEdit* commentTextEdit,
                          QTableView* itemsTableView,
                          QTextEdit* messagesTextEdit,
                          QTextEdit* regexTextEdit,
                          QPushButton* useRegexButton,
                          QLineEdit* pCurrentFileLineEdit,
                          QTextEdit* pFileViewTextEdit,
                          QObject *parent = nullptr);
    tCoverageNoteItemId addCoverageNoteItem() override;
    void setCoverageNoteItemRegex(const tCoverageNoteItemId& id, const QString& regex) override;
    void setCoverageNoteItemMessage(const tCoverageNoteItemId& id,
                            const QString& comment) override;
    void scrollToLastCoveageNoteItem() override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void setMainTableView(QTableView* pMainTableView) override;
    void shutdown() override;

private:
    void scrollToFirstCoveageNoteItem();
    void removeSelectedCoverageNote();
    void removeCoverageNoteItem(const tCoverageNoteItemId& id);
    bool loadCoverageNoteFile(const QString& filePath);
    bool saveCoverageNoteFile(const QString& filePath);
    void saveCoverageNote(bool saveAsMode = false);
    void loadCoverageNote();
    void addGeneralComment();
    void normalizeColumnsSize();
    bool clearCoverageNote();
    void fetchTextEfitorDataToModel();
    void exportCoverageNoteAsHTML();
    void setLoadedJsonFile(const QString& filePath);
    const QString& getLoadedJsonFile() const;
    void addFilesDataComment();
    void moveSelectedCoverageNoteUp();
    void moveSelectedCoverageNoteDown();
    QModelIndexList getSelectedRows() const;


private:
    tCoverageNote mCoverageNote;
    QTextEdit* mpCommentTextEdit;
    QTableView* mpItemsTableView;
    QTextEdit* mpMessagesTextEdit;
    QTextEdit* mpRegexTextEdit;
    QPushButton* mpUseRegexButton;
    std::shared_ptr<CoverageNoteTableModel> mpCoverageNoteTableModel;
    QString mLoadedJsonFilePath;
    QLineEdit* mpCurrentFileLineEdit;
    QTextEdit* mpFilesViewTextEdit;
    QTableView* mpMainTableView;
};
