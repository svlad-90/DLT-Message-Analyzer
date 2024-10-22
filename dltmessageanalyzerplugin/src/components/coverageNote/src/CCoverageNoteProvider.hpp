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
private:
    tCoverageNote& mCoverageNote;
};

class CCoverageNoteProvider : public ICoverageNoteProvider,
                              public CSettingsManagerClient
{
    Q_OBJECT
public:
    CCoverageNoteProvider(QTabWidget* pMainTabWidget,
                          const tSettingsManagerPtr& pSettingsManager,
                          QTextEdit* commentTextEdit,
                          QTableView* itemsTableView,
                          QTextEdit* messagesTextEdit,
                          QPushButton* openButton,
                          QTextEdit* regexTextEdit,
                          QPushButton* useRegexButton,
                          QObject *parent = nullptr);

    // file level API
    bool loadCoverageNoteFile(const QString& filePath) override;
    bool saveCoverageNoteFile(const QString& filePath) override;
    void clearCoverageNote() override;
    bool exportCoverageNoteAsHTML(const QString& targetPath) override;

    // coverage note item methods
    tCoverageNoteItemId addCoverageNoteItem() override;
    void moveCoverageNoteItem(const tCoverageNoteItemId& from,
                              const tCoverageNoteItemId& to,
                              bool after = true) override;

    void setCoverageNoteItemRegex(const tCoverageNoteItemId& id, const QString& regex) override;
    void setCoverageNoteMessage(const tCoverageNoteItemId& id,
                                const QString& coverageNoteMessage) override;
    void scrollToLastCoveageNoteItem() override;

    tCoverageNoteItemPtr getCoverageNoteItem(const tCoverageNoteItemId& id) const override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void removeSelectedCoverageNote();
    void removeCoverageNoteItem(const tCoverageNoteItemId& id);

private:
    tCoverageNote mCoverageNote;
    QTextEdit* mpCommentTextEdit;
    QTableView* mpItemsTableView;
    QTextEdit* mpMessagesTextEdit;
    QPushButton* mpOpenButton;
    QTextEdit* mpRegexTextEdit;
    QPushButton* mpUseRegexButton;
    std::shared_ptr<CoverageNoteTableModel> mpCoverageNoteTableModel;
    QTabWidget* mpMainTabWidget;
};
