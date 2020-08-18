/**
 * @file    CSearchResultView.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CSearchResultView.hpp class
 */
#ifndef CSEARCHRESULTVIEW_HPP
#define CSEARCHRESULTVIEW_HPP

#include <QTableView>
#include "../common/Definitions.hpp"

class CSearchResultModel;

class CSearchResultView : public QTableView
{
    Q_OBJECT

    typedef QTableView tParent;

public:
    explicit CSearchResultView(QWidget *parent = nullptr);

    void setFile( const tDLTFileWrapperPtr& pFile );
    virtual void setModel(QAbstractItemModel *model) override;

    void copySelectionToClipboard( bool copyAsHTML, bool copyOnlyPayload ) const;

signals:
    void searchRangeChanged( const tIntRangeProperty& searchRange, bool bReset );
    void clearSearchResultsRequested();
    void restartSearch();

protected:
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;
    virtual void keyPressEvent ( QKeyEvent * event ) override;

private:
    bool isVerticalScrollBarVisible() const;
    void updateColumnsVisibility();
    void getUserSearchRange();
    void copyMessageFiles();
    void switchToNextUMLItem(bool bNext);
    void selectAllUMLItems(bool select);
    void updateWidth();

private:
    enum class eUpdateRequired
    {
        eUpdateRequired_NO,
        eUpdateRequired_BE_READY,
        eUpdateRequired_REQUIRED
    };

    eUpdateRequired mWidthUpdateRequired;
    bool mbIsVerticalScrollBarVisible;
    tDLTFileWrapperPtr mpFile;
    tIntRangeProperty mSearchRange;
    CSearchResultModel* mpSpecificModel;
};

#endif // CSEARCHRESULTVIEW_HPP
