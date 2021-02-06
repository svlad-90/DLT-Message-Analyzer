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
#include "DefinitionsInternal.hpp"
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

    CDLTRegexAnalyzerWorker(const tSettingsManagerPtr& pSettingsManager);
    tWorkerId getWorkerId() const;

public slots:
    void analyzePortion( const tAnalyzePortionData& analyzePortionData );

signals:
    void portionAnalysisFinished( const tPortionRegexAnalysisFinishedData& portionRegexAnalysisFinishedData );

private: // members
    tWorkerId mWorkerId;
    QVector<QColor> mColors;
};
