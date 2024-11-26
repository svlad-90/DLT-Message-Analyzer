#include "QStyleFactory"
#include "QApplication"
#include "QTextStream"
#include "QKeyEvent"
#include "QStyle"
#include "QDir"
#include "QProcessEnvironment"
#include "QElapsedTimer"

#include "dlt_common.h"

#include "dltmessageanalyzerplugin.hpp"

#include "common/Definitions.hpp"

#include "CConsoleInputProcessor.hpp"
#include "../api/CLog.hpp"

#include "DMA_Plantuml.hpp"

static const int sMaxCommandsHistorySize = 50;
static const QString sHelpCommandName = "help";

namespace NShortcuts
{
    static bool isPrevHistoryItem( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->key() == Qt::Key_Up;
    }

    static bool isNextHistoryItem( QKeyEvent * pEvent )
    {
        return pEvent && pEvent->key() == Qt::Key_Down;
    }

    static bool isInput( QKeyEvent * pEvent )
    {
        return pEvent && ( pEvent->key() == Qt::Key_Enter || pEvent->key() == Qt::Key_Return);
    }

    static bool isAutocomplete( QKeyEvent * pEvent )
    {
        return pEvent && ( pEvent->key() == Qt::Key_Tab);
    }
}

static void webLink()
{
    SEND_MSG("Web resource of the plugin: https://svlad-90.github.io/DLT-Message-Analyzer/");
}

static void supportedColors()
{
    SEND_MSG("Supported colors:");

    const auto& colorsMap = getColorsMap();

    for(const auto& colorItem : colorsMap)
    {
        SEND_MSG_COLORED(colorItem.first, colorItem.second);
    }
}

static void supportedStyles()
{
    SEND_MSG( QString("Current style: %1").arg(QApplication::style()->objectName()) );

    SEND_MSG("Supported styles:");

    const QStringList styles = QStyleFactory::keys();
    for(const QString& style : styles)
    {
        SEND_MSG(QString("- ").append(style));
    }
}

static void availableUMLPackages(const QString& packageName)
{
    SEND_MSG("Available UML packages:");

    auto packageList = DMA::PlantUML::Creator::getInstance().findPackagesByName(packageName.toStdString());
    for(const auto& pPackage : packageList)
    {
        SEND_MSG(QString("- ").append(QString::fromStdString(*pPackage)));
    }
}

static void printClassDiagram(const QString& packageName, bool excludeExternalDependencies)
{
    auto printPackage = [&excludeExternalDependencies](const QString& packageName_)
    {
        SEND_MSG(QString("Class diagram for package \"%1\":").arg(packageName_));

        auto diagramResult = DMA::PlantUML::Creator::getInstance().getPackageClassDiagram(packageName_.toStdString(),
                                                                                          excludeExternalDependencies);

        if(true == diagramResult.bIsSuccessful)
        {
            SEND_MSG(QString::fromStdString(diagramResult.diagramContent));
        }
        else
        {
            SEND_ERR(QString("Error during producing UML diagram: %1").arg(QString::fromStdString(diagramResult.error)));
        }
    };

    QString toLowerCandidate = packageName.toLower();

    if(toLowerCandidate == "all")
    {
        auto packageList = DMA::PlantUML::Creator::getInstance().findPackagesByName("");

        for(const auto& pPackageName : packageList)
        {
            if(nullptr != pPackageName)
            {
                printPackage(QString::fromStdString(*pPackageName));
            }
        }
    }
    else
    {
        printPackage(packageName);
    }
}

static void exportClassDiagram(const QString& dir, const QString& packageName, bool excludeExternalDependencies)
{
    auto exportPackage = [&dir, &excludeExternalDependencies](const QString& packageName_)
    {
        auto diagramResult = DMA::PlantUML::Creator::getInstance().getPackageClassDiagram(packageName_.toStdString(),
                                                                                          excludeExternalDependencies);
        if(true == diagramResult.bIsSuccessful)
        {
            QString fileName = packageName_;

            if(true == excludeExternalDependencies)
            {
                fileName.append("_standalone");
            }

            fileName.append(".puml");

            QString filePath = dir;
            filePath.append(QDir::separator()).append(fileName);

            QFile file( filePath );
            if ( true == file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text) )
            {
                QTextStream textStream(&file);
                textStream << QString::fromStdString( diagramResult.diagramContent );
                file.close();

                SEND_MSG(QString("[exportClassDiagram] Diagram \"%1\" was successfully exported").arg(fileName));
            }
            else
            {
                SEND_ERR(QString("Was not able to open or create file \"%1\"").arg(filePath));
            }
        }
        else
        {
            SEND_ERR(QString("Error during producing UML diagram: %1").arg(QString::fromStdString(diagramResult.error)));
        }
    };

    QString toLowerCandidate = packageName.toLower();

    if(toLowerCandidate == "all")
    {
        auto packageList = DMA::PlantUML::Creator::getInstance().findPackagesByName("");

        for(const auto& pPackageName : packageList)
        {
            if(nullptr != pPackageName)
            {
                exportPackage(QString::fromStdString(*pPackageName));
            }
        }
    }
    else
    {
        exportPackage(packageName);
    }
}

static void printAppClassDiagram()
{
    SEND_MSG(QString("Class diagram of application:"));

    auto diagramResult = DMA::PlantUML::Creator::getInstance().getClassDiagram();

    if(true == diagramResult.bIsSuccessful)
    {
        SEND_MSG(QString::fromStdString(diagramResult.diagramContent));
    }
    else
    {
        SEND_ERR(QString("Error during producing UML diagram: %1").arg(QString::fromStdString(diagramResult.error)));
    }
}

static void exportAppClassDiagram(const QString& dir)
{
    auto diagramResult = DMA::PlantUML::Creator::getInstance().getClassDiagram();

    if(true == diagramResult.bIsSuccessful)
    {
        QString filePath = dir;
        filePath.append(QDir::separator()).append("DMA_Full").append(".puml");

        QFile file( filePath );
        if ( true == file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text) )
        {
            QTextStream textStream(&file);
            textStream << QString::fromStdString( diagramResult.diagramContent );
            file.close();

            SEND_MSG(QString("[%1] Diagram \"DMA_Full.puml\" was successfully exported").arg(__FUNCTION__));
        }
        else
        {
            SEND_ERR(QString("Was not able to open or create file \"%1\"").arg(filePath));
        }
    }
    else
    {
        SEND_ERR(QString("Error during producing UML diagram: %1").arg(QString::fromStdString(diagramResult.error)));
    }
}

static void version()
{
    SEND_MSG(QString("%1. Version: %2. Author: %3")
             .arg(DLT_MESSAGE_ANALYZER_NAME)
             .arg(DLT_MESSAGE_ANALYZER_PLUGIN_VERSION)
             .arg(DLT_MESSAGE_ANALYZER_PLUGIN_AUTHOR));
}

static void support()
{
    SEND_MSG("In case of facing issues with the plugin, please, report here - https://github.com/svlad-90/DLT-Message-Analyzer/issues");
}

static void clear()
{
    CLEAR_CONSOLE_VIEW();
}

static void UML_sequence_identifiers()
{
    SEND_MSG("UML regex group identifiers, which are supported by the plugin:");

    for(const auto& item : s_UML_IDs_Map)
    {
        QString id_type_msg = QString("Type - ").append(getUMLIDTypeAsString(item.second.id_type));

        if(item.first == eUML_ID::UML_TIMESTAMP)
        {
            id_type_msg.append(" - if not specified, the dlt message timestamp is used");
        }

        if(item.second.id_type == eUML_ID_Type::e_RequestType)
        {
            id_type_msg.append(" - at least one of the request types should be filled in");
        }

        SEND_MSG(QString("[%1] : %2 | <%3>")
                 .arg( getUMLIDAsString( item.first ) )
                 .arg(item.second.description)
                 .arg(id_type_msg));
    }
}

static void plot_identifiers()
{
    SEND_MSG("Plot regex group identifiers, which are supported by the plugin:");

    for(const auto& item : sPlotViewIDsMap)
    {
        QString id_type_msg = QString("Type - ").append(getPlotIDTypeAsString(item.second.id_type));

        SEND_MSG(QString("[%1] : %2 | <%3>")
                     .arg( getPlotIDAsString( item.first ) )
                     .arg(item.second.description)
                     .arg(id_type_msg));
    }
}

static void plot_operations()
{
    SEND_MSG("How to work with the plot?");
    SEND_MSG("- Scroll - zoom horizontally");
    SEND_MSG("- Click+move - drag horizontally");
    SEND_MSG("- Shift+scroll - zoom vertically");
    SEND_MSG("- Shift+click+move - drag vertically");
    SEND_MSG("- Ctrl+scroll - change opacity");
    SEND_MSG("- 'F' key with having a selected point on the graph - filter/unfilter the selected graph");
    SEND_MSG("- Click on the legend item - hide/show the selected graph");
    SEND_MSG("- Ctrl+click on the legend item - move the selected graph to the font");
    SEND_MSG("- Space - reset the vertical scale of all charts");
    SEND_MSG("- Click on the graph point - jump to the corresponding message in the dlt-viewer's main table, and show the point details");
}

void CConsoleInputProcessor::printHelp(const QString& command)
{
    static const auto scenarioMap = createScenariosMap();

    if(true == command.isEmpty())
    {
        SEND_MSG("Supported commands:");

        for(const auto& scenarioItem : scenarioMap)
        {
            SEND_MSG(QString("- ")
                     .append(scenarioItem.first)
                     .append(" ")
                     .append(scenarioItem.second.comment));
        }

        SEND_MSG("Note! Parameters should be mentioned in \"-param=value\" or \"--param=value\" format");
    }
    else
    {
        auto foundCommand = scenarioMap.find(command);

        if(foundCommand != scenarioMap.end())
        {
            SEND_MSG(QString("- ")
                     .append(foundCommand->first)
                     .append(" ")
                     .append(foundCommand->second.comment));
        }
        else
        {
            SEND_ERR(QString("Command with name \"%1\" was not found. Print \"help\" to see list of available commands").arg(command));
        }
    }
}

static bool strToBool( const QString& str, bool& val )
{
    bool bResult = false;

    QString lowerCandidate = str.toLower();

    if(lowerCandidate == "true")
    {
        bResult = true;
        val = true;
    }
    else if(lowerCandidate == "false")
    {
        bResult = true;
        val = false;
    }
    else
    {
        bResult = false;
    }

    return bResult;
}

CConsoleInputProcessor::tScenariosMap CConsoleInputProcessor::createScenariosMap()
{
    CConsoleInputProcessor::tScenariosMap result;

    result[sHelpCommandName] = CConsoleInputProcessor::tScenarioData([this](const CConsoleInputProcessor::tParamMap& params)
    {
        auto foundCommandParam = params.find("c");

        QString command;

        if(foundCommandParam != params.end())
        {
            command = foundCommandParam->second;
        }

        printHelp(command);
    }, "[-c=<command-name>] - show this help. If no \"c\" parameter is provided - "
       "help regarding all available commands will be dumped. "
       "Be aware, that [<command-name> <help>] syntax can also be used to get the help output regarding a single command. "
       "Such syntax is easier to use, considering limited auto-complete functionality of this console. E.g. \"help -help\" (ha-ha).");

    result["clear"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap&){clear();},
                      "- clears debug view");
    result["color-aliases"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap&){supportedColors();}
                              , "- prints all supported color aliases");
    result["plot-ids"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap&){plot_identifiers();}
                                  , "- prints information about regex names scripting in area of the plot diagrams.");
    result["plot-operations"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap&){plot_operations();}
                                  , "- prints information about regex names scripting in area of the plot diagrams.");
    result["support"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap&){support();}
                        , "- prints information regarding how to get support");
    result["version"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap&){version();}
                        , "- prints version of the plugin");
    result["web-link"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap&){webLink();}
                         , "- prints URL with location of the plugin on the Git hub");
    result["uml-sequence-ids"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap&){UML_sequence_identifiers();}
                       , "- prints information about regex names scripting in area of the UML sequence diagrams");
    result["styles"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap&){supportedStyles();}
                       , "- prints information about QT styles supported on target OS");
    result["uml-packages"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap& params)
    {
        if(false == params.empty())
        {
            auto foundParam = params.find("p");

            if(foundParam != params.end())
            {
                availableUMLPackages(foundParam->second);
            }
            else
            {
                SEND_ERR("Command [uml-packages]: required parameter \"p\" not found!");
            }
        }
        else
        {
            availableUMLPackages("");
        }
    }, "[-p=<packageName> // part of case insensitive package name] - prints information about available UML packages. "
       "In case if \"package\" parameter is empty - will print info about all available packages. "
       "Obtained names can be used to build UML class diagrams using \"uml-print-class-diagram\" command");

    result["uml-print-class-diagram"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap& params)
    {
        if(false == params.empty())
        {
            auto foundPackageParam = params.find("p");

            if(foundPackageParam != params.end())
            {
                auto foundExcludeDepsParam = params.find("e");

                bool bExcludeDeps = false;

                if(foundExcludeDepsParam != params.end())
                {
                    if(strToBool(foundExcludeDepsParam->second, bExcludeDeps))
                    {
                        printClassDiagram(foundPackageParam->second, bExcludeDeps);
                    }
                    else
                    {
                        SEND_ERR("Command [uml-print-class-diagram]: non-boolean value provided in \"e\" parameter");
                    }
                }
                else
                {
                    printClassDiagram(foundPackageParam->second, bExcludeDeps);
                }
            }
            else
            {
                SEND_ERR("Command [uml-print-class-diagram]: required parameter \"p\" not found!");
            }
        }
        else
        {
            printAppClassDiagram();
        }
    }, "[-p=<packageName> // case sensitive name of the package. Can contain special \"all\" value.]"
       "[-e=<exclude-external-dependencies> // whether to exclude external dependencies] "
       "- prints class diagram of the whole application or of the dedicated package(s) to the console. "
       "In case if no parameters provided - the whole application's diagram will be dumped.");

    result["uml-export-class-diagram"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap& params)
    {
        if(params.size() > 1)
        {
            auto foundPackageParam = params.find("p");

            if(foundPackageParam != params.end())
            {
                auto foundDirParam = params.find("d");

                if(foundDirParam != params.end())
                {
                    QFile file( foundDirParam->second );
                    if ( true == file.exists() )
                    {
                        auto foundExcludeDepsParam = params.find("e");

                        bool bExcludeDeps = false;

                        if(foundExcludeDepsParam != params.end())
                        {
                            if(strToBool(foundExcludeDepsParam->second, bExcludeDeps))
                            {
                                exportClassDiagram(foundDirParam->second, foundPackageParam->second, bExcludeDeps);
                            }
                            else
                            {
                                SEND_ERR("Command [uml-export-class-diagram]: non-boolean value provided in \"e\" parameter");
                            }
                        }
                        else
                        {
                            exportClassDiagram(foundDirParam->second, foundPackageParam->second, bExcludeDeps);
                        }
                    }
                    else
                    {
                        SEND_ERR(QString("Command [uml-export-class-diagram]: Provided path does not exist - \"%1\"").arg(foundDirParam->second));
                    }
                }
                else
                {
                    SEND_ERR("Command [uml-export-class-diagram]: required parameter \"d\" not found!");
                }
            }
            else
            {
                SEND_ERR("Command [uml-export-class-diagram]: required parameter \"p\" not found!");
            }
        }
        else if(params.size() == 1)
        {
            auto foundDirParam = params.find("d");

            if(foundDirParam != params.end())
            {
                QFile file( foundDirParam->second );
                if ( true == file.exists() )
                {
                    exportAppClassDiagram(foundDirParam->second);
                }
                else
                {
                    SEND_ERR(QString("Command [uml-export-class-diagram]: Provided path does not exist - \"%1\"").arg(foundDirParam->second));
                }
            }
            else
            {
                SEND_ERR("Command [uml-export-class-diagram]: required parameter \"d\" not found!");
            }
        }
        else
        {
            SEND_ERR("Command [uml-export-class-diagram]: required parameter \"d\" not found!");
        }
    }, "[-d=<directory> // mandatory! Directory, to which store the the diagrams]"
       "[-p=<packageName> // case sensitive name of the package. Can contain special \"all\" value.]"
       "[-e=<exclude-external-dependencies> // whether to exclude external dependencies] "
       "- exports class diagram of the whole application or of the dedicated package(s) to the file-system. "
       "In case if no optional parameters provided - the whole application's diagram will be exported.");

    result["plantuml-settings"] = CConsoleInputProcessor::tScenarioData([this](const CConsoleInputProcessor::tParamMap&)
    {
        SEND_MSG( "Current plantuml settings:" );
        SEND_MSG( QString("Plantuml path mode: \"%1\"")
        .arg(getPathModeAsString( static_cast<ePathMode>(getSettingsManager()->getPlantumlPathMode()) ) ) );
        SEND_MSG( QString("Plantuml default path: \"%1\"")
        .arg(getSettingsManager()->getDefaultPlantumlPath()));
        SEND_MSG( QString("Plantuml custom path: \"%1\"")
        .arg(getSettingsManager()->getPlantumlCustomPath()));
        SEND_MSG( QString("Plantuml path environment variable: \"%1\"")
        .arg(getSettingsManager()->getPlantumlPathEnvVar()));

        const auto systemEnv = QProcessEnvironment::systemEnvironment();

        const auto& plantumlPathEnvVar = getSettingsManager()->getPlantumlPathEnvVar();

        if(true == systemEnv.contains(plantumlPathEnvVar))
        {
            auto plantUMLPath = systemEnv.value(plantumlPathEnvVar);

            SEND_MSG( QString("Plantuml path environment variable value: \"%1\"")
            .arg( plantUMLPath ));
        }
        else
        {
            SEND_MSG( QString("Plantuml path environment variable value: \"No such variable exist\""));
        }
    },
    "- prints information about the currently used plantuml settings");

    result["java-settings"] = CConsoleInputProcessor::tScenarioData([this](const CConsoleInputProcessor::tParamMap&)
    {
        SEND_MSG( "Current java settings:" );
        SEND_MSG( QString("Java path mode: \"%1\"")
        .arg(getPathModeAsString( static_cast<ePathMode>(getSettingsManager()->getJavaPathMode()) ) ) );
        SEND_MSG( QString("Java default path: \"%1\"")
        .arg(getSettingsManager()->getDefaultJavaPath()));
        SEND_MSG( QString("Java custom path: \"%1\"")
        .arg(getSettingsManager()->getJavaCustomPath()));
        SEND_MSG( QString("Java path environment variable: \"%1\"")
        .arg(getSettingsManager()->getJavaPathEnvVar()));

        const auto systemEnv = QProcessEnvironment::systemEnvironment();

        const auto& javaPathEnvVar = getSettingsManager()->getJavaPathEnvVar();

        if(true == systemEnv.contains(javaPathEnvVar))
        {
            auto javaUMLPath = systemEnv.value(javaPathEnvVar);

            SEND_MSG( QString("Java path environment variable value: \"%1\"")
            .arg( javaUMLPath ));
        }
        else
        {
            SEND_MSG( QString("Java path environment variable value: \"No such variable exist\""));
        }
    },
    "- prints information about the currently used java settings");

    result["convert-txt-to-dlt-file"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap& params)
    {
        if(false == params.empty())
        {
            auto foundSourceFileParam = params.find("sf");

            if(foundSourceFileParam != params.end())
            {
                auto foundTargetFileParam = params.find("tf");

                if(foundTargetFileParam != params.end())
                {
                    bool bConvertionResult = false;

                    auto foundVersionParam = params.find("version");

                    if(foundVersionParam != params.end())
                    {
                        if(foundVersionParam->second.toLower() == "v2")
                        {
                            bConvertionResult = convertLogFileToDLTV2(foundSourceFileParam->second, foundTargetFileParam->second);
                        }
                        else if(foundVersionParam->second.toLower() == "v1")
                        {
                            bConvertionResult = convertLogFileToDLTV1(foundSourceFileParam->second, foundTargetFileParam->second);
                        }
                        else
                        {
                            SEND_ERR(QString("Command [convert-txt-to-dlt-file]: Wrong value '%1' was passed for the parameter 'version'! "
                                             "Supported values are 'v1' and 'v2'.")
                                         .arg(foundVersionParam->second));
                        }
                    }
                    else
                    {
                        bConvertionResult = convertLogFileToDLTV2(foundSourceFileParam->second, foundTargetFileParam->second);
                    }

                    if(false == bConvertionResult)
                    {
                        SEND_ERR(QString("Command [convert-txt-to-dlt-file]: Failed to convert the \"%1\" file to dlt format!")
                                     .arg(foundSourceFileParam->second));
                    }
                    else
                    {
                        SEND_MSG("Command [convert-txt-to-dlt-file]: Conversion successfully performed!");
                    }
                }
                else
                {
                    SEND_ERR("Command [convert-txt-to-dlt-file]: required parameter \"tf\" not found!");
                }
            }
            else
            {
                SEND_ERR("Command [convert-txt-to-dlt-file]: required parameter \"sf\" not found!");
            }
        }
        else
        {
            SEND_ERR("Command [convert-txt-to-dlt-file]: required parameters \"sf\" and \"tf\" not found!");
        }
    },
    "- converts specified file with '\\n' separated set of strings to the dlt format"
    "[-sf=<source_file> // mandatory! Source file which we should convert to the dlt format]"
    "[-tf=<target file> // mandatory! Target file, into which we should save the content]"
    "[-v=<version> // optional! Version of the dlt protocol. Supported values are 'v1' and 'v2'. Default value is 'v2']");

#ifdef DMA_TC_MALLOC_PROFILING_ENABLED
    result["dump-memory-stats"] = CConsoleInputProcessor::tScenarioData([](const CConsoleInputProcessor::tParamMap&){dumpMemoryStatistics();}
                              , "- prints tcmalloc memory stats.");
#endif

    return result;
}

CConsoleInputProcessor::CConsoleInputProcessor( QLineEdit* pTargetLineEdit,
                                                const tSettingsManagerPtr& pSettingsManager ):
    CSettingsManagerClient(pSettingsManager),
    mpTargetLineEdit(pTargetLineEdit),
    mScenariosMap(createScenariosMap()),
    mHistory(),
    mCurrentHistoryItem(0),
    mbBorderReached(eHistoryBorderStatus::Not_Near_Border)
{
    if(nullptr != mpTargetLineEdit)
    {
        pTargetLineEdit->installEventFilter(this);
    }
}

CConsoleInputProcessor::~CConsoleInputProcessor()
{}

bool CConsoleInputProcessor::eventFilter(QObject* pObj, QEvent* pEvent)
{
    bool bResult = false;

    if (pEvent->type() == QEvent::KeyPress)
    {
        QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(pEvent);

        if(NShortcuts::isPrevHistoryItem(pKeyEvent))
        {
            if(nullptr != mpTargetLineEdit)
            {
                if(false == mHistory.empty())
                {
                    if(mbBorderReached == eHistoryBorderStatus::Reached_Last ||
                       mbBorderReached == eHistoryBorderStatus::Reached_Both)
                    {
                        mbBorderReached = eHistoryBorderStatus::Not_Near_Border;
                        mpTargetLineEdit->setText(mHistory[static_cast<std::size_t>(mCurrentHistoryItem)]);
                    }
                    else
                    {
                        if(mCurrentHistoryItem > 0)
                        {
                            QString text = mpTargetLineEdit->text();

                            if(false == text.isEmpty())
                            {
                                --mCurrentHistoryItem;
                            }

                            mpTargetLineEdit->setText(mHistory[static_cast<std::size_t>(mCurrentHistoryItem)]);
                        }
                        else
                        {
                            mpTargetLineEdit->setText("");
                            mbBorderReached = eHistoryBorderStatus::Reached_First;
                        }
                    }

                }
            }

            bResult = QObject::eventFilter(pObj, pEvent);
        }
        else if(NShortcuts::isNextHistoryItem(pKeyEvent))
        {
            if(nullptr != mpTargetLineEdit)
            {
                if(false == mHistory.empty())
                {
                    if(mbBorderReached == eHistoryBorderStatus::Reached_First)
                    {
                        mbBorderReached = eHistoryBorderStatus::Not_Near_Border;
                        mpTargetLineEdit->setText(mHistory[static_cast<std::size_t>(mCurrentHistoryItem)]);
                    }
                    else
                    {
                        if(mCurrentHistoryItem < static_cast<int>(mHistory.size() - 1))
                        {
                            QString text = mpTargetLineEdit->text();

                            if(false == text.isEmpty())
                            {
                                ++mCurrentHistoryItem;
                            }

                            mpTargetLineEdit->setText(mHistory[static_cast<std::size_t>(mCurrentHistoryItem)]);
                        }
                        else
                        {
                            mpTargetLineEdit->setText("");
                            mbBorderReached = eHistoryBorderStatus::Reached_Last;
                        }
                    }
                }
            }

            bResult = QObject::eventFilter(pObj, pEvent);
        }
        else if(NShortcuts::isAutocomplete(pKeyEvent))
        {
            if(nullptr != mpTargetLineEdit)
            {
                QString text = mpTargetLineEdit->text();

                std::set<QString> autocompletionMap;

                for(const auto& scenarioPair : mScenariosMap)
                {
                    bool bStartsWith = scenarioPair.first.startsWith(text);

                    if(true == bStartsWith)
                    {
                        autocompletionMap.insert(scenarioPair.first);
                    }
                }

                QString result = text;

                if(autocompletionMap.size() == 1)
                {
                    result = *autocompletionMap.begin();
                }
                else if(false == autocompletionMap.empty() || true == text.isEmpty())
                {
                    QString msg;

                    std::size_t counter = 0;

                    for(const auto& autocompletionItem : autocompletionMap)
                    {
                        msg.append(autocompletionItem);

                        if(counter < autocompletionMap.size() - 1)
                        {
                            msg.append(" | ");
                        }

                        ++counter;
                    }

                    SEND_MSG(msg);
                }

                if(false == result.isEmpty())
                {
                    mpTargetLineEdit->setText(result);
                }
            }

            bResult = true;
        }
        else if(NShortcuts::isInput(pKeyEvent))
        {
            if(nullptr != mpTargetLineEdit)
            {
                QString text = mpTargetLineEdit->text();

                if(!text.isEmpty())
                {
                    auto foundHistoryItem = std::find_if( mHistory.begin(), mHistory.end(), [&text](const QString& historyItem)->bool
                    {
                        return historyItem == text;
                    });

                    if(foundHistoryItem == mHistory.end())
                    {
                        mHistory.push_back(text);

                        if(mHistory.size() == 1)
                        {
                            mbBorderReached = eHistoryBorderStatus::Reached_Both;
                        }
                        else if(mHistory.size() > 1)
                        {
                            mbBorderReached = eHistoryBorderStatus::Not_Near_Border;
                        }

                        if(mHistory.size() > sMaxCommandsHistorySize)
                        {
                            mHistory.resize(sMaxCommandsHistorySize);
                        }
                    }
                    else
                    {
                        mHistory.erase(foundHistoryItem);
                        mHistory.push_back(text);
                    }
                }

                mCurrentHistoryItem = mHistory.size() - 1;
                mbBorderReached = eHistoryBorderStatus::Reached_Last;

                mpTargetLineEdit->clear();

                SEND_MSG("");
                SEND_MSG("========================================");
                SEND_MSG(QString("| ").append(text));

                // let's split input text into function name, and its parameters

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                auto stirngList = text.split(" -", QString::SplitBehavior::SkipEmptyParts);
#else
                auto stirngList = text.split(" -", Qt::SkipEmptyParts);
#endif



                if(false == stirngList.empty())
                {
                    QString lowerCandidate = stirngList[0].toLower();
                    stirngList.pop_front(); // erase name, as it is already copied

                    auto foundScenario = mScenariosMap.find(lowerCandidate);

                    if( foundScenario != mScenariosMap.end() )
                    {
                        // if command was found, let's parse paprameters

                        tParamMap paramMap;

                        for(const auto& paramCandidate : stirngList)
                        {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                            QStringList parsedParam = paramCandidate.split("=", QString::SplitBehavior::SkipEmptyParts);
#else
                            QStringList parsedParam = paramCandidate.split("=", Qt::SkipEmptyParts);
#endif

                            if(parsedParam.size() == 2)
                            {
                                paramMap[parsedParam[0]] = parsedParam[1].trimmed();
                            }
                            else if(parsedParam.size() == 1)
                            {
                                paramMap[parsedParam[0]] = "";
                            }
                        }

                        SEND_MSG("----------------------------------------");

                        auto foundHelpParameter = paramMap.find(sHelpCommandName);

                        if(foundHelpParameter == paramMap.end())
                        {
                            if(foundScenario->second.scenarioHandler)
                            {
                                foundScenario->second.scenarioHandler(paramMap);
                            }
                        }
                        else
                        {
                            auto foundHelpCommand = mScenariosMap.find(sHelpCommandName);

                            if(foundHelpCommand != mScenariosMap.end())
                            {
                                tParamMap helpParamMap;
                                helpParamMap["c"] = foundScenario->first;

                                if(foundHelpCommand->second.scenarioHandler)
                                {
                                    foundHelpCommand->second.scenarioHandler(helpParamMap);
                                }
                            }
                        }

                        if(lowerCandidate != "clear")
                        {
                            SEND_MSG("========================================");
                        }
                    }
                    else
                    {
                        SEND_MSG("----------------------------------------");
                        SEND_WRN(QString("Command \"%1\" not supported. Input \"help\" in order to get the list of supported commands.").arg(text));
                        SEND_MSG("========================================");
                    }
                }
            }

            bResult = QObject::eventFilter(pObj, pEvent);
        }
        else
        {
            bResult = QObject::eventFilter(pObj, pEvent);
        }
    }
    else
    {
        bResult = QObject::eventFilter(pObj, pEvent);
    }

    return bResult;
}

CConsoleInputProcessor::tScenarioData::tScenarioData(
        const CConsoleInputProcessor::tScenarioHandler& scenarioHandler_,
        const QString& comment_ ):
scenarioHandler(scenarioHandler_),
 comment(comment_)
{}

PUML_PACKAGE_BEGIN(DMA_Log)
    PUML_CLASS_BEGIN_CHECKED(CConsoleInputProcessor)
        PUML_INHERITANCE_CHECKED(QObject, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QLineEdit, 1, 1, console input)
    PUML_CLASS_END()
PUML_PACKAGE_END()
