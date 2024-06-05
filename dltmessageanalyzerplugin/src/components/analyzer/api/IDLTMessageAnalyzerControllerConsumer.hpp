/**
 * @file    IDLTMessageAnalyzerControllerConsumer.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the IDLTMessageAnalyzerControllerConsumer class
 */
#pragma once

#include <QObject>

#include "Definitions.hpp"

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
    virtual void progressNotification( const tProgressNotificationData& progressNotificationData )=0;

protected:
    IDLTMessageAnalyzerControllerConsumer( const std::weak_ptr<IDLTMessageAnalyzerController>& pController );
    tRequestId requestAnalyze( const tRequestParameters& requestParameters,
                               bool bUMLFeatureActive,
                               bool bPlotViewFeatureActive,
                               bool bGroupedViewFeatureActive );
    void cancelRequest( const tRequestId& requestId );
    bool isGroupedViewFeatureActiveForCurrentAnalysis() const;

protected: // fields
    std::weak_ptr<IDLTMessageAnalyzerController> mpController;

private: // fields
    bool mbGroupedViewFeatureActiveForCurrentAnalysis;
};

typedef std::shared_ptr<IDLTMessageAnalyzerControllerConsumer> tDLTMessageAnalyzerControllerConsumerPtr;
typedef std::weak_ptr<IDLTMessageAnalyzerControllerConsumer> tDLTMessageAnalyzerControllerConsumerWeakPtr;
