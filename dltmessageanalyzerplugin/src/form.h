/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file form.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTableView>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QPlainTextEdit>

class DLTMessageAnalyzerPlugin;
class CSearchResultView;
class CGroupedView;
class CPatternsView;
class CFiltersView;

namespace Ui {
    class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(DLTMessageAnalyzerPlugin* pDLTMessageAnalyzerPlugin, QWidget *parent = nullptr);
    ~Form() override;

    CGroupedView* getGroupedResultView();
    QProgressBar* getProgresBar();
    QLabel* getProgresBarLabel();
    QLineEdit* getRegexLineEdit();
    QLabel* getErrorLabel();
    CPatternsView* getPatternsTableView();
    QComboBox* getNumberOfThreadsComboBox();
    CSearchResultView* getSearchResultTableView();
    QCheckBox* getContinuousSearchCheckBox();
    QTextEdit* getFilesLineEdit();
    QWidget* getSearchResultsTab();
    QWidget* getGroupedViewTab();
    QWidget* getFilesViewTab();
    QWidget* getConsoleViewTab();
    QPlainTextEdit* getConsoleView();
    QTabWidget* getMainTabWidget();
    QPushButton* getAnalyzeButton();
    QLabel* getCacheStatusLabel();
    QLineEdit* getPatternSearchInput();
    QComboBox* getConfigComboBox();
    CFiltersView* getFiltersView();
    QLineEdit* getFiltersSearchInput();
    QLineEdit* getConsoleViewInput();

private slots:
    void on_analyze_clicked();
    void on_regex_returnPressed();
    void on_pushButton_clicked();
    void on_hidePatterns_clicked();
    void on_splitter_patterns_results_splitterMoved(int pos, int index);
    void on_searchView_doubleClicked(const QModelIndex &index);
    void hidePatternLogic(bool userTriggered);
    void on_continuousAnalysis_clicked(bool checked);
    void on_numberOfThreads_currentIndexChanged(int index);

    void on_patternsSearchInput_textChanged(const QString& filter);

    void on_configComboBox_currentIndexChanged(int index);

    void on_filtersSearchInput_textChanged(const QString &filter);

private:
    Ui::Form *mpUI;
    DLTMessageAnalyzerPlugin* mpDLTMessageAnalyzerPlugin;

    QList<int> mSavedSplitterSizes;
    bool mPatternsHidden;
};

#endif // FORM_H
