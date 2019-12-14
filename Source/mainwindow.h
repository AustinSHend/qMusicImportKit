#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <settingswindow.h>
#include <aboutwindow.h>
#include <helper.h>

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFuture>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QtConcurrent/QtConcurrentRun>
#include <QThread>
#include <QUrl>

struct uiSelections_t {
    QDir inputDir;
    QDir tempDir;
    QDir outputDir;
    QString syntaxInput;
    bool RGEnabled;
    bool copyContentsEnabled;
    QString copyContents;
    bool renameLogCueEnabled;
    bool compressImagesEnabled;
    bool XLSXEnabled;
    QString XLSXLogScore;
    QString XLSXNotes;
    bool deleteTempEnabled;
    bool openFolderEnabled;
    QString codecInput;
    QString presetInput;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void applyUserSettings();
    void folderOpen(QLineEdit* initLineEdit);
    void folderChooser(QLineEdit* initLineEdit);
    void renameLogCue(QStringList inputFiles, QDir outputFolder, QString artist, QString album);
    QStringList folderCopy(QDir fromDir, QDir toDir, QStringList patternList = {"*"}, QStringList dontCopyList = {});
    void copyInputToTempWorker(QDir inputPath, QDir tempPath, bool convertWavs = false);
    void calculateReplayGain (QStringList inputFLACs);
    QStringList convertToFormat(conversionParameters_t *conversionParameters);
    void convertBackgroundWorker(uiSelections_t uiSelections);

private slots:
    void on_actionQuit_triggered();
    void on_actionConfigure_triggered();
    void on_actionAbout_triggered();
    void openInputFolderChooser();
    void openTempFolderChooser();
    void openOutputFolderChooser();
    void upOneInputFolder();
    void upOneTempFolder();
    void upOneOutputFolder();
    void openInputFolder();
    void openTempFolder();
    void openOutputFolder();
    void resetPaths();
    void guessButton(bool defaultTempProtection = true);
    void copyInputToTemp();
    void openDiscogs();
    void openMusicBrainz();
    void openRedacted();
    void openSpekInit();
    void openTagger();
    void openAlbumArtFetcher();
    void on_ConvertToComboBox_currentIndexChanged(const QString &format);
    void convertInitialize();
    void on_CopyContentsCheckBox_stateChanged(int state);
    void on_XLSXSheetCheckBox_stateChanged(int state);
};

#endif // MAINWINDOW_H
