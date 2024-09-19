/**
 * @file    CSearchResultModel.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CSearchResultModel class
 */

#pragma once

#include "QList"
#include "QPair"
#include "QModelIndex"
#include "QWidget"

#include "common/Definitions.hpp"

#include "../api/ISearchResultModel.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

class CSearchResultModel : public ISearchResultModel,
                           public CSettingsManagerClient
{
    Q_OBJECT

public:

    CSearchResultModel(const tSettingsManagerPtr& pSettingsManager,
                       QObject *parent=nullptr);

    // implementation of the ISearchResultModel
    void setFile(const tFileWrapperPtr& pFile) override;
    void updateView(const int& fromRow = 0) override;
    void resetData() override;
    int getFileIdx( const QModelIndex& idx ) const override;
    int getRowByMsgId( const tMsgId& id ) const override;
    std::pair<bool, tIntRange> addNextMessageIdxVec(const tFoundMatchesPack& foundMatchesPack) override;
    std::pair<int /*rowNumber*/, QString /*diagramContent*/> getUMLDiagramContent() const override;
    tPlotContent createPlotContent() const override;
    void setHighlightedRows( const tMsgIdSet& msgs) override;
    const tFoundMatchesPackItem& getFoundMatchesItemPack( const QModelIndex& modelIndex ) const override;
    const tMsgIdSet& getHighlightedRows() const override;
    // implementation of the ISearchResultModel ( END )

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setUML_Applicability( const QModelIndex& index, bool checked );
    void setPlotView_Applicability( const QModelIndex& index, bool checked );

    QString getStrValue(const int& row, const eSearchResultColumn& column) const;

private:

    tFoundMatchesPack mFoundMatchesPack;
    tFileWrapperPtr mpFile;
    tMsgIdSet mHighlightMessages;
};
