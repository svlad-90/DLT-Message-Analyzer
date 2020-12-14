#pragma once

#include "memory"

#include "QObject"

#include "common/Definitions.hpp"
#include "dma/component/IComponent.hpp"

class CGroupedView;
class IGroupedViewModel;

class CGroupedViewComponent : public QObject, public DMA::IComponent
{
    Q_OBJECT
public:

    CGroupedViewComponent( CGroupedView* pGroupedView );

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
