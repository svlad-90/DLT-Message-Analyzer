/**
 * @file    CSearchResultHighlightingDelegate.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CSearchResultHighlightingDelegate class
 */
#pragma once

#include "QMap"

#include "QStyledItemDelegate"

#include "components/settings/api/CSettingsManagerClient.hpp"

class CSearchResultHighlightingDelegate : public QStyledItemDelegate,
                                          public CSettingsManagerClient
{
    Q_OBJECT

public:

    CSearchResultHighlightingDelegate(QObject *parent = nullptr);
    ~CSearchResultHighlightingDelegate() override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

protected:
    void handleSettingsManagerChange() override;

private:

    bool mbMarkTimestampWithBold;
    tSearchResultColumnsVisibilityMap mSearchResultColumnsSearchMap;
};
