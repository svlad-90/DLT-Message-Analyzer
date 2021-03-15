/**
 * @file    CUMLView.hpp
 * @author  vgoncharuk
 * @brief   Declaration of the CUMLView class
 */

#pragma once

#include "memory"
#include "functional"

#include "QScrollArea"
#include "QProcess"

#include "components/settings/api/CSettingsManagerClient.hpp"

class QProcess;
class CImageViewer;
class QPushButton;
class QPlainTextEdit;

class CUMLView : public QScrollArea,
                 public CSettingsManagerClient
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

    void setUMLCreateDiagramFromTextButton(QPushButton* pUMLCreateDiagramFromTextButton);
    void setUMLTextEditor(QPlainTextEdit* pUMLTextEditor);

signals:
    void diagramGenerationStarted();
    void diagramGenerationFinished(bool success);

public:
    enum class eDiagramExtension
    {
        e_SVG = 0,
        e_PNG
    };

protected:
    void handleSettingsManagerChange() override;

private:

    void startGenerateUMLDiagram(const QString& diagramContent, bool isInternalCall);

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
    QString mLastSelectedFolder;
    QPushButton* mpUMLCreateDiagramFromTextButton;
    QPlainTextEdit* mpUMLTextEditor;
};
