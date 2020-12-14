/**
 * @file    CGroupedView.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CGroupedView class
 */
#pragma once

#include <QTreeView>

class CGroupedView : public QTreeView
{
    Q_OBJECT

    typedef QTreeView tParent;

public:
    explicit CGroupedView(QWidget *parent = nullptr);
    void copyGroupedViewSelection() const;
    void setModel(QAbstractItemModel *model) override;

protected:
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;
    void keyPressEvent ( QKeyEvent * event ) override;

private:
    void changeLevelExpansion(const QModelIndex& expandIdx, bool bExpand);
    void updateColumnsVisibility();
    void updateWidth();

private:
    enum class eUpdateRequired
    {
        eUpdateRequired_NO,
        eUpdateRequired_BE_READY,
        eUpdateRequired_REQUIRED
    };

    eUpdateRequired mWidthUpdateRequired;
    bool mbIsVerticalScrollBarVisible;
    bool mbRootExpandingRequired;
};
