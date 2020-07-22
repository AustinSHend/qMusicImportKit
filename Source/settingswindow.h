#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <helper.h>

#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QSettings>
#include <QStandardPaths>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

private:
    Ui::SettingsWindow *ui;
    void folderChooser(QLineEdit* initLineEdit);
    void fileChooser(QLineEdit* initLineEdit, QString filters = "All files (*)");

private slots:
    void openDefaultInputFolderChooser();
    void openDefaultTempFolderChooser();
    void openDefaultOutputFolderChooser();
    void openDefaultXLSXSheetFileChooser();
    void openDefaultFLACFileChooser();
    void openDefaultLAMEFileChooser();
    void openDefaultOpusFileChooser();
    void openDefaultReplaygainFileChooser();
    void openDefaultGifsicleFileChooser();
    void openDefaultJPEGOptimFileChooser();
    void openDefaultOxiPNGFileChooser();
    void openDefaultSoXFileChooser();
    void openDefaultAlbumArtFetcherFileChooser();
    void openDefaultSpectrogramAnalysisFileChooser();
    void openDefaultTaggerFileChooser();
    void settingsAccept();
    void updateConversionOptions();
    void updateFLACOptions();
    void updateReplaygainOptions();
    void updateCompressionOptions(bool compressorDetected = false);
    void updateGifsicleOptions();
    void updateJPEGOptimOptions();
    void updateOxiPNGOptions();
    void updateSoXOptions();
    void updateXLSXOptions();
    void updateConvertPresetsOnFormatChange(const QString &format);
};

#endif // SETTINGSWINDOW_H
