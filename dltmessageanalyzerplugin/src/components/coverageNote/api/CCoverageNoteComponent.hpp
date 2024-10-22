#pragma once

#include "memory"

#include "QTextEdit"
#include "QTableView"
#include "QPushButton"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"
#include "components/settings/api/ISettingsManager.hpp"

#include "ICoverageNoteProvider.hpp"

class CCoverageNoteComponent : public DMA::IComponent
{
public:

    CCoverageNoteComponent(QTabWidget* pMainTabWidget,
                           const tSettingsManagerPtr& pSettingsManager,
                           QTextEdit* commentTextEdit,
                           QTableView* itemsTableView,
                           QTextEdit* messagesTextEdit,
                           QPushButton* openButton,
                           QTextEdit* regexTextEdit,
                           QPushButton* useRegexButton);
    virtual const char* getName() const override;

    const tCoverageNoteProviderPtr& getCoverageNoteProviderPtr();

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    tCoverageNoteProviderPtr mpCoverageNoteProviderPtr;
    tSettingsManagerPtr mpSettingsManager;
    QTextEdit* mpCommentTextEdit;
    QTableView* mpItemsTableView;
    QTextEdit* mpMessagesTextEdit;
    QPushButton* mpOpenButton;
    QTextEdit* mpRegexTextEdit;
    QPushButton* mpUseRegexButton;
    QTabWidget* mpMainTabWidget;
};
