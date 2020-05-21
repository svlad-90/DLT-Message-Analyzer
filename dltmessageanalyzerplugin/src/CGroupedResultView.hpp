/**
 * @file    CGroupedResultView.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CGroupedResultView class
 */
#ifndef CGROUPEDRESULTVIEW_HPP
#define CGROUPEDRESULTVIEW_HPP

#include <QTreeView>

class CGroupedResultView : public QTreeView
{
    Q_OBJECT

    typedef QTreeView tParent;

public:
    explicit CGroupedResultView(QWidget *parent = nullptr);
    void copyGroupedViewSelection() const;

protected:
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;
    void keyPressEvent ( QKeyEvent * event ) override;

private:
    bool isVerticalScrollBarVisible() const;
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
};

#endif // CGROUPEDRESULTVIEW_HPP
