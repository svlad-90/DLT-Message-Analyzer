#include "assert.h"

#include "QScrollBar"

#include "CTableMemoryJumper.hpp"

#include "DMA_Plantuml.hpp"

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

                    auto pHorizontalScrollBar = mpTargetTable->horizontalScrollBar();
                    auto scrollPosition = 0;

                    if(nullptr != pHorizontalScrollBar)
                    {
                        scrollPosition = mpTargetTable->horizontalScrollBar()->value();
                    }

                    mpTargetTable->scrollTo( index );

                    mpTargetTable->setCurrentIndex( index );

                    if(nullptr != pHorizontalScrollBar)
                    {
                        mpTargetTable->horizontalScrollBar()->setValue(scrollPosition);
                    }

                    break;
                }
            }
        }
    }
}

PUML_PACKAGE_BEGIN(DMA_Common)
    PUML_CLASS_BEGIN_CHECKED(CTableMemoryJumper)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTableView, 1, 1, contains)
    PUML_CLASS_END()
PUML_PACKAGE_END()
