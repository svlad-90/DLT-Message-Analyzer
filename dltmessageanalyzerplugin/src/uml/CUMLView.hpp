/**
 * @file    CUMLView.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CUMLView class
 */

#ifndef CUMLVIEW_H
#define CUMLVIEW_H

#include "memory"
#include "functional"

#include "QScrollArea"
#include "QProcess"

class QProcess;
class CImageViewer;

class CUMLView : public QScrollArea
{
    Q_OBJECT
    typedef QScrollArea tParent;

public:
    CUMLView(QWidget *parent = nullptr);
    void generateUMLDiagram(const QString& diagramContent);
    bool isDiagramGenerationInProgress() const;
    void cancelDiagramGeneration();
    void clearDiagram();
    bool isDiagramShown() const;

signals:
    void diagramGenerationStarted();
    void diagramGenerationFinished(bool success);

public:
    enum class eDiagramExtension
    {
        e_SVG = 0,
        e_PNG
    };

private:

    typedef std::shared_ptr<QProcess> tProcessPtr;

    typedef std::function<void(int exitCode, QProcess::ExitStatus exitStatus)> tGenerateDiagramCallback;
    void generateUMLDiagramInternal(const QString& diagramContent,
                                    eDiagramExtension extension,
                                    const tGenerateDiagramCallback& callback,
                                    tProcessPtr& pSubProcess,
                                    bool blocking);

private:
    bool mbDiagramShown;
    CImageViewer* mpImageViewer;
    tProcessPtr mpDiagramCreationSubProcess;
    tProcessPtr mpSaveSVGSubProcess;
    QString mDiagramContent;
    bool mbDiagramGenerationInProgress;
};

#endif // CUMLVIEW_H
