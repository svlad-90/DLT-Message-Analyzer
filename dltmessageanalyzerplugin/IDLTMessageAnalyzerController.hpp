/**
 * @file    IDLTMessageAnalyzerController.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the IDLTMessageAnalyzerController class
 */
#ifndef IDLTMESSAGEANALYZERCONTROLLER_HPP
#define IDLTMESSAGEANALYZERCONTROLLER_HPP

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
     * @param pFile - file, which is used for analysis
     * @param fromMessage - from which message to analyze
     * @param numberOfMessages - until which message to analyze
     * @param regex - regex, to be used for analysis
     * @param numberOfThreads - number of threads to be used for analysis
     * @param regexScriptingMetadata - scripting metadata
     * @param isContinuous - whether search is continuous. This parameter can be ignored by one-shot implementations
     * @return - request id, if search was successfully started. Or INVALID_REQUEST_ID otherwise
     */
    virtual tRequestId requestAnalyze( const std::weak_ptr<IDLTMessageAnalyzerControllerConsumer>& pClient,
                                       const tDLTFileWrapperPtr& pFile,
                                       const int& fromMessage,
                                       const int& numberOfMessages,
                                       const QRegularExpression& regex,
                                       const int& numberOfThreads,
                                       const tRegexScriptingMetadata& regexScriptingMetadata,
                                       bool isContinuous)=0;

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
     * @param requestId - id of the request, about which we notify the client
     * @param requestState - the state of the request
     * @param progress - progress in percents
     * @param processedMatches - found matches
     * @param isContinuous - whether analysis is continuous.
     * Client might not remember about this fact, so we provide this information back to the client, to simplify the client's logic.
     */
    void progressNotification( const tRequestId& requestId,
                                       const eRequestState& requestState,
                                       const int8_t& progress,
                                       const tFoundMatchesPack& processedMatches,
                                       bool isContinuous );
};

typedef std::shared_ptr<IDLTMessageAnalyzerController> tDLTMessageAnalyzerControllerPtr;

#endif // IDLTMESSAGEANALYZERCONTROLLER_HPP
