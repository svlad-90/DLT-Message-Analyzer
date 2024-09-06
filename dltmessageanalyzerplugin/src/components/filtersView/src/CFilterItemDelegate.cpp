#include <QCompleter>
#include <QLineEdit>
#include <QPainter>
#include <QApplication>
#include <QStringListModel>

#include "components/log/api/CLog.hpp"
#include "components/settings/api/ISettingsManager.hpp"
#include "common/Definitions.hpp"
#include "CFiltersModel.hpp"
#include "CFilterItemDelegate.hpp"

#include "DMA_Plantuml.hpp"

const static QRegularExpression sFindLastPipeRegex("(.*)(?<!\\\\)(\\|)(?!.*(?<!\\\\)\\1)(.*)");

class CRegexLineEdit : public QLineEdit
{
public:

    typedef QLineEdit tParent;

    CRegexLineEdit(QWidget *parent = nullptr):
    tParent(parent),
    mbAppendMode(false)
    {
        //SEND_MSG(QString("%1").arg(__FUNCTION__));
    }

    void setAppendMode(bool val)
    {
        mbAppendMode = val;
    }

    void replaceSignals()
    {
        disconnect(completer(), nullptr, static_cast<QLineEdit*>(this), nullptr);
        connect(completer(), QOverload<const QString&>::of(&QCompleter::activated), this, &CRegexLineEdit::onActivatedHandler);
    }

protected:

    virtual void focusInEvent(QFocusEvent *event) override
    {
        //SEND_MSG(QString("%1").arg(__FUNCTION__));
        tParent::focusInEvent(event);

        if(nullptr != completer())
        {
            //SEND_MSG(QString("%1 completer not nullptr").arg(__FUNCTION__));
            replaceSignals();
        }
    }

private:

    void onActivatedHandler(const QString& text_)
    {
        //SEND_MSG(QString("%1").arg(__FUNCTION__));

        if(true == mbAppendMode)
        {
            QString previousText = text();

            if(true == sFindLastPipeRegex.isValid())
            {
                auto foundMatch = sFindLastPipeRegex.match(previousText);

                if(foundMatch.hasMatch())
                {
                    if(foundMatch.lastCapturedIndex() >= 3)
                    {
                        setText(foundMatch.captured(1) + "|" + QRegularExpression::escape(text_));
                    }
                }
                else
                {
                    setText(QRegularExpression::escape(text_));
                }
            }
        }
        else
        {
            setText(QRegularExpression::escape(text_));
        }
    }

private:

    bool mbAppendMode;
};

CFilterItemDelegate::CFilterItemDelegate( QTreeView* pParentTree ):
tParent(pParentTree),
mCompletionData(),
mRowAnimationDataMap(),
mTime(),
mUpdateTimer(),
mpParentTree(pParentTree),
mpModel()
{
    connect( this, &tParent::closeEditor, this, &CFilterItemDelegate::closeEditorHandler );

    QCompleter* pCompleter = new QCompleter(QStringList(), this);
    pCompleter->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    mCompletionData.pCompleter = pCompleter;
    mCompletionData.pPopUp = new QListView(mCompletionData.pEditor);
    mCompletionData.pPopUp->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mCompletionData.pPopUp->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mCompletionData.pPopUp->setSelectionBehavior(QAbstractItemView::SelectRows);
    mCompletionData.pPopUp->setSelectionMode(QAbstractItemView::SingleSelection);
    mCompletionData.pCompleter->setPopup(mCompletionData.pPopUp);
}

QWidget* CFilterItemDelegate::createEditor(QWidget *parent,
                      const QStyleOptionViewItem &,
                      const QModelIndex &idx) const
{
    QWidget* pResult = nullptr;

    auto* pNonConstThis = const_cast<CFilterItemDelegate*>(this);

    tCompletionData *pEditableData = const_cast<tCompletionData*>(&mCompletionData);
    pNonConstThis->mCompletionData.pEditor = new CRegexLineEdit(parent);
    pNonConstThis->mCompletionData.modelIndex = idx;

    pResult = pEditableData->pEditor;

    pNonConstThis->applySuggestions();

    return pResult;
}

void CFilterItemDelegate::delayedComplete()
{
    if(nullptr != mCompletionData.pCompleter)
    {
        mCompletionData.completionTimer.singleShot(1, this, [this]()
        {
             if(nullptr != mCompletionData.pCompleter)
             {
                 mCompletionData.pCompleter->complete();
             }

             if(nullptr != mCompletionData.pPopUp)
             {
                 mCompletionData.pPopUp->resize(getSettingsManager()->getFiltersCompletion_CompletionPopUpWidth(),
                                                mCompletionData.pPopUp->height());
             }
        });
    }
}

void CFilterItemDelegate::updateSuggestions(const QString& input)
{
    if(nullptr != mpModel &&
       true == mCompletionData.modelIndex.isValid() &&
       nullptr != mCompletionData.pCompleter)
    {
        QStringListModel* pModel = static_cast<QStringListModel*>(mCompletionData.pCompleter->model());

        auto rowType = mCompletionData.modelIndex.sibling(mCompletionData.modelIndex.row(), static_cast<int>(eRegexFiltersColumn::RowType)).data().value<eRegexFiltersRowType>();

        if(eRegexFiltersRowType::Text == rowType) // if current index is text
        {
            auto parentIndex = mCompletionData.modelIndex.parent();

            if(true == parentIndex.isValid()) //if parent index is valid
            {
                if(1 == mpModel->rowCount(parentIndex)) // if parent has only 1 child
                {
                    auto parentRowType = parentIndex.sibling(parentIndex.row(), static_cast<int>(eRegexFiltersColumn::RowType)).data().value<eRegexFiltersRowType>();

                    if(parentRowType == eRegexFiltersRowType::VarGroup) // if parent is var
                    {
                        auto groupIndex = parentIndex.sibling(parentIndex.row(), static_cast<int>(eRegexFiltersColumn::GroupIndex)).data().value<int>();

                        // we need to modify the input in order to consider pipe

                        auto normalizedInput = input;

                        if(true == sFindLastPipeRegex.isValid())
                        {
                            auto foundMatch = sFindLastPipeRegex.match(input);

                            if(foundMatch.hasMatch())
                            {
                                if(nullptr != mCompletionData.pEditor)
                                {
                                    mCompletionData.pEditor->setAppendMode(true);
                                }

                                if(foundMatch.lastCapturedIndex() >= 3)
                                {
                                    normalizedInput = foundMatch.captured(3);
                                }
                            }
                            else
                            {
                                if(nullptr != mCompletionData.pEditor)
                                {
                                    mCompletionData.pEditor->setAppendMode(false);
                                }
                            }
                        }

                        pModel->setStringList(mpModel->getCompletionData(groupIndex,
                                                                         normalizedInput,
                                                                         getSettingsManager()->getFiltersCompletion_MaxNumberOfSuggestions(),
                                                                         getSettingsManager()->getFiltersCompletion_MaxCharactersInSuggestion()));
                    }
                    else
                    {
                        pModel->setStringList(QStringList());
                    }
                }
                else
                {
                    pModel->setStringList(QStringList());
                }
            }
            else
            {
                pModel->setStringList(QStringList());
            }
        }
        else
        {
            pModel->setStringList(QStringList());
        }
    }
}

void CFilterItemDelegate::closeEditorHandler(QWidget *editor, QAbstractItemDelegate::EndEditHint)
{
    if(editor == mCompletionData.pEditor)
    {
        mCompletionData.pEditor = nullptr;
        mCompletionData.modelIndex = QModelIndex();
    }
}

void CFilterItemDelegate::applySuggestions()
{
    if(nullptr != mCompletionData.pEditor &&
       true == mCompletionData.modelIndex.isValid())
    {
        auto updateCompleter = [this](const QString& input)
        {
            if(nullptr != mCompletionData.pEditor)
            {
                updateSuggestions(input);
                mCompletionData.pEditor->setCompleter(mCompletionData.pCompleter);
                mCompletionData.pEditor->replaceSignals();
            }
        };

        updateCompleter(QString());
        delayedComplete();

        connect(mCompletionData.pEditor, &QLineEdit::textEdited, this,
        [this, updateCompleter](const QString& text)
        {
            if(nullptr != mCompletionData.pEditor)
            {
                updateCompleter(text);
                delayedComplete();
            }
        });
    }
}

void CFilterItemDelegate::animateRows(const QVector<QModelIndex>& rows, const QColor& color, const int64_t& duration)
{
    mRowAnimationDataMap.clear();

    const auto columnsNumber = mpParentTree->model()->columnCount();

    for(const auto& row : rows)
    {
        int64_t startTime = mTime.elapsed();

        for(int columnId = 0; columnId < columnsNumber; ++columnId)
        {
            if(false == mpParentTree->isColumnHidden(columnId))
            {
                auto animatedIdx = row.sibling(row.row(), columnId);
                QColor initialColor = getColor(animatedIdx);
                mRowAnimationDataMap.insert( animatedIdx, tRowAnimationData(initialColor, color, startTime, startTime + duration) );
            }
        }
    }

    auto updateAnimatedRows = [this, columnsNumber]()
    {
        for( auto it = mRowAnimationDataMap.begin(); it != mRowAnimationDataMap.end(); ++it )
        {
            for(int columnId = 0; columnId < columnsNumber; ++columnId)
            {
                if(false == mpParentTree->isColumnHidden(columnId))
                {
                    QModelIndex updateIdx = it.key().sibling(it.key().row(), columnId);
                    mpParentTree->update( updateIdx );
                }
            }
        }
    };

    if(nullptr != mpParentTree)
    {
        updateAnimatedRows();
        mUpdateTimer.start(16);
    }

    connect(&mUpdateTimer, &QTimer::timeout, [this, updateAnimatedRows]()
    {
        if(false == mRowAnimationDataMap.empty())
        {
            if(nullptr != mpParentTree)
            {
                updateAnimatedRows();
                mUpdateTimer.start(16);
            }

             excludeAnimatedRows();
        }
    });
}

void CFilterItemDelegate::excludeAnimatedRows()
{
    for( auto it = mRowAnimationDataMap.begin(); it != mRowAnimationDataMap.end(); )
    {
        auto& animatedObject = *it;

        auto nowToStartDiff = mTime.elapsed() - animatedObject.startTime;
        auto endToStartDiff = animatedObject.endTime - animatedObject.startTime;

        auto passedTimePercantage = static_cast<int>( 100 * ( static_cast<double>(nowToStartDiff) / endToStartDiff ) );

        if(passedTimePercantage > 100)
        {
            it = mRowAnimationDataMap.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

QColor CFilterItemDelegate::getColor( const QModelIndex& index ) const
{
    QColor result;

    auto rowType = index.sibling( index.row(), static_cast<int>(eRegexFiltersColumn::RowType) ).data().value<eRegexFiltersRowType>();

    auto parentIdx = index.parent();

    bool bParentExist = false;
    auto parentRowType = eRegexFiltersRowType::Text;

    if( parentIdx.isValid() )
    {
        bParentExist = true;
        parentRowType = parentIdx.sibling( parentIdx.row(), static_cast<int>(eRegexFiltersColumn::RowType) ).data().value<eRegexFiltersRowType>();
    }

    switch(rowType)
    {
        case eRegexFiltersRowType::Text:
        {
            if(true == bParentExist)
            {
                if(parentRowType == eRegexFiltersRowType::VarGroup)
                {
                    result = isDarkMode() ? QColor("#2623d9") : QColor("#F9F871");
                }
                else
                {
                    result = isDarkMode() ? QColor("#48454a") : QColor("#d6d6d6");
                }
            }
            else
            {
                result = isDarkMode() ? QColor("#48454a") : QColor("#d6d6d6");
            }
        }
            break;
        case eRegexFiltersRowType::VarGroup:
        result = isDarkMode() ? QColor("#7325b3") : QColor("#FFC04A");
            break;
        case eRegexFiltersRowType::NonVarGroup:
        result = isDarkMode() ? QColor("#48454a") : QColor("#d6d6d6");
            break;
    }

    return result;
}

void CFilterItemDelegate::drawText(const QString& inputStr,
              const QStyleOptionViewItem& opt,
              QPainter *painter,
              bool bold) const
{
    int baseShift = 0;

    if(Qt::AlignmentFlag::AlignLeft == opt.displayAlignment)
    {
        baseShift = 2;
    }

    QRect rect = opt.rect;

    //set pen color
    if (opt.state & QStyle::State_Selected)
    {
        painter->fillRect(opt.rect, opt.palette.highlight());

        QPalette::ColorGroup cg = QPalette::Normal;
        painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
    }
    else
    {
        QPalette::ColorGroup cg = QPalette::Normal;
        painter->setPen(opt.palette.color(cg, QPalette::Text));

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
                             opt.displayAlignment, inputStr);
}

void CFilterItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    if(nullptr == painter)
    {
        return;
    }

    painter->save();

    QStyleOptionViewItem viewItemOption = option;

    auto foundAnimatedObject = const_cast<tRowAnimationDataMap*>(&mRowAnimationDataMap)->find( index );

    if(foundAnimatedObject != mRowAnimationDataMap.end())
    {
        auto nowToStartDiff = mTime.elapsed() - foundAnimatedObject->startTime;
        auto endToStartDiff = foundAnimatedObject->endTime - foundAnimatedObject->startTime;

        auto passedTimePercantage = static_cast<int>( 100 * ( static_cast<double>(nowToStartDiff) / endToStartDiff ) );

        if( passedTimePercantage >= 100.0 ) // last frame
        {
            painter->fillRect( viewItemOption.rect, foundAnimatedObject.value().startColor );
            const_cast<tRowAnimationDataMap*>(&mRowAnimationDataMap)->erase(foundAnimatedObject);
        }
        else
        {
            auto calcColor = [](const QColor& fromColor, const QColor& toColor, const int& percentage)->QColor
            {
                double x = percentage / 100.0;
                double factor = 4 * -(x*x) + 4*x;

                int rDifference = ( toColor.red() - fromColor.red() );
                int r = fromColor.red() + static_cast<int>( rDifference * factor );
                int gDifference = ( toColor.green() - fromColor.green() );
                int g = fromColor.green() + static_cast<int>( gDifference * factor );
                int bDifference = ( toColor.blue() - fromColor.blue() );
                int b = fromColor.blue() + static_cast<int>( bDifference * factor );

                //SEND_MSG(QString("~~~ percentage - %1; factor - %2").arg(percentage).arg(factor));

                return QColor( r, g, b );
            };

            QColor color = calcColor(foundAnimatedObject->startColor, foundAnimatedObject->intermediateColor, passedTimePercantage);

            painter->fillRect( viewItemOption.rect, color );
        }
    }
    else
    {
        painter->fillRect( viewItemOption.rect, getColor(index) );
    }

    painter->restore();

#ifdef __linux__
    drawText( index.data().value<QString>(), viewItemOption, painter, true );
#else
    QStyledItemDelegate::paint(painter, viewItemOption, index);
#endif
}

void CFilterItemDelegate::setSpecificModel( CFiltersModel* pModel )
{
    if(nullptr != pModel)
    {
        mpModel = pModel;
    }
}

PUML_PACKAGE_BEGIN(DMA_FiltersView)
    PUML_CLASS_BEGIN_CHECKED(CFilterItemDelegate)
        PUML_INHERITANCE_CHECKED(QStyledItemDelegate, extends)
        PUML_INHERITANCE_CHECKED(CSettingsManagerClient, extends)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(CFiltersModel, 1, 1, uses)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QTreeView, 1, 1, parent view)
        PUML_AGGREGATION_DEPENDENCY_CHECKED(QCompleter, 1, 1, uses)
    PUML_CLASS_END()
PUML_PACKAGE_END()
