/**
 * @file    CSearchResultModel.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CSearchResultModel class
 */

#include "qdlt.h"

#include "CSearchResultModel.hpp"
#include "../dltWrappers/CDLTMsgWrapper.hpp"
#include "../dltWrappers/CDLTFileWrapper.hpp"

CSearchResultModel::CSearchResultModel(QObject *):
    mFoundMatchesPack(), mpFile(nullptr)
{

}

void CSearchResultModel::setFile(const tDLTFileWrapperPtr& pFile)
{
    mpFile = pFile;
}

void CSearchResultModel::updateView()
{
    emit dataChanged( index(0,0), index(static_cast<int>(mFoundMatchesPack.matchedItemVec.size()), 1) );
    emit layoutChanged();
}

void CSearchResultModel::resetData()
{
    beginResetModel();
    mFoundMatchesPack.matchedItemVec.clear();
    endResetModel();
    updateView();
}

int CSearchResultModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(mFoundMatchesPack.matchedItemVec.size());
}

int CSearchResultModel::columnCount(const QModelIndex &) const
{
    return static_cast<int>(eSearchResultColumn::Last);
}

const tFoundMatchesPackItem& CSearchResultModel::getFoundMatchesItemPack( const QModelIndex& modelIndex ) const
{
    if ( ( modelIndex.row() < 0 || modelIndex.row() >= static_cast<int>(mFoundMatchesPack.matchedItemVec.size()) ) ||
         ( modelIndex.column() < 0 || modelIndex.column() >= static_cast<int>(eSearchResultColumn::Last) ) )
    {
        static const tFoundMatchesPackItem sDummyValue;
        return sDummyValue;
    }

    return mFoundMatchesPack.matchedItemVec[ static_cast<std::size_t>(modelIndex.row()) ];
}

QVariant CSearchResultModel::data(const QModelIndex &index, int role) const
{
    if ( ( index.row() < 0 || index.row() >= static_cast<int>(mFoundMatchesPack.matchedItemVec.size()) ) ||
         ( index.column() < 0 || index.column() >= static_cast<int>(eSearchResultColumn::Last) ) )
        return QVariant();

    QVariant result;

    if (role == Qt::TextAlignmentRole)
    {
        Qt::AlignmentFlag alignment = Qt::AlignCenter;

        switch( static_cast<eSearchResultColumn>(index.column()) )
        {
            case eSearchResultColumn::Index: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Time: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Timestamp: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Count: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Ecuid: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Apid: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Ctid: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::SessionId: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Type: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Subtype: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Mode: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Args: { alignment = Qt::AlignCenter; } break;
            case eSearchResultColumn::Payload: { alignment = Qt::AlignLeft; } break;
            case eSearchResultColumn::Last: { alignment = Qt::AlignCenter; } break;
        }

        result = alignment;
    }
    else if ( role == Qt::DisplayRole )
    {
        if(nullptr != mpFile && 0 < mpFile->size())
        {
            const auto& pMsg = mpFile->getMsg(mFoundMatchesPack.matchedItemVec[static_cast<std::size_t>(index.row())].getItemMetadata().msgId);
            result = getDataStrFromMsg(index, pMsg, static_cast<eSearchResultColumn>(index.column()));
        }
    }

    return result;
}

QVariant CSearchResultModel::getDataStrFromMsg(const QModelIndex& modelIndex, const tDLTMsgWrapperPtr &pMsg, eSearchResultColumn field) const
{
    if(nullptr == pMsg)
    {
        return QVariant();
    }

    QString strRes;

    switch(field)
    {
        case eSearchResultColumn::Index:
        {
            int idx = mFoundMatchesPack.matchedItemVec[static_cast<std::size_t>(modelIndex.row())].getItemMetadata().msgId;

            strRes = QString("%1").arg(idx);
        }
            break;
        case eSearchResultColumn::Time:
        {
            strRes = QString("%1.%2").arg(pMsg->getTimeString()).arg(pMsg->getMicroseconds(),6,10,QLatin1Char('0'));
        }
            break;
        case eSearchResultColumn::Timestamp:
        {
            strRes = QString("%1.%2").arg(pMsg->getTimestamp()/10000).arg(pMsg->getTimestamp()%10000,4,10,QLatin1Char('0'));
        }
            break;
        case eSearchResultColumn::Count:
        {
            strRes = QString("%1").arg(pMsg->getMessageCounter());
        }
            break;
        case eSearchResultColumn::Ecuid:
        {
            strRes = pMsg->getEcuid();
        }
            break;
        case eSearchResultColumn::Apid:
        {
            strRes = pMsg->getApid();
        }
            break;
        case eSearchResultColumn::Ctid:
        {
            strRes = pMsg->getCtid();
        }
            break;
        case eSearchResultColumn::SessionId:
        {
            strRes = QString("%1").arg(pMsg->getSessionid());
        }
            break;
        case eSearchResultColumn::Type:
        {
            strRes = pMsg->getTypeString();
        }
            break;
        case eSearchResultColumn::Subtype:
        {
            strRes = pMsg->getSubtypeString();
        }
            break;
        case eSearchResultColumn::Mode:
        {
            strRes = pMsg->getModeString();
        }
            break;
        case eSearchResultColumn::Args:
        {
            strRes = QString("%1").arg(pMsg->getNumberOfArguments());
        }
            break;
        case eSearchResultColumn::Payload:
        {
            strRes = pMsg->getPayload();
        }
            break;
        case eSearchResultColumn::Last:
        {
            strRes = "Unhandled field type!";
        }
            break;
    }

    return QVariant(strRes);
}

QVariant CSearchResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        result = getName(static_cast<eSearchResultColumn>(section));
    }

    return result;
}

Qt::ItemFlags CSearchResultModel::flags(const QModelIndex &index) const
{
    if (false == index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

void CSearchResultModel::addNextMessageIdxVec(const tFoundMatchesPack &foundMatchesPack)
{
    beginInsertRows(QModelIndex(), static_cast<int>(mFoundMatchesPack.matchedItemVec.size()),
                    static_cast<int>(mFoundMatchesPack.matchedItemVec.size()));
    mFoundMatchesPack.matchedItemVec.insert(mFoundMatchesPack.matchedItemVec.end(),
                                            foundMatchesPack.matchedItemVec.begin(),
                                            foundMatchesPack.matchedItemVec.end());
    endInsertRows();
    updateView();
}

int CSearchResultModel::getFileIdx( const QModelIndex& idx ) const
{
   int result = -1;

   auto row = idx.row();

   if(row >= 0 && static_cast<size_t>(row) < mFoundMatchesPack.matchedItemVec.size())
   {
       result = mFoundMatchesPack.matchedItemVec[static_cast<std::size_t>(row)].getItemMetadata().msgIdFiltered;
   }

   return result;
}
