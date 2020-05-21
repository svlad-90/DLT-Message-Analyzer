#ifndef CFILTERSVIEW_HPP
#define CFILTERSVIEW_HPP

#include <QTreeView>

#include "Definitions.hpp"

class CFiltersModel;
class CRegexTreeRepresentationDelegate;

class CFiltersView : public QTreeView
{
    Q_OBJECT

    typedef QTreeView tParent;

public:
    CFiltersView(QWidget *parent = nullptr);
    ~CFiltersView() override;

    void setSpecificModel( CFiltersModel* pModel );
    void highlightInvalidRegex(const QModelIndex &index);

signals:
    void regexRangeSelectionRequested( const tRange& range );
    void returnPressed();

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void keyPressEvent ( QKeyEvent * event ) override;

private:
    void setModel(QAbstractItemModel *model) override;
    void updateColumnsVisibility();
    void updateWidth();
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;

private: // fields
    CFiltersModel* mpModel;

    enum class eUpdateRequired
    {
        eUpdateRequired_NO,
        eUpdateRequired_BE_READY,
        eUpdateRequired_REQUIRED
    };

    eUpdateRequired mWidthUpdateRequired;
    bool mbIsVerticalScrollBarVisible;
    CRegexTreeRepresentationDelegate * mpRepresentationDelegate;
    bool mbResizeOnExpandCollapse;
    bool mbSkipFirstUpdateWidth;
};

#endif // CFILTERSVIEW_HPP
