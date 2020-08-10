#include "assert.h"

#include "stack"

#include "QDebug"
#include "QVector"
#include "QApplication"
#include "QDateTime"
#include "QRegularExpression"

#include "../settings/CSettingsManager.hpp"
#include "CFiltersModel.hpp"
#include "../log/CConsoleCtrl.hpp"

CFiltersModel::CFiltersModel(QObject *parent)
    : QAbstractItemModel(parent),
      mRegex(),
      mSortingColumn(eRegexFiltersColumn::Index),
      mSortOrder(Qt::SortOrder::AscendingOrder),
      mSortingHandler(),
      mFilter()
{
    mSortingHandler = [](const QVector<tTreeItemPtr>& children,
                            const int& sortingColumn,
                            Qt::SortOrder sortingOrder) -> QVector<tTreeItemPtr>
    {
        tTreeItem::tChildrenVector result;

        switch(static_cast<eRegexFiltersColumn>(sortingColumn))
        {
            case eRegexFiltersColumn::ItemType:
            case eRegexFiltersColumn::Value:
            {
                struct tComparator
                {
                    QString val;
                    bool operator< (const tComparator& rVal) const
                    {
                        return val.compare( rVal.val, Qt::CaseInsensitive ) < 0;
                    }
                };

                QMultiMap<tComparator, tTreeItemPtr> sortedChildrenMap;

                for(const auto& pChild : children)
                {
                    if(nullptr != pChild)
                    {
                        const QString& subString = pChild->data(sortingColumn).get<QString>();
                        tComparator comparator;
                        comparator.val = subString;
                        sortedChildrenMap.insert(comparator, pChild);
                    }
                }

                for(const auto& pChild : sortedChildrenMap)
                {
                    if(nullptr != pChild)
                    {
                        switch(sortingOrder)
                        {
                            case Qt::SortOrder::AscendingOrder:
                            {
                                result.push_back(pChild);
                            }
                                break;
                            case Qt::SortOrder::DescendingOrder:
                            {
                                result.push_front(pChild);
                            }
                                break;
                        }
                    }
                }
            }
                break;
            case eRegexFiltersColumn::Index:
            {
                QMultiMap<int, tTreeItemPtr> sortedChildrenMap;

                for(const auto& pChild : children)
                {
                    if(nullptr != pChild)
                    {
                        int val = pChild->data(static_cast<int>(sortingColumn)).get<int>();
                        sortedChildrenMap.insert(val, pChild);
                    }
                }

                for(const auto& pChild : sortedChildrenMap)
                {
                    if(nullptr != pChild)
                    {
                        switch(sortingOrder)
                        {
                            case Qt::SortOrder::AscendingOrder:
                            {
                                result.push_back(pChild);
                            }
                                break;
                            case Qt::SortOrder::DescendingOrder:
                            {
                                result.push_front(pChild);
                            }
                                break;
                        }
                    }
                }
            }
            break;
            default:
                break;
        }

        return result;
    };

    connect(CSettingsManager::getInstance().get(), &CSettingsManager::filterVariablesChanged,
    [this](bool)
    {
        filterRegexTokensInternal();
    });

    mpRootItem = new tTreeItem(nullptr, static_cast<int>(mSortingColumn),
                               mSortingHandler,
                               CTreeItem::tHandleDuplicateFunc(),
                               CTreeItem::tFindItemFunc());
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Value) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Index) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::ItemType) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::AfterLastVisible) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Color) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Range) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::RowType) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::IsFiltered) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::GroupName) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::GroupSyntaxType) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Last) );
}

CFiltersModel::~CFiltersModel()
{
    if(nullptr != mpRootItem)
    {
        delete mpRootItem;
        mpRootItem = nullptr;
    }
}

QModelIndex CFiltersModel::rootIndex() const
{
    return QModelIndex();
}

void CFiltersModel::updateView()
{
    emit dataChanged( index(0,0), index ( rowCount(), columnCount()) );
    emit layoutChanged();
}

QModelIndex CFiltersModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    tTreeItemPtr pParentItem;

    if (parent == rootIndex())
        pParentItem = mpRootItem;
    else
        pParentItem = static_cast<tTreeItemPtr>(parent.internalPointer());

    tTreeItem *childItem = pParentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex CFiltersModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    tTreeItem *childItem = static_cast<tTreeItemPtr>(index.internalPointer());
    tTreeItem *parentItem = childItem->getParent();

    if (parentItem == mpRootItem)
        return rootIndex();
    else
        return createIndex(parentItem->row(), 0, parentItem);
}

int CFiltersModel::rowCount(const QModelIndex &parent) const
{
     tTreeItem *parentItem;

    if (parent == rootIndex())
        parentItem = mpRootItem;
    else
        parentItem = static_cast<tTreeItemPtr>(parent.internalPointer());

    parentItem->sort(static_cast<int>(mSortingColumn), mSortOrder, false);

    return parentItem->childCount();
}

int CFiltersModel::columnCount(const QModelIndex &) const
{
    return static_cast<int>(eRegexFiltersColumn::Last);
}

QVariant CFiltersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QVariant result;

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        tTreeItem *pItem = static_cast<tTreeItemPtr>(index.internalPointer());
        result = toQVariant(pItem->data(index.column()));
    }
    else if (role == Qt::TextAlignmentRole)
    {
        if( static_cast<int>(eRegexFiltersColumn::Index) == index.column() )
        {
            result = Qt::AlignCenter;
        }
        else if( static_cast<int>(eRegexFiltersColumn::ItemType) == index.column() ||
                 static_cast<int>(eRegexFiltersColumn::Value) == index.column() )
        {
            result = Qt::AlignLeft;
        }
    }

    return result;
}

bool CFiltersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(false == index.isValid())
    {
        return false;
    }

    bool bResult = false;

    if(role == Qt::EditRole)
    {
        auto pItem = static_cast<CTreeItem*>(index.internalPointer());
        const auto currentValue = pItem->data(index.column());
        auto newValue = toRegexDataItem(value, static_cast<eRegexFiltersColumn>(index.column()));

        if(currentValue != newValue)
        {
            bResult = pItem->setColumnData(newValue, index.column());
        }

        if( true == bResult )
        {
            auto packRes = packRegex();

            bResult = packRes.first;

            if(false == bResult)
            {
                bResult = pItem->setColumnData(currentValue, index.column());
                QString error;
                error.append("Regex update is ignored due to the following error: \"").append(packRes.second).append("\"");
                regexUpdatedByUserInvalid( index, error );
            }
        }
    }

    return bResult;
}

QPair<bool,QString> CFiltersModel::packRegex()
{
    QPair<bool,QString> result;
    result.first = false;

    if(nullptr != mpRootItem)
    {
        QString regexStr;

        auto preVisitFunction = [&regexStr](tTreeItem* pItem)
        {
            switch( pItem->data(static_cast<int>(eRegexFiltersColumn::RowType)).get<eRegexFiltersRowType>() )
            {
                case eRegexFiltersRowType::Text:
                {
                    regexStr.append( pItem->data(static_cast<int>(eRegexFiltersColumn::Value)).get<QString>() );
                }
                    break;
                case eRegexFiltersRowType::VarGroup:
                case eRegexFiltersRowType::NonVarGroup:
                {
                    const auto& groupName = pItem->data(static_cast<int>(eRegexFiltersColumn::GroupName)).get<QString>();

                    regexStr.append("(");

                    if(false == groupName.isEmpty())
                    {
                        const eGroupSyntaxType& groupSyntaxType = static_cast<eGroupSyntaxType>(pItem->data(static_cast<int>(eRegexFiltersColumn::GroupSyntaxType)).get<int>());

                        switch(groupSyntaxType)
                        {
                        case eGroupSyntaxType::SYNTAX_1:
                            regexStr.append("?<");
                            break;
                        case eGroupSyntaxType::SYNTAX_2:
                            regexStr.append("?'");
                            break;
                        case eGroupSyntaxType::SYNTAX_3:
                            regexStr.append("?P<");
                            break;
                        }

                        regexStr.append(groupName);

                        switch(groupSyntaxType)
                        {
                        case eGroupSyntaxType::SYNTAX_1:
                        case eGroupSyntaxType::SYNTAX_3:
                            regexStr.append(">");
                            break;
                        case eGroupSyntaxType::SYNTAX_2:
                            regexStr.append("'");
                            break;
                        }
                    }
                }
                    break;
                case eRegexFiltersRowType::NonCapturingGroup:
                {
                    regexStr.append("(?>");
                }
                    break;
            }

            return true;
        };

        auto postVisitFunction = [&regexStr](tTreeItem* pItem)
        {
            switch( pItem->data(static_cast<int>(eRegexFiltersColumn::RowType)).get<eRegexFiltersRowType>() )
            {
                case eRegexFiltersRowType::Text:{} break;
                case eRegexFiltersRowType::NonCapturingGroup:
                case eRegexFiltersRowType::NonVarGroup:
                case eRegexFiltersRowType::VarGroup:
                {
                    regexStr.append(")");
                }
            }

            return true;
        };

        mpRootItem->visit(preVisitFunction, postVisitFunction, false, true, false);

        SEND_MSG(regexStr);

        QRegularExpression regex( QString("(?J)").append( regexStr ) );

        result.first = regex.isValid();

        if(true == result.first)
        {
            regexUpdatedByUser(regexStr);
        }
        else
        {
            result.second = regex.errorString();
        }
    }

    return result;
}

Qt::ItemFlags CFiltersModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags result = QAbstractItemModel::flags(index);

    if(index.column() == static_cast<int>(eRegexFiltersColumn::Value) &&
       index.sibling(index.row(), static_cast<int>(eRegexFiltersColumn::RowType)).data().value<eRegexFiltersRowType>() == eRegexFiltersRowType::Text)
    {
        result |= Qt::ItemIsEditable;
    }

    return result;
}

QVariant CFiltersModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return toQVariant(mpRootItem->data(section));

    return QVariant();
}

void CFiltersModel::resetData()
{
    beginResetModel();
    if(mpRootItem)
        delete mpRootItem;
    mpRootItem = new tTreeItem(nullptr,
                               static_cast<int>(mSortingColumn),
                               mSortingHandler,
                               CTreeItem::tHandleDuplicateFunc(),
                               CTreeItem::tFindItemFunc());
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Value) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Index) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::ItemType) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::AfterLastVisible) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Color) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Range) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::RowType) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::IsFiltered) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::GroupName) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::GroupSyntaxType) );
    mpRootItem->appendColumn( getName(eRegexFiltersColumn::Last) );

    endResetModel();
    updateView();
}

enum class eGroupNameAnalysisState
{
    NotAnalyzed,
    AnalyzedFound,
    AnalyzedNotFound
};

enum class eGroupNonCapturingAnalysisState
{
    NotAnalyzed,
    AnalyzedFound,
    AnalyzedNotFound
};

struct tParsingDataItem
{
    tTreeItemPtr pTreeItem = nullptr;

    // data fields
    int index;
    QString name;
    QString value;
    tColorWrapper colorWrapper;
    tRange range;
    eRegexFiltersRowType rowType = eRegexFiltersRowType::Text;
    bool isFiltered = false;
    QString groupName;
    eGroupSyntaxType groupSyntaxType;

    // internal fields
    bool isRoot = false;
    eGroupNameAnalysisState groupNameAnalysisState = eGroupNameAnalysisState::NotAnalyzed;
    eGroupNonCapturingAnalysisState groupNonCapturingAnalysisState = eGroupNonCapturingAnalysisState::NotAnalyzed;
};

void CFiltersModel::setUsedRegex(const QString& regexStr)
{
    QString regex_ = QString("(?J)").append(regexStr);

    typedef std::stack<tParsingDataItem> tParsingStateStack;

    // we should reset a group
    resetData();

    //we need to parse regex into a set of groups here.
    if(nullptr != mpRootItem)
    {
        QRegularExpression regex(regex_);

        if(true == regex.isValid()) // let's use more deep functionality in order to check correctness of passed regex
        {
            tParsingStateStack parsingStateStack;  // stack of parsing states
            tParsingDataItem rootDataItem;
            rootDataItem.pTreeItem = mpRootItem;
            rootDataItem.isRoot = true;
            parsingStateStack.push(rootDataItem); // we should have only this item on the stack at the end of the parsing

            tParsingDataItem* pCurrentParsingDataItem = &parsingStateStack.top();

            mRegex = regexStr;
            const auto stringSize = regexStr.size();

            int symbolCounter = 0;
            int toRange = 0;
            int indexCounter = 0;

            auto pushParsingItem = [&indexCounter, &pCurrentParsingDataItem, &symbolCounter, &parsingStateStack]( eRegexFiltersRowType rowType )
            {
                // let's create a parsing item and fill as many fields as possible
                tParsingDataItem newParsingDataItem;
                newParsingDataItem.index = indexCounter;
                ++indexCounter;
                newParsingDataItem.rowType = rowType;
                newParsingDataItem.isRoot = false;
                newParsingDataItem.range.from = symbolCounter; // range start
                // create new tree node. As of now with empty data. It will be filled in afterwards.
                newParsingDataItem.pTreeItem = pCurrentParsingDataItem->pTreeItem->appendChild( symbolCounter, CTreeItem::tData() );
                parsingStateStack.push(newParsingDataItem);

                //SEND_MSG(QString("[CFiltersModel][%1] Parent |%2| assigned to child |%3|")
                //         .arg(__FUNCTION__)
                //         .arg(pCurrentParsingDataItem->index)
                //         .arg(newParsingDataItem.index));

                pCurrentParsingDataItem = &parsingStateStack.top();
            };

            auto popParsingItem = [this, &pCurrentParsingDataItem, &symbolCounter, &parsingStateStack, &indexCounter, &toRange]()
            {
                assert(parsingStateStack.size() > 1);

                // let's update missing fields and pop item out of the stack, as work with it is finished.
                pCurrentParsingDataItem->range.to = toRange;
                if(pCurrentParsingDataItem->rowType != eRegexFiltersRowType::Text)
                {
                    // expand range to cover "()" signs
                    pCurrentParsingDataItem->range.from -= 1;
                    pCurrentParsingDataItem->range.to += 1;
                }

                if( true == pCurrentParsingDataItem->name.isEmpty() )
                {
                    switch( pCurrentParsingDataItem->rowType )
                    {
                        case eRegexFiltersRowType::Text:
                        {
                            pCurrentParsingDataItem->name = QString("Text");
                        }
                            break;
                        case eRegexFiltersRowType::VarGroup:
                        {
                            pCurrentParsingDataItem->name = QString("VarGroup");
                        }
                            break;
                        case eRegexFiltersRowType::NonVarGroup:
                        {
                            pCurrentParsingDataItem->name = QString("Group");
                        }
                            break;
                        case eRegexFiltersRowType::NonCapturingGroup:
                        {
                            pCurrentParsingDataItem->name = QString("NonCapturingGroup");
                        }
                            break;
                    }
                }

                // check for empty group. Implicitly add empty text to it
                if(pCurrentParsingDataItem->rowType != eRegexFiltersRowType::Text)
                {
                    if(pCurrentParsingDataItem->pTreeItem->childCount() == 0)
                    {
                        CTreeItem::tData data;

                        data.push_back(tDataItem(QString()));
                        data.push_back(tDataItem(indexCounter++));
                        data.push_back(tDataItem(QString("Text")));
                        data.push_back(tDataItem(QString()));
                        data.push_back(tDataItem(tColorWrapper()));
                        data.push_back(tDataItem(pCurrentParsingDataItem->range));
                        data.push_back(tDataItem(eRegexFiltersRowType::Text));
                        data.push_back(tDataItem(false));
                        data.push_back(tDataItem(QString()));
                        data.push_back(tDataItem(0));

                        pCurrentParsingDataItem->pTreeItem->appendChild( symbolCounter, data );
                    }
                }

                // fill in the tree item

                auto pParent = pCurrentParsingDataItem->pTreeItem->getParent();

                if(nullptr != pParent)
                {
                    CTreeItem::tData data;

                    data.push_back(tDataItem(pCurrentParsingDataItem->value));
                    data.push_back(tDataItem(pCurrentParsingDataItem->index));
                    data.push_back(tDataItem(pCurrentParsingDataItem->name));
                    data.push_back(tDataItem(QString()));
                    data.push_back(tDataItem(pCurrentParsingDataItem->colorWrapper));
                    data.push_back(tDataItem(pCurrentParsingDataItem->range));
                    data.push_back(tDataItem(pCurrentParsingDataItem->rowType));
                    data.push_back(tDataItem(false));
                    data.push_back(tDataItem(pCurrentParsingDataItem->groupName));
                    data.push_back(tDataItem(static_cast<int>(pCurrentParsingDataItem->groupSyntaxType)));

                    pCurrentParsingDataItem->pTreeItem->setData(data);

                    pParent->sort(static_cast<int>(mSortingColumn), mSortOrder, false);
                    pCurrentParsingDataItem->pTreeItem->sort(static_cast<int>(mSortingColumn), mSortOrder, true);

                    auto parentIdx = createIndexInternal(pParent->getIdx(), 0, pParent);

                    beginInsertRows(parentIdx, 0, pCurrentParsingDataItem->pTreeItem->getIdx());

                    //SEND_MSG(QString("[CFiltersModel][%1] Finish processing of item |%2|%3|%4|")
                    //         .arg(__FUNCTION__)
                    //         .arg(pCurrentParsingDataItem->index)
                    //         .arg(pCurrentParsingDataItem->name)
                    //         .arg(pCurrentParsingDataItem->value));

                    parsingStateStack.pop();
                    pCurrentParsingDataItem = &parsingStateStack.top();

                    endInsertRows();
                }
            };

            auto checkGroup = [&regexStr, &symbolCounter, &toRange](bool checkLeavingGroup) -> bool
            {
                bool bResult = false;

                QChar checkChar = false == checkLeavingGroup ? '(' : ')';

                if(0 == symbolCounter && false == regexStr.isEmpty())
                {
                    bResult = regexStr[symbolCounter] == checkChar;
                }
                else if(symbolCounter >= 2)
                {
                    bResult = ( regexStr[symbolCounter - 2] == '\\'
                            && regexStr[symbolCounter - 1] == '\\'
                            && regexStr[symbolCounter] == checkChar )
                            ||
                            ( regexStr[symbolCounter - 1] != '\\'
                              && regexStr[symbolCounter] == checkChar );
                }
                else if(symbolCounter == 1)
                {
                    bResult = ( regexStr[symbolCounter - 1] != '\\' && regexStr[symbolCounter] == checkChar ) ;
                }

                if(true == bResult)
                {
                    toRange = symbolCounter;
                    ++symbolCounter;
                }

                return bResult;
            };

            auto checkNonCapturingGroup = [&regexStr, &symbolCounter]() -> bool
            {
                bool bResult = false;

                // if there are at least 2 symbols available - symbolCounter & symbolCounter + 1 - we can check whether group is non-captirung
                if( (symbolCounter + 1) < regexStr.size())
                {
                    bResult = ( regexStr[symbolCounter] == '?'
                            && regexStr[symbolCounter + 1] == '>' );

                    if(true == bResult) // in case if non-capturing group was found
                    {
                        // we should skip 2 symbols.
                        symbolCounter += 2;
                    }
                }

                return bResult;
            };

            auto checkGroupName = [&regexStr, &symbolCounter, &pCurrentParsingDataItem]()
            {
                // we will use this one to:
                // - check availability of the name
                // - if name exist - get the whole name, increasing the symbolCounter locally
                // - if name exist - we will parse it in order to identify var and color
                // - if var and color exist - we will assign them to the current parsing data item
                // When this method is called cursor should be one symbol AFTER the '(' character. We can validate this to be on the safe side
                if( symbolCounter > 0                   // if we have analyzed at least 1 symbol
                 && regexStr.size() > 1                 // and size of provided regex is bigger than 1 symbol
                 && regexStr[symbolCounter-1] == '(')  // and previous analyzed symbol is '('
                {
                    // we can move on with checking the '?' '<' sequence
                    // we should have at least one more symbol after cursor's position to succeed.
                    if( (symbolCounter + 2) < regexStr.size() )
                    {
                        QChar groupLeavingChar;
                        bool bEntranceToGroupNameFound = false;

                        if( regexStr[symbolCounter] == '?'
                         && regexStr[symbolCounter+1] == '<' )
                        {
                            pCurrentParsingDataItem->groupSyntaxType = eGroupSyntaxType::SYNTAX_1;
                            bEntranceToGroupNameFound = true;
                            groupLeavingChar = '>';

                            // we have a group name. Need to step over "group entering" entering characters
                            symbolCounter += 2;
                        }

                        if( regexStr[symbolCounter] == '?'
                         && regexStr[symbolCounter+1] == '\'' )
                        {
                            pCurrentParsingDataItem->groupSyntaxType = eGroupSyntaxType::SYNTAX_2;
                            bEntranceToGroupNameFound = true;
                            groupLeavingChar = '\'';

                            // we have a group name. Need to step over "group entering" entering characters
                            symbolCounter += 2;
                        }

                        if( regexStr[symbolCounter] == '?'
                         && regexStr[symbolCounter+1] == 'P'
                         && regexStr[symbolCounter+2] == '<' )
                        {
                            pCurrentParsingDataItem->groupSyntaxType = eGroupSyntaxType::SYNTAX_3;
                            bEntranceToGroupNameFound = true;
                            groupLeavingChar = '>';

                            // we have a group name. Need to step over "group entering" entering characters
                            symbolCounter += 2;
                        }


                        if( true == bEntranceToGroupNameFound )
                        {
                            bool groupNameEndFound = false;
                            QString groupName;
                            groupName.reserve(100);

                            if(symbolCounter < regexStr.size()) // if something left to be parsed
                            {
                                while( symbolCounter < regexStr.size() ) // let's fetch the group name out of the string
                                {
                                    const QChar& currentSymbol = regexStr[symbolCounter];

                                    if(currentSymbol == groupLeavingChar)
                                    {
                                        // we've found end of the group name
                                        ++symbolCounter;
                                        groupNameEndFound = true;
                                        break;           // break the loop, as further iteration is not needed
                                    }
                                    else
                                    {
                                        groupName.append(currentSymbol);
                                        ++symbolCounter;
                                    }
                                }

                                if(true == groupNameEndFound) // if we've found ending of group name without reaching the end of the regex
                                {
                                    // let's store group name
                                    pCurrentParsingDataItem->groupName = groupName;

                                    // let's parse the name and update the attribute of the analysis item
                                    auto pRegexMetadataItem = parseRegexGroupName(groupName, false);

                                    if(nullptr != pRegexMetadataItem)
                                    {
                                        if(true == pRegexMetadataItem->varName.first)
                                        {
                                            pCurrentParsingDataItem->rowType = eRegexFiltersRowType::VarGroup;
                                            pCurrentParsingDataItem->value = pRegexMetadataItem->varName.second;
                                        }
                                        else
                                        {
                                            pCurrentParsingDataItem->name = "Group";
                                            pCurrentParsingDataItem->value = pCurrentParsingDataItem->groupName;
                                        }

                                        pCurrentParsingDataItem->colorWrapper.optColor = pRegexMetadataItem->highlightingColor;
                                    }
                                }
                            }
                            else // we do not have a group name
                            {
                                pCurrentParsingDataItem->groupNameAnalysisState = eGroupNameAnalysisState::AnalyzedNotFound;
                            }

                        }
                        else // we do not have a group name
                        {
                            pCurrentParsingDataItem->groupNameAnalysisState = eGroupNameAnalysisState::AnalyzedNotFound;
                        }
                    }
                    else // group does not have a name
                    {
                        pCurrentParsingDataItem->groupNameAnalysisState = eGroupNameAnalysisState::AnalyzedNotFound;
                    }
                }
                else // group does not have a name
                {
                    pCurrentParsingDataItem->groupNameAnalysisState = eGroupNameAnalysisState::AnalyzedNotFound;
                }
            };

            for( ; symbolCounter < stringSize; ) // let's parse the regex manually searching for the groups
            {
                const QChar& symbol = regexStr[symbolCounter];

                //   (       - in the group                         - covered
                //   \(      - ignore                               - covered
                //   \\(     - in the group                         - covered
                //   (?>     - in the non capturing group           - covered
                //   ?<name> or ?'name' or ?P<name> - name          - next
                //   )       - out of the group                     - covered
                //   \)      - ignore                               - covered
                //   \\)     - out of the group                     - covered
                //   ?< after ( or \\( - start of regex name        - next
                //   > after ?< - end of regex name                 - next

                if( true == pCurrentParsingDataItem->isRoot ) // we are at the top of the analysis stack. Here we are searching for entering the group or for a text.
                {
                    if(true == checkGroup( false )) // we are entering the group
                    {
                        pushParsingItem(eRegexFiltersRowType::NonVarGroup); // add non var group
                    }
                    else // we've found a text
                    {
                        pushParsingItem(eRegexFiltersRowType::Text); // add text
                        pCurrentParsingDataItem->value.append(symbol); // append character to a value
                        ++symbolCounter;
                    }
                }
                else
                {
                    switch(pCurrentParsingDataItem->rowType)
                    {
                        case eRegexFiltersRowType::Text:
                        {
                            // here we can found entering a group or another text
                            // if group is found - it becomes a sibling of current analysis item.
                            // so we should first pop current analysis item and append new child to its parent.
                            if(true == checkGroup( false )) // we are entering the group
                            {
                                popParsingItem();
                                pushParsingItem(eRegexFiltersRowType::NonVarGroup); // add non var group
                            }
                            else if(true == checkGroup(true)) // also inside the text we can find leaving the parent group.
                            {
                                // in such case we should pop-out 2 items out of the analysis stack - text and group.
                                popParsingItem();
                                popParsingItem();
                            }
                            else // we've found another text char
                            {
                                pCurrentParsingDataItem->value.append(symbol); // append character to a value
                                ++symbolCounter;
                            }
                        }
                            break;
                        case eRegexFiltersRowType::VarGroup:
                        case eRegexFiltersRowType::NonCapturingGroup:
                        {
                            // As in this case group's name was already analyzed and parsed, we are searching only for:
                            // - entering sub-group
                            // - leaving the group
                            // - text

                            if(true == checkGroup( false )) // we are entering the sub-group
                            {
                                pushParsingItem(eRegexFiltersRowType::NonVarGroup); // add non var group
                            }
                            else if(true == checkGroup( true )) // we are leaving the group
                            {
                                popParsingItem();
                            }
                            else // we've found a text
                            {
                                pushParsingItem(eRegexFiltersRowType::Text); // add text
                                pCurrentParsingDataItem->value.append(symbol); // append character to a value
                                ++symbolCounter;
                            }
                        }
                            break;
                        case eRegexFiltersRowType::NonVarGroup:
                        {
                            // Hardest case
                            // we are searching for:
                            // - non-capturing sign
                            // - group name
                            // - end of group name, if we are inside a group name
                            // - in case if we've reached end of the group name - we should parse it
                            // - entering sub-group
                            // - text
                            // - leaving the group
                            // If group name analysis finished - we are parsing the name in order to find out VarName and color.

                            if( eGroupNonCapturingAnalysisState::NotAnalyzed == pCurrentParsingDataItem->groupNonCapturingAnalysisState )
                            {
                                // first of all, let's see whether it is a non-capturing group or not.
                                if(true == checkNonCapturingGroup())
                                {
                                    // In such case we need to update the group type
                                    pCurrentParsingDataItem->rowType = eRegexFiltersRowType::NonCapturingGroup;
                                    pCurrentParsingDataItem->groupNonCapturingAnalysisState = eGroupNonCapturingAnalysisState::AnalyzedFound;
                                }
                                else
                                {
                                    pCurrentParsingDataItem->groupNonCapturingAnalysisState = eGroupNonCapturingAnalysisState::AnalyzedNotFound;
                                }
                            }
                            else
                            {
                                // at this point we know that we are not inside the non-capturing group.
                                // we are inside the capturing group.
                                // We should check the group name if it was not checked yet and parse it

                                if(pCurrentParsingDataItem->groupNameAnalysisState == eGroupNameAnalysisState::NotAnalyzed)
                                {
                                    checkGroupName();
                                }
                                else
                                {
                                    // at this section analysis is similar to the other parts of groups
                                    if(true == checkGroup( false )) // we are entering the sub-group
                                    {
                                        pushParsingItem(eRegexFiltersRowType::NonVarGroup); // add non var group
                                    }
                                    else if(true == checkGroup( true )) // we are leaving the group
                                    {
                                        popParsingItem();
                                    }
                                    else // we've found a text
                                    {
                                        pushParsingItem(eRegexFiltersRowType::Text); // add text
                                        pCurrentParsingDataItem->value.append(symbol); // append character to a value
                                        ++symbolCounter;
                                    }
                                }

                            }
                        }
                            break;
                    }
                }
            }

            // Analysis is over. Let's finalize filling in the tree items.
            while(parsingStateStack.size() > 1)
            {
                popParsingItem();
            }

            mpRootItem->sort(static_cast<int>(mSortingColumn), mSortOrder, true);

            updateView();

            filterRegexTokensInternal();
        }
    }
}

void CFiltersModel::sort(int column, Qt::SortOrder order)
{
    mSortingColumn = toRegexFiltersColumn(column);
    mSortOrder = order;

    if(nullptr != mpRootItem)
    {
        mpRootItem->sort(static_cast<int>(mSortingColumn), mSortOrder, true);
        filterRegexTokensInternal();
    }

    updateView();
}

QModelIndex CFiltersModel::createIndexInternal(int arow, int acolumn, void *adata) const
{
    QModelIndex result;

    // root node's index should be invalid
    if(reinterpret_cast<CTreeItem*>(adata) != mpRootItem)
    {
        result = createIndex(arow, acolumn, adata);
    }

    return result;
}

void CFiltersModel::filterRegexTokens( const QString& filter )
{
    mFilter = filter;
    filterRegexTokensInternal();
}

void CFiltersModel::filterRegexTokensInternal()
{
    //QElapsedTimer time;

    //time.start();

    bool bFilterVariables = CSettingsManager::getInstance()->getFilterVariables();

    if(nullptr != mpRootItem)
    {
        //SEND_MSG(QString("~0 [CFiltersModel][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(time.elapsed()));

        QVector<CTreeItem*> visibleElements;

        {
            QRegularExpression regex = QRegularExpression(mFilter, QRegularExpression::CaseInsensitiveOption);

            auto preVisitFunction = [this, &visibleElements, &bFilterVariables, &regex](tTreeItem* pItem)
            {
                if(nullptr != pItem)
                {
                    bool bFiltered = false;

                    if(true == bFilterVariables)
                    {
                        auto rowType = pItem->data(static_cast<int>(eRegexFiltersColumn::RowType)).get<eRegexFiltersRowType>();

                        bFiltered = rowType != eRegexFiltersRowType::VarGroup;
                    }

                    if(false == bFiltered)
                    {
                        if(false == mFilter.isEmpty())
                        {
                            if(true == regex.isValid())
                            {
                                const QString& checkString = pItem->data(static_cast<int>(eRegexFiltersColumn::Value)).get<QString>();

                                QRegularExpressionMatch match = regex.match(checkString);

                                bFiltered = !match.hasMatch();

                                pItem->getWriteableData(static_cast<int>(eRegexFiltersColumn::IsFiltered)) = bFiltered;

                                if(false == bFiltered)
                                {
                                    visibleElements.push_back(pItem);
                                }
                            }
                        }
                    }

                    pItem->getWriteableData(static_cast<int>(eRegexFiltersColumn::IsFiltered)) = bFiltered;

                    if(false == bFiltered)
                    {
                        visibleElements.push_back(pItem);
                    }
                }

                return true;
            };

            mpRootItem->visit(preVisitFunction, CTreeItem::tVisitFunction(), false);
        }

        {
            auto preVisitFunction = [this, &visibleElements](tTreeItem* pItem)
            {
                auto pParent = pItem->getParent();

                if(nullptr != pParent && pParent != mpRootItem)
                {
                    bool bIsParentFiltered = pParent->getWriteableData(static_cast<int>(eRegexFiltersColumn::IsFiltered)).get<bool>();
                    bool bIsItemFiltered = pItem->getWriteableData(static_cast<int>(eRegexFiltersColumn::IsFiltered)).get<bool>();

                    if(false == bIsParentFiltered && bIsItemFiltered != bIsParentFiltered)
                    {
                        // child inherits parents status
                        pItem->getWriteableData(static_cast<int>(eRegexFiltersColumn::IsFiltered)) = false;
                        visibleElements.push_back(pItem);
                    }
                }

                return true;
            };

            mpRootItem->visit(preVisitFunction, CTreeItem::tVisitFunction(), false);
        }

        //SEND_MSG(QString("~1 [CFiltersModel][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(time.elapsed()));

        for(auto& pVisibleElement : visibleElements)
        {
            if(nullptr != pVisibleElement)
            {
                auto preVisitParentsFunction = [](tTreeItem* pItem)
                {
                    if(nullptr != pItem)
                    {
                        pItem->getWriteableData(static_cast<int>(eRegexFiltersColumn::IsFiltered)) = false;
                    }

                    return true;
                };

                pVisibleElement->visitParents(preVisitParentsFunction, CTreeItem::tVisitFunction(), false, false);
            }
        }

        //SEND_MSG(QString("~2 [CFiltersModel][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(time.elapsed()));

        {
            tFilteredEntryVec filteredEntryVec;

            auto preVisitFunction = [this, &filteredEntryVec](tTreeItem* pItem)
            {
                if(nullptr != pItem)
                {
                    tFilteredEntry filteredEntry;
                    filteredEntry.row = pItem->getIdx();
                    filteredEntry.filtered = pItem->data(static_cast<int>(eRegexFiltersColumn::IsFiltered)).get<bool>();

                    auto pParent = pItem->getParent();

                    if(nullptr != pParent)
                    {
                        filteredEntry.parentIdx = createIndexInternal(pParent->getIdx(), 0, pParent);

                        //if(mpRootItem != pParent)
                        //{
                        //    SEND_MSG(QString("[CFiltersModel][%1] Parent |%2|%3|%4|. Parent index |%5|%6|%7| assigned to index |%8|%9|%10|")
                        //             .arg(__FUNCTION__)
                        //             .arg(pParent->data(static_cast<int>(eRegexFiltersColumn::Index)).get<int>())
                        //             .arg(pParent->data(static_cast<int>(eRegexFiltersColumn::ItemType)).get<QString>())
                        //             .arg(pParent->data(static_cast<int>(eRegexFiltersColumn::Value)).get<QString>())
                        //             .arg(filteredEntry.parentIdx.sibling(filteredEntry.parentIdx.row(),
                        //                                                  static_cast<int>(eRegexFiltersColumn::Index)).data().value<int>())
                        //             .arg(filteredEntry.parentIdx.sibling(filteredEntry.parentIdx.row(),
                        //                                                  static_cast<int>(eRegexFiltersColumn::ItemType)).data().value<QString>())
                        //             .arg(filteredEntry.parentIdx.sibling(filteredEntry.parentIdx.row(),
                        //                                                  static_cast<int>(eRegexFiltersColumn::Value)).data().value<QString>())
                        //             .arg(pItem->data(static_cast<int>(eRegexFiltersColumn::Index)).get<int>())
                        //             .arg(pItem->data(static_cast<int>(eRegexFiltersColumn::ItemType)).get<QString>())
                        //             .arg(pItem->data(static_cast<int>(eRegexFiltersColumn::Value)).get<QString>()));
                        //}
                        //else
                        //{
                        //    SEND_MSG(QString("[CFiltersModel][%1] Parent |%2|%3|%4|. Parent index |%5|%6|%7| assigned to index |%8|%9|%10|")
                        //             .arg(__FUNCTION__)
                        //             .arg(pParent->data(static_cast<int>(eRegexFiltersColumn::Index)).get<QString>())
                        //             .arg(pParent->data(static_cast<int>(eRegexFiltersColumn::ItemType)).get<QString>())
                        //             .arg(pParent->data(static_cast<int>(eRegexFiltersColumn::Value)).get<QString>())
                        //             .arg(filteredEntry.parentIdx.sibling(filteredEntry.parentIdx.row(),
                        //                                                  static_cast<int>(eRegexFiltersColumn::Index)).data().value<int>())
                        //             .arg(filteredEntry.parentIdx.sibling(filteredEntry.parentIdx.row(),
                        //                                                  static_cast<int>(eRegexFiltersColumn::ItemType)).data().value<QString>())
                        //             .arg(filteredEntry.parentIdx.sibling(filteredEntry.parentIdx.row(),
                        //                                                  static_cast<int>(eRegexFiltersColumn::Value)).data().value<QString>())
                        //            .arg(pItem->data(static_cast<int>(eRegexFiltersColumn::Index)).get<int>())
                        //             .arg(pItem->data(static_cast<int>(eRegexFiltersColumn::ItemType)).get<QString>())
                        //             .arg(pItem->data(static_cast<int>(eRegexFiltersColumn::Value)).get<QString>()));
                        //}
                    }

                    filteredEntryVec.push_back(filteredEntry);
                }

                return true;
            };

            if(false == mpRootItem->isWholeSorted())
            {
                mpRootItem->sort(static_cast<int>(mSortingColumn), mSortOrder, true);
            }

            mpRootItem->visit(preVisitFunction, CTreeItem::tVisitFunction(), false);

            filteredEntriesChanged(filteredEntryVec, true );
        }
    }

    //SEND_MSG(QString("~3 [CFiltersModel][%1] Processing took - %2 ms").arg(__FUNCTION__).arg(time.elapsed()));
}
