#include "assert.h"

#include "CTableMemoryJumper.hpp"

static const CTableMemoryJumper::tRowKey sInvalidRowKey = -1;

CTableMemoryJumper::CTableMemoryJumper(QTableView* pTargetTable):
mpTargetTable(pTargetTable),
mSelectedRowKey(sInvalidRowKey),
mbRowSelected(false)
{}

void CTableMemoryJumper::setSelectedRow( const tRowKey& selectedRowKey )
{
    mSelectedRowKey = selectedRowKey;
    mbRowSelected = true;
}

void CTableMemoryJumper::resetSelectedRow()
{
    mSelectedRowKey = sInvalidRowKey;
    mbRowSelected = false;
}

void CTableMemoryJumper::checkRows( const tCheckSet& checkSet )
{
    if(true == mbRowSelected)
    {
        if(nullptr != mpTargetTable)
        {
            for(const auto& analysisItem : checkSet)
            {
                if(analysisItem.first == mSelectedRowKey)
                {
                    auto index = mpTargetTable->model()->index(analysisItem.second, 0);
                    mpTargetTable->scrollTo( index );
                    mpTargetTable->setCurrentIndex( index );
                    break;
                }
            }
        }
    }
}
