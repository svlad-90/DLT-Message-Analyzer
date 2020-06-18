#ifndef CTABLEMEMORYJUMPER_HPP
#define CTABLEMEMORYJUMPER_HPP

#include <set>
#include <cstdint>

#include <QTableView>

/**
 * @brief The CTableMemoryJumper class - this class is used to:
 * - remember the selected line of the specified table by an integer key.
 * The integer key not always should be identical to the idx of the row within the target table.
 * That is up to the client, which meaning the key would have.
 * E.g. it might be the idx of the row within another table.
 * Within this project this class will be used to remember the id of selected DLT message within the "search view".
 * The used integer key would be the idx of row within the MAIN TABLE of the dlt-viewer.
 * - check remembered idx against the range of the provided the id-s, and if range contains the remembered message - jump to it.
 * In context of this project that would mean, that if user has selected some row in "search view" within the previous search session,
 * and the same row appeares in the next search result - we will jump to it.
 * That will make the investigation process more intuitive.
 */

class CTableMemoryJumper
{
public:
    typedef int tRowKey;
    typedef int tRowID;
    typedef std::pair<tRowKey /*key*/, tRowID /*row id within a target table*/> tCheckItem;
    typedef std::set<tCheckItem> tCheckSet;

    CTableMemoryJumper(QTableView* pTargetTable);

    void setSelectedRow( const tRowKey& selectedRowKey );
    void resetSelectedRow();
    void checkRows( const tCheckSet& checkSet );

private:
    QTableView* mpTargetTable;
    tRowKey mSelectedRowKey;
    bool mbRowSelected;
};

#endif // CTABLEMEMORYJUMPER_HPP
