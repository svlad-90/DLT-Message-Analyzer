#pragma once

#include "memory"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

class ISettingsManager;
typedef std::shared_ptr<ISettingsManager> tSettingsManagerPtr;

class CSettingsComponent : public DMA::IComponent
{
public:

    CSettingsComponent();

    const tSettingsManagerPtr& getSettingsManager() const;

    virtual const char* getName() const override;

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    tSettingsManagerPtr mpSettingsManager;
};
