#include "assert.h"

#include "QString"

#include "QLocale"

#include "PCRELexer.h"
#include "PCREParser.h"
#include "PCREBaseVisitor.h"

#include "../CTreeItem.hpp"
#include "components/log/api/CLog.hpp"
#include "PCREHelper.hpp"

#ifdef DEBUG_BUILD
#include "QElapsedTimer"
#endif

using namespace std;
using namespace antlr4;
using namespace pcre_parser;

struct tParsingDataItem
{
    tTreeItemPtr pTreeItem = nullptr;

    // data fields
    QString name;                                               // done
    QString value;                                              // done
    tColorWrapper colorWrapper;                                 // done
    tIntRange range;                                            // done
    tIntRange contentRange;                                     // done
    eRegexFiltersRowType rowType = eRegexFiltersRowType::Text;  // done
    QString groupName;                                          // done
    eGroupSyntaxType groupSyntaxType;                           // done
    bool isRoot = false;                                        // done

    struct tCoveredRangesItem
    {
        tTreeItemPtr pTreeItem;
        int index;
    };
    typedef std::map<tIntRange, tCoveredRangesItem> tCoveredRanges;

    tCoveredRanges coveredRanges;                               // done
};

class  CParseFiltersViewVisitor : pcre_parser::PCREBaseVisitor {
public:

    CParseFiltersViewVisitor(const tTreeItemSharedPtr& pFiltersViewTree, const QString& regex):
        mParsingStateStack(),
        mpFiltersViewTree(pFiltersViewTree),
        mRegex(regex),
        mGroupIndex(0)
    {
        if(nullptr != pFiltersViewTree)
        {
            tParsingDataItem rootDataItem;
            rootDataItem.pTreeItem = pFiltersViewTree.get();
            rootDataItem.isRoot = true;

            if(false == regex.isEmpty())
            {
                rootDataItem.range = tIntRange(0, regex.size() - 1);
                rootDataItem.contentRange = tIntRange(0, regex.size() - 1);
            }

            mParsingStateStack.push_back(rootDataItem);
        }
    }

    virtual antlrcpp::Any visitParse(PCREParser::ParseContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));

            tIntRange rootRange( static_cast<int>(ctx->getStart()->getStartIndex()),
                                 static_cast<int>(ctx->getStop()->getStopIndex()));

            if(false == mParsingStateStack.empty())
            {
                mParsingStateStack.back().range = rootRange;
            }
        }

        auto result = visitChildren(ctx);

        // after all children were visited
        if(nullptr != ctx)
        {
            assert(mParsingStateStack.size() == 1);

            afterChildrenVisited(-1, true, mParsingStateStack[0].range);
        }

        return result;
    }

    virtual antlrcpp::Any visitCapture(PCREParser::CaptureContext *ctx) override
    {
        auto groupIndex = ++mGroupIndex;

        // before visiting the children
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));

            auto* pName = ctx->name();

            tParsingDataItem newParsingDataItem;
            newParsingDataItem.pTreeItem = mParsingStateStack.back().pTreeItem->appendChild( static_cast<int>(ctx->getStart()->getStartIndex()), CTreeItem::tData() );
            mParsingStateStack.push_back(newParsingDataItem);
            auto& currentParsingItem = mParsingStateStack.back();

            if(nullptr != pName)
            {
                auto text = pName->getText();

                if(false == text.empty()) // group name found
                {
                    //    enum class eRegexFiltersColumn : int
                    //    {
                    //        Value = 0, /*QString*/                                            // OK
                    //        Index, /*int*/                                                    // OK
                    //        ItemType, /*QString*/                                             // OK
                    //        AfterLastVisible, /*empty string*/                                // OK
                    //        Color, /*tColorWrapper*/                                          // OK
                    //        Range, /*tIntRange*/                                              // OK
                    //        RowType, /*eRegexFiltersRowType*/                                 // OK
                    //        IsFiltered, /*bool*/                                              // OK
                    //        GroupName, /*QString*/                                            // OK
                    //        GroupSyntaxType, /*int ... enough of enums in our variant*/       // OK
                    //        GroupIndex,      /*index of the group, if item is group.*/        // OK
                    //        Last /*nothing*/                                                  // OK
                    //    };

                    currentParsingItem.groupName = QString::fromStdString(text);

                    auto pRegexMetadataItem = parseRegexGroupName(QString::fromStdString(text),
                                                                  false,
                                                                  false,
                                                                  false);

                    if(nullptr != pRegexMetadataItem)
                    {
                        if(true == pRegexMetadataItem->varName.first) // if we have a variable
                        {
                            currentParsingItem.rowType = eRegexFiltersRowType::VarGroup;
                            currentParsingItem.value = pRegexMetadataItem->varName.second;
                            currentParsingItem.colorWrapper.optColor = pRegexMetadataItem->highlightingColor;
                        }
                        else // non var group with name
                        {
                            currentParsingItem.rowType = eRegexFiltersRowType::NonVarGroup;
                            currentParsingItem.value = QString::fromStdString(text);
                        }

                        QString stmt = QString::fromStdString(ctx->getText());

                        if(stmt.startsWith("(?<"))
                        {
                            currentParsingItem.groupSyntaxType = eGroupSyntaxType::SYNTAX_1;
                        }
                        else if(stmt.startsWith("(?'"))
                        {
                            currentParsingItem.groupSyntaxType = eGroupSyntaxType::SYNTAX_2;
                        }
                        else if(stmt.startsWith("(?P<"))
                        {
                            currentParsingItem.groupSyntaxType = eGroupSyntaxType::SYNTAX_3;
                        }
                        else
                        {
                            assert(false);
                        }
                    }
                }
                else // group with empty name
                {
                    currentParsingItem.rowType = eRegexFiltersRowType::NonVarGroup;
                }
            }
            else // group with no name
            {
                currentParsingItem.rowType = eRegexFiltersRowType::NonVarGroup;
            }

            currentParsingItem.name = getName(currentParsingItem.rowType);
            currentParsingItem.range = tIntRange(static_cast<int>(ctx->getStart()->getStartIndex()), static_cast<int>(ctx->getStop()->getStopIndex()));

            auto pAlteration = ctx->alternation();

            if(nullptr != pAlteration)
            {
                currentParsingItem.contentRange = tIntRange(static_cast<int>(ctx->alternation()->getStart()->getStartIndex()), static_cast<int>(ctx->alternation()->getStop()->getStopIndex()));
            }
            else
            {
                currentParsingItem.contentRange = currentParsingItem.range;
            }

            if(mParsingStateStack.size() > 1) // if we have parent item
            {
                // let's mark that it has a covered range
                tParsingDataItem::tCoveredRangesItem item;
                item.pTreeItem = currentParsingItem.pTreeItem;
                auto it = (mParsingStateStack.end() - 2);
                it->coveredRanges.insert(std::make_pair( currentParsingItem.range, item ));
            }
        }

        auto result = visitChildren(ctx);

        // after all children were visited
        if(nullptr != ctx)
        {
            auto* pAlternation = ctx->alternation();

            if(nullptr != pAlternation)
            {
                afterChildrenVisited(groupIndex,
                                     pAlternation != nullptr,
                                     pAlternation != nullptr ? tIntRange( static_cast<int>(pAlternation->getStart()->getStartIndex()),
                                                                          static_cast<int>(pAlternation->getStop()->getStopIndex())) : tIntRange());
            }
        }

        return result;
    }

    virtual antlrcpp::Any visitAlternation(PCREParser::AlternationContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitExpr(PCREParser::ExprContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitQuantifier(PCREParser::QuantifierContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitQuantifier_type(PCREParser::Quantifier_typeContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitCharacter_class(PCREParser::Character_classContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitBackreference(PCREParser::BackreferenceContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitBackreference_or_octal(PCREParser::Backreference_or_octalContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitNon_capture(PCREParser::Non_captureContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitComment(PCREParser::CommentContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitOption(PCREParser::OptionContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitLook_around(PCREParser::Look_aroundContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitSubroutine_reference(PCREParser::Subroutine_referenceContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitConditional(PCREParser::ConditionalContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitBacktrack_control(PCREParser::Backtrack_controlContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitNewline_convention(PCREParser::Newline_conventionContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitCallout(PCREParser::CalloutContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitAtom(PCREParser::AtomContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitCc_atom(PCREParser::Cc_atomContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitShared_atom(PCREParser::Shared_atomContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitLiteral(PCREParser::LiteralContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitCc_literal(PCREParser::Cc_literalContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitNumber(PCREParser::NumberContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitName(PCREParser::NameContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitNon_close_parens(PCREParser::Non_close_parensContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitNon_close_paren(PCREParser::Non_close_parenContext *ctx) override
    {
        if(nullptr != ctx)
        {
            //SEND_MSG(QString("[%1]: %2").arg(__FUNCTION__).arg(QString::fromStdString(ctx->getText())));
        }

        return visitChildren(ctx);
    }

private:

    void afterChildrenVisited(const int& groupIndex, bool useFullRange, const tIntRange& fullRange)
    {
        assert(false == mParsingStateStack.empty());

        const auto& currentParsingItem = mParsingStateStack.back();

        assert(nullptr != currentParsingItem.pTreeItem);

        if(false == currentParsingItem.isRoot)
        {
            CTreeItem::tData data;

            data.push_back(tDataItem(currentParsingItem.value));
            data.push_back(tDataItem(currentParsingItem.range.from));
            data.push_back(tDataItem(currentParsingItem.name));
            data.push_back(tDataItem(QString()));
            data.push_back(tDataItem(currentParsingItem.colorWrapper));
            data.push_back(tDataItem(currentParsingItem.range));
            data.push_back(tDataItem(currentParsingItem.rowType));
            data.push_back(tDataItem(false));
            data.push_back(tDataItem(currentParsingItem.groupName));
            data.push_back(tDataItem(static_cast<int>(currentParsingItem.groupSyntaxType)));
            data.push_back(tDataItem(groupIndex));

            currentParsingItem.pTreeItem->setData(data);
        }

        // let's also place all non-covered ranges as Text nodes
        const auto& coveredRanges = currentParsingItem.coveredRanges;
        std::size_t counter = 0;

        auto addText = [this, &currentParsingItem](const tIntRange& range)
        {
            auto pTreeItem = currentParsingItem.pTreeItem->appendChild( range.from, CTreeItem::tData() );

            CTreeItem::tData data_;

            data_.push_back(tDataItem(mRegex.mid(range.from, range.to - range.from + 1)));           // OK
            data_.push_back(tDataItem(range.from));                                                  // OK
            data_.push_back(tDataItem(getName(eRegexFiltersRowType::Text)));                         // OK
            data_.push_back(tDataItem(QString()));                                                   // OK
            data_.push_back(tDataItem(tColorWrapper()));                                             // OK
            data_.push_back(tDataItem(range));                                                       // OK
            data_.push_back(tDataItem(eRegexFiltersRowType::Text));                                  // OK
            data_.push_back(tDataItem(false));                                                       // OK
            data_.push_back(tDataItem(QString()));                                                   // OK
            data_.push_back(tDataItem(0));                                                           // OK
            data_.push_back(tDataItem(-1));                                                          // OK

            pTreeItem->setData(data_);
        };

        const tIntRange* pPreviousRange = nullptr;

        if(false == coveredRanges.empty())
        {
            for(const auto& coveredRange : coveredRanges)
            {
                if(counter == 0) // first iteration
                {
                    tIntRange range(currentParsingItem.contentRange.from, coveredRange.first.from - 1);

                    if(range.to - range.from >= 0)
                    {
                        addText(range);
                    }
                }

                if(pPreviousRange != nullptr) // all iterations except first
                {
                    tIntRange range(pPreviousRange->to + 1, coveredRange.first.from - 1);

                    if(range.to - range.from >= 0)
                    {
                        addText(range);
                    }
                }

                if(counter == ( coveredRanges.size() - 1 ) )
                {
                    tIntRange range(coveredRange.first.to + 1, currentParsingItem.contentRange.to);

                    if(range.to - range.from >= 0)
                    {
                        addText(range);
                    }
                }

                pPreviousRange = &coveredRange.first;

                ++counter;
            }
        }
        else
        {
            if(true == useFullRange)
            {
                addText( fullRange );
            }
        }

        mParsingStateStack.pop_back();
    }

    typedef std::vector<tParsingDataItem> tParsingStateStack;
    tParsingStateStack mParsingStateStack;
    tTreeItemSharedPtr mpFiltersViewTree;
    QString mRegex;
    int mGroupIndex;
};

void parseRegexFiltersView( const tTreeItemSharedPtr& pFiltersViewTree, const QString& regex )
{
#ifdef DEBUG_BUILD
    QElapsedTimer timer;
    qint64 elapsed = 0;
    qint64 offset = 0;
    timer.start();
#endif

    if(false == regex.isEmpty())
    {
        //SEND_MSG(QString("%1: Input regex is: %2").arg(__FUNCTION__).arg(regex));

        std::stringstream regex_stream( regex.toStdString() );

        ANTLRInputStream input(regex_stream);

        pcre_parser::PCRELexer lexer( &input );

#ifdef DEBUG_BUILD
        elapsed = timer.elapsed();
        SEND_MSG( QString("[%1] Regex size - %2; lexer creation took - %3 ms")
                  .arg(__FUNCTION__)
                  .arg(regex.size())
                  .arg(QLocale().toString(elapsed - offset), 4) );
        offset += elapsed - offset;
#endif

        CommonTokenStream tokens( &lexer );

#ifdef DEBUG_BUILD
        elapsed = timer.elapsed();
        SEND_MSG( QString("[%1] Regex size - %2; tokens creation took - %3 ms")
                  .arg(__FUNCTION__)
                  .arg(regex.size())
                  .arg(QLocale().toString(elapsed - offset), 4) );
        offset += elapsed - offset;
#endif

        pcre_parser::PCREParser parser( &tokens );

        PCREParser::ParseContext* pTree = parser.parse();

        //SEND_MSG(QString("%1: Number of common tokens after parsing: %2").arg(__FUNCTION__).arg(tokens.size()));

        //SEND_MSG(QString("%1: Tree to string: %2").arg(__FUNCTION__).arg(QString::fromStdString(pTree->getText())));

#ifdef DEBUG_BUILD
        elapsed = timer.elapsed();
        SEND_MSG( QString("[%1] Regex size - %2; regex parsing took - %3 ms")
                  .arg(__FUNCTION__)
                  .arg(regex.size())
                  .arg(QLocale().toString(elapsed - offset), 4) );
        offset += elapsed - offset;
#endif

        CParseFiltersViewVisitor visitor(pFiltersViewTree, regex);
        visitor.visitParse(pTree);

#ifdef DEBUG_BUILD
        elapsed = timer.elapsed();
        SEND_MSG( QString("[%1] Regex size - %2; visit call took - %3 ms")
                  .arg(__FUNCTION__)
                  .arg(regex.size())
                  .arg(QLocale().toString(elapsed - offset), 4) );
        offset += elapsed - offset;
#endif
    }
}
