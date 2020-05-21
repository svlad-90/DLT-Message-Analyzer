#include "QTime"
#include "QTimer"
#include "QPainter"
#include "QApplication"

#include "QHeaderView"
#include "QScrollBar"
#include "QMenu"
#include "QStyledItemDelegate"

#include "CFiltersModel.hpp"
#include "CSettingsManager.hpp"
#include "CConsoleCtrl.hpp"

#include "CFiltersView.hpp"

class CRegexTreeRepresentationDelegate : public QStyledItemDelegate
{
public:
    CRegexTreeRepresentationDelegate(QTreeView* pParentTree)
        : QStyledItemDelegate(pParentTree),
          mRowAnimationDataMap(),
          mTime(),
          mUpdateTimer(),
          mpParentTree(pParentTree)
    {
        mTime.start();
    }

    void animateRows(const QVector<QModelIndex>& rows, const QColor& color, const int& duration)
    {
        mRowAnimationDataMap.clear();

        const auto columnsNumber = mpParentTree->model()->columnCount();

        for(const auto& row : rows)
        {
            int startTime = mTime.elapsed();

            for(int columnId = 0; columnId < columnsNumber; ++columnId)
            {
                if(false == mpParentTree->isColumnHidden(columnId))
                {
                    auto animatedIdx = row.sibling(row.row(), columnId);
                    QColor initialColor = getColor(animatedIdx);
                    mRowAnimationDataMap.insert( animatedIdx, tRowAnimationData(initialColor, color, startTime, startTime + duration) );
                }
            }
        }

        auto updateAnimatedRows = [this, columnsNumber]()
        {
            for( auto it = mRowAnimationDataMap.begin(); it != mRowAnimationDataMap.end(); ++it )
            {
                for(int columnId = 0; columnId < columnsNumber; ++columnId)
                {
                    if(false == mpParentTree->isColumnHidden(columnId))
                    {
                        QModelIndex updateIdx = it.key().sibling(it.key().row(), columnId);
                        mpParentTree->update( updateIdx );
                    }
                }
            }
        };

        if(nullptr != mpParentTree)
        {
            updateAnimatedRows();
            mUpdateTimer.start(16);
        }

        connect(&mUpdateTimer, &QTimer::timeout, [this, updateAnimatedRows]()
        {
            if(false == mRowAnimationDataMap.empty())
            {
                if(nullptr != mpParentTree)
                {
                    updateAnimatedRows();
                    mUpdateTimer.start(16);
                }

                 excludeAnimatedRows();
            }
        });
    }

private:

    // as only visible rows are rendered, we need to exclude some of them on each timer hit
    // otherwise the high CPU load is observed in case if some of animated rows are outside the visible area
    void excludeAnimatedRows()
    {
        for( auto it = mRowAnimationDataMap.begin(); it != mRowAnimationDataMap.end(); )
        {
            auto& animatedObject = *it;

            int nowToStartDiff = mTime.elapsed() - animatedObject.startTime;
            int endToStartDiff = animatedObject.endTime - animatedObject.startTime;

            int passedTimePercantage = static_cast<int>( 100 * ( static_cast<double>(nowToStartDiff) / endToStartDiff ) );

            if(passedTimePercantage > 100)
            {
                it = mRowAnimationDataMap.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    QColor getColor( const QModelIndex& index ) const
    {
        QColor result;

        auto rowType = index.sibling( index.row(), static_cast<int>(eRegexFiltersColumn::RowType) ).data().value<eRegexFiltersRowType>();

        auto parentIdx = index.parent();

        bool bParentExist = false;
        auto parentRowType = eRegexFiltersRowType::Text;

        if( parentIdx.isValid() )
        {
            bParentExist = true;
            parentRowType = parentIdx.sibling( parentIdx.row(), static_cast<int>(eRegexFiltersColumn::RowType) ).data().value<eRegexFiltersRowType>();
        }

        switch(rowType)
        {
            case eRegexFiltersRowType::Text:
            {
                if(true == bParentExist)
                {
                    if(parentRowType == eRegexFiltersRowType::VarGroup)
                    {
                        result = QColor("#F9F871");
                    }
                    else
                    {
                        result = QColor("#B3A7B8");
                    }
                }
                else
                {
                    result = QColor("#B3A7B8");
                }
            }
                break;
            case eRegexFiltersRowType::VarGroup:
            result = QColor("#FFC04A");
                break;
            case eRegexFiltersRowType::NonVarGroup:
            result = QColor("#B3A7B8");
                break;
            case eRegexFiltersRowType::NonCapturingGroup:
            result = QColor("#7E7383");
                break;
        }

        return result;
    }

    void drawText(const QString& inputStr,
                  const QStyleOptionViewItemV4& opt,
                  QPainter *painter,
                  bool bold) const
    {
        int baseShift = 0;

        if(Qt::AlignmentFlag::AlignLeft == opt.displayAlignment)
        {
            baseShift = 2;
        }

        QRect rect = opt.rect;

        //set pen color
        if (opt.state & QStyle::State_Selected)
        {
            painter->fillRect(opt.rect, opt.palette.highlight());

            QPalette::ColorGroup cg = QPalette::Normal;
            painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
        }
        else
        {
            QPalette::ColorGroup cg = QPalette::Normal;
            painter->setPen(opt.palette.color(cg, QPalette::Text));

            if(true == bold)
            {
                if(false == painter->font().bold())
                {
                    auto font = painter->font();
                    font.setBold(true);
                    painter->setFont(font);
                }
            }
            else
            {
                if(true == painter->font().bold())
                {
                    auto font = painter->font();
                    font.setBold(false);
                    painter->setFont(font);
                }
            }
        }

        painter->drawText(QRect(rect.left()+baseShift, rect.top(), rect.width(), rect.height()),
                                 opt.displayAlignment, inputStr);
    }

    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
    {
        if(nullptr == painter)
        {
            return;
        }

        painter->save();

        QStyleOptionViewItemV4 viewItemOption(option);

        auto foundAnimatedObject = const_cast<tRowAnimationDataMap*>(&mRowAnimationDataMap)->find( index );

        if(foundAnimatedObject != mRowAnimationDataMap.end())
        {
            int nowToStartDiff = mTime.elapsed() - foundAnimatedObject->startTime;
            int endToStartDiff = foundAnimatedObject->endTime - foundAnimatedObject->startTime;

            int passedTimePercantage = static_cast<int>( 100 * ( static_cast<double>(nowToStartDiff) / endToStartDiff ) );

            if( passedTimePercantage >= 100.0 ) // last frame
            {
                painter->fillRect( viewItemOption.rect, foundAnimatedObject.value().startColor );
                const_cast<tRowAnimationDataMap*>(&mRowAnimationDataMap)->erase(foundAnimatedObject);
            }
            else
            {
                auto calcColor = [](const QColor& fromColor, const QColor& toColor, const int& percentage)->QColor
                {
                    double x = percentage / 100.0;
                    double factor = 4 * -(x*x) + 4*x;

                    int rDifference = ( toColor.red() - fromColor.red() );
                    int r = fromColor.red() + static_cast<int>( rDifference * factor );
                    int gDifference = ( toColor.green() - fromColor.green() );
                    int g = fromColor.green() + static_cast<int>( gDifference * factor );
                    int bDifference = ( toColor.blue() - fromColor.blue() );
                    int b = fromColor.blue() + static_cast<int>( bDifference * factor );

                    //SEND_MSG(QString("~~~ percentage - %1; factor - %2").arg(percentage).arg(factor));

                    return QColor( r, g, b );
                };

                QColor color = calcColor(foundAnimatedObject->startColor, foundAnimatedObject->intermediateColor, passedTimePercantage);

                painter->fillRect( viewItemOption.rect, color );
            }
        }
        else
        {
            painter->fillRect( viewItemOption.rect, getColor(index) );
        }

        painter->restore();

#ifdef __linux__
        drawText( index.data().value<QString>(), viewItemOption, painter, true );
#else
        QStyledItemDelegate::paint(painter, viewItemOption, index);
#endif
    }

    struct tRowAnimationData
    {
        tRowAnimationData(const QColor& startColor_,
                          const QColor& intermediateColor_,
                          const int& startTime_,
                          const int& endTime_):
            startColor(startColor_),
            intermediateColor(intermediateColor_),
            startTime(startTime_),
            endTime(endTime_)
        {}

        QColor startColor;
        QColor intermediateColor;
        int startTime;
        int endTime;
    };

    typedef QMap<QModelIndex, tRowAnimationData> tRowAnimationDataMap;
    tRowAnimationDataMap mRowAnimationDataMap;
    QTime mTime;
    QTimer mUpdateTimer;
    QTreeView* mpParentTree;
};

CFiltersView::CFiltersView(QWidget *parent):
    QTreeView(parent),
    mpModel(nullptr),
    mWidthUpdateRequired(eUpdateRequired::eUpdateRequired_NO),
    mbIsVerticalScrollBarVisible(false),
    mpRepresentationDelegate(new CRegexTreeRepresentationDelegate(this)),
    mbResizeOnExpandCollapse(true),
    mbSkipFirstUpdateWidth(true)
{
    header()->setDefaultAlignment(Qt::AlignCenter);
    header()->setSectionsMovable(false);

    setSortingEnabled(true);
    sortByColumn( static_cast<int>(eRegexFiltersColumn::Index), Qt::SortOrder::AscendingOrder );

    connect(this, &QTreeView::doubleClicked, [this](const QModelIndex &index)
    {
        if(false == index.isValid())
        {
            return;
        }

        if( static_cast<eRegexFiltersColumn>(index.column()) == eRegexFiltersColumn::Value )
        {
            isExpanded(index) ? collapse(index) : expand(index);
        }
    });

    auto showContextMenu = [this](const QPoint &pos)
    {
        QMenu contextMenu("Context menu", this);

        {
            QAction* pAction = new QAction("Filter variables", this);
            connect(pAction, &QAction::triggered, [](bool checked)
            {
                CSettingsManager::getInstance()->setFilterVariables(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(CSettingsManager::getInstance()->getFilterVariables());
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Switch to regex edit", this);
            pAction->setShortcut(QKeySequence(tr("Enter")));
            connect(pAction, &QAction::triggered, [this]()
            {
                returnPressed();
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("Visible columns", this);

            {
                const auto& regexFiltersColumnsVisibilityMap =
                        CSettingsManager::getInstance()->getRegexFiltersColumnsVisibilityMap();

                for( int i = static_cast<int>(eRegexFiltersColumn::Value);
                     i < static_cast<int>(eRegexFiltersColumn::AfterLastVisible);
                     ++i)
                {
                    auto foundItem = regexFiltersColumnsVisibilityMap.find(static_cast<eRegexFiltersColumn>(i));

                    if(foundItem != regexFiltersColumnsVisibilityMap.end())
                    {
                        QAction* pAction = new QAction(getName(static_cast<eRegexFiltersColumn>(i)), this);
                        connect(pAction, &QAction::triggered, [i](bool checked)
                        {
                            auto regexFiltersColumnsVisibilityMap_ =
                                    CSettingsManager::getInstance()->getRegexFiltersColumnsVisibilityMap();

                            auto foundItem_ = regexFiltersColumnsVisibilityMap_.find(static_cast<eRegexFiltersColumn>(i));

                            if(foundItem_ != regexFiltersColumnsVisibilityMap_.end()) // if item is in the map
                            {
                                foundItem_.value() = checked; // let's update visibility value
                                CSettingsManager::getInstance()->setRegexFiltersColumnsVisibilityMap(regexFiltersColumnsVisibilityMap_);
                            }
                        });
                        pAction->setCheckable(true);
                        pAction->setChecked(foundItem.value());
                        pSubMenu->addAction(pAction);
                    }
                }
            }

            contextMenu.addMenu(pSubMenu);
        }

        contextMenu.exec(mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );

    connect( CSettingsManager::getInstance().get(),
             &CSettingsManager::regexFiltersColumnsVisibilityMapChanged,
             [this](const tRegexFiltersColumnsVisibilityMap&)
    {
        updateColumnsVisibility();
        updateWidth();
    });

    setItemDelegate(mpRepresentationDelegate);
}

void CFiltersView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if(true == current.isValid())
    {
        auto pTreeItem = static_cast<CTreeItem*>(current.internalPointer());

        if(nullptr != pTreeItem)
        {
            regexRangeSelectionRequested( pTreeItem->data(static_cast<int>(eRegexFiltersColumn::Range)).get<tRange>() );
        }
    }

    tParent::currentChanged(current, previous);
}

CFiltersView::~CFiltersView()
{
    if(nullptr != mpRepresentationDelegate)
    {
        delete mpRepresentationDelegate;
        mpRepresentationDelegate = nullptr;
    }
}

void CFiltersView::setModel(QAbstractItemModel *model)
{
    tParent::setModel(model);
    updateColumnsVisibility();
}

void CFiltersView::updateColumnsVisibility()
{
    const tRegexFiltersColumnsVisibilityMap& visibilityMap = CSettingsManager::getInstance()->getRegexFiltersColumnsVisibilityMap();

    for( int i = static_cast<int>(eRegexFiltersColumn::Value);
         i < static_cast<int>(eRegexFiltersColumn::Last);
         ++i)
    {
        if(i >= static_cast<int>(eRegexFiltersColumn::AfterLastVisible))
        {
            hideColumn(i);
        }
        else
        {
            auto foundColumn = visibilityMap.find(static_cast<eRegexFiltersColumn>(i));

            if( foundColumn != visibilityMap.end() )
            {
                if(true == foundColumn.value()) // if column is visible
                {
                    showColumn(i);
                }
                else
                {
                    hideColumn(i);
                }
            }
            else
            {
                 hideColumn(static_cast<int>(i));
            }
        }
    }
}

void CFiltersView::highlightInvalidRegex(const QModelIndex &index)
{
    if(true == index.isValid() && nullptr != mpRepresentationDelegate)
    {
        QVector<QModelIndex> animatedRows;
        animatedRows.push_back(index);
        mpRepresentationDelegate->animateRows(animatedRows, QColor(255,0,0), 300);
    }
}

void CFiltersView::setSpecificModel( CFiltersModel* pModel )
{
    if(nullptr != pModel)
    {
        mpModel = pModel;

        connect( mpModel, &CFiltersModel::filteredEntriesChanged, this,
                 [this](const CFiltersModel::tFilteredEntryVec& filteredEntries,
                 bool expandVisible)
        {
            mbResizeOnExpandCollapse = false;

            //QTime timer;
            //timer.start();

            //SEND_MSG(QString("~0 [CFiltersView][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(timer.elapsed()));

            if(false == expandVisible)
            {
                collapseAll();
            }

            //SEND_MSG(QString("~1 [CFiltersView][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(timer.elapsed()));

            for(const auto& filteredEntry : filteredEntries)
            {
                setRowHidden(filteredEntry.row, filteredEntry.parentIdx, filteredEntry.filtered);

                if(true == expandVisible && false == filteredEntry.filtered)
                {
                    setExpanded(filteredEntry.parentIdx, true);
                    //SEND_MSG(QString("~2 [CFiltersView][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(timer.elapsed()));
                }
            }

            //SEND_MSG(QString("~3 [CFiltersView][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(timer.elapsed()));

            mbResizeOnExpandCollapse = true;

            if(false == mbSkipFirstUpdateWidth)
            {
                updateWidth();
            }
            else
            {
                mbSkipFirstUpdateWidth = false;
            }
        }, Qt::AutoConnection);
    }

    setModel(pModel);
}

void CFiltersView::updateWidth()
{
    for(int i = static_cast<int>(eRegexFiltersColumn::Value);
        i < static_cast<int>(eRegexFiltersColumn::AfterLastVisible);
        ++i)
    {
        resizeColumnToContents(i);
    }
}

void CFiltersView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    tParent::dataChanged(topLeft,bottomRight,roles);

    auto pVerticalScrollBar = verticalScrollBar();

    auto isVerticalScrollBarVisible_ = pVerticalScrollBar->isVisible();

    {
        if(0 == model()->rowCount())
        {
            mWidthUpdateRequired = eUpdateRequired::eUpdateRequired_BE_READY;
        }

        if(0 < model()->rowCount())
        {
            if(eUpdateRequired::eUpdateRequired_BE_READY == mWidthUpdateRequired)
            {
                mWidthUpdateRequired = eUpdateRequired::eUpdateRequired_REQUIRED;
            }
        }
    }

    bool bUpdated = false;

    if(false == bUpdated && eUpdateRequired::eUpdateRequired_REQUIRED == mWidthUpdateRequired)
    {
        updateWidth();
        mWidthUpdateRequired = eUpdateRequired::eUpdateRequired_NO;
        bUpdated = true;
    }

    if(false == bUpdated && mbIsVerticalScrollBarVisible != isVerticalScrollBarVisible_)
    {
        updateWidth();
        mbIsVerticalScrollBarVisible = isVerticalScrollBarVisible_;
        bUpdated = true;
    }
}

void CFiltersView::keyPressEvent ( QKeyEvent * event )
{
    if(event->key() == Qt::Key::Key_Enter)
    {
        returnPressed();
    }
    else
    {
        tParent::keyPressEvent(event);
    }
}
