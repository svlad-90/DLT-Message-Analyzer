/**
 * @file    CSearchResultHighlightingDelegate.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CSearchResultHighlightingDelegate class
 */
#pragma once

#include "QMap"

#include "QStyledItemDelegate"

class CSearchResultHighlightingDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    CSearchResultHighlightingDelegate();
    ~CSearchResultHighlightingDelegate() override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:

    bool mbMarkTimestampWithBold;
};
