#include "settingswindow.h"
#include "ui_settingswindow.h"

SettingsWindow::SettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    // Disable resizing hints
    setWindowFlags(Qt::Widget | Qt::MSWindowsFixedSizeDialogHint);

    // Set the placeholder text of the DefaultSyntaxComboBox manually, as it can't be changed in the UI designer
    ui->DefaultSyntaxComboBox->lineEdit()->setPlaceholderText("Output Folder + File Name (syntax in tooltip)");

    // If the program is being run from Windows, set the tagger to display as "MP3Tag" instead of "PuddleTag"
#if defined(Q_OS_WIN)
    ui->DefaultTaggerLineEdit->setPlaceholderText("MP3Tag Binary Location");
#endif

    // Read in user settings
    QSettings MIKSettings;

    if(QDir(MIKSettings.value("sDefaultInput", "").toString()).exists()) {
        ui->DefaultInputLineEdit->setText(QDir::toNativeSeparators(MIKSettings.value("sDefaultInput", "").toString()));
    }
    if(QDir(MIKSettings.value("sDefaultTemp", "").toString()).exists()) {
        ui->DefaultTempLineEdit->setText(QDir::toNativeSeparators(MIKSettings.value("sDefaultTemp", "").toString()));
    }
    if(QDir(MIKSettings.value("sDefaultOutput", "").toString()).exists()) {
        ui->DefaultOutputLineEdit->setText(QDir::toNativeSeparators(MIKSettings.value("sDefaultOutput", "").toString()));
    }
    ui->DefaultSyntaxComboBox->setCurrentText(MIKSettings.value("sDefaultSyntax", "").toString());
    ui->DefaultAutoWAVConvertCheckBox->setChecked(MIKSettings.value("bDefaultAutoWAVConvert", true).toBool());
    ui->DefaultRGCheckBox->setChecked(MIKSettings.value("bDefaultRG", true).toBool());
    ui->DefaultCopySpecificFileTypesCheckBox->setChecked(MIKSettings.value("bDefaultSpecificFileTypes", false).toBool());
    ui->DefaultCopySpecificFileTypesLineEdit->setText(MIKSettings.value("sDefaultSpecificFileTypesText", "").toString());
    ui->DefaultRenameLogCueCheckBox->setChecked(MIKSettings.value("bDefaultRenameLogCue", true).toBool());
    ui->DefaultCompressImagesCheckBox->setChecked(MIKSettings.value("bDefaultCompressImages", false).toBool());
    ui->DefaultCompressBMPCheckBox->setChecked(MIKSettings.value("bDefaultCompressBMP", false).toBool());
    ui->DefaultCompressGIFCheckBox->setChecked(MIKSettings.value("bDefaultCompressGIF", false).toBool());
    ui->DefaultCompressJPGCheckBox->setChecked(MIKSettings.value("bDefaultCompressJPG", false).toBool());
    ui->DefaultCompressPNGCheckBox->setChecked(MIKSettings.value("bDefaultCompressPNG", false).toBool());
    ui->DefaultXLSXSheetCheckBox->setChecked(MIKSettings.value("bDefaultXLSXSheet", false).toBool());
    ui->DefaultXLSXSheetLineEdit->setText(MIKSettings.value("sDefaultXLSXSheetLocation", "").toString());
    ui->DefaultDeleteSourceFolderCheckBox->setChecked(MIKSettings.value("bDefaultDeleteSourceFolder", true).toBool());
    ui->DefaultOpenFolderCheckBox->setChecked(MIKSettings.value("bDefaultOpenFolder", false).toBool());
    ui->DefaultConvertFormatComboBox->setCurrentText(MIKSettings.value("sDefaultConvertFormat", "FLAC").toString());
    updateConvertPresetsOnFormatChange(ui->DefaultConvertFormatComboBox->currentText());
    ui->DefaultConvertPresetComboBox->setCurrentText(MIKSettings.value("sDefaultConvertPreset", "Standard").toString());

    // Performance-conscious variable for holding the result of a checkInstalledProgram
    QString tempProgramLocation = "";

    // FLAC check
    tempProgramLocation = checkInstalledProgram("sDefaultFLACLocation", "flac");
    if(tempProgramLocation != "") {
        if(tempProgramLocation != "flac") {
            ui->DefaultFLACLineEdit->setText(QDir::toNativeSeparators(tempProgramLocation));
        }
        else {
            ui->DefaultFLACLineEdit->setText(QString("(FLAC autodetected)"));
        }
    }

    // LAME check
    tempProgramLocation = checkInstalledProgram("sDefaultLAMELocation", "lame");
    if(tempProgramLocation != "") {
        if(tempProgramLocation != "lame") {
            ui->DefaultLAMELineEdit->setText(QDir::toNativeSeparators(tempProgramLocation));
        }
        else {
            ui->DefaultLAMELineEdit->setText(QString("(LAME autodetected)"));
        }
    }

    // Opus check
    tempProgramLocation = checkInstalledProgram("sDefaultOpusLocation", "opusenc");
    if(tempProgramLocation != "") {
        if(tempProgramLocation != "opusenc") {
            ui->DefaultOpusLineEdit->setText(QDir::toNativeSeparators(tempProgramLocation));
        }
        else {
            ui->DefaultOpusLineEdit->setText(QString("(Opus autodetected)"));
        }
    }

    // BS1770GAIN check
    tempProgramLocation = checkInstalledProgram("sDefaultBS1770GAINLocation", "bs1770gain");
    if(tempProgramLocation != "") {
        if(tempProgramLocation != "bs1770gain") {
            ui->DefaultBS1770GAINLineEdit->setText(QDir::toNativeSeparators(tempProgramLocation));
        }
        else {
            ui->DefaultBS1770GAINLineEdit->setText(QString("(BS1770GAIN autodetected)"));
        }
    }

    // Gifsicle check
    tempProgramLocation = checkInstalledProgram("sDefaultGifsicleLocation", "gifsicle");
    if(tempProgramLocation != "") {
        if(tempProgramLocation != "gifsicle") {
            ui->DefaultGifsicleLineEdit->setText(QDir::toNativeSeparators(tempProgramLocation));
        }
        else {
            ui->DefaultGifsicleLineEdit->setText(QString("(Gifsicle autodetected)"));
        }
    }

    // JPEGOptim check
    tempProgramLocation = checkInstalledProgram("sDefaultJPEGOptimLocation", "jpegoptim");
    if(tempProgramLocation != "") {
        if(tempProgramLocation != "jpegoptim") {
            ui->DefaultJPEGOptimLineEdit->setText(QDir::toNativeSeparators(tempProgramLocation));
        }
        else {
            ui->DefaultJPEGOptimLineEdit->setText(QString("(JPEGOptim autodetected)"));
        }
    }

    // OxiPNG check
    tempProgramLocation = checkInstalledProgram("sDefaultOxiPNGLocation", "oxipng");
    if(tempProgramLocation != "") {
        if(tempProgramLocation != "oxipng") {
            ui->DefaultOxiPNGLineEdit->setText(QDir::toNativeSeparators(tempProgramLocation));
        }
        else {
            ui->DefaultOxiPNGLineEdit->setText(QString("(OxiPNG autodetected)"));
        }
    }

    // SoX check
    tempProgramLocation = checkInstalledProgram("sDefaultSoXLocation", "sox");
    if(tempProgramLocation != "") {
        if(tempProgramLocation != "sox") {
            ui->DefaultSoXLineEdit->setText(QDir::toNativeSeparators(tempProgramLocation));
        }
        else {
            ui->DefaultSoXLineEdit->setText(QString("(SoX autodetected)"));
        }
    }

    // AlbumArtDownloader check
#if defined(Q_OS_WIN)
    tempProgramLocation = checkInstalledProgram("sDefaultAlbumArtFetcherLocation");
    if(tempProgramLocation != "") {
        ui->DefaultAlbumArtFetcherLineEdit->setText(QDir::toNativeSeparators(tempProgramLocation));
    }
#elif defined(Q_OS_LINUX)
    ui->DefaultAlbumArtFetcherLineEdit->setVisible(false);
    ui->DefaultAlbumArtFetcherFileChooserButton->setVisible(false);
#endif

    // Spek check
    tempProgramLocation = checkInstalledProgram("sDefaultSpectrogramAnalysisLocation", "spek");
    if(tempProgramLocation != "") {
        if(tempProgramLocation != "spek") {
            ui->DefaultSpectrogramAnalysisLineEdit->setText(QDir::toNativeSeparators(tempProgramLocation));
        }
        else {
            ui->DefaultSpectrogramAnalysisLineEdit->setText(QString("(Spek autodetected)"));
        }
    }

    // Tagger check
    tempProgramLocation = checkInstalledProgram("sDefaultTaggerLocation", "puddletag");
    if(tempProgramLocation != "") {
        if(tempProgramLocation != "puddletag") {
            ui->DefaultTaggerLineEdit->setText(QDir::toNativeSeparators(tempProgramLocation));
        }
        else {
            ui->DefaultTaggerLineEdit->setText(QString("(PuddleTag autodetected)"));
        }
    }

    updateFLACOptions();
//    updateConversionOptions(); Necessary but updateFLACOptions also calls this so not needed for now
    updateBS1770GAINOptions();
    updateGifsicleOptions();
    updateJPEGOptimOptions();
    updateOxiPNGOptions();
    updateXLSXOptions();
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

// Open folder chooser dialog and tie it to a QLineEdit element with some error checking and defaults
void SettingsWindow::folderChooser(QLineEdit* initLineEdit) {
    QDir initialDir(initLineEdit->text());

    // Set the eventual dialog's default starting point to the Home folder
    QString fileDialogDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    // If the QLineEdit's manual path is valid
    if (initialDir.path() != "." && initialDir.exists()) {
        // Change the dialog to use it instead of the default
        fileDialogDir = initialDir.path();
    }

    // Launch folder picker
    QDir pickedDir = QDir(QFileDialog::getExistingDirectory(this, "Open Directory", fileDialogDir, QFileDialog::ShowDirsOnly));

    // If the folder chooser succeeded, set the QLineEdit to the result (folder chooser returns null string on cancellation)
    if (pickedDir.path() != ".") {
        initLineEdit->setText(QDir::toNativeSeparators(pickedDir.path()));
    }
}

// Open file chooser dialog and tie it to a QLineEdit element with some error checking and defaults
void SettingsWindow::fileChooser(QLineEdit* initLineEdit, QString filters) {
    // Create QDir from the nearest parent of the passed-in QLineEdit's text
    QDir initialDir(getNearestParent(initLineEdit->text()).path());

    // Set the eventual dialog's default starting point to the Home folder
    QString fileDialogDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    // If the QLineEdit's manual path is valid
    if (initialDir.path() != "." && initialDir.exists()) {
        // Change the dialog to use it instead of the default
        fileDialogDir = initialDir.path();
    }

    // Launch file picker
    QFile pickedFile(QFileDialog::getOpenFileName(this, tr("Open File"), fileDialogDir, filters));

    // If the file chooser succeeded, set the QLineEdit to the result (folder chooser returns null string on cancellation)
    if (pickedFile.fileName() != "") {
        initLineEdit->setText(QDir::toNativeSeparators(pickedFile.fileName()));
    }
}

void SettingsWindow::openDefaultInputFolderChooser() {
    folderChooser(ui->DefaultInputLineEdit);
}

void SettingsWindow::openDefaultTempFolderChooser() {
    folderChooser(ui->DefaultTempLineEdit);
}

void SettingsWindow::openDefaultOutputFolderChooser() {
    folderChooser(ui->DefaultOutputLineEdit);
}

void SettingsWindow::openDefaultXLSXSheetFileChooser() {
    fileChooser(ui->DefaultXLSXSheetLineEdit, "Microsoft Excel Open XML Spreadsheet (*.xlsx)");
    // Manually trigger a check for XLSX capability
    updateXLSXOptions();
}

void SettingsWindow::openDefaultFLACFileChooser() {
#if defined(Q_OS_LINUX)
    fileChooser(ui->DefaultFLACLineEdit);
#elif defined(Q_OS_WIN)
    fileChooser(ui->DefaultFLACLineEdit, "FLAC Binary (flac.exe)");
#endif
    // Manually trigger a check for FLAC capability
    updateFLACOptions();
}

void SettingsWindow::openDefaultLAMEFileChooser() {
#if defined(Q_OS_LINUX)
    fileChooser(ui->DefaultLAMELineEdit);
#elif defined(Q_OS_WIN)
    fileChooser(ui->DefaultLAMELineEdit, "LAME Binary (lame.exe)");
#endif
    // Manually trigger a check for LAME capability
    updateConversionOptions();
}

void SettingsWindow::openDefaultOpusFileChooser() {
#if defined(Q_OS_LINUX)
    fileChooser(ui->DefaultOpusLineEdit);
#elif defined(Q_OS_WIN)
    fileChooser(ui->DefaultOpusLineEdit, "Opus Binary (opusenc.exe)");
#endif
    // Manually trigger a check for Opus capability
    updateConversionOptions();
}

void SettingsWindow::openDefaultBS1770GAINFileChooser() {
#if defined(Q_OS_LINUX)
    fileChooser(ui->DefaultBS1770GAINLineEdit);
#elif defined(Q_OS_WIN)
    fileChooser(ui->DefaultBS1770GAINLineEdit, "BS1770GAIN Binary (bs1770gain.exe)");
#endif
    // Manually trigger a check for BS1770GAIN capability
    updateBS1770GAINOptions();
}

void SettingsWindow::openDefaultGifsicleFileChooser() {
#if defined(Q_OS_LINUX)
    fileChooser(ui->DefaultGifsicleLineEdit);
#elif defined(Q_OS_WIN)
    fileChooser(ui->DefaultGifsicleLineEdit, "Gifsicle Binary (gifsicle.exe)");
#endif
    // Manually trigger a check for Gifsicle capability
    updateGifsicleOptions();
}

void SettingsWindow::openDefaultJPEGOptimFileChooser() {
#if defined(Q_OS_LINUX)
    fileChooser(ui->DefaultJPEGOptimLineEdit);
#elif defined(Q_OS_WIN)
    fileChooser(ui->DefaultJPEGOptimLineEdit, "JPEGOptim Binary (jpegoptim.exe)");
#endif
    // Manually trigger a check for JPEGOptim capability
    updateJPEGOptimOptions();
}

void SettingsWindow::openDefaultOxiPNGFileChooser() {
#if defined(Q_OS_LINUX)
    fileChooser(ui->DefaultOxiPNGLineEdit);
#elif defined(Q_OS_WIN)
    fileChooser(ui->DefaultOxiPNGLineEdit, "OxiPNG Binary (oxipng.exe)");
#endif
    // Manually trigger a check for OxiPNG capability
    updateOxiPNGOptions();
}

void SettingsWindow::openDefaultSoXFileChooser() {
#if defined(Q_OS_LINUX)
    fileChooser(ui->DefaultSoXLineEdit);
#elif defined(Q_OS_WIN)
    fileChooser(ui->DefaultSoXLineEdit, "SoX Binary (sox.exe)");
#endif
    // Manually trigger a check for SoX capability
    updateConvertPresetsOnFormatChange(ui->DefaultConvertFormatComboBox->currentText());
}

void SettingsWindow::openDefaultAlbumArtFetcherFileChooser() {
#if defined(Q_OS_LINUX)
    fileChooser(ui->DefaultAlbumArtFetcherLineEdit);
#elif defined(Q_OS_WIN)
    fileChooser(ui->DefaultAlbumArtFetcherLineEdit, "Album Art Downloader Binary (AlbumArt.exe)");
#endif
}

void SettingsWindow::openDefaultSpectrogramAnalysisFileChooser() {
#if defined(Q_OS_LINUX)
    fileChooser(ui->DefaultSpectrogramAnalysisLineEdit);
#elif defined(Q_OS_WIN)
    fileChooser(ui->DefaultSpectrogramAnalysisLineEdit, "Spek Binary (spek.exe)");
#endif
}

void SettingsWindow::openDefaultTaggerFileChooser() {
#if defined(Q_OS_LINUX)
    fileChooser(ui->DefaultTaggerLineEdit);
#elif defined(Q_OS_WIN)
    fileChooser(ui->DefaultTaggerLineEdit, "Mp3tag Binary (Mp3tag.exe)");
#endif
}

// Runs when the user presses "Accept" and stores selected settings into the config file
// Mostly input validation
void SettingsWindow::settingsAccept() {
    // Save user settings
    QSettings MIKSettings;
    if(QDir(ui->DefaultInputLineEdit->text()).exists()) {
        MIKSettings.setValue("sDefaultInput", QDir::toNativeSeparators(ui->DefaultInputLineEdit->text()));
    }
    if(QDir(ui->DefaultTempLineEdit->text()).exists()) {
        MIKSettings.setValue("sDefaultTemp", QDir::toNativeSeparators(ui->DefaultTempLineEdit->text()));
    }
    if(QDir(ui->DefaultOutputLineEdit->text()).exists()) {
        MIKSettings.setValue("sDefaultOutput", QDir::toNativeSeparators(ui->DefaultOutputLineEdit->text()));
    }

    MIKSettings.setValue("sDefaultSyntax", ui->DefaultSyntaxComboBox->currentText());
    MIKSettings.setValue("bDefaultAutoWAVConvert", ui->DefaultAutoWAVConvertCheckBox->isChecked());
    MIKSettings.setValue("bDefaultRG", ui->DefaultRGCheckBox->isChecked());
    MIKSettings.setValue("bDefaultSpecificFileTypes", ui->DefaultCopySpecificFileTypesCheckBox->isChecked());
    MIKSettings.setValue("sDefaultSpecificFileTypesText", ui->DefaultCopySpecificFileTypesLineEdit->text());
    MIKSettings.setValue("bDefaultRenameLogCue", ui->DefaultRenameLogCueCheckBox->isChecked());
    MIKSettings.setValue("bDefaultCompressImages", ui->DefaultCompressImagesCheckBox->isChecked());
    MIKSettings.setValue("bDefaultCompressBMP", ui->DefaultCompressBMPCheckBox->isChecked());
    MIKSettings.setValue("bDefaultCompressGIF", ui->DefaultCompressGIFCheckBox->isChecked());
    MIKSettings.setValue("bDefaultCompressJPG", ui->DefaultCompressJPGCheckBox->isChecked());
    MIKSettings.setValue("bDefaultCompressPNG", ui->DefaultCompressPNGCheckBox->isChecked());
    MIKSettings.setValue("bDefaultDeleteSourceFolder", ui->DefaultDeleteSourceFolderCheckBox->isChecked());
    MIKSettings.setValue("bDefaultOpenFolder", ui->DefaultOpenFolderCheckBox->isChecked());
    MIKSettings.setValue("sDefaultConvertFormat", ui->DefaultConvertFormatComboBox->currentText());
    MIKSettings.setValue("sDefaultConvertPreset", ui->DefaultConvertPresetComboBox->currentText());

    // Don't allow XLSX to be enabled if the XLSX file is invalid
    MIKSettings.setValue("bDefaultXLSXSheet", false);
    if(ui->DefaultXLSXSheetLineEdit->text() == "" || QFileInfo(ui->DefaultXLSXSheetLineEdit->text()).isFile()) {
        MIKSettings.setValue("sDefaultXLSXSheetLocation", QDir::toNativeSeparators(ui->DefaultXLSXSheetLineEdit->text()));

        if(ui->DefaultXLSXSheetCheckBox->isChecked() && ui->DefaultXLSXSheetLineEdit->text() != "") {
            MIKSettings.setValue("bDefaultXLSXSheet", ui->DefaultXLSXSheetCheckBox->isChecked());
        }
    }

    if(ui->DefaultFLACLineEdit->text() == "" || QFileInfo(ui->DefaultFLACLineEdit->text()).isFile()) {
        MIKSettings.setValue("sDefaultFLACLocation", QDir::toNativeSeparators(ui->DefaultFLACLineEdit->text()));
    }
    if(ui->DefaultLAMELineEdit->text() == "" || QFileInfo(ui->DefaultLAMELineEdit->text()).isFile()) {
        MIKSettings.setValue("sDefaultLAMELocation", QDir::toNativeSeparators(ui->DefaultLAMELineEdit->text()));
    }
    if(ui->DefaultOpusLineEdit->text() == "" || QFileInfo(ui->DefaultOpusLineEdit->text()).isFile()) {
        MIKSettings.setValue("sDefaultOpusLocation", QDir::toNativeSeparators(ui->DefaultOpusLineEdit->text()));
    }
    if(ui->DefaultBS1770GAINLineEdit->text() == "" || QFileInfo(ui->DefaultBS1770GAINLineEdit->text()).isFile()) {
        MIKSettings.setValue("sDefaultBS1770GAINLocation", QDir::toNativeSeparators(ui->DefaultBS1770GAINLineEdit->text()));
    }
    if(ui->DefaultGifsicleLineEdit->text() == "" || QFileInfo(ui->DefaultGifsicleLineEdit->text()).isFile()) {
        MIKSettings.setValue("sDefaultGifsicleLocation", QDir::toNativeSeparators(ui->DefaultGifsicleLineEdit->text()));
    }
    if(ui->DefaultJPEGOptimLineEdit->text() == "" || QFileInfo(ui->DefaultJPEGOptimLineEdit->text()).isFile()) {
        MIKSettings.setValue("sDefaultJPEGOptimLocation", QDir::toNativeSeparators(ui->DefaultJPEGOptimLineEdit->text()));
    }
    if(ui->DefaultOxiPNGLineEdit->text() == "" || QFileInfo(ui->DefaultOxiPNGLineEdit->text()).isFile()) {
        MIKSettings.setValue("sDefaultOxiPNGLocation", QDir::toNativeSeparators(ui->DefaultOxiPNGLineEdit->text()));
    }
    if(ui->DefaultSoXLineEdit->text() == "" || QFileInfo(ui->DefaultSoXLineEdit->text()).isFile()) {
        MIKSettings.setValue("sDefaultSoXLocation", QDir::toNativeSeparators(ui->DefaultSoXLineEdit->text()));
    }
    if(ui->DefaultAlbumArtFetcherLineEdit->text() == "" || QFileInfo(ui->DefaultAlbumArtFetcherLineEdit->text()).isFile()) {
        MIKSettings.setValue("sDefaultAlbumArtFetcherLocation", QDir::toNativeSeparators(ui->DefaultAlbumArtFetcherLineEdit->text()));
    }
    if(ui->DefaultSpectrogramAnalysisLineEdit->text() == "" || QFileInfo(ui->DefaultSpectrogramAnalysisLineEdit->text()).isFile()) {
        MIKSettings.setValue("sDefaultSpectrogramAnalysisLocation", QDir::toNativeSeparators(ui->DefaultSpectrogramAnalysisLineEdit->text()));
    }
    if(ui->DefaultTaggerLineEdit->text() == "" || QFileInfo(ui->DefaultTaggerLineEdit->text()).isFile()) {
        MIKSettings.setValue("sDefaultTaggerLocation", QDir::toNativeSeparators(ui->DefaultTaggerLineEdit->text()));
    }
    this->close();
}

// Logic for automatically updating the format and preset comboboxes
void SettingsWindow::updateConversionOptions() {
    QSettings MIKSettings;

    // Holds the user's current temporary setting so it doesn't get reset when we clear and re-add encoders
    QString previousFormat = "";
    QString previousPreset = "";

    if(ui->DefaultConvertFormatComboBox->count() != 0) {
        previousFormat = ui->DefaultConvertFormatComboBox->currentText();
    }
    if(ui->DefaultConvertPresetComboBox->count() != 0) {
        previousPreset = ui->DefaultConvertPresetComboBox->currentText();
    }

    ui->DefaultConvertFormatComboBox->clear();

    // Boolean to handle the enabling/disabling of the combo boxes later on
    bool encoderDetected = false;

    if(checkInstalledProgram(ui->DefaultFLACLineEdit->text(), "flac", false) != "") {
        ui->DefaultConvertFormatComboBox->addItem("FLAC");
        encoderDetected = true;
    }
    if(checkInstalledProgram(ui->DefaultOpusLineEdit->text(), "opusenc", false) != "") {
        ui->DefaultConvertFormatComboBox->addItem("Opus");
        encoderDetected = true;
    }
    if(checkInstalledProgram(ui->DefaultLAMELineEdit->text(), "lame", false) != "") {
        ui->DefaultConvertFormatComboBox->addItem("MP3");
        encoderDetected = true;
    }

    if(encoderDetected) {
        ui->DefaultConvertFormatComboBox->setEnabled(true);
        ui->DefaultConvertPresetComboBox->setEnabled(true);
    }
    else {
        ui->DefaultConvertFormatComboBox->setEnabled(false);
        ui->DefaultConvertPresetComboBox->setEnabled(false);
    }

    // If the user's previous format setting still exists, put it back
    if(ui->DefaultConvertFormatComboBox->findText(previousFormat)) {
        ui->DefaultConvertFormatComboBox->setCurrentText(previousFormat);
    }
    else {
        ui->DefaultConvertFormatComboBox->setCurrentText(MIKSettings.value("sDefaultConvertFormat", "FLAC").toString());
    }

    // Manually trigger the second combo box to populate based on the format chosen above
    updateConvertPresetsOnFormatChange(ui->DefaultConvertFormatComboBox->currentText());

    // If the user's previous preset setting still exists, put it back
    if(ui->DefaultConvertPresetComboBox->findText(previousPreset)) {
        ui->DefaultConvertPresetComboBox->setCurrentText(previousPreset);
    }
    else {
        ui->DefaultConvertPresetComboBox->setCurrentText(MIKSettings.value("sDefaultConvertPreset", "Standard").toString());
    }
}

// Automatically enable FLAC capabilities when a valid FLAC binary is found
void SettingsWindow::updateFLACOptions() {
    QSettings MIKSettings;
    if(checkInstalledProgram(ui->DefaultFLACLineEdit->text(), "flac", false) != "") {
        if(!ui->DefaultAutoWAVConvertCheckBox->isEnabled()) {
            ui->DefaultAutoWAVConvertCheckBox->setChecked(MIKSettings.value("bDefaultAutoWAVConvert", true).toBool());
            ui->DefaultAutoWAVConvertCheckBox->setEnabled(true);
        }
    }
    else {
        ui->DefaultAutoWAVConvertCheckBox->setEnabled(false);
        ui->DefaultAutoWAVConvertCheckBox->setChecked(false);
    }

    updateConversionOptions();
}

// Automatically enable BS1770GAIN capabilities when a valid BS1770GAIN binary is found
void SettingsWindow::updateBS1770GAINOptions() {
    QSettings MIKSettings;
    if(checkInstalledProgram(ui->DefaultBS1770GAINLineEdit->text(), "bs1770gain", false) != "") {
        if(!ui->DefaultRGCheckBox->isEnabled()) {
            ui->DefaultRGCheckBox->setChecked(MIKSettings.value("bDefaultRG", true).toBool());
            ui->DefaultRGCheckBox->setEnabled(true);
        }
    }
    else {
        ui->DefaultRGCheckBox->setEnabled(false);
        ui->DefaultRGCheckBox->setChecked(false);
    }
}

// Enables the compression settings group if any compressors are detected
void SettingsWindow::updateCompressionOptions(bool compressorDetected) {
    QSettings MIKSettings;

    // compressorDetected gives a small average-case performance increase by preventing unnecessary "which" usage on Linux
    if(compressorDetected ||
       checkInstalledProgram(ui->DefaultGifsicleLineEdit->text(), "gifsicle", false) != "" ||
       checkInstalledProgram(ui->DefaultJPEGOptimLineEdit->text(), "jpegoptim", false) != "" ||
       checkInstalledProgram(ui->DefaultOxiPNGLineEdit->text(), "oxipng", false) != "") {
        if(!ui->DefaultCompressImagesCheckBox->isEnabled()) {
            ui->DefaultCompressImagesCheckBox->setChecked(MIKSettings.value("bDefaultCompressImages", false).toBool());
            ui->DefaultCompressImagesCheckBox->setEnabled(true);
        }
    }
    else {
        ui->DefaultCompressImagesCheckBox->setEnabled(false);
        ui->DefaultCompressImagesCheckBox->setChecked(false);
    }
}

// Automatically enable Gifsicle capabilities when a valid Gifsicle binary is found
void SettingsWindow::updateGifsicleOptions() {
    QSettings MIKSettings;
    if(checkInstalledProgram(ui->DefaultGifsicleLineEdit->text(), "gifsicle", false) != "") {
        if(!ui->DefaultCompressGIFCheckBox->isEnabled()) {
            ui->DefaultCompressGIFCheckBox->setChecked(MIKSettings.value("bDefaultCompressGIF", false).toBool());
            ui->DefaultCompressGIFCheckBox->setEnabled(true);
        }
        updateCompressionOptions(true);
    }
    else {
        ui->DefaultCompressGIFCheckBox->setEnabled(false);
        ui->DefaultCompressGIFCheckBox->setChecked(false);
    }

}

// Automatically enable JPEGOptim capabilities when a valid JPEGOptim binary is found
void SettingsWindow::updateJPEGOptimOptions() {
    QSettings MIKSettings;
    if(checkInstalledProgram(ui->DefaultJPEGOptimLineEdit->text(), "jpegoptim", false) != "") {
        if(!ui->DefaultCompressJPGCheckBox->isEnabled()) {
            ui->DefaultCompressJPGCheckBox->setChecked(MIKSettings.value("bDefaultCompressJPG", false).toBool());
            ui->DefaultCompressJPGCheckBox->setEnabled(true);
        }
        updateCompressionOptions(true);
    }
    else {
        ui->DefaultCompressJPGCheckBox->setEnabled(false);
        ui->DefaultCompressJPGCheckBox->setChecked(false);
    }
}

// Automatically enable OxiPNG capabilities when a valid OxiPNG binary is found
void SettingsWindow::updateOxiPNGOptions() {
    QSettings MIKSettings;
    if(checkInstalledProgram(ui->DefaultOxiPNGLineEdit->text(), "oxipng", false) != "") {
        if(!ui->DefaultCompressBMPCheckBox->isEnabled()) {
            ui->DefaultCompressBMPCheckBox->setChecked(MIKSettings.value("bDefaultCompressBMP", false).toBool());
            ui->DefaultCompressBMPCheckBox->setEnabled(true);
        }

        if(!ui->DefaultCompressPNGCheckBox->isEnabled()) {
            ui->DefaultCompressPNGCheckBox->setChecked(MIKSettings.value("bDefaultCompressPNG", false).toBool());
            ui->DefaultCompressPNGCheckBox->setEnabled(true);
        }
        updateCompressionOptions(true);
    }
    else {
        ui->DefaultCompressBMPCheckBox->setEnabled(false);
        ui->DefaultCompressBMPCheckBox->setChecked(false);
        ui->DefaultCompressPNGCheckBox->setEnabled(false);
        ui->DefaultCompressPNGCheckBox->setChecked(false);
    }

}

// Manually trigger a refresh of the preset combo-box to account for possible new SoX capabilities
void SettingsWindow::updateSoXOptions() {
    updateConvertPresetsOnFormatChange(ui->DefaultConvertFormatComboBox->currentText());
}

// Automatically enable XLSX capabilities when a valid XLSX file is found
void SettingsWindow::updateXLSXOptions() {
    QSettings MIKSettings;
    if(QFileInfo(ui->DefaultXLSXSheetLineEdit->text()).isFile()) {
        if(!ui->DefaultXLSXSheetCheckBox->isEnabled()) {
            ui->DefaultXLSXSheetCheckBox->setChecked(MIKSettings.value("bDefaultXLSXSheet", false).toBool());
            ui->DefaultXLSXSheetCheckBox->setEnabled(true);
        }
    }
    else {
        ui->DefaultXLSXSheetCheckBox->setEnabled(false);
        ui->DefaultXLSXSheetCheckBox->setChecked(false);
    }
}

// Runs when the conversion format is changed, in order to update available options in the preset QComboBox
void SettingsWindow::updateConvertPresetsOnFormatChange(const QString &format) {
    // Save previousPreset in order to restore it later
    // Required as this function will be triggered by SoX updates which do not change the base format but can add/remove presets
    QString previousPreset = "";

    // Clear presets
    ui->DefaultConvertPresetComboBox->clear();

    // FLAC
    if(format == "FLAC") {
        ui->DefaultConvertPresetComboBox->addItem("Standard");

        if(checkInstalledProgram(ui->DefaultSoXLineEdit->text(), "sox", false) != "") {
            ui->DefaultConvertPresetComboBox->addItem("Force 16-bit");
            ui->DefaultConvertPresetComboBox->addItem("Force 44.1kHz/48kHz");
            ui->DefaultConvertPresetComboBox->addItem("Force 16-bit and 44.1kHz/48kHz");
        }
    }

    // Opus
    else if(format == "Opus") {
        ui->DefaultConvertPresetComboBox->addItem("192kbps VBR");
        ui->DefaultConvertPresetComboBox->addItem("160kbps VBR");
        ui->DefaultConvertPresetComboBox->addItem("128kbps VBR");
        ui->DefaultConvertPresetComboBox->addItem("96kbps VBR");
        ui->DefaultConvertPresetComboBox->addItem("64kbps VBR");
        ui->DefaultConvertPresetComboBox->addItem("32kbps VBR");
    }

    // MP3
    else if(format == "MP3") {
        ui->DefaultConvertPresetComboBox->addItem("245kbps VBR (V0)");
        ui->DefaultConvertPresetComboBox->addItem("225kbps VBR (V1)");
        ui->DefaultConvertPresetComboBox->addItem("190kbps VBR (V2)");
        ui->DefaultConvertPresetComboBox->addItem("175kbps VBR (V3)");
        ui->DefaultConvertPresetComboBox->addItem("165kbps VBR (V4)");
        ui->DefaultConvertPresetComboBox->addItem("130kbps VBR (V5)");
        ui->DefaultConvertPresetComboBox->addItem("115kbps VBR (V6)");
        ui->DefaultConvertPresetComboBox->addItem("100kbps VBR (V7)");
        ui->DefaultConvertPresetComboBox->addItem("85kbps VBR (V8)");
        ui->DefaultConvertPresetComboBox->addItem("65kbps VBR (V9)");
        ui->DefaultConvertPresetComboBox->addItem("320kbps CBR");
        ui->DefaultConvertPresetComboBox->addItem("256kbps CBR");
        ui->DefaultConvertPresetComboBox->addItem("192kbps CBR");
        ui->DefaultConvertPresetComboBox->addItem("128kbps CBR");
        ui->DefaultConvertPresetComboBox->addItem("64kbps CBR");
    }

    // Restore saved preset if it still exists
    if(ui->DefaultConvertPresetComboBox->findText(previousPreset)) {
        ui->DefaultConvertPresetComboBox->setCurrentText(previousPreset);
    }
}
