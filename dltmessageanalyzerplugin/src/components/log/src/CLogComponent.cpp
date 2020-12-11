#include "../api/CLogComponent.hpp"
#include "CConsoleInputProcessor.hpp"
#include "CConsoleCtrl.hpp"

#include "DMA_Plantuml.hpp"

CLogComponent::CLogComponent(QLineEdit* pConsoleViewInput,
                             QTabWidget* pMainTabWidget,
                             QWidget* pConsoleViewTab,
                             QPlainTextEdit* pConsoleView):
mpConsoleViewInput(pConsoleViewInput),
mpConsoleInputProcessor(nullptr)
{
    NDLTMessageAnalyzer::NConsole::tConsoleConfig consoleConfig;
    consoleConfig.maxMsgSize = 10240;
    consoleConfig.logSize = 1000;
    consoleConfig.pTabWidget = pMainTabWidget;
    consoleConfig.pConsoleTab = pConsoleViewTab;
    consoleConfig.pConsoleTextEdit = pConsoleView;
    NDLTMessageAnalyzer::NConsole::CConsoleCtrl::createInstance(consoleConfig);
}

CLogComponent::~CLogComponent()
{
    NDLTMessageAnalyzer::NConsole::CConsoleCtrl::destroyInstance();
}

const char* CLogComponent::getName() const
{
    return "CLogComponent";
}

DMA::tSyncInitOperationResult CLogComponent::init()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpConsoleViewInput)
        {
            mpConsoleInputProcessor = std::make_shared<CConsoleInputProcessor>(mpConsoleViewInput);
            result.bIsOperationSuccessful = true;
            result.returnCode = 0;
        }
        else
        {
            result.bIsOperationSuccessful = false;
            result.returnCode = -1;
        }
    }
    catch (...)
    {
        result.bIsOperationSuccessful = false;
        result.returnCode = -1;
    }

    return result;
}

DMA::tSyncInitOperationResult CLogComponent::shutdown()
{
    DMA::tSyncInitOperationResult result;

    try
    {
        if(nullptr != mpConsoleInputProcessor)
        {
            mpConsoleInputProcessor.reset();
        }

        result.bIsOperationSuccessful = true;
        result.returnCode = 0;
    }
    catch (...)
    {
        result.bIsOperationSuccessful = false;
        result.returnCode = -1;
    }

    return result;
}

PUML_PACKAGE_BEGIN(DMA_Log_API)
    PUML_CLASS_BEGIN(CLogComponent)
        PUML_INHERITANCE_CHECKED(DMA::IComponent, implements)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CConsoleInputProcessor, 1, 1, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
