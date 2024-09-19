/**
 * @file    CSearchResultHighlightingDelegate.cpp
 * @author  vgoncharuk
 * @brief   Implementation of the CSearchResultHighlightingDelegate class
 */

#include "QPainter"
#include "QDebug"
#include "QApplication"

#include "CSearchResultHighlightingDelegate.hpp"
#include "common/Definitions.hpp"
#include "../api/ISearchResultModel.hpp"
#include "components/settings/api/ISettingsManager.hpp"

#include "DMA_Plantuml.hpp"

//#define DEBUG_CSearchResultHighlightingDelegate

#ifdef DEBUG_CSearchResultHighlightingDelegate
#include "../log/CLog.hpp"
#include <QElapsedTimer>
#endif

const QColor sCustomHighlightingColor(237,235,178);

CSearchResultHighlightingDelegate::CSearchResultHighlightingDelegate(QObject *parent):
QStyledItemDelegate(parent),
mbMarkTimestampWithBold(false),
mSearchResultColumnsSearchMap()
{
}

CSearchResultHighlightingDelegate::~CSearchResultHighlightingDelegate()
{

}

void drawText(const QString& inputStr,
              QPainter *painter,
              const QStyleOptionViewItem &option,
              bool bold,
              bool bCustomBackgroundHighlighting)
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
        if(false == bCustomBackgroundHighlighting)
        {
            if(isDarkMode())
            {
                painter->fillRect(option.rect, option.palette.window());
            }
            else
            {
               painter->fillRect(option.rect, QColor(255,255,255));
            }
        }
        else
        {
            painter->fillRect(option.rect, sCustomHighlightingColor);
        }

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

static void collectDrawData(const QString& inputStr,
                            tDrawDataPack& drawDataPack,
                            const QStyleOptionViewItem &option,
                            const tHighlightingRange& range,
                            bool isHighlighted,
                            bool isMonoColorHighlighting,
                            const QColor& regexMonoHighlightingColor)
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
            bool isExplicitColor = range.explicitColor;

            if(true == isExplicitColor)
            {
                drawData.color = range.color_code;
            }
            else
            {
                if(true == isMonoColorHighlighting)
                {
                    drawData.color = regexMonoHighlightingColor;
                }
                else
                {
                    drawData.color = range.color_code;
                }
            }

            drawData.isBold = true;
        }
        else
        {
            drawData.color = option.palette.text().color();
            drawData.isBold = false;
        }
    }

    drawDataPack.drawDataList.push_back(drawData);
}

static int calculateShifts( tDrawDataPack& drawDataPack,
                             const QStyleOptionViewItem& option )
{
    int shift = 0;

    auto font = option.font;

    for(auto& drawDataItem : drawDataPack.drawDataList)
    {
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

    return shift;
}

static void drawText( const tDrawDataPack& drawDataPack,
                      QPainter *painter,
                      const QStyleOptionViewItem& option,
                      bool bCustomBackgroundHighlighting )
{
    if(true == drawDataPack.isSelected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    else
    {
        if(false == bCustomBackgroundHighlighting)
        {
            if(isDarkMode())
            {
                painter->fillRect(option.rect, option.palette.window());
            }
            else
            {
               painter->fillRect(option.rect, QColor(255,255,255));
            }
        }
        else
        {
            painter->fillRect(option.rect, sCustomHighlightingColor);
        }
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
                          drawDataPack.alignment == Qt::AlignCenter ?
                                                    static_cast<Qt::AlignmentFlag>((Qt::AlignLeft | Qt::AlignVCenter).operator QFlags<Qt::AlignmentFlag>::Int()) :
                                                    drawDataPack.alignment, drawDataItem.subStr);
    }
}

static void collectDrawDataPack(const QString& inputStr,
                                const tHighlightingRangeVec& highlightingData,
                                tDrawDataPack& drawDataPack,
                                const QStyleOptionViewItem& option,
                                bool isMonoColorHighlighting,
                                const QColor& regexMonoHighlightingColor)
{
    if (option.state & QStyle::State_Selected)
    {
        drawDataPack.isSelected = true;
    }

    tHighlightingRangeSet highlightingRangeSet(highlightingData.begin(), highlightingData.end());

    int i = 0;
    for(auto it = highlightingRangeSet.begin(); it != highlightingRangeSet.end(); ++it)
    {
        const auto& range = *it;

        if(0 == i)
        {
            if(0 != range.from)
            {
                collectDrawData( inputStr,
                                 drawDataPack,
                                 option,
                                 tHighlightingRange(0, range.from - 1, range.color_code, range.explicitColor),
                                 false,
                                 isMonoColorHighlighting,
                                 regexMonoHighlightingColor);
            }

            collectDrawData( inputStr,
                             drawDataPack,
                             option,
                             tHighlightingRange( range.from, range.to, range.color_code, range.explicitColor ),
                             true,
                             isMonoColorHighlighting,
                             regexMonoHighlightingColor );
        }
        else if(0 < i)
        {
            auto itPrev = it;
            --itPrev;
            const auto& prevRange = *(itPrev);

            if(prevRange.to < range.from)
            {
                collectDrawData( inputStr,
                                 drawDataPack,
                                 option,
                                 tHighlightingRange(prevRange.to + 1, range.from - 1, range.color_code, range.explicitColor),
                                 false,
                                 isMonoColorHighlighting,
                                 regexMonoHighlightingColor );
            }

            collectDrawData( inputStr,
                             drawDataPack,
                             option,
                             tHighlightingRange( range.from, range.to, range.color_code, range.explicitColor ),
                             true,
                             isMonoColorHighlighting,
                             regexMonoHighlightingColor );
        }

        if(i == static_cast<int>(highlightingRangeSet.size() - 1)) // last element
        {
            if( range.to < inputStr.size() - 1 )
            {
                collectDrawData( inputStr,
                                 drawDataPack,
                                 option,
                                 tHighlightingRange(range.to + 1, inputStr.size() - 1, range.color_code, range.explicitColor),
                                 false,
                                 isMonoColorHighlighting,
                                 regexMonoHighlightingColor );
            }
        }

        ++i;
    }
}

static void drawHighlightedText(eSearchResultColumn field,
                    const tFoundMatchesPackItem& foundMatchPackItem,
                    const QString& inputStr,
                    QPainter *painter,
                    const QStyleOptionViewItem& option,
                    bool isMonoColorHighlighting,
                    const QColor& regexMonoHighlightingColor,
                    bool bCustomBackgroundHighlighting)
{
    tDrawDataPack drawDataPack;

    drawDataPack.alignment = option.displayAlignment;

    const auto& highlightingInfo = foundMatchPackItem.getItemMetadata().highlightingInfoMultiColor;
    auto foundHighlightingInfoItem = highlightingInfo.find(field);

    if(highlightingInfo.end() != foundHighlightingInfoItem)
    {
        const auto& highlightingData = foundHighlightingInfoItem.value();

        if(false == highlightingData.empty())
        {
#ifdef DEBUG_CSearchResultHighlightingDelegate
            QElapsedTimer timer;
            timer.start();
#endif

            collectDrawDataPack(inputStr,
                                highlightingData,
                                drawDataPack,
                                option,
                                isMonoColorHighlighting,
                                regexMonoHighlightingColor);

#ifdef DEBUG_CSearchResultHighlightingDelegate
            SEND_MSG(QString("collectDrawData - %1").arg(timer.elapsed()));
            timer.restart();
#endif
            static_cast<void>(calculateShifts(drawDataPack, option));
#ifdef DEBUG_CSearchResultHighlightingDelegate
            SEND_MSG(QString("calculateShifts - %1").arg(timer.elapsed()));
            timer.restart();
#endif
            drawText(drawDataPack, painter, option, bCustomBackgroundHighlighting);
#ifdef DEBUG_CSearchResultHighlightingDelegate
            SEND_MSG(QString("drawText - %1").arg(timer.elapsed()));
            timer.invalidate();
#endif
        }
        else
        {
            drawText(inputStr, painter, option, false, bCustomBackgroundHighlighting);
        }
    }
    else
    {
        drawText(inputStr, painter, option, false, bCustomBackgroundHighlighting);
    }

    {
        painter->setPen(option.palette.text().color());
        auto font = painter->font();
        font.setBold(false);
        painter->setFont(font);
    }
}

void CSearchResultHighlightingDelegate::paint(QPainter *painter,
           const QStyleOptionViewItem &option, const QModelIndex &index) const
{
#ifdef DEBUG_CSearchResultHighlightingDelegate
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    SEND_MSG(QString("CSearchResultHighlightingDelegate::paint(start:<index:row-%1:col-%2>)")
             .arg(index.row())
             .arg(index.column()));
#endif

    //SEND_MSG(QString("CSearchResultHighlightingDelegate::paint: row %1").arg(index.row()));

    const auto* pModel = dynamic_cast<const ISearchResultModel*>(index.model());

    if(nullptr != pModel)
    {
        auto stringData = pModel->data(index, Qt::DisplayRole).value<QString>();

        bool bCustomBackgroundHighlighting = false;

        const auto& customHighlightingRows = pModel->getHighlightedRows();

        const auto& matchData = pModel->getFoundMatchesItemPack(index);
        const auto& msgId = matchData.getItemMetadata().msgId;

        if(customHighlightingRows.find(msgId) != customHighlightingRows.end())
        {
            bCustomBackgroundHighlighting = true;
        }

        QStyleOptionViewItem opt = option;

        initStyleOption(&opt, index);

        auto field = static_cast<eSearchResultColumn>(index.column());

        Qt::CheckState UML_Applicability = index.sibling(index.row(), static_cast<int>(eSearchResultColumn::UML_Applicability)).data(Qt::CheckStateRole).value<Qt::CheckState>();
        Qt::CheckState PlotView_Applicability = index.sibling(index.row(), static_cast<int>(eSearchResultColumn::PlotView_Applicability)).data(Qt::CheckStateRole).value<Qt::CheckState>();

        if(Qt::Checked == UML_Applicability || Qt::Checked == PlotView_Applicability)
        {
            painter->fillRect(opt.rect, QBrush(QColor(200,200,200)));
        }
        else
        {
            if(false == bCustomBackgroundHighlighting)
            {
                if(isDarkMode())
                {
                    painter->fillRect(option.rect, option.palette.window());
                }
                else
                {
                   painter->fillRect(option.rect, QColor(255,255,255));
                }
            }
            else
            {
                painter->fillRect(option.rect, sCustomHighlightingColor);
            }
        }

        if(static_cast<eSearchResultColumn>(index.column()) == eSearchResultColumn::UML_Applicability ||
           static_cast<eSearchResultColumn>(index.column()) == eSearchResultColumn::PlotView_Applicability)
        {
            QStyleOptionViewItem viewItemOption = option;

            const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
            QRect newRect = QStyle::alignedRect(viewItemOption.direction, Qt::AlignCenter,
                                                QSize(viewItemOption.decorationSize.width() + 5, option.decorationSize.height()),
                                                QRect(viewItemOption.rect.x() + textMargin, option.rect.y(),
                                                      viewItemOption.rect.width() - (2 * textMargin), viewItemOption.rect.height()));
            viewItemOption.rect = newRect;

            QStyledItemDelegate::paint(painter, viewItemOption, index);
        }
        else if(static_cast<eSearchResultColumn>(index.column()) == eSearchResultColumn::Timestamp &&
                true == mbMarkTimestampWithBold)
        {
            drawText( stringData, painter, opt, mbMarkTimestampWithBold, bCustomBackgroundHighlighting );
        }
        else
        {
            auto foundSearchColumn = mSearchResultColumnsSearchMap.find(static_cast<eSearchResultColumn>(index.column()));

            if(foundSearchColumn != mSearchResultColumnsSearchMap.end() && true == foundSearchColumn.value())
            {
#ifdef DEBUG_CSearchResultHighlightingDelegate
               SEND_MSG(QString("CSearchResultHighlightingDelegate::drawHighlightedText(start:<index:row-%1:col-%2>)")
                            .arg(index.row())
                            .arg(index.column()));
#endif

               bool isMonoColorHighlighting = getSettingsManager()->getSearchResultMonoColorHighlighting();
               const QColor& regexMonoHighlightingColor = getSettingsManager()->getRegexMonoHighlightingColor();

               drawHighlightedText(field,
                                   matchData,
                                   stringData,
                                   painter, opt,
                                   isMonoColorHighlighting,
                                   regexMonoHighlightingColor,
                                   bCustomBackgroundHighlighting);
#ifdef DEBUG_CSearchResultHighlightingDelegate
               SEND_MSG(QString("CSearchResultHighlightingDelegate::drawHighlightedText(end:<index:row-%2:col-%3>): took %1 ms")
                            .arg(elapsedTimer.elapsed())
                            .arg(index.row())
                            .arg(index.column()));
#endif
            }
            else
            {
               drawText( stringData, painter, opt, false, bCustomBackgroundHighlighting );
            }
        }
    }

#ifdef DEBUG_CSearchResultHighlightingDelegate
    SEND_MSG(QString("CSearchResultHighlightingDelegate::paint(end:<index:row-%2:col-%3>): took %1 ms")
             .arg(elapsedTimer.elapsed())
             .arg(index.row())
             .arg(index.column()));
    elapsedTimer.invalidate();
#endif
}

QSize CSearchResultHighlightingDelegate::sizeHint(const QStyleOptionViewItem &option,
               const QModelIndex &index) const
{
    QSize result;

    auto column = static_cast<eSearchResultColumn>(index.column());

    auto foundSearchColumn = mSearchResultColumnsSearchMap.find(static_cast<eSearchResultColumn>(column));

    if(foundSearchColumn != mSearchResultColumnsSearchMap.end() && true == foundSearchColumn.value())
    {
        const auto* pModel = dynamic_cast<const ISearchResultModel*>(index.model());

        if(nullptr != pModel)
        {
            const auto& matchData = pModel->getFoundMatchesItemPack(index);

            tDrawDataPack drawDataPack;

            drawDataPack.alignment = option.displayAlignment;

            const auto& highlightingInfo = matchData.getItemMetadata().highlightingInfoMultiColor;
            auto foundHighlightingInfoItem = highlightingInfo.find(column);

            if(highlightingInfo.end() != foundHighlightingInfoItem)
            {
               const auto& highlightingData = foundHighlightingInfoItem.value();

               if(false == highlightingData.empty())
               {
                    auto inputStr = pModel->data(index, Qt::DisplayRole).value<QString>();

                    bool isMonoColorHighlighting = getSettingsManager()->getSearchResultMonoColorHighlighting();
                    const QColor& regexMonoHighlightingColor = getSettingsManager()->getRegexMonoHighlightingColor();

                    collectDrawDataPack(inputStr,
                                        highlightingData,
                                        drawDataPack,
                                        option,
                                        isMonoColorHighlighting,
                                        regexMonoHighlightingColor);

                    int shift = calculateShifts(drawDataPack, option);

                    result = QStyledItemDelegate::sizeHint(option, index);
                    result.setWidth(shift);
               }
               else
               {
                    result = QStyledItemDelegate::sizeHint(option, index);
               }
            }
            else
            {
               result = QStyledItemDelegate::sizeHint(option, index);
            }
        }
    }
    else
    {
        result = QStyledItemDelegate::sizeHint(option, index);
    }

    return result;
}

void CSearchResultHighlightingDelegate::handleSettingsManagerChange()
{
    mbMarkTimestampWithBold = getSettingsManager()->getMarkTimeStampWithBold();

    connect( getSettingsManager().get(),
             &ISettingsManager::markTimeStampWithBoldChanged,
             this, [this](bool val)
    {
        mbMarkTimestampWithBold = val;
    });

    mSearchResultColumnsSearchMap = getSettingsManager()->getSearchResultColumnsSearchMap();

    connect( getSettingsManager().get(),
            &ISettingsManager::searchResultColumnsSearchMapChanged,
            this, [this](tSearchResultColumnsVisibilityMap val)
            {
                mSearchResultColumnsSearchMap = val;
            });
}

PUML_PACKAGE_BEGIN(DMA_SearchView)
    PUML_CLASS_BEGIN_CHECKED(CSearchResultHighlightingDelegate)
        PUML_INHERITANCE_CHECKED(QStyledItemDelegate, extends)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_COMPOSITION_DEPENDENCY_CHECKED(ISearchResultModel, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
