#pragma once

#include "memory"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

class CFiltersView;
class IFiltersModel;

class CFiltersViewComponent : public DMA::IComponent, CSettingsManagerClient
{
public:

    CFiltersViewComponent( CFiltersView* pFiltersView,
                           const tSettingsManagerPtr& pSettingsManager );

    CFiltersView* getFiltersView() const;

    virtual const char* getName() const override;

    std::shared_ptr<IFiltersModel> getFiltersModel();

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    std::shared_ptr<IFiltersModel> mpFiltersModel;
    CFiltersView* mpFiltersView;
};
