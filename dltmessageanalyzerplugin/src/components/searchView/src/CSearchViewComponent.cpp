#include "../api/CSearchViewComponent.hpp"

#include "CSearchResultModel.hpp"

#include "../api/CSearchResultView.hpp"
#include "common/CTableMemoryJumper.hpp"

#include "DMA_Plantuml.hpp"

#include "dma/base/ForceLink.hpp"

CSearchViewComponent::CSearchViewComponent( CSearchResultView* pSearchResultView ):
mpSearchResultModel(nullptr),
mpSearchResultView(pSearchResultView),
mpSearchViewTableJumper(nullptr)
{
    DMA_FORCE_LINK_REFERENCE(ISearchResultModel)
}

std::shared_ptr<ISearchResultModel> CSearchViewComponent::getSearchResultModel()
{
    return std::static_pointer_cast<ISearchResultModel>(mpSearchResultModel);
}

const char* CSearchViewComponent::getName() const
{
    return "CSearchViewComponent";
}

void CSearchViewComponent::setFile( const tDLTFileWrapperPtr& pFile )
{
    if(nullptr != mpSearchViewTableJumper)
    {
        mpSearchViewTableJumper->resetSelectedRow();
    }

    if(nullptr != mpSearchResultView)
    {
        mpSearchResultView->setFile(pFile);
    }
}

DMA::tSyncInitOperationResult CSearchViewComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {        
        if(nullptr != mpSearchResultView)
        {
            mpSearchResultModel = std::make_shared<CSearchResultModel>();

            mpSearchViewTableJumper = std::make_shared<CTableMemoryJumper>(mpSearchResultView);

            if(nullptr != mpSearchResultView &&
                    nullptr != mpSearchResultModel)
            {
                mpSearchResultView->setModel(mpSearchResultModel.get());
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
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CSearchResultModel, 1, 1, contains)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CSearchResultView, 1, 1, uses)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CTableMemoryJumper, 1, 1, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
