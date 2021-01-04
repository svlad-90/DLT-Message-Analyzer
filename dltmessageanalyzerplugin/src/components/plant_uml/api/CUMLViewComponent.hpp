#pragma once

#include "memory"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

class CUMLView;
class IUMLModel;

class CUMLViewComponent : public DMA::IComponent,
                          public CSettingsManagerClient
{
public:

    CUMLViewComponent( CUMLView* pUMLView,
                       const tSettingsManagerPtr& pSettingsManager);

    CUMLView* getUMLView() const;

    virtual const char* getName() const override;

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    CUMLView* mpUMLView;
};
