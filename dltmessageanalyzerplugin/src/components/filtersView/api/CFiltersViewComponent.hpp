#pragma once

#include "memory"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

class CFiltersView;
class IFiltersModel;

class CFiltersViewComponent : public DMA::IComponent
{
public:

    CFiltersViewComponent( CFiltersView* pFiltersView );

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
