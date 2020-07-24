#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Disable resizing hints
    setWindowFlags(Qt::Widget | Qt::MSWindowsFixedSizeDialogHint);

    // Set the placeholder text of the SyntaxComboBox manually, as it can't be changed in the UI designer
    ui->SyntaxComboBox->lineEdit()->setPlaceholderText("Output Folder + File Name (syntax in tooltip)");

    // If the program is being run from Windows, set the tagger to display as "MP3Tag" instead of "PuddleTag"
#if defined(Q_OS_WIN)
    ui->TagButton->setText("MP3Tag");
#endif

    applyUserSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Reads and applies settings from the configuration file
// Intended to be run after setting new settings so the UI updates its features
void MainWindow::applyUserSettings() {
    QSettings MIKSettings;

    // Reset artist/album QLineEdits
    ui->ArtistLineEdit->setText("");
    ui->AlbumLineEdit->setText("");

    // Read in user settings
    ui->InputLineEdit->setText(QDir::toNativeSeparators(MIKSettings.value("sDefaultInput", "").toString()));
    ui->TempLineEdit->setText(QDir::toNativeSeparators(MIKSettings.value("sDefaultTemp", "").toString()));
    ui->OutputLineEdit->setText(QDir::toNativeSeparators(MIKSettings.value("sDefaultOutput", "").toString()));

    ui->SyntaxComboBox->setCurrentText(MIKSettings.value("sDefaultSyntax", "").toString());
    ui->ReplayGainCheckBox->setChecked(MIKSettings.value("bDefaultRG", true).toBool());

    ui->DeleteTempFolderCheckBox->setChecked(MIKSettings.value("bDefaultDeleteSourceFolder", true).toBool());
    ui->ConvertOpenFolderCheckBox->setChecked(MIKSettings.value("bDefaultOpenFolder", false).toBool());

    // Clear the convert format and preset in preparation for dynamically adding options in the next section
    ui->ConvertToComboBox->clear();
    ui->ConvertToPresetComboBox->clear();

    // Set the convert format and preset to disabled, for future enablement on detecting a valid encoder
    ui->ConvertToComboBox->setEnabled(false);
    ui->ConvertToPresetComboBox->setEnabled(false);

    // FLAC check
    if(checkInstalledProgram("sDefaultFLACLocation", "flac") != "") {
        ui->ConvertToComboBox->setEnabled(true);
        ui->ConvertToPresetComboBox->setEnabled(true);
        ui->ConvertToComboBox->addItem("FLAC");
        ui->AutoWavConvertCheckBox->setEnabled(true);
        ui->AutoWavConvertCheckBox->setChecked(MIKSettings.value("bDefaultAutoWAVConvert", true).toBool());
        ui->AutoWavConvertCheckBox->setText("Convert input .wav files to .flac");
    }
    else {
        ui->AutoWavConvertCheckBox->setEnabled(false);
        ui->AutoWavConvertCheckBox->setChecked(false);
        ui->AutoWavConvertCheckBox->setText("Convert input .wav files to .flac\n(requires FLAC)");
    }

    // Opus check
    if(checkInstalledProgram("sDefaultOpusLocation", "opusenc") != "") {
        ui->ConvertToComboBox->setEnabled(true);
        ui->ConvertToPresetComboBox->setEnabled(true);
        ui->ConvertToComboBox->addItem("Opus");
    }

    // LAME check
    if(checkInstalledProgram("sDefaultLAMELocation", "lame") != "") {
        ui->ConvertToComboBox->setEnabled(true);
        ui->ConvertToPresetComboBox->setEnabled(true);
        ui->ConvertToComboBox->addItem("MP3");
    }

    // AlbumArtDownloader check
#if defined(Q_OS_WIN)
    if(checkInstalledProgram("sDefaultAlbumArtFetcherLocation") != "") {
#elif defined(Q_OS_LINUX)
    if(MIKSettings.value("sDefaultAlbumArtFetcherLocation", "").toString() != "") {
#endif
        ui->AlbumArtButton->setEnabled(true);
    }
    else {
        ui->AlbumArtButton->setEnabled(false);
    }

    // Spek check
    if(checkInstalledProgram("sDefaultSpectrogramAnalysisLocation", "spek") != "") {
        ui->SpekButton->setEnabled(true);
    }
    else {
        ui->SpekButton->setEnabled(false);
    }

    // Puddletag/MP3Tag check
    if(checkInstalledProgram("sDefaultTaggerLocation", "puddletag") != "") {
        ui->TagButton->setEnabled(true);
    }
    else {
        ui->TagButton->setEnabled(false);
    }

// Linux Loudgain check is normal
#if defined(Q_OS_LINUX)
    if(checkInstalledProgram("sDefaultLoudgainLocation", "loudgain") != "") {
// Windows Loudgain needs to be detected through WSL
#elif defined(Q_OS_WIN)
    if(isWSLLoudgainAvailable()) {
#endif
        ui->ReplayGainCheckBox->setEnabled(true);
        ui->ReplayGainCheckBox->setChecked(MIKSettings.value("bDefaultRG", true).toBool());
        ui->ReplayGainCheckBox->setText("Apply ReplayGain");
    }
    else {
        ui->ReplayGainCheckBox->setEnabled(false);
        ui->ReplayGainCheckBox->setChecked(false);
        ui->ReplayGainCheckBox->setText("Apply ReplayGain (requires Loudgain)");
    }

    // Read in more user settings
    ui->RenameLogCueCheckBox->setChecked(MIKSettings.value("bDefaultRenameLogCue", true).toBool());
    ui->CopyContentsLineEdit->setText(MIKSettings.value("sDefaultSpecificFileTypesText", "").toString());
    ui->CopyContentsCheckBox->setChecked(MIKSettings.value("bDefaultSpecificFileTypes", false).toBool());

    // Manually trigger the logic for checking/unchecking CopyContentsCheckBox for dynamic disabling of other elements
    if(ui->CopyContentsCheckBox->isChecked() == true) {
        on_CopyContentsCheckBox_stateChanged(2);
    }
    else {
        on_CopyContentsCheckBox_stateChanged(0);
    }

    // Set the conversion format and preset to the user's default. Handles properly if the user setting is missing or junk
    ui->ConvertToComboBox->setCurrentText(MIKSettings.value("sDefaultConvertFormat", "").toString());
    ui->ConvertToPresetComboBox->setCurrentText(MIKSettings.value("sDefaultConvertPreset", "").toString());

    // Allows search on redacted
    if(MIKSettings.value("bRedactedEnabled", false).toBool()) {
        ui->RedactedButton->setEnabled(true);
        ui->RedactedButton->setHidden(false);
    }
    else {
        ui->RedactedButton->setHidden(true);
        ui->RedactedButton->setEnabled(false);
    }
}

// Open folder chooser dialog and tie it to a QLineEdit element with some error checking and defaults
void MainWindow::folderChooser(QLineEdit *initLineEdit) {
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

void MainWindow::on_actionQuit_triggered() {
    this->close();
}

void MainWindow::on_actionConfigure_triggered() {
    SettingsWindow *settingsWindow = new SettingsWindow();
    settingsWindow->exec();
    applyUserSettings();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutWindow *aboutWindow = new AboutWindow();
    aboutWindow->exec();
}


// Open the directory from a QLineEdit in the user's default file manager
void MainWindow::folderOpen(QLineEdit *initLineEdit) {
    QDir initialDir(initLineEdit->text());

    // Set the eventual file manager's default starting point to the Home folder
    QString fileDialogPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    // If the QLineEdit's text is valid
    if (initialDir.path() != "." && initialDir.exists()) {
        // Change the dialog to use it instead of the default
        fileDialogPath = initialDir.path();
    }

    // Launch file manager
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileDialogPath));
}

void MainWindow::openInputFolderChooser() {
    folderChooser(ui->InputLineEdit);
}

void MainWindow::openTempFolderChooser() {
    folderChooser(ui->TempLineEdit);
    // Manually cause a "guess" to occur
    guessButton();
}

void MainWindow::openOutputFolderChooser() {
    folderChooser(ui->OutputLineEdit);
}

void MainWindow::upOneInputFolder() {
    ui->InputLineEdit->setText(QDir::toNativeSeparators(getNearestParent(ui->InputLineEdit->text()).path()));
}

void MainWindow::upOneTempFolder() {
    ui->TempLineEdit->setText(QDir::toNativeSeparators(getNearestParent(ui->TempLineEdit->text()).path()));
}

void MainWindow::upOneOutputFolder() {
    ui->OutputLineEdit->setText(QDir::toNativeSeparators(getNearestParent(ui->OutputLineEdit->text()).path()));
}

void MainWindow::openInputFolder() {
    folderOpen(ui->InputLineEdit);
}

void MainWindow::openTempFolder() {
    folderOpen(ui->TempLineEdit);
}

void MainWindow::openOutputFolder() {
    folderOpen(ui->OutputLineEdit);
}

// Resets the input, temp, and output QLineEdit text
void MainWindow::resetPaths() {
    QSettings MIKSettings;
    QString defaultInputPath = MIKSettings.value("sDefaultInput", "").toString();
    QString defaultTempPath = MIKSettings.value("sDefaultTemp", "").toString();
    QString defaultOutputPath = MIKSettings.value("sDefaultTemp", "").toString();

    // Change paths into platform-specific directory structure
    defaultInputPath = QDir::toNativeSeparators(defaultInputPath);
    defaultTempPath = QDir::toNativeSeparators(defaultTempPath);
    defaultOutputPath = QDir::toNativeSeparators(defaultOutputPath);
    
    QDir defaultInputDir(defaultInputPath);
    QDir defaultTempDir(defaultTempPath);
    QDir defaultOutputDir(defaultOutputPath);

    // If stored defaultInputPath is valid
    if(defaultInputDir.path() != "." && defaultInputDir.exists()) {
        // Transfer it to the QLineEdit text
        ui->InputLineEdit->setText(QDir::toNativeSeparators(defaultInputPath));
    }
    else {
        // Else reset to blank
        ui->InputLineEdit->setText("");
    }

    // If stored defaultTempPath is valid
    if(defaultTempDir.path() != "." && defaultTempDir.exists()) {
        // Transfer it to the QLineEdit text
        ui->TempLineEdit->setText(QDir::toNativeSeparators(defaultTempPath));
    }
    else {
        // Else reset to blank
        ui->TempLineEdit->setText("");
    }

    // If stored defaultOutputPath is valid
    if(defaultOutputDir.path() != "." && defaultOutputDir.exists()) {
        // Transfer it to the QLineEdit text
        ui->OutputLineEdit->setText(QDir::toNativeSeparators(defaultOutputPath));
    }
    else {
        // Else reset to blank
        ui->OutputLineEdit->setText("");
    }
}

// Scans the first available file's metadata to fill out the artist and album textboxes
void MainWindow::guessButton(bool defaultTempProtection) {
    QSettings MIKSettings;

    QDir tempDir(ui->TempLineEdit->text());

    // Return if the temp path is invalid
    if (tempDir.path() == "." || !tempDir.exists()) {
        return;
    }

    // Return if the temp path is the same as the default temp path
    if(defaultTempProtection == true && QDir::toNativeSeparators(ui->TempLineEdit->text()) == QDir::toNativeSeparators(MIKSettings.value("sDefaultTemp", "").toString())) {
        return;
    }

    // Add all FLACs in the temp path to a QStringList
    QStringList inputFLACs = findFiles(tempDir.path(), {"*.flac"});

    // If no FLACs are found, return
    if(inputFLACs.count() == 0) {
        return;
    }

    // Get the tags of the first FLAC in the list

    // Linux only wants StdStrings, while Windows prefers StdWStrings (char encoding errors possible if Windows uses StdStrings)
#if defined(Q_OS_LINUX)
    TagLib::FLAC::File firstFLACTagFile(inputFLACs[0].toStdString().data());
#elif defined(Q_OS_WIN)
    TagLib::FLAC::File firstFLACTagFile(inputFLACs[0].toStdWString().data());
#endif

    // Parse the tags for artist (preferred: albumartist, then album artist, then artist)
    if(firstFLACTagFile.properties().contains("albumartist")) {
        ui->ArtistLineEdit->setText(TStringToQString(firstFLACTagFile.properties()["albumartist"].front()).trimmed());
    }
    else if(firstFLACTagFile.properties().contains("album artist")) {
        ui->ArtistLineEdit->setText(TStringToQString(firstFLACTagFile.properties()["album artist"].front()).trimmed());
    }
    else if(firstFLACTagFile.properties().contains("artist")) {
        ui->ArtistLineEdit->setText(TStringToQString(firstFLACTagFile.properties()["artist"].front()).trimmed());
    }

    // Parse the tags for album
    if(firstFLACTagFile.properties().contains("album")) {
        ui->AlbumLineEdit->setText(TStringToQString(firstFLACTagFile.properties()["album"].front()).trimmed());
    }
}

// Renames .logs and .cues in an input file list to the standard naming scheme used by EAC (%artist% - %album%.log and %album%.cue)
// Fails if there are multiple .logs or multiple .cues in the input list, as this means there are multiple discs and we cannot determine what their ordering is
void MainWindow::renameLogCue(QStringList inputFiles, QDir outputDir, QString artist, QString album) {
    // Find .logs and .cues that were copied
    QStringList logList = inputFiles.filter(QRegExp("^.*\\.log$", Qt::CaseInsensitive));
    QStringList cueList = inputFiles.filter(QRegExp("^.*\\.cue$", Qt::CaseInsensitive));

    // If there are 2 or more .logs/.cues in the output, alert the user to rename manually (not possible to detect which is from CD1/CD2/etc)
    if(logList.count() >= 2 || cueList.count() >= 2) {
        QMessageBox::warning(this, "Warning", "More than one .log/.cue detected in output folder. Rename manually.", QMessageBox::Ok);
        // Launch file manager
        QDesktopServices::openUrl(QUrl(outputDir.path(), QUrl::TolerantMode));
    }
    else {
        // If there's only one .log, rename it (only if it's not already named properly)
        if(logList.count() == 1 && !QFile(outputDir.path() + "/" + artist + " - " + album + ".log").exists()) {
            if(QFile(logList[0]).exists()) {
                QFile(logList[0]).rename(outputDir.path() + "/" + artist + " - " + album + ".log");
            }
        }
        // If there's only one .cue, rename it (only if it's not already named properly)
        if(cueList.count() == 1 && !QFile(outputDir.path() + "/" + album + ".cue").exists()) {
            if(QFile(cueList[0]).exists()) {
                QFile(cueList[0]).rename(outputDir.path() + "/" + album + ".cue");
            }
        }
    }
}

// Copies a folder+files into another
QStringList MainWindow::folderCopy(QDir fromDir, QDir toDir, QStringList patternList, QStringList dontCopyList) {
    // List of successfully copied files for eventual return
    QStringList copiedFiles;

    // Trim spaces from the beginning and end of each string
    patternList.replaceInStrings(QRegExp("^\\s+|\\s+$"), "");

    // If the source directory doesn't exist or the source and target directory are the same, return an empty string list
    if(!fromDir.exists() || fromDir.path() == toDir.path() || !getNearestParent(toDir.path()).exists()) {
        return QStringList{};
    }

    // Get a list of all matched files that are in the source folder
    QStringList pendingFiles = findFiles(fromDir.path(), patternList);

    // Remove any files that are specifically banned from being copied (usually used to ban the copying of original FLACs over new FLACs)
    foreach (QString dontCopyString, dontCopyList) {
        pendingFiles.removeAll(dontCopyString);
    }

    // For every file in the pendingFiles
    foreach (QString currentFileString, pendingFiles) {
        QFile currentFile(currentFileString);
        // Make a string to hold the file's current location
        QString toFilePath = currentFileString;
        // Change the old directory to the new directory in the string
        toFilePath.replace(fromDir.path(), toDir.path());
        // Make sure the path for this new file exists
        QDir().mkpath(QFileInfo(toFilePath).path());
        // Copy the old file to this new string location
        currentFile.copy(toFilePath);
        // Add it to the list of copied files
        copiedFiles += toFilePath;
    }

    return copiedFiles;
}

// Worker for copying the input folder into the temp folder, intended so the GUI thread doesn't lock up
void MainWindow::copyInputToTempWorker(QDir inputDir, QDir tempDir, bool convertWavs) {
    // Copy the input to the output folder and get a list of files that were successfully copied
    QStringList copiedFiles = folderCopy(inputDir, tempDir);

    // If the WAV conversion checkbox is checked
    if(convertWavs == true) {
        // Get a list of "*.wav" files from the copiedFiles list
        QStringList inputWAVs = copiedFiles.filter(".wav");

        // If there are WAVs to be converted
        if(!inputWAVs.empty()) {
            // Update the copy stage
            ui->CopyButton->setText("Converting WAVs..."); // Technically not thread-safe but no competing events

            // Initialize a pool for parallel threads. Default number of parallel threads is equal to processor's logical core count
            QThreadPool copyPool;

            // For each WAV in the inputWAVs list
            foreach (QString currentWAV, inputWAVs) {
                // Pass that WAV into the convertWAV function in its own thread.
                // The pool will execute the proper number of threads in parallel and will block subsequent WAVs until it has a slot open
                QtConcurrent::run(&copyPool, convertWAV, currentWAV);
            }

            // Wait for all WAVs to be converted before proceeding
            copyPool.waitForDone();
        }
    }

    // Set the UI back to normal to indicate copying is finished
    ui->CopyButton->setText("Copy input folder to temp folder"); // Technically not thread-safe but no competing events
    ui->CopyButton->setEnabled(true); // Technically not thread-safe but no competing events

    // Set the UI's temp path to the folder we just copied for further manipulation
    QString tempText = QDir::toNativeSeparators(tempDir.path());
    ui->TempLineEdit->setText(tempText); // Technically not thread-safe but no competing events

    // Autoguess the metadata
    guessButton();
}

// Copies the contents of the input path to the temp path
void MainWindow::copyInputToTemp() {
    // Read in user settings
    QSettings MIKSettings;
    QDir defaultInputDir(MIKSettings.value("inputPath", "").toString());
    // Create QDirs from QLineEdits
    QDir inputDir(ui->InputLineEdit->text());
    QDir tempDir(ui->TempLineEdit->text());

    // Return if either of the paths are invalid
    if(inputDir.path() == "." || tempDir.path() == "." || !inputDir.exists() || !tempDir.exists()) {
        QMessageBox::critical(this, "Alert", "Input or temp path invalid.", QMessageBox::Ok);
        return;
    }

    // Display a warning if the inputDir is the same as the user's stored defaultInputPath from settings
    if(inputDir.path() == defaultInputDir.path()) {
        QMessageBox::StandardButton warning = QMessageBox::warning(this, "Warning", "Are you sure you want to copy the default input folder?", QMessageBox::Yes | QMessageBox::No);
        if(warning == QMessageBox::No){
            return;
        }
    }

    // Disable copy button to denote process is executing
    ui->CopyButton->setText("Copying...");
    ui->CopyButton->setEnabled(false);

    // Start the copy process in another thread
    QtConcurrent::run(this, &MainWindow::copyInputToTempWorker, inputDir, QDir(tempDir.path() + "/" + inputDir.dirName()), ui->AutoWavConvertCheckBox->isChecked());
}

// Opens Discogs in the default web browser based on guessed metadata
void MainWindow::openDiscogs() {
    // If artist and album textboxes are not empty
    if (ui->ArtistLineEdit->text() != "" && ui->AlbumLineEdit->text() != "") {
        QDesktopServices::openUrl("https://www.discogs.com/search/?type=release&artist=" + ui->ArtistLineEdit->text() + "&title=" + ui->AlbumLineEdit->text());
    }
    // If only artist is not empty
    else if (ui->ArtistLineEdit->text() != "") {
        QDesktopServices::openUrl("https://www.discogs.com/search/?q=" + ui->ArtistLineEdit->text() + "&type=artist");
    }
    // If only album is not empty
    else if (ui->AlbumLineEdit->text() != "") {
        QDesktopServices::openUrl("https://www.discogs.com/search/?q=" + ui->AlbumLineEdit->text() + "&type=release");
    }
    else {
        QMessageBox::warning(this, "Warning", "No artist or album specified.", QMessageBox::Ok);
    }
}

// Opens MusicBrainz in the default web browser based on guessed metadata
void MainWindow::openMusicBrainz() {
    // If artist and album textboxes are not empty
    if (ui->ArtistLineEdit->text() != "" && ui->AlbumLineEdit->text() != "") {
        QDesktopServices::openUrl("https://musicbrainz.org/taglookup?tag-lookup.artist=" + ui->ArtistLineEdit->text() + "&tag-lookup.release=" + ui->AlbumLineEdit->text());
    }
    // If only artist is not empty
    else if (ui->ArtistLineEdit->text() != "") {
        QDesktopServices::openUrl("https://musicbrainz.org/search?query=" + ui->ArtistLineEdit->text() + "&type=artist");
    }
    // If only album is not empty
    else if (ui->AlbumLineEdit->text() != "") {
        QDesktopServices::openUrl("https://musicbrainz.org/search?query=" + ui->AlbumLineEdit->text() + "&type=release");
    }
    else {
        QMessageBox::warning(this, "Warning", "No artist or album specified.", QMessageBox::Ok);
    }
}

// Opens redacted in the default web browser based on guessed metadata
void MainWindow::openRedacted() {
    // If artist and album textboxes are not empty
    if (ui->ArtistLineEdit->text() != "" && ui->AlbumLineEdit->text() != "") {
        QDesktopServices::openUrl("https://redacted.ch/torrents.php?artistname=" + ui->ArtistLineEdit->text() + "&groupname=" + ui->AlbumLineEdit->text() + "&order_by=seeders&order_way=desc&group_results=1&filter_cat[1]=1&action=basic&searchsubmit=1");
    }
    // If only artist is not empty
    else if (ui->ArtistLineEdit->text() != "") {
        QDesktopServices::openUrl("https://redacted.ch/artist.php?artistname=" + ui->ArtistLineEdit->text() + "&type=artist");
    }
    // If only album is not empty
    else if (ui->AlbumLineEdit->text() != "") {
        QDesktopServices::openUrl("https://redacted.ch/artist.php?groupname=" + ui->AlbumLineEdit->text() + "&order_by=seeders&order_way=desc&group_results=1&filter_cat[1]=1&action=basic&searchsubmit=1");
    }
    else {
        QMessageBox::warning(this, "Warning", "No artist or album specified.", QMessageBox::Ok);
    }
}

// Opens the default tagger (Puddletag on Linux/Mac and MP3tag on Windows), pointed at the temp folder
void MainWindow::openTagger() {
    QDir tempDir(ui->TempLineEdit->text());

    // Return if the path is invalid
    if(tempDir.path() == "." || !tempDir.exists()) {
        QMessageBox::critical(this, "Alert", "Temp path is invalid.", QMessageBox::Ok);
        return;
    }

    // QStringList of FLACs that reside in the temp path
    QStringList inputFLACs = findFiles(tempDir, QStringList({"*.flac"}));

    // If no FLACs are found, return
    if(inputFLACs.count() == 0) {
        QMessageBox::critical(this, "Alert", "No FLACs found in the temp path.", QMessageBox::Ok);
        return;
    }

    // Initiate the process for PuddleTag or MP3Tag, depending on the OS
    QProcess taggerProcess;
    QString programLocation = checkInstalledProgram("sDefaultTaggerLocation", "puddletag");
    if(programLocation == "") {
        return;
    }
    taggerProcess.setProgram(programLocation);
    // Both PuddleTag and MP3Tag accept the same argument format
    taggerProcess.setArguments({QDir::toNativeSeparators(tempDir.path())});

    taggerProcess.startDetached();
}

// Opens AlbumArtDownloader, pointed at the temp folder
void MainWindow::openAlbumArtFetcher() {
    // Read in user settings
    QSettings MIKSettings;

    // Return if either the artist or album textbox is empty
    if(ui->ArtistLineEdit->text().isEmpty() || ui->AlbumLineEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "No artist or album specified.", QMessageBox::Ok);
        return;
    }

    QDir tempDir(ui->TempLineEdit->text());

    // Return if temp path is invalid
    if(tempDir.path() == "." || !tempDir.exists()) {
        QMessageBox::critical(this, "Alert", "Temp path is invalid.", QMessageBox::Ok);
        return;
    }

    // Initiate AAD process
    QProcess albumArtFetcherProcess;
    // AAD Arguments
    // -ar: artist to query
    // -al: album to query
    // -p: path to save the resultant download to
    QStringList arguments;
#if defined(Q_OS_WIN)
    QString programLocation = checkInstalledProgram("sDefaultAlbumArtFetcherLocation");
    if(programLocation == "") {
        return;
    }
    arguments << "-ar" << ui->ArtistLineEdit->text() << "-al" << ui->AlbumLineEdit->text() << "-p" << QDir::toNativeSeparators(tempDir.path() + "/folder.%extension%");
#elif defined(Q_OS_LINUX)
    // On Linux we run with sh and -c (command) so it can handle commands more robustly than we can mimick
    QString programLocation = "sh";
    // We need to quote and escape the arguments manually for sh to understand
    arguments << "-c" << MIKSettings.value("sDefaultAlbumArtFetcherLocation", "").toString() + " -ar \"" + ui->ArtistLineEdit->text() + "\" -al \"" + ui->AlbumLineEdit->text() + "\" -p \"" + QDir::toNativeSeparators(tempDir.path() + "/folder.%extension%\"");
#endif
    albumArtFetcherProcess.setProgram(programLocation);
    albumArtFetcherProcess.setArguments(arguments);

    albumArtFetcherProcess.startDetached();
}

// Helper function for opening Spek, pointed at the temp folder
// Passes arguments into openSpekWorker, which will run in its own thread
void MainWindow::openSpekInit() {
    QDir tempDir(ui->TempLineEdit->text());

    // Return if temp path is invalid
    if(tempDir.path() == "." || !tempDir.exists()) {
        QMessageBox::critical(this, "Alert", "Temp path is invalid.", QMessageBox::Ok);
        return;
    }

    // QStringList of FLACs that reside in the temp path
    QStringList inputFLACs = findFiles(tempDir, QStringList({"*.flac"}));

    // If no FLACs are found, return
    if(inputFLACs.count() == 0) {
        QMessageBox::critical(this, "Alert", "No FLACs found in the temp path.", QMessageBox::Ok);
        return;
    }

    // Start openSpekWorker in another thread
    QtConcurrent::run(QThreadPool::globalInstance(), openSpekWorker, inputFLACs);
}

// Runs when the conversion format is changed, in order to update available options in the preset QComboBox
void MainWindow::on_ConvertToComboBox_currentIndexChanged(const QString &format)
{
    // Clear presets
    ui->ConvertToPresetComboBox->clear();

    // FLAC
    if(format == "FLAC") {
        ui->ConvertToPresetComboBox->addItem("Standard");
        if(checkInstalledProgram("sDefaultSoXLocation", "sox") != "") {
            ui->ConvertToPresetComboBox->addItem("Force 16-bit");
            ui->ConvertToPresetComboBox->addItem("Force 44.1kHz/48kHz");
            ui->ConvertToPresetComboBox->addItem("Force 16-bit and 44.1kHz/48kHz");
        }

        // Set to "Standard" by default
        ui->ConvertToPresetComboBox->setCurrentText("Standard");
    }

    // Opus
    else if(format == "Opus") {
        ui->ConvertToPresetComboBox->addItem("192kbps VBR");
        ui->ConvertToPresetComboBox->addItem("160kbps VBR");
        ui->ConvertToPresetComboBox->addItem("128kbps VBR");
        ui->ConvertToPresetComboBox->addItem("96kbps VBR");
        ui->ConvertToPresetComboBox->addItem("64kbps VBR");
        ui->ConvertToPresetComboBox->addItem("32kbps VBR");

        // Set to "192kbps VBR" by default
        ui->ConvertToPresetComboBox->setCurrentText("192kbps VBR");
    }

    // MP3
    else if(format == "MP3") {
        ui->ConvertToPresetComboBox->addItem("245kbps VBR (V0)");
        ui->ConvertToPresetComboBox->addItem("225kbps VBR (V1)");
        ui->ConvertToPresetComboBox->addItem("190kbps VBR (V2)");
        ui->ConvertToPresetComboBox->addItem("175kbps VBR (V3)");
        ui->ConvertToPresetComboBox->addItem("165kbps VBR (V4)");
        ui->ConvertToPresetComboBox->addItem("130kbps VBR (V5)");
        ui->ConvertToPresetComboBox->addItem("115kbps VBR (V6)");
        ui->ConvertToPresetComboBox->addItem("100kbps VBR (V7)");
        ui->ConvertToPresetComboBox->addItem("85kbps VBR (V8)");
        ui->ConvertToPresetComboBox->addItem("65kbps VBR (V9)");
        ui->ConvertToPresetComboBox->addItem("320kbps CBR");
        ui->ConvertToPresetComboBox->addItem("256kbps CBR");
        ui->ConvertToPresetComboBox->addItem("192kbps CBR");
        ui->ConvertToPresetComboBox->addItem("128kbps CBR");
        ui->ConvertToPresetComboBox->addItem("64kbps CBR");

        // Set to "245kbps VBR (V0)" by default
        ui->ConvertToPresetComboBox->setCurrentText("245kbps VBR (V0)");
    }
}

// Calculates ReplayGain information (album and track-based) for the QStringList of inputFLACs
void MainWindow::calculateReplayGain (QStringList inputFLACs) {
    // Sort the files to ensure we process them in the right order
    inputFLACs.sort();

    QProcess LoudgainProcess;
    // Linux uses normal Loudgain
#if defined(Q_OS_LINUX)
    QString programLocation = checkInstalledProgram("sDefaultLoudgainLocation", "loudgain");
    if(programLocation == "") {
        return;
    }
    // Windows requires WSL Loudgain as there is no native binary (yet)
#elif defined(Q_OS_WIN)
    QString programLocation = "wsl";
#endif
    LoudgainProcess.setProgram(programLocation);

    // Loudgain arguments
    // -a: calculates album gain
    // -k: prevents clipping
    // -s e: extra information calculation (Reference loudness and range)
    QStringList arguments;
#if defined(Q_OS_WIN)
    arguments << "loudgain";
#endif
    arguments << "-a" << "-k" << "-s" << "e";
    foreach (QString currentFLAC, inputFLACs) {
#if defined(Q_OS_LINUX)
        arguments << QDir::toNativeSeparators(currentFLAC);
#elif defined(Q_OS_WIN)
        // Windows needs special handholding to convert from a NT path to a WSL path (C:\Users -> /mnt/c/Users)
        arguments << getWSLPath(currentFLAC);
#endif
    }

    LoudgainProcess.setArguments(arguments);

    // Start and wait
    LoudgainProcess.start();
    LoudgainProcess.waitForFinished(-1);

    // Manually insert a traditional reference loudness (e.g. 89 dB) instead of loudgain's relative reference loudness (e.g. -18 dB)
    // The formula to get the reference loudness is "107 dB + Reference Loudness." RG 2.0 relative reference loudness is at -18 dB.
    // Thus, 107 + -18 = 89 dB
    // This is a temporary workaround until the loudgain author gets back to me on fixing this
    // Other programs do not expect the relative reference loudness format and will interpret it as -125 dB instead of 89 dB (107 + x = -18)
    // This is an extremely significant difference and will likely cause damage to audio equipment, including your ears
    foreach (QString currentFLAC, inputFLACs) {
        // Open a TagFile and PropertyMap of each input file
        // Linux only wants StdStrings, while Windows prefers StdWStrings (char encoding errors possible if Windows uses StdStrings)
#if defined(Q_OS_LINUX)
        TagLib::FLAC::File currentFLACTagFile(currentFLAC.toStdString().data());
#elif defined(Q_OS_WIN)
        TagLib::FLAC::File currentFLACTagFile(currentFLAC.toStdWString().data());
#endif
        TagLib::PropertyMap currentFLACTagMap = currentFLACTagFile.properties();

        // We manually insert the correct reference loudness, which needs to be correct for Opus's RG calculation (matches other scanners' format as well)
        currentFLACTagMap.replace("REPLAYGAIN_REFERENCE_LOUDNESS", TagLib::String("89.00 dB"));

        // Apply the map and save
        currentFLACTagFile.setProperties(currentFLACTagMap);
        currentFLACTagFile.save();
    }
}

// Conversion controller to send each file and its parameters to the correct encoder with multi-threading
QStringList MainWindow::convertToFormat(conversionParameters_t *conversionParameters) {
    QStringList outputFiles;
    // Disambiguate folder names later on, putting lower samplerates and lower BPS into higher folders
    int highestSampleRate = 0;
    int highestBPS = 0;
    // Holds the base sample rate for SoX to use
    int highestBaseSampleRate = 0;

    // Open the firstFLAC with TagLib, read some important data, then immediately destroy it
    // Under Windows, TagLib cannot open the same file multiple times so must be completely destroyed before the next access
    {
        // Used partially in guesswork, pulls tag/file data from first .flac file
        // Linux only wants StdStrings, while Windows prefers StdWStrings (char encoding errors possible if Windows uses StdStrings)
#if defined(Q_OS_LINUX)
        TagLib::FLAC::File firstFLACTagFile(conversionParameters->inputFLACs[0].toStdString().data());
#elif defined(Q_OS_WIN)
        TagLib::FLAC::File firstFLACTagFile(conversionParameters->inputFLACs[0].toStdWString().data());
#endif
        highestSampleRate = firstFLACTagFile.audioProperties()->sampleRate();
        highestBPS = firstFLACTagFile.audioProperties()->bitsPerSample();
    }

    // Find the highest BPS and samplerate in the input files
    foreach(QString currentFLAC, conversionParameters->inputFLACs) {
        // Linux only wants StdStrings, while Windows prefers StdWStrings (char encoding errors possible if Windows uses StdStrings)
#if defined(Q_OS_LINUX)
        TagLib::FLAC::File tempLoopTagFile(currentFLAC.toStdString().data());
#elif defined(Q_OS_WIN)
        TagLib::FLAC::File tempLoopTagFile(currentFLAC.toStdWString().data());
#endif

        if(tempLoopTagFile.audioProperties()->sampleRate() > highestSampleRate) {
            highestSampleRate = tempLoopTagFile.audioProperties()->sampleRate();
        }
        if(tempLoopTagFile.audioProperties()->bitsPerSample() > highestBPS) {
            highestBPS = tempLoopTagFile.audioProperties()->bitsPerSample();
        }
    }

    // Holds the highest base sample rate, aka 44100 for CD audio or 48000 for digital
    // This will result in 48000 for 192kHz, 44100 for 88.2kHz etc.
    if(highestSampleRate % 44100 == 0) {
        highestBaseSampleRate = 44100;
    }
    else if(highestSampleRate % 48000 == 0) {
        highestBaseSampleRate = 48000;
    }
    else {
        highestBaseSampleRate = highestSampleRate;
    }

    // FLAC
    if(conversionParameters->codecInput == "FLAC") {
        // Initialize a variable for input into ParseNamingSyntax, disambiguating output
        int futureBPS = highestBPS;

        // If other files are going to reduce bit depth, change the futureBPS accordingly
        if(conversionParameters->presetInput == "Force 16-bit" || conversionParameters->presetInput == "Force 16-bit and 44.1kHz/48kHz") {
            futureBPS = 16;
        }

        // Initialize a variable for input into ParseNamingSyntax, disambiguating output
        int futureSampleRate = highestSampleRate;

        // If other files are going to resample, change the futureSampleRate accordingly
        if(conversionParameters->presetInput == "Force 44.1kHz/48kHz" || conversionParameters->presetInput == "Force 16-bit and 44.1kHz/48kHz") {
            futureSampleRate = highestBaseSampleRate;
        }

        QThreadPool convertFLACPool;
        // QList that will hold the QFuture of every thread we launch, allowing us to launch many threads and check their results later
        QList<QFuture<QString>> futureList;

        // For every FLAC in the parameters
        foreach(QString currentFLAC, conversionParameters->inputFLACs) {
            // Send the FLAC and its parameters to convertToFLAC and store its QFuture into the futureList
            futureList.append(QtConcurrent::run(&convertFLACPool, convertToFLAC, currentFLAC, conversionParameters, futureBPS, futureSampleRate));
        }
        convertFLACPool.waitForDone();

        // For every QFuture
        foreach(QFuture<QString> currentFuture, futureList) {
            // Add its returned value to outputFiles
            outputFiles += currentFuture.result();
        }
    }

    // Opus
    else if (conversionParameters->codecInput == "Opus") {
        QThreadPool convertOpusPool;
        // QList that will hold the QFuture of every thread we launch, allowing us to launch many threads and check their results later
        QList<QFuture<QString>> futureList;

        // For every FLAC in the parameters
        foreach(QString currentFLAC, conversionParameters->inputFLACs) {
            // Send the FLAC and its parameters to convertToOpus and store its QFuture into the futureList
            futureList.append(QtConcurrent::run(&convertOpusPool, convertToOpus, currentFLAC, conversionParameters));
        }
        convertOpusPool.waitForDone();

        // For every QFuture
        foreach(QFuture<QString> currentFuture, futureList) {
            // Add its returned value to outputFiles
            outputFiles += currentFuture.result();
        }
    }

    // MP3
    else if (conversionParameters->codecInput == "MP3") {
        QThreadPool convertMP3Pool;
        // QList that will hold the QFuture of every thread we launch, allowing us to launch many threads and check their results later
        QList<QFuture<QString>> futureList;

        // For every FLAC in the parameters
        foreach(QString currentFLAC, conversionParameters->inputFLACs) {
            // Send the FLAC and its parameters to convertToMP3 and store its QFuture into the futureList
            futureList.append(QtConcurrent::run(&convertMP3Pool, convertToMP3, currentFLAC, conversionParameters));
        }
        convertMP3Pool.waitForDone();

        // For every QFuture
        foreach(QFuture<QString> currentFuture, futureList) {
            // Add its returned value to outputFiles
            outputFiles += currentFuture.result();
        }
    }

    outputFiles.sort();
    return outputFiles;
}

// Helper function that runs in a background thread and is the backbone for the full conversion process, including pre and post tasks
void MainWindow::convertBackgroundWorker(uiSelections_t uiSelections) {
    QSettings MIKSettings;

    // Get a list of all FLACs in the tempDir
    QStringList inputFLACs = findFiles(uiSelections.tempDir, {"*.flac"});

    // Return if there are no FLACs
    if(inputFLACs.count() == 0) {
        QMessageBox::critical(this, "Alert", "No valid files to convert.", QMessageBox::Ok);
        return;
    }

    QStringList outputFiles;
    QStringList copiedFiles;

    // Struct that contains many parameters for passing into a later thread. QThreads don't allow more than 5 parameters to be passed in, so they are all packaged into a struct
    conversionParameters_t conversionParameters{inputFLACs, uiSelections.outputDir, uiSelections.presetInput, uiSelections.syntaxInput, uiSelections.codecInput};

    // If the codec is FLAC, calculate ReplayGain after we convert.
    // Resampling and reducing bit depth will affect audio data and thus ReplayGain, so it needs to be calculated afterwards
    if(uiSelections.codecInput == "FLAC") {
        ui->ConvertButton->setText("Converting..."); // Technically not thread-safe but no competing events
        // Send the necessary info to the conversion function and get back a list of converted files
        outputFiles += convertToFormat(&conversionParameters);
        if(uiSelections.RGEnabled) {
            ui->ConvertButton->setText("Calculating ReplayGain..."); // Technically not thread-safe but no competing events
            calculateReplayGain(outputFiles);
        }
    }
    // Else if a file is lossy, calculate ReplayGain before we convert.
    // Opus and MP3 both use their parent FLAC's ReplayGain data to calculate their own ReplayGain so it needs to be calculated for the parent before conversion
    else {
        if(uiSelections.RGEnabled) {
            ui->ConvertButton->setText("Calculating ReplayGain..."); // Technically not thread-safe but no competing events
            calculateReplayGain(inputFLACs);
        }
        ui->ConvertButton->setText("Converting..."); // Technically not thread-safe but no competing events
        outputFiles += convertToFormat(&conversionParameters);
    }

    // Folder that files were copied to
    QDir outputDir(QFileInfo(outputFiles[0]).dir());

    // Used partially in guesswork, pulls data from first .flac file
    // Linux only wants StdStrings, while Windows prefers StdWStrings (char encoding errors possible if Windows uses StdStrings)
#if defined(Q_OS_LINUX)
    TagLib::FLAC::File firstFLACTagFile(inputFLACs[0].toStdString().data());
#elif defined(Q_OS_WIN)
    TagLib::FLAC::File firstFLACTagFile(inputFLACs[0].toStdWString().data());
#endif
    QString artist = "";
    QString album = "";

    // Parse the tags for artist (preferred, albumartist, then album artist, then artist)
    if(firstFLACTagFile.properties().contains("albumartist")) {
        artist = cleanString(TStringToQString(firstFLACTagFile.properties()["albumartist"].front()));
    }
    else if(firstFLACTagFile.properties().contains("album artist")) {
        artist = cleanString(TStringToQString(firstFLACTagFile.properties()["album artist"].front()));
    }
    else if(firstFLACTagFile.properties().contains("artist")) {
        artist = cleanString(TStringToQString(firstFLACTagFile.properties()["artist"].front()));
    }

    // Parse the tags for album
    if(firstFLACTagFile.properties().contains("album")) {
        album = cleanString(TStringToQString(firstFLACTagFile.properties()["album"].front()));
    }

    // If copying files is enabled and the list of filetypes to copy isn't empty
    if(uiSelections.copyContentsEnabled && uiSelections.copyContents != "") {
        ui->ConvertButton->setText("Copying other files..."); // Technically not thread-safe but no competing events
        QStringList patternList = uiSelections.copyContents.split(';');

        // Copy, then store copied files into a list for later use
        copiedFiles += folderCopy(uiSelections.tempDir, outputDir, patternList, outputFiles);
    }

    // Rename .logs and .cues if enabled
    if(uiSelections.renameLogCueEnabled) {
        renameLogCue(copiedFiles, outputDir, artist, album);
    }

    // Compress images if enabled
    if(uiSelections.compressImagesEnabled) {
        ui->ConvertButton->setText("Compressing images..."); // Technically not thread-safe but no competing events
        compressImages(copiedFiles);
    }

    // Delete temp folder if enabled (and the temp folder isn't the output folder)
    if(uiSelections.deleteTempEnabled && uiSelections.tempDir != outputDir) {
        removeDir(uiSelections.tempDir.path());
        //uiSelections.tempDir.removeRecursively(); // FIXME
        //QDir().rmdir(uiSelections.tempDir.path()); // FIXME
    }

    // Open resultant folder if enabled
    if(uiSelections.openFolderEnabled) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(outputDir.path()));
    }

    // Set the convert button's text back to normal to denote process completion
    ui->ConvertButton->setText("Convert"); // Technically not thread-safe but no competing events
    ui->ConvertButton->setEnabled(true);   // Technically not thread-safe but no competing events

    // If delete temp folder is enabled, also reset some UI elements (assuming user is finished with this album)
    if(uiSelections.deleteTempEnabled) {
        QDir parentDir = getNearestParent(uiSelections.tempDir);
        // If temp folder has a valid parent and the QLineEdit hasn't been changed since the process started
        if(parentDir.exists() && uiSelections.tempDir.path() == QDir(ui->TempLineEdit->text()).path()) {
            // Set it to its parent
            ui->TempLineEdit->setText(QDir::toNativeSeparators(parentDir.path())); // Technically not thread-safe but no competing events
        }

        // If input folder has a valid parent and the QLineEdit hasn't been changed since the process started
        if(uiSelections.inputDir.path() == QDir(ui->InputLineEdit->text()).path()) {
            // Set it to its parent
            ui->InputLineEdit->setText(QDir::toNativeSeparators(MIKSettings.value("sDefaultInput", "").toString())); // Technically not thread-safe but no competing events
        }

        // Clear the artist and album QLineEdits
        ui->ArtistLineEdit->setText(""); // Technically not thread-safe but no competing events
        ui->AlbumLineEdit->setText(""); // Technically not thread-safe but no competing events
    }
}

// Initial conversion function to handle the initialization and pass control to a non-GUI thread
void MainWindow::convertInitialize() {
    // Read in user settings
    QSettings MIKSettings;
    QDir defaultTempDir(MIKSettings.value("sDefaultTemp", "").toString());
    QDir inputDir(ui->InputLineEdit->text());
    QDir tempDir(ui->TempLineEdit->text());
    QDir outputDir(ui->OutputLineEdit->text());

    // Return if temp path is invalid
    if(tempDir.path() == "." || !tempDir.exists()) {
        QMessageBox::critical(this, "Alert", "Temp folder does not exist.", QMessageBox::Ok);
        return;
    }
    // Return if output paths is invalid
    if(outputDir.path() == "." || !outputDir.exists()) {
        QMessageBox::critical(this, "Alert", "Output folder does not exist.", QMessageBox::Ok);
        return;
    }
    // Return if syntax is blank
    if(ui->SyntaxComboBox->currentText() == "") {
        QMessageBox::critical(this, "Alert", "Naming syntax is blank.", QMessageBox::Ok);
        return;
    }

    // Return if conversion format is invalid
    if(ui->ConvertToComboBox->currentText() == "" || ui->ConvertToPresetComboBox->currentText() == "") {
        QMessageBox::critical(this, "Alert", "Conversion format is invalid.", QMessageBox::Ok);
        return;
    }
    // Return if copy contents enabled but no copy file types specified
    if(ui->CopyContentsCheckBox->isChecked() == true && ui->CopyContentsLineEdit->text() == "") {
        QMessageBox::critical(this, "Alert", "Copyfiles enabled but no filetypes specified.", QMessageBox::Ok);
        return;
    }

    // If the tempPath is the same as the user's stored defaultTempPath from settings
    if(tempDir.path() == defaultTempDir.path()) {
        // Display a warning
        QMessageBox::StandardButton warning = QMessageBox::warning(this, "Warning", "Are you sure you want to convert the default temp folder?", QMessageBox::Yes | QMessageBox::No);
        if(warning == QMessageBox::No){
            return;
        }
    }

    // Disable convert button to denote process is executing
    ui->ConvertButton->setEnabled(false);

    // Pack the UI state into a struct so functions can use the data as it was when the user launched the conversion process
    // Allows the user to change the UI after starting without it affecting the process
    uiSelections_t uiSelections{inputDir,
                                tempDir,
                                outputDir,
                                ui->SyntaxComboBox->currentText(),
                                ui->ReplayGainCheckBox->isChecked(),
                                ui->CopyContentsCheckBox->isChecked(),
                                ui->CopyContentsLineEdit->text(),
                                ui->RenameLogCueCheckBox->isChecked(),
                                ui->CompressImagesCheckBox->isChecked(),
                                ui->DeleteTempFolderCheckBox->isChecked(),
                                ui->ConvertOpenFolderCheckBox->isChecked(),
                                ui->ConvertToComboBox->currentText(),
                                ui->ConvertToPresetComboBox->currentText()};

    // Pass the struct into a non-GUI thread
    QtConcurrent::run(this, &MainWindow::convertBackgroundWorker, uiSelections);
}

// Runs when the "copy contents" QCheckBox is changed. Dynamically enables/disables other settings that are only relevant depending on this QCheckBox's status
void MainWindow::on_CopyContentsCheckBox_stateChanged(int state)
{
    QSettings MIKSettings;
    // If true
    if(state == 2) {
        ui->CopyContentsLineEdit->setEnabled(true);
        ui->RenameLogCueCheckBox->setEnabled(true);
        ui->RenameLogCueCheckBox->setChecked(MIKSettings.value("bDefaultRenameLogCue", true).toBool());
        // If user wants to compress images and has at least one image format selected in the settings page with a valid compressor
        if(MIKSettings.value("bDefaultCompressImages", false).toBool() &&
                ((MIKSettings.value("bDefaultCompressBMP", false).toBool() && checkInstalledProgram("sDefaultOxiPNGLocation", "oxipng") != "") ||
                (MIKSettings.value("bDefaultCompressGIF", false).toBool() && checkInstalledProgram("sDefaultGifsicleLocation", "gifsicle") != "") ||
                (MIKSettings.value("bDefaultCompressJPG", false).toBool() && checkInstalledProgram("sDefaultJPEGOptimLocation", "jpegoptim") != "") ||
                (MIKSettings.value("bDefaultCompressPNG", false).toBool() && checkInstalledProgram("sDefaultOxiPNGLocation", "oxipng") != ""))) {
            ui->CompressImagesCheckBox->setEnabled(true);
            ui->CompressImagesCheckBox->setChecked(MIKSettings.value("bDefaultCompressImages", false).toBool());
            ui->CompressImagesCheckBox->setText("Compress and strip BMPs, GIFs, JPGs, and PNGs");
        }
        else {
            ui->CompressImagesCheckBox->setEnabled(false);
            ui->CompressImagesCheckBox->setChecked(false);
            ui->CompressImagesCheckBox->setText("Compress and strip BMPs, GIFs, JPGs, and PNGs (check settings page for requirements)");
        }
    }
    // Else false
    else {
        ui->CopyContentsLineEdit->setEnabled(false);
        ui->RenameLogCueCheckBox->setEnabled(false);
        ui->RenameLogCueCheckBox->setChecked(false);
        ui->CompressImagesCheckBox->setEnabled(false);
        ui->CompressImagesCheckBox->setChecked(false);
    }
}
