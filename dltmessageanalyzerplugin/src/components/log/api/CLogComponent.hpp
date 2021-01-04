#pragma once

#include "QLineEdit"
#include "QTabWidget"
#include "QPlainTextEdit"

#include "memory"
#include "dma/component/IComponent.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

class CConsoleInputProcessor;

class CLogComponent : public DMA::IComponent,
                      public CSettingsManagerClient
{
public:

    CLogComponent(QLineEdit* pConsoleViewInput,
                  QTabWidget* pMainTabWidget,
                  QWidget* pConsoleViewTab,
                  QPlainTextEdit* pConsoleView,
                  const tSettingsManagerPtr& pSettingsManager
                  );
    ~CLogComponent();

    virtual const char* getName() const override;

protected:
    virtual DMA::tSyncInitOperationResult init() override;
    virtual DMA::tSyncInitOperationResult shutdown() override;

private:
    QLineEdit* mpConsoleViewInput;
    std::shared_ptr<CConsoleInputProcessor> mpConsoleInputProcessor;
};
