/**
 * @file    IDLTMessageAnalyzerControllerConsumer.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the IDLTMessageAnalyzerControllerConsumer class
 */
#pragma once

#include <QObject>

#include "common/Definitions.hpp"

//Forward declarations

class IDLTMessageAnalyzerController;

//IDLTMessageAnalyzerControllerConsumer
class IDLTMessageAnalyzerControllerConsumer : public QObject, public std::enable_shared_from_this<IDLTMessageAnalyzerControllerConsumer>
{
    Q_OBJECT
public:

    template<typename T, typename ... Args >
    static std::shared_ptr<T> createInstance( Args&& ... args  )
    {
        static_assert ( std::is_base_of<IDLTMessageAnalyzerControllerConsumer, T>::value, "error: T should inherit from IDLTMessageAnalyzerControllerConsumer!" );
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    virtual ~IDLTMessageAnalyzerControllerConsumer();

public slots:
    virtual void progressNotification( const tRequestId& requestId,
                                       const eRequestState& requestState,
                                       const int8_t& progress,
                                       const tFoundMatchesPack& processedMatches)=0;

protected:
    IDLTMessageAnalyzerControllerConsumer( const std::weak_ptr<IDLTMessageAnalyzerController>& pController );
    tRequestId requestAnalyze( const tDLTFileWrapperPtr& pFile,
                               const int& fromMessage,
                               const int& numberOfMessages,
                               const QRegularExpression& regex,
                               const int& numberOfThreads,
                               bool isContinuous );
    void cancelRequest( const tRequestId& requestId );

protected: // fields
    std::weak_ptr<IDLTMessageAnalyzerController> mpController;

};

typedef std::shared_ptr<IDLTMessageAnalyzerControllerConsumer> tDLTMessageAnalyzerControllerConsumerPtr;
typedef std::weak_ptr<IDLTMessageAnalyzerControllerConsumer> tDLTMessageAnalyzerControllerConsumerWeakPtr;
