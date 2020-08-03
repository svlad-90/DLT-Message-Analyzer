/**
 * @file    CSearchResultHighlightingDelegate.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CSearchResultHighlightingDelegate class
 */

#include "QPainter"
#include "QDebug"
#include "QApplication"

#include "CSearchResultHighlightingDelegate.hpp"
#include "../common/Definitions.hpp"
#include "CSearchResultModel.hpp"
#include "../settings/CSettingsManager.hpp"

CSearchResultHighlightingDelegate::CSearchResultHighlightingDelegate():
mbMarkTimestampWithBold(CSettingsManager::getInstance()->getMarkTimeStampWithBold())
{
    connect( CSettingsManager::getInstance().get(),
             &CSettingsManager::markTimeStampWithBoldChanged,
             [this](bool val)
    {
        mbMarkTimestampWithBold = val;
    });
}

CSearchResultHighlightingDelegate::~CSearchResultHighlightingDelegate()
{

}

void drawText(const QString& inputStr,
              QPainter *painter,
              const QStyleOptionViewItem &option,
              bool bold)
{
    int baseShift = 0;

    if(Qt::AlignmentFlag::AlignLeft == option.displayAlignment)
    {
        baseShift = 2;
    }

    QRect rect = option.rect;

    //set pen color
    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());

        QPalette::ColorGroup cg = QPalette::Normal;
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    }
    else
    {
        QPalette::ColorGroup cg = QPalette::Normal;
        painter->setPen(option.palette.color(cg, QPalette::Text));

        if(true == bold)
        {
            if(false == painter->font().bold())
            {
                auto font = painter->font();
                font.setBold(true);
                painter->setFont(font);
            }
        }
        else
        {
            if(true == painter->font().bold())
            {
                auto font = painter->font();
                font.setBold(false);
                painter->setFont(font);
            }
        }
    }

    painter->drawText(QRect(rect.left()+baseShift, rect.top(), rect.width(), rect.height()),
                             option.displayAlignment, inputStr);
}

static void drawHighlightedText(eSearchResultColumn field,
                    const tFoundMatchesPackItem& foundMatchPackItem,
                    const QString& inputStr,
                    QPainter *painter,
                    const QStyleOptionViewItem &option)
{
    struct tDrawData
    {
        QString subStr;
        bool isHighlighted = false;
        bool isBold = false;
        QColor color;
        QPalette::ColorGroup colorGroup;
        int shift = 0;
    };

    typedef QVector<tDrawData> tDrawDataList;

    struct tDrawDataPack
    {
        Qt::Alignment alignment;
        tDrawDataList drawDataList;
        int baseShift = 0;
        bool isSelected = false;
    };

    tDrawDataPack drawDataPack;

    drawDataPack.alignment = option.displayAlignment;

    const auto& highlightingInfo = foundMatchPackItem.getItemMetadata().highlightingInfoMultiColor;
    auto foundHighlightingInfoItem = highlightingInfo.find(field);

    if(highlightingInfo.end() != foundHighlightingInfoItem)
    {
        const auto& highlightingData = foundHighlightingInfoItem.value();

        if(false == highlightingData.empty())
        {
            if (option.state & QStyle::State_Selected)
            {
                drawDataPack.isSelected = true;
            }

            auto collectDrawData = [&inputStr, &drawDataPack, &option](const tHighlightingRange& range, bool isHighlighted)
            {
                tDrawData drawData;

                QString subStr = inputStr.mid( range.from, range.to - range.from + 1 );
                drawData.subStr = subStr;

                if (true == drawDataPack.isSelected)
                {
                    drawData.colorGroup = QPalette::Normal;
                    drawData.color = option.palette.color(QPalette::Normal, QPalette::HighlightedText);

                    if(true == isHighlighted)
                    {
                        drawData.isBold = true;
                        drawData.isHighlighted = true;
                    }
                    else
                    {
                        drawData.isBold = false;
                        drawData.isHighlighted = false;
                    }
                }
                else
                {
                    drawData.colorGroup = QPalette::Normal;

                    if(true == isHighlighted)
                    {
                        bool isMonoColorHighlighting = CSettingsManager::getInstance()->getSearchResultMonoColorHighlighting();
                        bool isExplicitColor = range.explicitColor;

                        if(true == isExplicitColor)
                        {
                            drawData.color = range.color;
                        }
                        else
                        {
                            if(true == isMonoColorHighlighting)
                            {
                                drawData.color = CSettingsManager::getInstance()->getRegexMonoHighlightingColor();
                            }
                            else
                            {
                                drawData.color = range.color;
                            }
                        }

                        drawData.isBold = true;
                    }
                    else
                    {
                        drawData.color = QColor(0,0,0);
                        drawData.isBold = false;
                    }
                }

                drawDataPack.drawDataList.push_back(drawData);
            };

            auto calculateShifts = [&drawDataPack, &painter, &option]()
            {
                int shift = 0;

                for(auto& drawDataItem : drawDataPack.drawDataList)
                {
                    auto font = painter->font();

                    if(drawDataItem.isBold)
                    {
                        font.setBold(true);
                    }
                    else
                    {
                        font.setBold(false);
                    }

                    drawDataItem.shift = shift;
                    QFontMetrics fm(font);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    shift += fm.width(drawDataItem.subStr);
#else
    shift += fm.horizontalAdvance(drawDataItem.subStr);
#endif

                     //qDebug() << "drawDataItem.shift - " << drawDataItem.shift;
                }

                if(Qt::AlignmentFlag::AlignCenter == drawDataPack.alignment)
                {
                    QRect rect = option.rect;
                    drawDataPack.baseShift = (( rect.width() -  shift ) / 2 );
                }
                else
                {
                    drawDataPack.baseShift = 2;
                }

                //qDebug() << "drawDataPack.baseShift - " << drawDataPack.baseShift;
            };

            auto drawText = [&drawDataPack, &option, &painter]()
            {
                if(true == drawDataPack.isSelected)
                {
                    painter->fillRect(option.rect, option.palette.highlight());
                }

                for(const auto& drawDataItem : drawDataPack.drawDataList)
                {
                    const QRect& rect = option.rect;

                    painter->setPen(drawDataItem.color);

                    auto font = painter->font();

                    if(drawDataItem.isBold)
                    {
                        font.setBold(true);
                    }
                    else
                    {
                        font.setBold(false);
                    }

                    painter->setFont(font);

                    //qDebug() << "rect.left() - " << rect.left() << "; drawDataPack.baseShift - " << drawDataPack.baseShift << "; drawDataItem.shift - " << drawDataItem.shift;

                    painter->drawText(QRect(rect.left()+drawDataPack.baseShift+drawDataItem.shift, rect.top(), rect.width(), rect.height()),
                                      drawDataPack.alignment == Qt::AlignCenter ? Qt::AlignLeft : drawDataPack.alignment, drawDataItem.subStr);
                }
            };

            int i = 0;
            for(auto it = highlightingData.begin(); it != highlightingData.end(); ++it)
            {
                const auto& range = *it;

                if(0 == i)
                {
                    if(0 != range.from)
                    {
                        collectDrawData( tHighlightingRange(0, range.from - 1, range.color, range.explicitColor), false );
                    }

                    collectDrawData( tHighlightingRange( range.from, range.to, range.color, range.explicitColor ), true );
                }
                else if(0 < i)
                {
                    auto itPrev = it;
                    --itPrev;
                    const auto& prevRange = *(itPrev);

                    if(prevRange.to < range.from)
                    {
                        collectDrawData( tHighlightingRange(prevRange.to + 1, range.from - 1, range.color, range.explicitColor), false );
                    }

                    collectDrawData( tHighlightingRange( range.from, range.to, range.color, range.explicitColor ), true );
                }

                if(i == static_cast<int>(highlightingData.size() - 1)) // last element
                {
                    if( range.to < inputStr.size() - 1 )
                    {
                        collectDrawData( tHighlightingRange(range.to + 1, inputStr.size() - 1, range.color, range.explicitColor), false );
                    }
                }

                ++i;
            }

            calculateShifts();
            drawText();
        }
        else
        {
            drawText(inputStr,painter,option,false);
        }
    }
    else
    {
        drawText(inputStr,painter,option,false);
    }

    {
        painter->setPen(QColor(0,0,0));
        auto font = painter->font();
        font.setBold(false);
        painter->setFont(font);
    }
}

void CSearchResultHighlightingDelegate::paint(QPainter *painter,
           const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const auto* pModel = qobject_cast<const CSearchResultModel*>(index.model());

    if(nullptr != pModel)
    {
        auto stringData = pModel->data(index, Qt::DisplayRole).value<QString>();

        QStyleOptionViewItem opt = option;

        initStyleOption(&opt, index);

        auto field = static_cast<eSearchResultColumn>(index.column());

        bool UML_Applicability = index.sibling(index.row(), static_cast<int>(eSearchResultColumn::UML_Applicability)).data().value<bool>();

        if(true == UML_Applicability)
        {
            painter->fillRect(opt.rect, QBrush(QColor(200,200,200)));
        }

        switch(static_cast<eSearchResultColumn>(index.column()))
        {
            case eSearchResultColumn::Apid:
            case eSearchResultColumn::Ctid:
            case eSearchResultColumn::Payload:
            {
                const auto& matchData = pModel->getFoundMatchesItemPack(index);
                drawHighlightedText(field, matchData, stringData, painter, opt);
            }
                break;
            case eSearchResultColumn::Args:
            case eSearchResultColumn::Last:
            case eSearchResultColumn::Mode:
            case eSearchResultColumn::Time:
            case eSearchResultColumn::Type:
            case eSearchResultColumn::Count:
            case eSearchResultColumn::Ecuid:
            case eSearchResultColumn::Index:
            case eSearchResultColumn::Subtype:
            case eSearchResultColumn::SessionId:
            {
                drawText( stringData, painter, opt, false );
            }
            break;
            case eSearchResultColumn::Timestamp:
            {
                drawText( stringData, painter, opt, mbMarkTimestampWithBold );
            }
            break;
            case eSearchResultColumn::UML_Applicability:
            {
                QStyleOptionViewItem viewItemOption = option;

                const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
                QRect newRect = QStyle::alignedRect(viewItemOption.direction, Qt::AlignCenter,
                                                    QSize(viewItemOption.decorationSize.width() + 5,option.decorationSize.height()),
                                                    QRect(viewItemOption.rect.x() + textMargin, option.rect.y(),
                                                          viewItemOption.rect.width() - (2 * textMargin), viewItemOption.rect.height()));
                viewItemOption.rect = newRect;

                QStyledItemDelegate::paint(painter, viewItemOption, index);
            }
            break;
        }
    }
}

QSize CSearchResultHighlightingDelegate::sizeHint(const QStyleOptionViewItem &option,
               const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}
