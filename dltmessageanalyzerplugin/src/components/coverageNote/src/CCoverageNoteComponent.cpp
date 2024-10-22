#include "../api/CCoverageNoteComponent.hpp"

#include "components/log/api/CLog.hpp"
#include "CCoverageNoteProvider.hpp"

#include "DMA_Plantuml.hpp"

#include "dma/base/ForceLink.hpp"

CCoverageNoteComponent::CCoverageNoteComponent(QTabWidget* pMainTabWidget,
                                               const tSettingsManagerPtr& pSettingsManager,
                                               QTextEdit* commentTextEdit,
                                               QTableView* itemsTableView,
                                               QTextEdit* messagesTextEdit,
                                               QPushButton* openButton,
                                               QTextEdit* regexTextEdit,
                                               QPushButton* useRegexButton):
    mpCoverageNoteProviderPtr(nullptr),
    mpSettingsManager(pSettingsManager),
    mpCommentTextEdit(commentTextEdit),
    mpItemsTableView(itemsTableView),
    mpMessagesTextEdit(messagesTextEdit),
    mpOpenButton(openButton),
    mpRegexTextEdit(regexTextEdit),
    mpUseRegexButton(useRegexButton),
    mpMainTabWidget(pMainTabWidget)
{
    // force linkage references in order to have consistent diagrams
    DMA_FORCE_LINK_REFERENCE(IRegexHistoryProvider)
}

const char* CCoverageNoteComponent::getName() const
{
    return "CCoverageNoteComponent";
}

DMA::tSyncInitOperationResult CCoverageNoteComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        mpCoverageNoteProviderPtr = std::make_shared<CCoverageNoteProvider>(mpMainTabWidget,
                                                                            mpSettingsManager,
                                                                            mpCommentTextEdit,
                                                                            mpItemsTableView,
                                                                            mpMessagesTextEdit,
                                                                            mpOpenButton,
                                                                            mpRegexTextEdit,
                                                                            mpUseRegexButton);
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

DMA::tSyncInitOperationResult CCoverageNoteComponent::shutdown()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        // do something
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

const tCoverageNoteProviderPtr& CCoverageNoteComponent::getCoverageNoteProviderPtr()
{
    return mpCoverageNoteProviderPtr;
}

PUML_PACKAGE_BEGIN(DMA_CoverageNote_API)
    PUML_CLASS_BEGIN_CHECKED(CCoverageNoteComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_USE_DEPENDENCY_CHECKED(ISettingsManager, 1, 1, passes to nested entities)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTextEdit, 1, 2, passes to nested entities)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTableView, 1, 1, passes to nested entities)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QPushButton, 1, 3, passes to nested entities)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTabWidget, 1, 3, passes to nested entities)
    PUML_CLASS_END()
PUML_PACKAGE_END()
