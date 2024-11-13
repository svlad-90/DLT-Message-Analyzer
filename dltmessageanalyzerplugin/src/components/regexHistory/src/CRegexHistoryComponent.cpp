#include "../api/CRegexHistoryComponent.hpp"

#include "components/log/api/CLog.hpp"
#include "CRegexHistoryProvider.hpp"

#include "DMA_Plantuml.hpp"

#include "dma/base/ForceLink.hpp"

CRegexHistoryComponent::CRegexHistoryComponent( const tSettingsManagerPtr& pSettingsManager,
                                                CRegexHistoryTextEdit* pRegexHistoryLineEdit, CPatternsView* pPatternsView,
                                                const tDLTMessageAnalyzerControllerPtr& pDLTMessageAnalyzerController ):
mpRegexHistoryProvider(std::make_shared<CRegexHistoryProvider>(pSettingsManager,
                                                               pRegexHistoryLineEdit,
                                                               pPatternsView,
                                                               pDLTMessageAnalyzerController))
{
    // force linkage references in order to have consistent diagrams
    DMA_FORCE_LINK_REFERENCE(IRegexHistoryProvider)

    if(nullptr != pRegexHistoryLineEdit)
    {
        pRegexHistoryLineEdit->setRegexHistoryProvider(mpRegexHistoryProvider);
        pRegexHistoryLineEdit->setSettingsManager(pSettingsManager);
    }
}

const char* CRegexHistoryComponent::getName() const
{
    return "CRegexHistoryComponent";
}

DMA::tSyncInitOperationResult CRegexHistoryComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {
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

DMA::tSyncInitOperationResult CRegexHistoryComponent::shutdown()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        mpRegexHistoryProvider.reset();
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

const tRegexHistoryProviderPtr& CRegexHistoryComponent::getRegexHistoryProvider()
{
    return mpRegexHistoryProvider;
}

PUML_PACKAGE_BEGIN(DMA_RegexHistory_API)
    PUML_CLASS_BEGIN(CRegexHistoryComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CPatternsView, 1, 1, uses)
        PUML_USE_DEPENDENCY_CHECKED(ISettingsManager, 1, 1, passes)
        PUML_USE_DEPENDENCY_CHECKED(CRegexHistoryTextEdit, 1, 1, passes)
        PUML_USE_DEPENDENCY_CHECKED(IDLTMessageAnalyzerController, 1, 1, passes)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CRegexHistoryProvider, 1, 1, creates)
    PUML_CLASS_END()
PUML_PACKAGE_END()
