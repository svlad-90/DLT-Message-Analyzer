/**
 * @file    CSearchResultView.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CSearchResultView.hpp class
 */
#pragma once

#include <QTableView>
#include "common/Definitions.hpp"

class CSearchResultModel;

class CSearchResultView : public QTableView
{
    Q_OBJECT

    typedef QTableView tParent;

public:
    explicit CSearchResultView(QWidget *parent = nullptr);

    void setFile( const tFileWrapperPtr& pFile );
    virtual void setModel(QAbstractItemModel *model) override;
    void copySelectionToClipboard( bool copyAsHTML, bool copyOnlyPayload ) const;
    void newSearchStarted();
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;

signals:
    void searchRangeChanged( const tIntRangeProperty& searchRange, bool bReset );
    void clearSearchResultsRequested();
    void restartSearch();

protected:
    void verticalScrollbarAction(int action) override;
    void currentChanged(const QModelIndex &current,
                          const QModelIndex &previous) override;
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;
    virtual void keyPressEvent ( QKeyEvent * event ) override;

private:
    bool isVerticalScrollBarVisible() const;
    void updateColumnsVisibility();
    void getUserSearchRange();
    void copyMessageFiles();
    void switchToNextUMLItem(bool bNext);
    void selectAllUMLItems(bool select);
    void updateWidthLogic(const int& rowFrom, const int& rowTo);

    typedef std::set<eSearchResultColumn> tUpdateWidthSet;

    void updateWidth(bool force, tUpdateWidthSet updateWidthSet = tUpdateWidthSet());
    void forceUpdateWidthAndResetContentMap();

    eSearchResultColumn getLastVisibleColumn() const;

private:

    bool mbIsVerticalScrollBarVisible;
    bool mbIsViewFull;
    bool mbUserManuallyAdjustedLastVisibleColumnWidth;
    tFileWrapperPtr mpFile;
    tIntRangeProperty mSearchRange;
    CSearchResultModel* mpSpecificModel;

    typedef std::map<eSearchResultColumn, int /*max size of content*/> tContentSizeMap;
    tContentSizeMap mContentSizeMap;
};
