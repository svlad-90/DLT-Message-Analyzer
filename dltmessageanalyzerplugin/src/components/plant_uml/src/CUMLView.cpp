/**
 * @file    CUMLView.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CUMLView class
 */

#include "QFile"
#include "QTextStream"
#include "QDir"
#include "QMenu"
#include "QFileDialog"
#include "QFile"
#include "QInputDialog"
#include "QCoreApplication"

#include "CImageViewer.hpp"
#include "../api/CUMLView.hpp"
#include "components/settings/api/ISettingsManager.hpp"
#include "components/log/api/CLog.hpp"

#include "DMA_Plantuml.hpp"

static QString get_UML_Storage_Path(const QString& settingsFilePath)
{
    const QString result( settingsFilePath + QString("/uml/") );
    return result;
}

static QString get_UML_File_Name()
{
    static const QString result("sequence");
    return result;
}

static QString get_PNG_Extension()
{
    static const QString result("png");
    return result;
}

static QString get_SVG_Extension()
{
    static const QString result("svg");
    return result;
}

static QString get_PUML_Extension()
{
    static const QString result("puml");
    return result;
}

static QString get_UML_PUML_File_Name()
{
    static const QString result = get_UML_File_Name() + "." + get_PUML_Extension();
    return result;
}

static QString get_UML_PNG_File_Name()
{
    static const QString result = get_UML_File_Name() + "." + get_PNG_Extension();
    return result;
}

static QString get_UML_SVG_File_Name()
{
    static const QString result = get_UML_File_Name() + "." + get_SVG_Extension();
    return result;
}

static std::pair<QString /*command*/, QStringList /*arguments*/> get_PlantUML_Command(const QString& PUMLFilePath,
                                                                                      CUMLView::eDiagramExtension extension,
                                                                                      const int& maxRows)
{
    std::pair<QString /*command*/, QStringList /*arguments*/> result;

    result.first = "java";

    auto usedPixelsInt = maxRows * 3500 /*pixels*/; // we give here some really big value to ensure that the whole diagram will be rendered.

    const int minPixelsInt = 10000;

    if(usedPixelsInt < minPixelsInt)
    {
        usedPixelsInt = minPixelsInt;
    }

    QString plantUMLPath = QCoreApplication::applicationDirPath() + QDir::separator() + "plugins/plantuml.jar";

    QString extensionStr;

    switch(extension)
    {
        case CUMLView::eDiagramExtension::e_PNG:
        {
            extensionStr = get_PNG_Extension();
        }
            break;
        case CUMLView::eDiagramExtension::e_SVG:
        {
            extensionStr = get_SVG_Extension();
        }
            break;
    }

    result.second.push_back(QString("-DPLANTUML_LIMIT_SIZE=%1").arg(usedPixelsInt));
    result.second.push_back(QString("-jar"));
    result.second.push_back(QString("%1").arg(plantUMLPath));
    result.second.push_back(QString("-t%1").arg(extensionStr));
    result.second.push_back(QString("%1").arg(PUMLFilePath));

    return result;
}


CUMLView::CUMLView(QWidget *parent):
tParent(parent),
mbDiagramShown(false),
mpImageViewer(nullptr),
mpDiagramCreationSubProcess(nullptr),
mpSaveSVGSubProcess(nullptr),
mDiagramContent(),
mbDiagramGenerationInProgress(false),
mLastSelectedFolder(QString(".") + QDir::separator())
{
    mpImageViewer = new CImageViewer(this);
    setWidget(mpImageViewer);
}

void CUMLView::generateUMLDiagramInternal(const QString& diagramContent,
                                          eDiagramExtension extension,
                                          const tGenerateDiagramCallback& callback,
                                          tProcessPtr& pSubProcess,
                                          bool blocking)
{
    if(nullptr != pSubProcess)
    {
        pSubProcess.reset();
    }

    // check that target folder exists
    if(false == QDir(get_UML_Storage_Path(getSettingsManager()->getSettingsFilepath())).exists())
    {
        QDir().mkdir(get_UML_Storage_Path(getSettingsManager()->getSettingsFilepath()));
    }

    pSubProcess = std::make_shared<QProcess>(this);

    QString PUML_file_path = get_UML_Storage_Path(getSettingsManager()->getSettingsFilepath()) + get_UML_PUML_File_Name();

    QFile PUML_file(PUML_file_path);

    // check whether puml exists
    if(true == PUML_file.exists())
    {
        // if it exists - delete it
        bool bRemoveResult = PUML_file.remove();

        if(false == bRemoveResult)
        {
            SEND_WRN(QString("Was not able to remove file - \"%1\"").arg(PUML_file_path));
        }
    }

    // create new puml file
    if(PUML_file.open(QFile::OpenModeFlag::WriteOnly | QFile::OpenModeFlag::Text))
    {
        QTextStream textStream(&PUML_file);
        textStream << diagramContent;
        PUML_file.close();
    }

    QString result_file_path = get_UML_Storage_Path(getSettingsManager()->getSettingsFilepath()) + ( extension == eDiagramExtension::e_PNG ? get_UML_PNG_File_Name() : get_UML_SVG_File_Name() );

    QFile result_file(result_file_path);

    // check whether result file exists
    if(true == result_file.exists())
    {
        // if it exists - delete it
        bool bRemoveResult = result_file.remove();

        if(false == bRemoveResult)
        {
            SEND_WRN(QString("Was not able to remove file - \"%1\"").arg(result_file_path));
        }
    }

    auto pSubProcessWeak = std::weak_ptr<QProcess>(pSubProcess);

    connect(pSubProcess.get(),
            static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [callback, pSubProcessWeak](int exitCode, QProcess::ExitStatus exitStatus)
    {
        if(false == pSubProcessWeak.expired())
        {
            callback(exitCode, exitStatus);
        }
    });

    // call plantuml to generate PNG
    auto resCommand = get_PlantUML_Command(PUML_file_path, extension, getSettingsManager()->getUML_MaxNumberOfRowsInDiagram());
    pSubProcess->start(resCommand.first, resCommand.second);

    if(true == blocking)
    {
        pSubProcess->waitForFinished();
    }
}

void CUMLView::generateUMLDiagram(const QString& diagramContent)
{
    QString PNG_file_path = get_UML_Storage_Path(getSettingsManager()->getSettingsFilepath()) + get_UML_PNG_File_Name();

    mbDiagramShown = false;

    auto callback = [this, PNG_file_path, diagramContent](int exitCode, QProcess::ExitStatus exitStatus)
    {
        // view PNG
        if(0 == exitCode && exitStatus == QProcess::ExitStatus::NormalExit)
        {
            if(nullptr != mpImageViewer)
            {
                if(mpImageViewer->loadFile(PNG_file_path))
                {
                    mbDiagramShown = true;
                    mDiagramContent = diagramContent;
                }
                else
                {
                    mbDiagramShown = false;
                    SEND_ERR(QString("PNG viewer was not able to open file - \"%1\"").arg(PNG_file_path));
                }
            }

            diagramGenerationFinished(mbDiagramShown);
        }
        else
        {
            SEND_ERR(QString("Diagram creation error. Exit code - %1. Exit status %2").arg(exitCode).arg(exitStatus));
            SEND_ERR(mDiagramContent);
            diagramGenerationFinished(false);
        }

        mbDiagramGenerationInProgress = false;
    };

    generateUMLDiagramInternal(diagramContent, eDiagramExtension::e_PNG, callback, mpDiagramCreationSubProcess, false);

    // clear the view
    clearDiagram();

    diagramGenerationStarted();
    mbDiagramGenerationInProgress = true;
}

bool CUMLView::isDiagramGenerationInProgress() const
{
    return mbDiagramGenerationInProgress;
}

void CUMLView::cancelDiagramGeneration()
{
    if(true == mbDiagramGenerationInProgress)
    {
        mpDiagramCreationSubProcess.reset();
        mbDiagramGenerationInProgress = false;
        diagramGenerationFinished(false);
    }
}

void CUMLView::clearDiagram()
{
    if(nullptr != mpImageViewer)
    {
        mbDiagramShown = false;
        mpImageViewer->clear();
    }
}

bool CUMLView::isDiagramShown() const
{
    return mbDiagramShown;
}

void CUMLView::handleSettingsManagerChange()
{
    auto showContextMenu = [this](const QPoint &pos)
    {
        QMenu contextMenu("Context menu", this);

        {
            QAction* pAction = new QAction("Save as ...", this);
            connect(pAction, &QAction::triggered, [this]()
            {
                QString targetFilePath = QFileDialog::getSaveFileName(this, tr("Save as"),
                                                                mLastSelectedFolder + get_UML_File_Name(),
                                                                tr("Portable network graphics (*.png);;"
                                                                   "Plantuml (*.puml);;"
                                                                   "Scalable vector graphics (*.svg)"));

                if(false == targetFilePath.isEmpty())
                {
                    QFileInfo fileInfo(targetFilePath);

                    auto extension = fileInfo.suffix();

                    // try to remove the file if it exists
                    if(fileInfo.exists())
                    {
                        mLastSelectedFolder = fileInfo.dir().path() + QDir::separator();
                        QFile::remove(targetFilePath);
                    }

                    if(extension == get_PNG_Extension())
                    {
                        auto sourceFilePath = get_UML_Storage_Path(getSettingsManager()->getSettingsFilepath()) + get_UML_PNG_File_Name();

                        if(!QFile::copy(sourceFilePath, targetFilePath))
                        {
                            SEND_ERR(QString("Failed to copy file \"%1\" to \"%2\"").arg(sourceFilePath).arg(targetFilePath));
                        }
                    }
                    else if(extension == get_PUML_Extension())
                    {
                        auto sourceFilePath = get_UML_Storage_Path(getSettingsManager()->getSettingsFilepath()) + get_UML_PUML_File_Name();
                        if(!QFile::copy(sourceFilePath, targetFilePath))
                        {
                            SEND_ERR(QString("Failed to copy file \"%1\" to \"%2\"").arg(sourceFilePath).arg(targetFilePath));
                        }
                    }
                    else if(extension == get_SVG_Extension())
                    {
                        auto callback = [this, targetFilePath](int exitCode, QProcess::ExitStatus exitStatus)
                        {
                            // copy resulting SVG
                            if(0 == exitCode && exitStatus == QProcess::ExitStatus::NormalExit)
                            {
                                auto sourceFilePath = get_UML_Storage_Path(getSettingsManager()->getSettingsFilepath()) + get_UML_SVG_File_Name();
                                if(!QFile::copy(sourceFilePath, targetFilePath))
                                {
                                    SEND_ERR(QString("Failed to copy file \"%1\" to \"%2\"").arg(sourceFilePath).arg(targetFilePath));
                                }
                            }
                            else
                            {
                                SEND_ERR(QString("Diagram creation error. Exit code - %1. Exit status %2").arg(exitCode).arg(exitStatus));
                                diagramGenerationFinished(false);
                            }

                            mpSaveSVGSubProcess.reset();
                        };

                        generateUMLDiagramInternal(mDiagramContent, eDiagramExtension::e_SVG, callback, mpSaveSVGSubProcess, true);
                    }
                }
            });

            pAction->setEnabled(mbDiagramShown);
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        if(true == mbDiagramShown)
        {
            QAction* pAction = new QAction("Clear diagram", this);
            connect(pAction, &QAction::triggered, [this]()
            {
                if(true == mbDiagramShown)
                {
                    mbDiagramShown = false;
                    mDiagramContent.clear();
                    clearDiagram();
                }
            });

            pAction->setEnabled(mbDiagramShown);
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QMenu* pSubMenu = new QMenu("UML settings", this);

            {
                {
                    QAction* pAction = new QAction("Show arguments", this);
                    connect(pAction, &QAction::triggered, [this](bool checked)
                    {
                        getSettingsManager()->setUML_ShowArguments(checked);
                    });
                    pAction->setCheckable(true);
                    pAction->setChecked(getSettingsManager()->getUML_ShowArguments());

                    pSubMenu->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Wrap output", this);
                    connect(pAction, &QAction::triggered, [this](bool checked)
                    {
                        getSettingsManager()->setUML_WrapOutput(checked);
                    });
                    pAction->setCheckable(true);
                    pAction->setChecked(getSettingsManager()->getUML_WrapOutput());

                    pSubMenu->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Autonumber", this);
                    connect(pAction, &QAction::triggered, [this](bool checked)
                    {
                        getSettingsManager()->setUML_Autonumber(checked);
                    });
                    pAction->setCheckable(true);
                    pAction->setChecked(getSettingsManager()->getUML_Autonumber());

                    pSubMenu->addAction(pAction);
                }

                {
                    QAction* pAction = new QAction("Max rows number ...", this);
                    connect(pAction, &QAction::triggered, [this]()
                    {
                        bool ok;

                        QString maxRowsStr = QInputDialog::getText(  nullptr, "Max rows number",
                                   "Set max rows number in diagram:",
                                   QLineEdit::Normal,
                                   QString::number(getSettingsManager()->getUML_MaxNumberOfRowsInDiagram()), &ok );

                        if(true == ok)
                        {
                            auto maxRows = maxRowsStr.toInt(&ok);

                            if(true == ok)
                            {
                                getSettingsManager()->setUML_MaxNumberOfRowsInDiagram(maxRows);
                            }
                        }
                    });

                    pSubMenu->addAction(pAction);
                }
            }

            contextMenu.addMenu(pSubMenu);
        }

        contextMenu.exec(mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );
}

PUML_PACKAGE_BEGIN(DMA_PlantumlView_API)
    PUML_CLASS_BEGIN_CHECKED(CUMLView)
        PUML_INHERITANCE_CHECKED(QWidget, extends)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CImageViewer, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(QProcess, 1, 2, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
