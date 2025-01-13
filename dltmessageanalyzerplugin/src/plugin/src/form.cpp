/**
 * @file    form.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the Form class
 */

#include "../api/form.h"
#include "ui_form.h"
#include "dltmessageanalyzerplugin.hpp"
#include "../api/CDLTMessageAnalyzer.hpp"
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

#include "components/settings/api/ISettingsManager.hpp"
#include "common/CBGColorAnimation.hpp"
#include "components/patternsView/api/CPatternsView.hpp"
#include "components/log/api/CLog.hpp"
#include "components/plant_uml/api/CUMLView.hpp"
#include "common/OSHelper.hpp"
#include "common/CQtHelper.hpp"

#include "DMA_Plantuml.hpp"

Form::Form(DLTMessageAnalyzerPlugin* pDLTMessageAnalyzerPlugin,
           const tSettingsManagerPtr& pSettingsManager,
           QWidget *parent) :
    QWidget(parent),
    CSettingsManagerClient(pSettingsManager),
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

        connect(mpUI->iconButton, &QPushButton::clicked, this, [this]()
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
        mpUI->searchView->setAutoScroll(true);
    }

    if(mpUI->CNSplitter_1)
    {
        QList<int> sizes;
        sizes.push_back(100);
        sizes.push_back(300);
        mpUI->CNSplitter_1->setSizes(sizes);
    }

    if(mpUI->CNSplitter_2)
    {
        QList<int> sizes;
        sizes.push_back(300);
        sizes.push_back(50);
        sizes.push_back(50);
        mpUI->CNSplitter_2->setSizes(sizes);
    }

    if(mpUI->CNItems)
    {
        mpUI->CNItems->resizeColumnsToContents();
    }

    auto showContextMenu = [this](const QPoint &pos)
    {
        QMenu contextMenu("Context menu", this);

        {
            QAction* pAction = new QAction("Write settings on each update", this);
            connect(pAction, &QAction::triggered, this, [this](bool checked)
            {
                getSettingsManager()->setWriteSettingsOnEachUpdate(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(getSettingsManager()->getWriteSettingsOnEachUpdate());
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Enable cache", this);
            connect(pAction, &QAction::triggered, this, [this](bool checked)
            {
                getSettingsManager()->setCacheEnabled(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(getSettingsManager()->getCacheEnabled());
            contextMenu.addAction(pAction);
        }

        {
            if(true == getSettingsManager()->getCacheEnabled())
            {
                QString msg = QString("Set cache size (cur. value - %1 Mb) ...").arg(getSettingsManager()->getCacheMaxSizeMB());

                QAction* pAction = new QAction(msg, this);
                connect(pAction, &QAction::triggered, this, [this]()
                {
                    tCacheSizeMB maxRAMSize = 0;

                    if(getRAMSize(maxRAMSize))
                    {
                        tCacheSizeMB inputVal;

                        bool bInputSuccess = getRangedArithmeticValue<tCacheSizeMB>(inputVal,
                                                                                    0,
                                                                                    maxRAMSize,
                                                                                    getSettingsManager()->getCacheMaxSizeMB(),
                                                                                    this,
                                                                                    "RAM cache size",
                                                                                    "RAM cache size",
                                                                                    [](bool* ok, const QString& str)->tCacheSizeMB
                                                                                    {
                                                                                        return str.toUInt(ok);
                                                                                    });

                        if(true == bInputSuccess)
                        {
                            getSettingsManager()->setCacheMaxSizeMB(inputVal);
                        }
                    }
                });
                contextMenu.addAction(pAction);
            }
        }

        {
            if(true == getSettingsManager()->getCacheEnabled())
            {
                QAction* pAction = new QAction("Reset cache ...", this);
                connect(pAction, &QAction::triggered, this, [this]()
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
            QAction* pAction = new QAction("Grouped view", this);
            connect(pAction, &QAction::triggered, this, [this](bool checked)
            {
                getSettingsManager()->setGroupedViewFeatureActive(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(getSettingsManager()->getGroupedViewFeatureActive());
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("PlantUML", this);
            connect(pAction, &QAction::triggered, this, [this](bool checked)
            {
                getSettingsManager()->setUML_FeatureActive(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(getSettingsManager()->getUML_FeatureActive());
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Plot view", this);
            connect(pAction, &QAction::triggered, this, [this](bool checked)
                    {
                        getSettingsManager()->setPlotViewFeatureActive(checked);
                    });
            pAction->setCheckable(true);
            pAction->setChecked(getSettingsManager()->getPlotViewFeatureActive());
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Open settings folder", this);
            connect(pAction, &QAction::triggered, this, [this]()
            {
                SEND_MSG(QString("[Form]: Attempt to open path - \"%1\"")
                         .arg(getSettingsManager()->getSettingsFilepath()));

                QDesktopServices::openUrl( QUrl::fromLocalFile( getSettingsManager()->getSettingsFilepath() ) );
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("RDP mode", this);
            connect(pAction, &QAction::triggered, this, [this](bool checked)
            {
                getSettingsManager()->setRDPMode(checked);
            });
            pAction->setCheckable(true);
            pAction->setChecked(getSettingsManager()->getRDPMode());
            contextMenu.addAction(pAction);
        }

        contextMenu.addSeparator();

        {
            QAction* pAction = new QAction("Set username ...", this);
            connect(pAction, &QAction::triggered, this, [this](bool)
            {
                QDialog dialog(this);
                // Use a layout allowing to have a label next to each field
                QFormLayout form(&dialog);

                // Add some text above the fields
                form.addRow(new QLabel("Set username"));

                // Add the lineEdits with their respective labels
                QList<QLineEdit *> fields;

                QLineEdit* usernameLineEdit = new QLineEdit(&dialog);

                {
                    usernameLineEdit->setText( getSettingsManager()->getUsername() );
                    QString label("Username:");
                    form.addRow(label, usernameLineEdit);
                    fields << usernameLineEdit;
                }

                dialog.resize(400, dialog.height());

                // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
                QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                           Qt::Horizontal, &dialog);
                form.addRow(&buttonBox);

                auto accept_handler = [this, &usernameLineEdit, &dialog]()
                {
                    if(usernameLineEdit)
                    {
                        getSettingsManager()->setUserName(usernameLineEdit->text());
                    }
                    dialog.accept();
                };

                auto reject_handler = [&dialog]()
                {
                    dialog.reject();
                };

                QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, accept_handler);
                QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, reject_handler);

                // Show the dialog as modal
                dialog.exec();
            });
            contextMenu.addAction(pAction);
        }

        contextMenu.exec(this->mapToGlobal(pos));
    };

    connect( this, &QWidget::customContextMenuRequested, showContextMenu );

    if(nullptr != mpUI->patternsTreeView)
    {
        connect(mpUI->patternsTreeView, &CPatternsView::patternSelected, this, [this]( const QString&, const QStringList& )
        {
            if(nullptr != mpDLTMessageAnalyzerPlugin)
            {
                if(true == getSettingsManager()->getMinimizePatternsViewOnSelection())
                {
                    hidePatternLogic(true);
                }
            }
        });
    }

    if(nullptr != mpUI->PNG_UML_View
    && nullptr != mpUI->createSequenceDiagram)
    {
        connect(mpUI->PNG_UML_View, &CUMLView::diagramGenerationStarted, this, [this]()
        {
            mpUI->createSequenceDiagram->setText("Cancel");
        });

        connect(mpUI->PNG_UML_View, &CUMLView::diagramGenerationFinished, this, [this](bool)
        {
            mpUI->createSequenceDiagram->setText("Create sequence diagram");
        });

        {
            auto pGroupedViewWidget = mpUI->tabWidget->findChild<QWidget*>(QString("groupedViewTab"));

            if(nullptr != pGroupedViewWidget)
            {
                auto enableGroupedViewWidget = [this, pGroupedViewWidget](bool val)
                {
                    if(nullptr != mpUI && nullptr != mpUI->tabWidget)
                    {
                        if(mpUI->tabWidget->count() > 0)
                        {
                            if(true != val)
                            {
                                mpUI->tabWidget->removeTab(mpUI->tabWidget->indexOf(pGroupedViewWidget));
                            }
                            else
                            {
                                mpUI->tabWidget->insertTab(static_cast<int>(eTabIndexes::GROUPED_VIEW), pGroupedViewWidget, "Grouped view");
                            }
                        }
                    }
                };

                enableGroupedViewWidget(getSettingsManager()->getGroupedViewFeatureActive());

                connect(getSettingsManager().get(), &ISettingsManager::groupedViewFeatureActiveChanged,
                        this, [enableGroupedViewWidget](bool val)
                {
                    enableGroupedViewWidget(val);
                });
            }
        }

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
                                mpUI->tabWidget->insertTab(true == getSettingsManager()->getGroupedViewFeatureActive() ?
                                                           static_cast<int>(eTabIndexes::UML_VIEW) :
                                                           static_cast<int>(eTabIndexes::UML_VIEW) - 1,
                                                           pUMLWidget, "UML View");
                            }
                        }
                    }
                };

                enableUMLWidget(getSettingsManager()->getUML_FeatureActive());

                connect(getSettingsManager().get(), &ISettingsManager::UML_FeatureActiveChanged,
                        this, [enableUMLWidget](bool val)
                {
                    enableUMLWidget(val);
                });
            }
        }

        {
            auto pPlotViewWidget = mpUI->tabWidget->findChild<QWidget*>(QString("plotViewTab"));

            if(nullptr != pPlotViewWidget)
            {
                auto enablePlotViewWidget = [this, pPlotViewWidget](bool val)
                {
                    if(nullptr != mpUI && nullptr != mpUI->tabWidget)
                    {
                        if(mpUI->tabWidget->count() > 0)
                        {
                            if(true != val)
                            {
                                mpUI->tabWidget->removeTab(mpUI->tabWidget->indexOf(pPlotViewWidget));
                            }
                            else
                            {
                                auto bGroupedViewFeatureActive = getSettingsManager()->getGroupedViewFeatureActive();
                                auto bUML_FeatureActive = getSettingsManager()->getUML_FeatureActive();
                                int tabIndex = static_cast<int>(eTabIndexes::PLOT_VIEW) - !bGroupedViewFeatureActive - !bUML_FeatureActive;
                                mpUI->tabWidget->insertTab(tabIndex, pPlotViewWidget, "Plot view");
                            }
                        }
                    }
                };

                enablePlotViewWidget(getSettingsManager()->getPlotViewFeatureActive());

                connect(getSettingsManager().get(), &ISettingsManager::plotViewFeatureActiveChanged,
                        this, [enablePlotViewWidget](bool val)
                {
                    enablePlotViewWidget(val);
                });
            }
        }
    }

    connect(mpUI->tabWidget, &QTabWidget::currentChanged, this, [this](int index)
    {
        int32_t plotViewTabIndex = -1;

        for (int i = 0; i < mpUI->tabWidget->count(); ++i)
        {
            if (mpUI->tabWidget->tabText(i) == "Plot view")
            {
                plotViewTabIndex = i;
                break;
            }
        }

        if(plotViewTabIndex >= 0)
        {
            if(index == plotViewTabIndex)
            {
                mpUI->plot->setFocus();
            }
        }
    });

    connect(mpUI->CreatePlotButton, &QPushButton::clicked, this, [this](bool)
    {
        mpUI->plot->setFocus();
    });

    QList<int> newSplitterSizes;
    newSplitterSizes.push_back(1000);
    newSplitterSizes.push_back(0);

    mpUI->splitter_tabWidget_filters->setSizes(newSplitterSizes);
    mpUI->UMLViewSplitter->setSizes(newSplitterSizes);
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

CRegexHistoryTextEdit* Form::getRegexTextEdit()
{
    CRegexHistoryTextEdit* pResult = nullptr;

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

CCustomPlotExtended* Form::getCustomPlot()
{
    return mpUI->plot;
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

CLogo* Form::getLogo()
{
    CLogo* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->iconButton;
    }

    return pResult;
}

QPushButton* Form::getUMLCreateDiagramFromTextButton()
{
    QPushButton* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->UMLCreateDiagramFromTextButton;
    }

    return pResult;
}

QPushButton* Form::getCreatePlotButton()
{
    QPushButton* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->CreatePlotButton;
    }

    return pResult;
}

QPlainTextEdit* Form::getUMLTextEditor()
{
    QPlainTextEdit* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->UMLTextEditor;
    }

    return pResult;
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

QTableView* Form::getCNItemsTableView()
{
    QTableView* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->CNItems;
    }

    return pResult;
}

QTextEdit* Form::getCNMessageTextEdit()
{
    QTextEdit* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->CNMessagesTextEdit;
    }

    return pResult;
}

QTextEdit* Form::getCNCommentTextEdit()
{
    QTextEdit* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->CNCommentTextEdit;
    }

    return pResult;
}

QTextEdit* Form::getCNRegexTextEdit()
{
    QTextEdit* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->CNRegexTextEdit;
    }

    return pResult;
}

QPushButton* Form::getCNUseRegexButton()
{
    QPushButton* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->CNUseRegexButton;
    }

    return pResult;
}

QLineEdit* Form::getCNCurrentFileLineEdit()
{
    QLineEdit* pResult = nullptr;

    if(mpUI)
    {
        pResult = mpUI->CNCurrentFileLineEdit;
    }

    return pResult;
}

PUML_PACKAGE_BEGIN(DMA_Plugin_API)
    PUML_CLASS_BEGIN_CHECKED(Form)
        PUML_INHERITANCE_CHECKED(QWidget, extends)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
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
        PUML_COMPOSITION_DEPENDENCY_CHECKED(CRegexHistoryTextEdit, 1, 1, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
