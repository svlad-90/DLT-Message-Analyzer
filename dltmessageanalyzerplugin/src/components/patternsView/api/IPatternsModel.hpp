#pragma once

#include "QAbstractItemModel"
#include "QModelIndex"

#include "common/Definitions.hpp"

class IPatternsModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    IPatternsModel(QObject *parent);
    virtual ~IPatternsModel();

    virtual void updateView() = 0;
    virtual void resetData() = 0;
    virtual QModelIndex addData(const QString& alias, const QString& regex, Qt::CheckState isDefault = Qt::Unchecked) = 0;
    virtual QModelIndex addData(const QString& alias, const QString& regex, Qt::CheckState isCombine, Qt::CheckState isDefault) = 0;
    virtual void updatePatternsInPersistency() = 0;
    virtual void refreshRegexPatterns() = 0;

    struct tSearchResult
    {
        bool bFound { false };
        QModelIndex foundIdx;
    };
    /**
     * @brief search - searches pattern by its alias
     * @param alias - alias to be searched
     * @return - instance of tSearchResult, which provides result status of the search
     */
    virtual tSearchResult search( const QString& alias ) = 0;
    /**
     * @brief editData - edits pattern
     * @param idx - index of item to be edited
     * @param alias - new alias value
     * @param regex - new regex value
     * @param isDefault - new default value
     * @param isCombine - new combine value
     * @return - index of new item, or index of input "idx" parameter. Depends on type of change.
     * Anyway, client should avoid usage of input "idx" index, and stick to returned index instance after this call.
     */
    virtual QModelIndex editData(const QModelIndex& idx,
                                 const QString& alias,
                                 const QString& regex,
                                 Qt::CheckState isDefault, Qt::CheckState isCombine) = 0;

    virtual void removeData(const QModelIndex& idx) = 0;

    /**
     * @brief getAliasEditName - gets update name for given pattern
     * @param idx - input model index
     * @return - name to be used for edit purposes
     */
    virtual QString getAliasEditName( const QModelIndex& idx ) = 0;

    virtual void filterPatterns( const QString& filter ) = 0;

signals:
    void patternsRefreshed();
};
