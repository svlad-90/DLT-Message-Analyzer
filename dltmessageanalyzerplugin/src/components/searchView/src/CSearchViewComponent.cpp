#include "../api/CSearchViewComponent.hpp"

#include "CSearchResultModel.hpp"

#include "../api/CSearchResultView.hpp"
#include "common/CTableMemoryJumper.hpp"

#include "DMA_Plantuml.hpp"

#include "dma/base/ForceLink.hpp"

CSearchViewComponent::CSearchViewComponent( QTabWidget* pMainTabWidget,
                                            CSearchResultView* pSearchResultView,
                                            const tSettingsManagerPtr& pSettingsManager,
                                            const tCoverageNoteProviderPtr& pCoverageNoteProviderPtr ):
CSettingsManagerClient(pSettingsManager),
mpSearchResultModel(nullptr),
mpSearchResultView(pSearchResultView),
mpSearchViewTableJumper(nullptr),
mpCoverageNoteProviderPtr(pCoverageNoteProviderPtr)
{
    DMA_FORCE_LINK_REFERENCE(ISearchResultModel)
    mpSearchResultView->setCoverageNoteProvider(mpCoverageNoteProviderPtr);
    mpSearchResultView->setMainTabWidget(pMainTabWidget);
}

std::shared_ptr<ISearchResultModel> CSearchViewComponent::getSearchResultModel()
{
    return mpSearchResultModel;
}

const char* CSearchViewComponent::getName() const
{
    return "CSearchViewComponent";
}

void CSearchViewComponent::setMainTableView(QTableView* pMainTableView)
{
    if(mpSearchResultView)
    {
        mpSearchResultView->setMainTableView(pMainTableView);
    }
}

DMA::tSyncInitOperationResult CSearchViewComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {        
        if(nullptr != mpSearchResultView)
        {
            auto pSearchResultModel = std::make_shared<CSearchResultModel>(getSettingsManager());

            mpSearchViewTableJumper = std::make_shared<CTableMemoryJumper>(mpSearchResultView);

            mpSearchResultView->setSettingsManager(getSettingsManager());

            if(nullptr != pSearchResultModel)
            {
                mpSearchResultModel = pSearchResultModel;
                mpSearchResultView->setModel(pSearchResultModel.get());
            }

            connect(mpSearchResultView->selectionModel(), &QItemSelectionModel::currentChanged,
            [this](const QModelIndex &current, const QModelIndex &)
            {
                if(nullptr != mpSearchResultView)
                {
                    auto index = current.sibling(current.row(), static_cast<int>(eSearchResultColumn::Index));
                    mpSearchViewTableJumper->setSelectedRow(index.data().value<int>());
                }
            });

            result.bIsOperationSuccessful = true;
            result.returnCode = 0;
        }
        else
        {
            result.bIsOperationSuccessful = false;
            result.returnCode = -1;
        }
    }
    catch (...)
    {
        result.bIsOperationSuccessful = false;
        result.returnCode = -1;
    }

    return result;
}

DMA::tSyncInitOperationResult CSearchViewComponent::shutdown()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpSearchResultModel)
        {
            mpSearchResultModel.reset();
        }

        result.bIsOperationSuccessful = true;
        result.returnCode = 0;
    }
    catch (...)
    {
        result.bIsOperationSuccessful = false;
        result.returnCode = -1;
    }

    return result;
}

std::shared_ptr<CTableMemoryJumper> CSearchViewComponent::getTableMemoryJumper() const
{
    return mpSearchViewTableJumper;
}

CSearchResultView* CSearchViewComponent::getSearchResultView() const
{
    return mpSearchResultView;
}

PUML_PACKAGE_BEGIN(DMA_SearchView_API)
    PUML_CLASS_BEGIN(CSearchViewComponent)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(ISearchResultModel, 1, 1, contains)
        PUML_USE_DEPENDENCY_CHECKED(CSearchResultModel, 1, 1, using to create ISearchResultModel)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CSearchResultView, 1, 1, uses)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CTableMemoryJumper, 1, 1, contains)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(ICoverageNoteProvider, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTabWidget, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
