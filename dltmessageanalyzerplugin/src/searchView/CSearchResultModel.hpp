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
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    std::pair<bool, tIntRange> addNextMessageIdxVec(const tFoundMatchesPack& foundMatchesPack);
    std::pair<int /*rowNumber*/, QString /*diagramContent*/> getUMLDiagramContent() const;

    const tFoundMatchesPackItem& getFoundMatchesItemPack( const QModelIndex& modelIndex ) const;

    void setUML_Applicability( const QModelIndex& index, bool checked );

private:

    QString getDataStrFromMsg(const QModelIndex& modelIndex, const tDLTMsgWrapperPtr &pMsg, eSearchResultColumn field) const;

private:

    tFoundMatchesPack mFoundMatchesPack;
    tDLTFileWrapperPtr mpFile;
};
#endif // CSEARCHRESULTMODEL_HPP
