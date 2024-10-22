#pragma once

#include "memory"

#include "QTextEdit"
#include "QTableView"
#include "QPushButton"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"
#include "components/settings/api/ISettingsManager.hpp"

#include "ICoverageNoteProvider.hpp"

/**
 * @brief Class representing the coverage note component.
 *
 * This component is responsible for managing the coverage notes functionality.
 */
class CCoverageNoteComponent : public DMA::IComponent
{
public:
    /**
     * @brief Constructor for CCoverageNoteComponent.
     *
     * @param pSettingsManager Pointer to the settings manager.
     * @param pCommentTextEdit Pointer to the QTextEdit for comments.
     * @param pItemsTableView Pointer to the QTableView for items.
     * @param MessagesTextEdit Pointer to the QTextEdit for messages.
     * @param pRegexTextEdit Pointer to the QTextEdit for regex.
     * @param pUseRegexButton Pointer to the QPushButton for using regex.
     * @param pCurrentFileLineEdit Pointer to the QLineEdit for current file.
     * @param pFilesViewTextEdit Pointer to the QTextEdit for files view.
     */
    CCoverageNoteComponent(const tSettingsManagerPtr& pSettingsManager,
                           QTextEdit* pCommentTextEdit,
                           QTableView* pItemsTableView,
                           QTextEdit* MessagesTextEdit,
                           QTextEdit* pRegexTextEdit,
                           QPushButton* pUseRegexButton,
                           QLineEdit* pCurrentFileLineEdit,
                           QTextEdit* pFilesViewTextEdit);

    /**
     * @brief Gets the name of the component.
     *
     * @return The name of the component as a const char pointer.
     */
    virtual const char* getName() const override;

    /**
     * @brief Gets the coverage note provider pointer.
     *
     * @return A shared pointer to the ICoverageNoteProvider.
     */
    const tCoverageNoteProviderPtr& getCoverageNoteProviderPtr();

    /**
     * @brief Sets the main table view for the component.
     *
     * @param pMainTableView Pointer to the QTableView to be set as the main table view.
     */
    void setMainTableView(QTableView* pMainTableView);

protected:
    /**
     * @brief Initializes the component.
     *
     * @return The result of the initialization operation.
     */
    virtual DMA::tSyncInitOperationResult init() override;

    /**
     * @brief Shuts down the component.
     *
     * @return The result of the shutdown operation.
     */
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    tCoverageNoteProviderPtr mpCoverageNoteProvider; ///< Pointer to the coverage note provider.
    tSettingsManagerPtr mpSettingsManager;          ///< Pointer to the settings manager.
    QTextEdit* mpCommentTextEdit;                   ///< Pointer to the comment text edit.
    QTableView* mpItemsTableView;                   ///< Pointer to the items table view.
    QTextEdit* mpMessagesTextEdit;                  ///< Pointer to the messages text edit.
    QTextEdit* mpRegexTextEdit;                     ///< Pointer to the regex text edit.
    QPushButton* mpUseRegexButton;                  ///< Pointer to the use regex button.
    QLineEdit* mpCurrentFileLineEdit;               ///< Pointer to the current file line edit.
    QTextEdit* mpFilesViewTextEdit;                  ///< Pointer to the file view text edit.
    QTableView* mpMainTableView;                    ///< Pointer to the main table view.
};
