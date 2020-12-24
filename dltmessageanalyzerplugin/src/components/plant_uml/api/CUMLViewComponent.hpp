#pragma once

#include "memory"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

class CUMLView;
class IUMLModel;

class CUMLViewComponent : public DMA::IComponent
{
public:

    CUMLViewComponent( CUMLView* pUMLView );

    CUMLView* getUMLView() const;

    virtual const char* getName() const override;

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    CUMLView* mpUMLView;
};
