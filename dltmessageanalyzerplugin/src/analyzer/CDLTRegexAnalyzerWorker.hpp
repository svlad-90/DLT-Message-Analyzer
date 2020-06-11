/**
 * @file    CDLTRegexAnalyzerWorker.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CDLTRegexAnalyzerWorker class
 */
#ifndef CDLTREGEXANALYZERWORKER_HPP
#define CDLTREGEXANALYZERWORKER_HPP

#include "memory"
#include "set"
#include "map"

#include <QRegularExpression>

#include "../common/Definitions.hpp"
#include "IDLTMessageAnalyzerController.hpp"

typedef int tWorkerThreadCookie;

/**
 * @brief The CDLTRegexAnalyzerWorker class - an object, which is actually performs a regex analysis.
 * Intention is to use one or more of these instances in context of worker-thread(s).
 */
class CDLTRegexAnalyzerWorker : public QObject
{
    Q_OBJECT
public:

    enum class ePortionAnalysisState
    {
        ePortionAnalysisState_SUCCESSFUL,
        ePortionAnalysisState_ERROR
    };

    CDLTRegexAnalyzerWorker();
    tWorkerId getWorkerId() const;

public slots:
    void analyzePortion( const tRequestId& requestId,
                         const tProcessingStrings& processingStrings,
                         const QRegularExpression& regex,
                         const tRegexScriptingMetadata& regexMetadata,
                         const tWorkerThreadCookie& workerThreadCookie );

signals:
    void portionAnalysisFinished( const tRequestId& requestId, int numberOfProcessedString, ePortionAnalysisState portionAnalysisState, const tFoundMatchesPack& processedMatches, const tWorkerId& workerId, const tWorkerThreadCookie& workerThreadCookie);

private: // members
    tWorkerId mWorkerId;
    QVector<QColor> mColors;
};

#endif // CDLTREGEXANALYZERWORKER_HPP
