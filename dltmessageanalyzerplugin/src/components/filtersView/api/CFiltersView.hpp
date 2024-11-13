#pragma once

#include <QTreeView>

#include "common/Definitions.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

class CFiltersModel;
class CFilterItemDelegate;
class CRegexHistoryTextEdit;

class CFiltersView : public QTreeView,
                     public CSettingsManagerClient
{
    Q_OBJECT

    typedef QTreeView tParent;

public:
    CFiltersView(QWidget *parent = nullptr);
    ~CFiltersView() override;

    void setSpecificModel( CFiltersModel* pModel );
    void highlightInvalidRegex(const QModelIndex &index);
    void setRegexInputField(CRegexHistoryTextEdit* pRegexTextEdit);

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void keyPressEvent ( QKeyEvent * event ) override;

    void handleSettingsManagerChange() override;

private:
    void setModel(QAbstractItemModel *model) override;
    void updateColumnsVisibility();
    void updateWidth();
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;
    void copySelectedRowToClipboard();

private: // fields
    CFiltersModel* mpModel;

    enum class eUpdateRequired
    {
        eUpdateRequired_NO,
        eUpdateRequired_BE_READY,
        eUpdateRequired_REQUIRED
    };

    eUpdateRequired mWidthUpdateRequired;
    bool mbIsVerticalScrollBarVisible;
    bool mbResizeOnExpandCollapse;
    bool mbSkipFirstUpdateWidth;
    CRegexHistoryTextEdit* mpRegexTextEdit;
    CFilterItemDelegate* mpFilterItemDelegate;
};
