/**
 * @file    CSearchResultModel.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CSearchResultModel class
 */
#ifndef CSEARCHRESULTMODEL_HPP
#define CSEARCHRESULTMODEL_HPP

#include "QList"
#include "QPair"
#include "QModelIndex"
#include "QWidget"

#include "../common/Definitions.hpp"

class CSearchResultModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    CSearchResultModel(QObject *parent=nullptr);

    void setFile(const tDLTFileWrapperPtr& pFile);
    void updateView();
    void resetData();
    int getFileIdx( const QModelIndex& idx ) const;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void addNextMessageIdxVec(const tFoundMatchesPack& foundMatchesPack);

    const tFoundMatchesPackItem& getFoundMatchesItemPack( const QModelIndex& modelIndex ) const;

private:

    QVariant getDataStrFromMsg(const QModelIndex& modelIndex, const tDLTMsgWrapperPtr &pMsg, eSearchResultColumn field) const;

private:

    tFoundMatchesPack mFoundMatchesPack;
    tDLTFileWrapperPtr mpFile;
};
#endif // CSEARCHRESULTMODEL_HPP
