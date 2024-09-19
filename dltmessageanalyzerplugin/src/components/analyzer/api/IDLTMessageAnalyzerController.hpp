/**
 * @file    IDLTMessageAnalyzerController.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the IDLTMessageAnalyzerController class
 */
#pragma once

#include <QObject>

#include "Definitions.hpp"

//Forward declarations
class IDLTMessageAnalyzerControllerConsumer;

//IDLTMessageAnalyzerController
class IDLTMessageAnalyzerController: public QObject
{
    Q_OBJECT
public:

    template<typename T, typename ... Args >
    static std::shared_ptr<T> createInstance( Args&& ... args  )
    {
        static_assert ( std::is_base_of<IDLTMessageAnalyzerController, T>::value, "error: T should inherit from IDLTMessageAnalyzerController!" );
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    virtual ~IDLTMessageAnalyzerController();

    /**
     * @brief requestAnalyze - used to triger analysis
     * @param pClient - client, which should be notified about the progress
     * @param requestParameters - inout request parameters
     * @param regexScriptingMetadata - regex scripting metadata
     * @return - request id, if search was successfully started. Or INVALID_REQUEST_ID otherwise
     */
    virtual tRequestId requestAnalyze( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient,
                                       const tRequestParameters& requestParameters,
                                       const tRegexScriptingMetadata& regexScriptingMetadata )=0;

    /**
     * @brief cancelRequest - used to stop previously started analysis iteration
     * @param pClient - client, which tries to cancel its request
     * @param requestId - id of the request
     */
    virtual void cancelRequest( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient, const tRequestId& requestId )=0;

    /**
     * @brief getMaximumNumberOfThreads - gets maximum number of threads, which can be used for analysis
     * @return - maximum number of threads, which can be used for analysis.
     */
    virtual int getMaximumNumberOfThreads() const = 0;

    protected:

    /**
     * @brief IDLTMessageAnalyzerController - default constructor.
     * Protected, as we want to highlight, that client should inherit from this class.
     */
    IDLTMessageAnalyzerController();

signals:
    /**
     * @brief progressNotification - signal, which notifies client about the analysis progress.
     * @param progressNotificationData - progress notification data
     * Client might not remember about this fact, so we provide this information back to the client, to simplify the client's logic.
     */
    void progressNotification( const tProgressNotificationData& progressNotificationData );

    /**
     * @brief analysisStarted - signal, which notifies client about the start of the analysis.
     * @param requestId - the request id of the search
     * @param usedRegex - the regex with which the anlysis was started
     * @param selectedAliases - the selected regex alises that were used to form the search query
     */
    void analysisStarted( const tRequestId& requestId, const QString& usedRegex, const QStringList& selectedAliases );

    /**
     * @brief analysisFinished - signal, which notifies client about the finish of the analysis.
     * @param requestId - the request id of the search
     */
    void analysisFinished( const tRequestId& requestId );
};

typedef std::shared_ptr<IDLTMessageAnalyzerController> tDLTMessageAnalyzerControllerPtr;
