#pragma once

#include "memory"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

class CPatternsView;
class IPatternsModel;

class CPatternsViewComponent : public DMA::IComponent,
                               public CSettingsManagerClient
{
public:

    CPatternsViewComponent( CPatternsView* pPatternsView,
                            const tSettingsManagerPtr& pSettingsManagerPtr );

    CPatternsView* getPatternsView() const;

    virtual const char* getName() const override;

    std::shared_ptr<IPatternsModel> getPatternsModel();

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    std::shared_ptr<IPatternsModel> mpPatternsModel;
    CPatternsView* mpPatternsView;
};
