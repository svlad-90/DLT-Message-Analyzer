#pragma once

#include <QMap>
#include <QElapsedTimer>
#include <QModelIndex>
#include <QTreeView>
#include <QTimer>
#include <QListView>

#include <QStyledItemDelegate>

#include "components/settings/api/CSettingsManagerClient.hpp"

class QCompleter;
class CFiltersModel;
class CRegexLineEdit;

class CFilterItemDelegate : public QStyledItemDelegate,
                            public CSettingsManagerClient
{
public:
    typedef QStyledItemDelegate tParent;

    CFilterItemDelegate( QTreeView* pParentTree );

    void setSpecificModel( CFiltersModel* pModel );

    QWidget* createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void animateRows(const QVector<QModelIndex>& rows, const QColor& color, const int64_t& duration);

private:
    void updateSuggestions(const QString& input);
    void applySuggestions();
    void delayedComplete();

private slots:
    void closeEditorHandler(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);

private:
    // as only visible rows are rendered, we need to exclude some of them on each timer hit
    // otherwise the high CPU load is observed in case if some of animated rows are outside the visible area
    void excludeAnimatedRows();
    QColor getColor( const QModelIndex& index ) const;
    void drawText(const QString& inputStr,
                  const QStyleOptionViewItem& opt,
                  QPainter *painter,
                  bool bold) const;
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;

private:

    struct tCompletionData
    {
        CRegexLineEdit* pEditor = nullptr; // currently used editor
        QModelIndex modelIndex;
        QTimer completionTimer;
        QCompleter* pCompleter = nullptr;
        QListView* pPopUp = nullptr;
    };

    tCompletionData mCompletionData;

    struct tRowAnimationData
    {
        tRowAnimationData(const QColor& startColor_,
                          const QColor& intermediateColor_,
                          const int64_t& startTime_,
                          const int64_t& endTime_):
            startColor(startColor_),
            intermediateColor(intermediateColor_),
            startTime(startTime_),
            endTime(endTime_)
        {}

        QColor startColor;
        QColor intermediateColor;
        int64_t startTime;
        int64_t endTime;
    };

    typedef QMap<QModelIndex, tRowAnimationData> tRowAnimationDataMap;
    tRowAnimationDataMap mRowAnimationDataMap;
    QElapsedTimer mTime;
    QTimer mUpdateTimer;
    QTreeView* mpParentTree;
    CFiltersModel* mpModel;
};
