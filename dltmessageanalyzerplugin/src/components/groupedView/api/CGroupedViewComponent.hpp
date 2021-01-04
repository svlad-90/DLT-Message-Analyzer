#pragma once

#include "memory"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

class CGroupedView;
class IGroupedViewModel;

class CGroupedViewComponent : public DMA::IComponent,
                              public CSettingsManagerClient
{
public:

    CGroupedViewComponent( CGroupedView* pGroupedView,
                           const tSettingsManagerPtr& pSettingsManager );

    CGroupedView* getGroupedView() const;

    virtual const char* getName() const override;

    std::shared_ptr<IGroupedViewModel> getGroupedViewModel();

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    std::shared_ptr<IGroupedViewModel> mpGroupedViewModel;
    CGroupedView* mpGroupedView;
};
