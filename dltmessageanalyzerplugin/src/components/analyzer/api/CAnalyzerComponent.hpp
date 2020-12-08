#pragma once

#include "memory"

#include "dma/component/IComponent.hpp"

#include "components/analyzer/api/IDLTMessageAnalyzerController.hpp"

class CAnalyzerComponent : public DMA::IComponent
{
public:

    CAnalyzerComponent();
    std::shared_ptr<IDLTMessageAnalyzerController> getAnalyzerController() const;

    virtual const char* getName() const override;

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    std::shared_ptr<IDLTMessageAnalyzerController> mpMessageAnalyzerController;
};
