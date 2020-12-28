#pragma once

#include "memory"

#include "dma/component/IComponent.hpp"

class CLogo;

class CLogoComponent : public DMA::IComponent
{
public:
    CLogoComponent( CLogo* pLogoView );

    CLogo* getLogoView() const;

    virtual const char* getName() const override;

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    CLogo* mpLogoView;
};
