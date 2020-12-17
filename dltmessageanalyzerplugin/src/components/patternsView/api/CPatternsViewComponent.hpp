#pragma once

#include "memory"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

class CPatternsView;
class IPatternsModel;

class CPatternsViewComponent : public DMA::IComponent
{
public:

    CPatternsViewComponent( CPatternsView* pPatternsView );

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
