/**
 * @file    CGroupedViewModel.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CGroupedViewModel class
 */

#ifndef CGroupedViewModel_HPP
#define CGroupedViewModel_HPP

#include "memory"

#include "QAbstractItemModel"

#include "../common/Definitions.hpp"
#include "../common/CTreeItem.hpp"

class CGroupedViewModel : public QAbstractItemModel
{
public:
    explicit CGroupedViewModel(QObject *parent = nullptr);
    ~CGroupedViewModel() override;

    void addMatches( const tFoundMatches& matches, bool update );
    void resetData();
    void setUsedRegex(const QString& regex);

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    std::pair<bool /*result*/, QString /*error*/> exportToHTML(QString& resultHTML);

    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:

    void updateView();
    QModelIndex rootIndex() const;
    void updateAverageValues(CTreeItem* pItem, bool updatePayload, bool updateMessages);
    void updatePercentageValues(CTreeItem* pItem, bool updatePayload, bool updateMessages);

    CGroupedViewModel(const CGroupedViewModel&) = delete;
    CGroupedViewModel& operator=(const CGroupedViewModel&) = delete;
    CGroupedViewModel(const CGroupedViewModel&&) = delete;
    CGroupedViewModel& operator=(const CGroupedViewModel&&) = delete;

private:

    tTreeItemPtr mpRootItem;
    QString mRegex;
    eGroupedViewColumn mSortingColumn;
    Qt::SortOrder mSortOrder;

    CTreeItem::tHandleDuplicateFunc mDuplicatesHandler;
    CTreeItem::tSortingFunction mSortingHandler;
    CTreeItem::tFindItemFunc mFindHandler;

    struct tAnalyzedValues
    {
        tTimeStamp prevMessageTimestamp = 0u;
        tMsgId prevMessageId = 0;
        tTimeStamp minTime = 0u;
        tTimeStamp maxTime = 0u;
        bool perSecondStatisticsDiscarded = false;
        unsigned int analyzedPayload = 0u;
        unsigned int analyzedMessages = 0u;
    };

    tAnalyzedValues mAnalyzedValues;
};

#endif // CGroupedViewModel_HPP
