#pragma once

#include "memory"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

class CUMLView;
class IUMLModel;
class QPushButton;
class QPlainTextEdit;

class CUMLViewComponent : public DMA::IComponent,
                          public CSettingsManagerClient
{
public:

    CUMLViewComponent( CUMLView* pUMLView,
                       const tSettingsManagerPtr& pSettingsManager,
                       QPushButton* pUMLCreateDiagramFromTextButton,
                       QPlainTextEdit* pUMLTextEditor );

    CUMLView* getUMLView() const;

    virtual const char* getName() const override;

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    CUMLView* mpUMLView;
    QPushButton* mpUMLCreateDiagramFromTextButton;
    QPlainTextEdit* mpUMLTextEditor;
};
