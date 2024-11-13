#pragma once

#include "memory"

#include "QLineEdit"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"
#include "components/settings/api/ISettingsManager.hpp"
#include "components/analyzer/api/IDLTMessageAnalyzerController.hpp"
#include "components/regexHistory/api/IRegexHistoryProvider.hpp"
#include "components/regexHistory/api/CRegexHistoryTextEdit.hpp"

class CPatternsView;

/**
 * @brief Component for managing regex history integration.
 *
 * This class provides the functionality for integrating a regex history
 * feature into a user interface. It handles communication between the
 * regex input line edit, patterns view, and the message analyzer controller.
 */
class CRegexHistoryComponent : public DMA::IComponent
{
public:

    /**
     * @brief Constructs a CRegexHistoryComponent object.
     *
     * This constructor initializes the component with the required settings manager,
     * regex history line edit, patterns view, and DLT message analyzer controller.
     *
     * @param pSettingsManager Pointer to the settings manager.
     * @param pRegexHistoryLineEdit Pointer to the regex history line edit widget.
     * @param pPatternsView Pointer to the patterns view.
     * @param pDLTMessageAnalyzerController Shared pointer to the DLT message analyzer controller.
     */
    CRegexHistoryComponent( const tSettingsManagerPtr& pSettingsManager,
                            CRegexHistoryTextEdit* pRegexHistoryLineEdit,
                            CPatternsView* pPatternsView,
                            const tDLTMessageAnalyzerControllerPtr& pDLTMessageAnalyzerController);

    /**
     * @brief Gets the name of the component.
     *
     * This method returns the name of the component, as required by the base class.
     *
     * @return const char* The name of the component.
     */
    virtual const char* getName() const override;

    /**
     * @brief Gets the regex history provider.
     *
     * This method returns a shared pointer to the regex history provider, which
     * manages regex history suggestions.
     *
     * @return const tRegexHistoryProviderPtr& Shared pointer to the regex history provider.
     */
    const tRegexHistoryProviderPtr& getRegexHistoryProvider();

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    tRegexHistoryProviderPtr mpRegexHistoryProvider;
};
