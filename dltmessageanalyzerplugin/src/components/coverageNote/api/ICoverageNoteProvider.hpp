#pragma once

#include "QObject"
#include "QTableView"

#include "../api/Types.hpp"

/**
 * @brief Interface class for providing coverage note functionalities.
 *
 * This class serves as an interface for managing coverage note items,
 * including adding items, setting regex and messages, and handling
 * externally triggered coverage note UI interactions.
 */
class ICoverageNoteProvider : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for ICoverageNoteProvider.
     * @param parent The parent QObject, default is nullptr.
     */
    ICoverageNoteProvider(QObject *parent = nullptr);

    /**
     * @brief Virtual destructor for ICoverageNoteProvider.
     */
    virtual ~ICoverageNoteProvider();

    /**
     * @brief Adds a new coverage note item.
     * @return The ID of the newly added coverage note item.
     */
    virtual tCoverageNoteItemId addCoverageNoteItem() = 0;

    /**
     * @brief Sets the regex pattern for a coverage note item.
     * @param id The ID of the coverage note item.
     * @param regex The regex pattern to set.
     */
    virtual void setCoverageNoteItemRegex(const tCoverageNoteItemId& id, const QString& regex) = 0;

    /**
     * @brief Sets the message for a coverage note item.
     * @param id The ID of the coverage note item.
     * @param comment The message/comment to set.
     */
    virtual void setCoverageNoteItemMessage(const tCoverageNoteItemId& id, const QString& comment) = 0;

    /**
     * @brief Scrolls to the last coverage note item in the UI.
     */
    virtual void scrollToLastCoveageNoteItem() = 0;

    /**
     * @brief Sets the main table view for creation of comments
     * from the main dlt-viewer's table.
     * @param pMainTableView Pointer to the main QTableView.
     */
    virtual void setMainTableView(QTableView* pMainTableView) = 0;

    /**
     * @brief Shuts down the coverage note provider, releasing any resources.
     */
    virtual void shutdown() = 0;

signals:
    /**
     * @brief Signal emitted when regex application is requested.
     * @param regex The regex pattern to apply.
     */
    void regexApplicationRequested(const QString& regex);

    /**
     * @brief Signal emitted when a request is made to add a comment from the main table.
     */
    void addCommentFromMainTableRequested();
};

/**
 * @brief Shared pointer for ICoverageNoteProvider.
 */
typedef std::shared_ptr<ICoverageNoteProvider> tCoverageNoteProviderPtr;
