/**
 * @file    CPlotViewComponent.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CPlotViewComponent class
 */

#pragma once

#include "dma/component/IComponent.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

class QPushButton;
class CCustomPlotExtended;
class ISearchResultModel;
class QCustomPlot;

class CPlotViewComponent : public QObject,
                           public DMA::IComponent,
                           public CSettingsManagerClient
{
Q_OBJECT
public:

    CPlotViewComponent( CCustomPlotExtended* pPlot,
                        QPushButton* pCreatePlotButton,
                        std::shared_ptr<ISearchResultModel> pSearchResultModel,
                        const tSettingsManagerPtr& pSettingsManager );

    virtual const char* getName() const override;
    CCustomPlotExtended* getPlot();

signals:
    void messageIdSelected(const tMsgId& msgId);

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    void generatePlot();

private:
    CCustomPlotExtended* mpPlot;
    QPushButton* mpCreatePlotButton;
    std::shared_ptr<ISearchResultModel> mpSearchResultModel;
};
