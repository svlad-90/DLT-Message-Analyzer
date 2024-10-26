#include "../api/CCoverageNoteComponent.hpp"

#include "components/log/api/CLog.hpp"
#include "CCoverageNoteProvider.hpp"

#include "DMA_Plantuml.hpp"

#include "dma/base/ForceLink.hpp"

CCoverageNoteComponent::CCoverageNoteComponent(const tSettingsManagerPtr& pSettingsManager,
                                               QTextEdit* commentTextEdit,
                                               QTableView* itemsTableView,
                                               QTextEdit* messagesTextEdit,
                                               QTextEdit* regexTextEdit,
                                               QPushButton* useRegexButton,
                                               QLineEdit* pCurrentFileLineEdit,
                                               QTextEdit* pFilesViewTextEdit):
    mpCoverageNoteProvider(nullptr),
    mpSettingsManager(pSettingsManager),
    mpCommentTextEdit(commentTextEdit),
    mpItemsTableView(itemsTableView),
    mpMessagesTextEdit(messagesTextEdit),
    mpRegexTextEdit(regexTextEdit),
    mpUseRegexButton(useRegexButton),
    mpCurrentFileLineEdit(pCurrentFileLineEdit),
    mpFilesViewTextEdit(pFilesViewTextEdit),
    mpMainTableView(nullptr)
{
    // force linkage references in order to have consistent diagrams
    DMA_FORCE_LINK_REFERENCE(ICoverageNoteProvider)
}

void CCoverageNoteComponent::setMainTableView(QTableView* pMainTableView)
{
    mpMainTableView = pMainTableView;

    if(mpCoverageNoteProvider)
    {
        mpCoverageNoteProvider->setMainTableView(mpMainTableView);
    }
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
        mpCoverageNoteProvider = std::make_shared<CCoverageNoteProvider>(mpSettingsManager,
                                                                         mpCommentTextEdit,
                                                                         mpItemsTableView,
                                                                         mpMessagesTextEdit,
                                                                         mpRegexTextEdit,
                                                                         mpUseRegexButton,
                                                                         mpCurrentFileLineEdit,
                                                                         mpFilesViewTextEdit);
        mpCoverageNoteProvider->setMainTableView(mpMainTableView);
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
        if(mpCoverageNoteProvider)
        {
            mpCoverageNoteProvider->shutdown();
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

const tCoverageNoteProviderPtr& CCoverageNoteComponent::getCoverageNoteProviderPtr()
{
    return mpCoverageNoteProvider;
}

PUML_PACKAGE_BEGIN(DMA_CoverageNote_API)
    PUML_CLASS_BEGIN_CHECKED(CCoverageNoteComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(ICoverageNoteProvider, 1, 1, contains)
        PUML_USE_DEPENDENCY_CHECKED(ISettingsManager, 1, 1, passes to nested entities)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTextEdit, 1, 4, passes to nested entities)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTableView, 1, 2, passes to nested entities)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QPushButton, 1, 1, passes to nested entities)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QLineEdit, 1, 1, passes to nested entities)
    PUML_CLASS_END()
PUML_PACKAGE_END()
