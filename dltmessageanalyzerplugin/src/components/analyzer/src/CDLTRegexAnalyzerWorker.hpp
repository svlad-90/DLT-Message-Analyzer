/**
 * @file    CDLTRegexAnalyzerWorker.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CDLTRegexAnalyzerWorker class
 */
#pragma once

#include "memory"
#include "set"
#include "map"

#include <QRegularExpression>

#include "common/Definitions.hpp"
#include "../api/IDLTMessageAnalyzerController.hpp"

#include "components/settings/api/CSettingsManagerClient.hpp"

typedef int tWorkerThreadCookie;

/**
 * @brief The CDLTRegexAnalyzerWorker class - an object, which is actually performs a regex analysis.
 * Intention is to use one or more of these instances in context of worker-thread(s).
 */
class CDLTRegexAnalyzerWorker : public QObject,
                                public CSettingsManagerClient
{
    Q_OBJECT
public:

    enum class ePortionAnalysisState
    {
        ePortionAnalysisState_SUCCESSFUL,
        ePortionAnalysisState_ERROR
    };

    CDLTRegexAnalyzerWorker(const tSettingsManagerPtr& pSettingsManager);
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
