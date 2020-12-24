/**
 * @file    form.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the Form class
 */

#include "form.h"
#include "ui_form.h"
#include "dltmessageanalyzerplugin.hpp"
#include "CDLTMessageAnalyzer.hpp"
#include "QKeyEvent"
#include "QScrollBar"
#include "QDebug"
#include "QMenu"
#include "QAction"
#include "QInputDialog"
#include "QSequentialAnimationGroup"
#include "QPropertyAnimation"
#include "QFormLayout"
#include "QDialogButtonBox"
#include "QDesktopServices"

#include "settings/CSettingsManager.hpp"
#include "common/CBGColorAnimation.hpp"
#include "components/patternsView/api/CPatternsView.hpp"
#include "components/log/api/CLog.hpp"
#include "components/plant_uml/api/CUMLView.hpp"
#include "common/OSHelper.hpp"
#include "common/CQtHelper.hpp"

#include "DMA_Plantuml.hpp"

Form::Form(DLTMessageAnalyzerPlugin* pDLTMessageAnalyzerPlugin, QWidget *parent) :
    QWidget(parent),
    mpUI(new Ui::Form),
    mpDLTMessageAnalyzerPlugin(pDLTMessageAnalyzerPlugin),
    mSavedSplitterSizes(),
    mPatternsHidden(false)
{
    mpUI->setupUi(this);

    mpUI->tabWidget->setFocusPolicy(Qt::ClickFocus);

    if(mpUI->tabWidget->count() > 0)
    {
        mpUI->tabWidget->setCurrentIndex(0);
    }

    {
        mpUI->error->setStyleSheet("QLabel { background-color: gray;"
                                   "border: 3px solid white; }");
    }

    {
        mpUI->progressLabel->setStyleSheet("QLabel { background-color: gray;"
                                           "border: 3px solid white; }");
    }

    {
        mpUI->iconButton->setStyleSheet("QPushButton { "
                                        "qproperty-icon: url(:/dltmessageanalyzer/png/manuscript.png); }");

        QPalette palette = mpUI->iconButton->palette();
        palette.setColor( QPalette::Button, QColor(125,125,125) );
        mpUI->iconButton->setPalette(palette);

        mpUI->iconButton->setText("");

        connect(mpUI->iconButton, &QPushButton::clicked, [this]()
        {
            if(nullptr != mpDLTMessageAnalyzerPlugin)
            {
                mpUI->error->setText(mpDLTMessageAnalyzerPlugin->pluginData());
            }

            mpUI->error->setStyleSheet("QLabel { background-color: gray;"
                                       "color: black;"
                                       "border: 3px solid white; }");

            CBGColorAnimation* pAnimationProxy = new CBGColorAnimation(mpUI->iconButton, QPalette::Button);

            QSequentialAnimationGroup* pAnimSeq = new QSequentialAnimationGroup();

            QPropertyAnimation *pColor = new QPropertyAnimation(pAnimationProxy, "color");
            pColor->setDuration(250);
            pColor->setStartValue(QColor(125,125,125));
            pColor->setEndValue(QColor(0, 0, 0));

            pAnimSeq->addAnimation(pColor);

            QPropertyAnimation *pUncolor = new QPropertyAnimation(pAnimationProxy, "color");
            pUncolor->setDuration(250);
            pUncolor->setStartValue(QColor(0, 0, 0));
            pUncolor->setEndValue(QColor(125,125,125));

            pAnimSeq->addAnimation(pUncolor);

            pAnimSeq->start();
        });
    }

    {
        mpUI->progressBar->setStyleSheet("QProgressBar {"
                                         "border: 2px solid grey;"
                                         "border-radius: 5px;"
                                         "text-align: center;"
                                         "}"

                                         "QProgressBar::chunk {"
                                         "background-color: #05B8CC;"
                                         "width: 20px;"
                                         "}");
    }

    {
        mpUI->cacheStatusNameLabel->setStyleSheet("QLabel { background-color: gray;"
                                           "border: 3px solid white; }");

        mpUI->cacheStatusLabel->setStyleSheet("QLabel { background-color: gray;"
                                           "border: 3px solid white; }");
    }

    {
        mpUI->patternsTreeView->header()->setVisible(true);
        mpUI->patternsTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        mpUI->searchView->horizontalHeader()->setVisible(true);
        mpUI->groupedView->header()->setVisible(true);
    }

    auto showContextMenu = [this](const QPoint &pos)
    {
        QMenu contextMenu("Context menu", this);

        {
            QAction* pAction = new QAction("Write settings on each update", this);
            connect(pAction, &QAction::triggered, [](bool checked)
            {
                CSettingsManager::getInstance()->setWriteSettingsOnEachUpdate(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(CSettingsManager::getInstance()->getWriteSettingsOnEachUpdate());
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Enable cache", this);
            connect(pAction, &QAction::triggered, [](bool checked)
            {
                CSettingsManager::getInstance()->setCacheEnabled(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(CSettingsManager::getInstance()->getCacheEnabled());
            contextMenu.addAction(pAction);
        }

        {
            if(true == CSettingsManager::getInstance()->getCacheEnabled())
            {
                QString msg = QString("Set cache size (cur. value - %1 Mb) ...").arg(CSettingsManager::getInstance()->getCacheMaxSizeMB());

                QAction* pAction = new QAction(msg, this);
                connect(pAction, &QAction::triggered, [this]()
                {
                    tCacheSizeMB maxRAMSize = 0;

                    if(getRAMSize(maxRAMSize))
                    {
                        tCacheSizeMB inputVal;

                        bool bInputSuccess = getRangedArithmeticValue<tCacheSizeMB>(inputVal,
                                                                                    0,
                                                                                    maxRAMSize,
                                                                                    CSettingsManager::getInstance()->getCacheMaxSizeMB(),
                                                                                    this,
                                                                                    "RAM cache size",
                                                                                    "RAM cache size",
                                                                                    [](bool* ok, const QString& str)->tCacheSizeMB
                                                                                    {
                                                                                        return str.toUInt(ok);
                                                                                    });

                        if(true == bInputSuccess)
                        {
                            CSettingsManager::getInstance()->setCacheMaxSizeMB(inputVal);
                        }
                    }
                });
                contextMenu.addAction(pAction);
            }
        }

        {
            if(true == CSettingsManager::getInstance()->getCacheEnabled())
            {
                QAction* pAction = new QAction("Reset cache ...", this);
                connect(pAction, &QAction::triggered, [this]()
                {
                    if(nullptr != mpDLTMessageAnalyzerPlugin)
                    {
                        mpDLTMessageAnalyzerPlugin->resetCache();
                    }
                });
                contextMenu.addAction(pAction);
            }
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("PlantUML", this);
            connect(pAction, &QAction::triggered, [](bool checked)
            {
                CSettingsManager::getInstance()->setUML_FeatureActive(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(CSettingsManager::getInstance()->getUML_FeatureActive());
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Open settings folder", this);
            connect(pAction, &QAction::triggered, []()
            {
                SEND_MSG(QString("[Form]: Attempt to open path - \"%1\"")
                         .arg(CSettingsManager::getInstance()->getSettingsFilepath()));

                QDesktopServices::openUrl( QUrl::fromLocalFile( CSettingsManager::getInstance()->getSettingsFilepath() ) );
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("RDP mode", this);
            connect(pAction, &QAction::triggered, [](bool checked)
            {
                CSettingsManager::getInstance()->setRDPMode(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(CSettingsManager::getInstance()->getRDPMode());
            contextMenu.addAction(pAction);
        }

        contextMenu.exec(this->mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );

    if(nullptr != mpUI->patternsTreeView)
    {
        connect(mpUI->patternsTreeView, &CPatternsView::patternSelected, [this]( const QString& )
        {
            if(nullptr != mpDLTMessageAnalyzerPlugin)
            {
                if(true == CSettingsManager::getInstance()->getMinimizePatternsViewOnSelection())
                {
                    hidePatternLogic(true);
                }
            }
        });
    }

    if(nullptr != mpUI->PNG_UML_View
    && nullptr != mpUI->createSequenceDiagram)
    {
        connect(mpUI->PNG_UML_View, &CUMLView::diagramGenerationStarted, [this]()
        {
            mpUI->createSequenceDiagram->setText("Cancel");
        });

        connect(mpUI->PNG_UML_View, &CUMLView::diagramGenerationFinished, [this](bool)
        {
            mpUI->createSequenceDiagram->setText("Create sequence diagram");
        });

        {
            auto pUMLWidget = mpUI->tabWidget->findChild<QWidget*>(QString("UMLView"));

            if(nullptr != pUMLWidget)
            {
                auto enableUMLWidget = [this, pUMLWidget](bool val)
                {
                    if(nullptr != mpUI && nullptr != mpUI->tabWidget)
                    {
                        if(mpUI->tabWidget->count() > 0)
                        {
                            if(true != val)
                            {
                                mpUI->tabWidget->removeTab(mpUI->tabWidget->indexOf(pUMLWidget));
                            }
                            else
                            {
                                mpUI->tabWidget->insertTab(3, pUMLWidget, "UML View");
                            }
                        }
                    }
                };

                enableUMLWidget(CSettingsManager::getInstance()->getUML_FeatureActive());

                connect(CSettingsManager::getInstance().get(), &CSettingsManager::UML_FeatureActiveChanged, [enableUMLWidget](bool val)
                {
                    enableUMLWidget(val);
                });
            }
        }
    }

    QList<int> newSplitterSizes;
    newSplitterSizes.push_back(1000);
    newSplitterSizes.push_back(0);
    mpUI->splitter_tabWidget_filters->setSizes(newSplitterSizes);
}

Form::~Form()
{
    delete mpUI;
}

void Form::on_analyze_clicked()
{
    if(nullptr != mpDLTMessageAnalyzerPlugin)
    {
        mpDLTMessageAnalyzerPlugin->analysisTrigger();
    }
}

CGroupedView* Form::getGroupedResultView()
{
    CGroupedView* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->groupedView;
    }

    return pResult;
}

QProgressBar* Form::getProgresBar()
{
    QProgressBar* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->progressBar;
    }

    return pResult;
}

QLabel* Form::getProgresBarLabel()
{
    QLabel* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->progressLabel;
    }

    return pResult;
}

QLineEdit* Form::getRegexLineEdit()
{
    QLineEdit* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->regex;
    }

    return pResult;
}

QLabel* Form::getErrorLabel()
{
    QLabel* pLabel = nullptr;

    if(mpUI)
    {
        pLabel = mpUI->error;
    }

    return pLabel;
}

CPatternsView* Form::getPatternsTableView()
{
    CPatternsView* pTableView = nullptr;

    if(mpUI)
    {
        pTableView = mpUI->patternsTreeView;
    }

    return pTableView;
}

CSearchResultView* Form::getSearchResultTableView()
{
    CSearchResultView* pTableView = nullptr;

    if(mpUI)
    {
        pTableView = mpUI->searchView;
    }

    return pTableView;
}


QComboBox* Form::getNumberOfThreadsComboBox()
{
    QComboBox* pComboBox = nullptr;

    if(mpUI)
    {
        pComboBox = mpUI->numberOfThreads;
    }

    return pComboBox;
}

QCheckBox* Form::getContinuousSearchCheckBox()
{
    QCheckBox* pCheckBox = nullptr;

    if(mpUI)
    {
        pCheckBox = mpUI->continuousAnalysis;
    }

    return pCheckBox;
}

QWidget* Form::getSearchResultsTab()
{
    QWidget* pWidgetTab = nullptr;

    if(mpUI)
    {
        pWidgetTab = mpUI->searchViewTab;
    }

    return pWidgetTab;
}

QWidget* Form::getGroupedViewTab()
{
    QWidget* pWidgetTab = nullptr;

    if(mpUI)
    {
        pWidgetTab = mpUI->groupedViewTab;
    }

    return pWidgetTab;
}

QWidget* Form::getFilesViewTab()
{
    QWidget* pWidgetTab = nullptr;

    if(mpUI)
    {
        pWidgetTab = mpUI->filesViewTab;
    }

    return pWidgetTab;
}

QWidget* Form::getConsoleViewTab()
{
    QWidget* pWidgetTab = nullptr;

    if(mpUI)
    {
        pWidgetTab = mpUI->consoleViewTab;
    }

    return pWidgetTab;
}

QPlainTextEdit* Form::getConsoleView()
{
    QPlainTextEdit* pPlainTextEdit = nullptr;

    if(mpUI)
    {
        pPlainTextEdit = mpUI->consoleView;
    }

    return pPlainTextEdit;
}

QTextEdit* Form::getFilesLineEdit()
{
    QTextEdit* pFilesTextEdit = nullptr;

    if(mpUI)
    {
        pFilesTextEdit = mpUI->filesTextEdit;
    }

    return pFilesTextEdit;
}

QTabWidget*  Form::getMainTabWidget()
{
    QTabWidget* pTabWidget = nullptr;

    if(mpUI)
    {
        pTabWidget = mpUI->tabWidget;
    }

    return pTabWidget;
}

QPushButton* Form::getAnalyzeButton()
{
    QPushButton* pButton = nullptr;

    if(mpUI)
    {
        pButton = mpUI->analyze;
    }

    return pButton;
}

QLabel* Form::getCacheStatusLabel()
{
    QLabel* pLabel = nullptr;

    if(mpUI)
    {
        pLabel = mpUI->cacheStatusLabel;
    }

    return pLabel;
}

QLineEdit* Form::getPatternSearchInput()
{
    QLineEdit* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->patternsSearchInput;
    }

    return pResult;
}

QComboBox* Form::getConfigComboBox()
{
    QComboBox* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->configComboBox;
    }

    return pResult;
}

CFiltersView* Form::getFiltersView()
{
    CFiltersView* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->filtersTreeView;
    }

    return pResult;
}

QLineEdit* Form::getFiltersSearchInput()
{
    QLineEdit* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->filtersSearchInput;
    }

    return pResult;
}

QLineEdit* Form::getConsoleViewInput()
{
    QLineEdit* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->consoleViewInputLineEdit;
    }

    return pResult;
}

CUMLView* Form::getUMLView()
{
    CUMLView* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->PNG_UML_View;
    }

    return pResult;
}

QPushButton* Form::getCreateSequenceDiagramButton()
{
    QPushButton* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->createSequenceDiagram;
    }

    return pResult;
}

void Form::on_regex_returnPressed()
{
    if(nullptr != mpDLTMessageAnalyzerPlugin)
    {
        mpDLTMessageAnalyzerPlugin->analyze();
    }
}

void Form::on_pushButton_clicked()
{
    if(nullptr != mpDLTMessageAnalyzerPlugin)
    {
        mpDLTMessageAnalyzerPlugin->exportToHTML();
    }
}

void Form::hidePatternLogic(bool userTriggered)
{
    if(nullptr != mpUI->splitter_patterns_results &&
            nullptr != mpUI->hidePatterns)
    {
        if(false == mPatternsHidden)
        {
            mSavedSplitterSizes = mpUI->splitter_patterns_results->sizes();
            if(2 == mSavedSplitterSizes.size())
            {
                QList<int> newSplitterSizes;
                newSplitterSizes.push_back(0);
                newSplitterSizes.push_back(mSavedSplitterSizes[0] + mSavedSplitterSizes[1]);
                mpUI->splitter_patterns_results->setSizes(newSplitterSizes);
                mpUI->hidePatterns->setText(">>");
                mPatternsHidden = true;
            }
        }
        else
        {
            const int defaultPatternsWidth = 400;

            if(true == mSavedSplitterSizes.empty())
            {
                auto width = mpUI->splitter_patterns_results->width();
                mSavedSplitterSizes.push_back(defaultPatternsWidth);
                mSavedSplitterSizes.push_back(width-defaultPatternsWidth);
            }

            if(true == userTriggered)
            {
                if(mSavedSplitterSizes[0] < defaultPatternsWidth)
                {
                    mSavedSplitterSizes[0] = defaultPatternsWidth;
                }
            }

            mpUI->splitter_patterns_results->setSizes(mSavedSplitterSizes);
            mpUI->hidePatterns->setText("<<");
            mPatternsHidden = false;

            mSavedSplitterSizes.clear();
        }
    }
}

void Form::on_hidePatterns_clicked()
{
    hidePatternLogic(true);
}

void Form::on_splitter_patterns_results_splitterMoved(int, int)
{
    if(nullptr != mpUI->splitter_patterns_results)
    {
        if(true == mPatternsHidden && mpUI->splitter_patterns_results->sizes().operator[](0) > 0)
        {
            mSavedSplitterSizes = mpUI->splitter_patterns_results->sizes();
            hidePatternLogic(false);
        }
    }
}

void Form::on_searchView_doubleClicked(const QModelIndex &index)
{
    if(nullptr != mpDLTMessageAnalyzerPlugin)
    {
        mpDLTMessageAnalyzerPlugin->searchView_clicked_jumpTo_inMainTable(index);
    }
}

void Form::on_continuousAnalysis_clicked(bool checked)
{
    if(nullptr != mpDLTMessageAnalyzerPlugin)
    {
        mpDLTMessageAnalyzerPlugin->triggerContinuousAnalysis(checked);
    }
}

void Form::on_numberOfThreads_currentIndexChanged(int)
{
    if(nullptr != mpDLTMessageAnalyzerPlugin)
    {
        mpDLTMessageAnalyzerPlugin->triggerNumberOfThreads();
    }
}

void Form::on_patternsSearchInput_textChanged(const QString& filter)
{
    if(nullptr != mpDLTMessageAnalyzerPlugin)
    {
        mpDLTMessageAnalyzerPlugin->filterPatterns( filter );
    }
}

void Form::on_configComboBox_currentIndexChanged(int index)
{
    if(index >= 0)
    {
        if(nullptr != mpDLTMessageAnalyzerPlugin)
        {
            mpDLTMessageAnalyzerPlugin->triggerRegexConfig();
        }
    }
}

void Form::on_filtersSearchInput_textChanged(const QString &filter)
{
    if(nullptr != mpDLTMessageAnalyzerPlugin)
    {
        mpDLTMessageAnalyzerPlugin->filterRegexTokens( filter );
    }
}

void Form::on_createSequenceDiagram_clicked()
{
    if(nullptr != mpDLTMessageAnalyzerPlugin)
    {
        if(false == mpUI->PNG_UML_View->isDiagramGenerationInProgress())
        {
            mpDLTMessageAnalyzerPlugin->createSequenceDiagram();
        }
        else
        {
            mpUI->PNG_UML_View->cancelDiagramGeneration();
        }
    }
}

PUML_PACKAGE_BEGIN(DMA_Root)
    PUML_CLASS_BEGIN_CHECKED(Form)
        PUML_INHERITANCE_CHECKED(QWidget, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(Ui::Form, 1, 1, contains)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(DLTMessageAnalyzerPlugin, 1, 1, uses)
    PUML_CLASS_END()

    PUML_CLASS_BEGIN_CHECKED(Ui::Form)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CFiltersView, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CPatternsView, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CConsoleView, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CGroupedView, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CSearchResultView, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CUMLView, 1, 1, contains)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CLogo, 1, 1, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
