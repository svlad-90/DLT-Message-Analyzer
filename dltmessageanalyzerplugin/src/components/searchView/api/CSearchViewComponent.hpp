#pragma once

#include "memory"

#include "QObject"
#include "QTabWidget"
#include "QTableView"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"
#include "components/coverageNote/api/ICoverageNoteProvider.hpp"

class CTableMemoryJumper;
class CSearchResultView;
class ISearchResultModel;

class CSearchViewComponent : public QObject,
                             public DMA::IComponent,
                             public CSettingsManagerClient
{
    Q_OBJECT
public:

    CSearchViewComponent( QTabWidget* pMainTabWidget,
                          CSearchResultView* pSearchResultView,
                          const tSettingsManagerPtr& pSettingsManager,
                          const tCoverageNoteProviderPtr& pCoverageNoteProviderPtr);

    CSearchResultView* getSearchResultView() const;
    std::shared_ptr<CTableMemoryJumper> getTableMemoryJumper() const;

    virtual const char* getName() const override;

    std::shared_ptr<ISearchResultModel> getSearchResultModel();
    void setMainTableView(QTableView* pMainTableView);

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    std::shared_ptr<ISearchResultModel> mpSearchResultModel;
    CSearchResultView* mpSearchResultView;
    std::shared_ptr<CTableMemoryJumper> mpSearchViewTableJumper;
    tCoverageNoteProviderPtr mpCoverageNoteProviderPtr;
};
