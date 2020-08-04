#include "helper.h"

#if defined(Q_OS_LINUX)
// Get user's shell-manipulated PATH environment variable (including .bashrc, .zshrc, .profile, etc)
void getShellPATH() {
    // Get the current user's name
    QString name = qgetenv("USER");
    if(name.isEmpty()) {
        name = qgetenv("USERNAME");
    }

    // Process to pull the user's default shell out of /etc/passwd using awk
    QProcess defaultShellExtractionProcess;
    defaultShellExtractionProcess.setProgram("sh");
    defaultShellExtractionProcess.setArguments({"-c", "awk -F: -v user=\"" + name + "\" \'$1 == user {print $NF}\' /etc/passwd"});
    defaultShellExtractionProcess.start();
    defaultShellExtractionProcess.waitForFinished(-1);
    // Read the default shell out of the output. In the format of /bin/sh or /bin/bash etc
    QString defaultShell = defaultShellExtractionProcess.readAllStandardOutput().trimmed();

    QProcess pathExtractionProcess;
    pathExtractionProcess.setProgram(defaultShell);
    // Tested and working shells: bash, zsh, fish, sh, tcsh, csh, ksh, dash
    // If you're using a hipster shell that's not on here, you better hope it uses standard shell syntax
    // -i: interactive mode (necessary for some shells to load the environment variable that we need)
    // -c: input a custom command
    // "printenv PATH": prints the shell's interpretation of the PATH environment variable to stdout
    pathExtractionProcess.setArguments({"-i", "-c", "printenv PATH"});

    // Start and wait
    pathExtractionProcess.start();
    pathExtractionProcess.waitForFinished(-1);

    // Set Qt's PATH variable to the shell-manipulated PATH variable, overwriting it
    qputenv("PATH", pathExtractionProcess.readAllStandardOutput().trimmed());
}
#endif

#if defined(Q_OS_WIN)
// Get the WSL location of a Windows file (C:\Users -> /mnt/c/Users)
QString getWSLPath(QString winLocation) {
    // Get the drive letter (C:\ -> c)
    QString drive = winLocation.at(0).toLower();
    // Remove the first three characters of the file location (C:\Users -> Users)
    winLocation = winLocation.remove(0,3);
    // Convert to the proper WSL path notation (/mnt/ + c + / + Users), and replace backslashes with forward slashes
    QString WSLLocation = QString("/mnt/" + drive + "/" + winLocation.replace("\\", "/"));

    return WSLLocation;
}
#endif

#if defined(Q_OS_WIN)
// Checks if Loudgain is available via WSL
bool isWSLLoudgainAvailable() {
    QProcess WSLProcess;
    WSLProcess.setProgram("wsl");
    QStringList arguments;
    // Use the built-in "which" command to locate loudgain. It's not actually important that we get a specific path, just that we get anything back at all
    arguments << "which" << "loudgain";
    WSLProcess.setArguments(arguments);

    // Start and wait
    // Note that initializing WSL is slow, so a native binary will eventually help performance
    WSLProcess.start();
    WSLProcess.waitForFinished(-1);

    // If we got something back
    if(WSLProcess.readAllStandardOutput().trimmed() != "") {
        // Loudgain available
        return true;
    }
    else {
        // Loudgain unavailable
        return false;
    }
}
#endif

// Remove target directory
bool removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);

    if(dir.exists(dirName)) {
        // For every file nested in the directory
        foreach (QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            
            // If it's a directory
            if(info.isDir()) {
                // Recursively remove it
                result = removeDir(info.absoluteFilePath());
            }
            // If it's a file
            else {
                // Remove it
                result = QFile::remove(info.absoluteFilePath());
            }

            // If one of the operations failed, return false
            if(!result) {
                return false;
            }
        }

        // Remove the directory
        result = dir.rmdir(dirName);
    }

    return result;
}

// Get the parent folder of a path
QDir getNearestParent(QDir pathDir)
{
    // Preclude root and empty paths
    if(!pathDir.isRoot() && pathDir.path() != ".") {
        // Back up one folder
        pathDir.setPath(QDir::cleanPath(pathDir.filePath(QStringLiteral(".."))));
    }

    // If the path is invalid, return empty
    if(pathDir.path() == "." || !pathDir.exists()) {
        return QDir("");
    }
    // Else return our found pathDir
    else {
        return pathDir;
    }
}

// Used for cleaning a string of invalid file name characters
QString cleanString (QString input, QString ignoredChars) {
    // If string is blank or null, return it
    if(input == "") {
        return input;
    }

    // Clear spaces from beginning and end
    input.replace(QRegExp("^\\s+|\\s+$"), "");

    // First string holds characters to replace, second holds the replacement. Replacements are done serially so make sure they line up
    QString replaceableIllegalChars = "\\/:*?\"“”<>|";
    QString fullWidthReplacements = "＼／：＊？＂＂＂＜＞｜";

    // For every character in first string
    for (int i = 0; i < replaceableIllegalChars.length(); i++) {
        // If the character is not ignored via parameter
        if (!ignoredChars.contains(replaceableIllegalChars[i])) {
            // Replace the character with its replacement in the input string
            input.replace(replaceableIllegalChars[i], fullWidthReplacements[i]);
        }
    }

    // For each of 0-31 (control characters) and 127 (delete character) unicode symbols
    foreach (QChar currentChar, input) {
        if(currentChar.unicode() <= 31 || currentChar.unicode() == 127) {
            // Remove the illegal character
            input.remove(currentChar);
        }
    }

    return input;
}

// Returns a list of files that match a passed-in patternList
QStringList findFiles(QDir rootDir, QStringList patternList)
{
    // List of found files for eventual return
    QStringList foundFiles;

    // Remove null and empty strings
    patternList.removeAll(QString(""));

    // Trim spaces from the beginning and end of each string
    patternList.replaceInStrings(QRegExp("^\\s+|\\s+$"), "");

    // Set filters to only list directories in the current path
    rootDir.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);
    // For every dir in rootDir
    foreach (QFileInfo fileInfo, rootDir.entryInfoList()) {
        // If the file is a directory
        if(fileInfo.isDir() && fileInfo.isReadable()) {
            // Recursively add that directory's files to foundFiles
            foundFiles += findFiles(QDir(fileInfo.filePath()), patternList);
        }
    }

    // Set filters to only list files in the current path
    rootDir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);
    // Set name filters to filter out unwanted files (e.g. *.flac only)
    rootDir.setNameFilters(patternList);
    // For every file in rootDir
    foreach (QFileInfo fileInfo, rootDir.entryInfoList()) {
        // Add the file's full path to foundFiles
        foundFiles += fileInfo.filePath();
    }

    foundFiles.sort();
    return foundFiles;
}

// Checks if a program exists and returns its location. Prefers program location from input over programName
// Note that this doesn't check if the user-defined file is the right program or even a program
QString checkInstalledProgram(QString location, QString programName, bool useSettingsKey) {
    if(useSettingsKey) {
        // If the program from the settings is a file, return it
        QSettings MIKSettings;
        if(QFileInfo(MIKSettings.value(location, "").toString()).isFile()) {
            return MIKSettings.value(location, "").toString();
        }
    }
    else {
        if(QFileInfo(location).isFile()) {
            return location;
        }
    }

    // On Linux, use the "which" command to print out where the program lives on the OS, and if it finds it, return the name
    // This uses the user's PATH variable
#if defined(Q_OS_LINUX)
    if(programName != NULL && system(qPrintable("which " + programName + ">> /dev/null")) == 0) {
        return programName;
    }
#endif

    // If all checks have failed, return a blank string
    return "";
}

// Worker to process feeding spek inputs
// Spek cannot accept a full directory or list of files, so it must be fed the files one at a time. This worker handles this so the GUI thread can remain unfrozen
void openSpekWorker(QStringList inputFLACs) {
    QProcess SpekProcess;
    QString programLocation = checkInstalledProgram("sDefaultSpectrogramAnalysisLocation", "spek");
    if(programLocation == "") {
        return;
    }
    SpekProcess.setProgram(programLocation);


    // Opens each FLAC file in Spek one at a time and in order
    foreach (QString currentFLAC, inputFLACs) {
        // Set the argument to be the currentFLAC file. Doesn't need to be escaped.
        SpekProcess.setArguments({QDir::toNativeSeparators(currentFLAC)});

        // Start and wait
        SpekProcess.start();
        SpekProcess.waitForFinished(-1);
    }
}

// Parses custom syntax (e.g. %tag% and &codec&) and returns a QString based on the metadata/tags of a file
QString parseNamingSyntax(QString syntax, QString codec, QString preset, QString inputFLAC, int futureBPS, int futureSampleRate) {
    QString parsedString = "";
    QString formattedString = "";
    QString currentElement = "";
    int nextMarkerIndex = 0;

    // Replace NT folder delimiters with UNIX style
    syntax.replace("\\", "/");

    // Create a TagLib::File of the inputFLAC to reference tag data from
#if defined(Q_OS_LINUX)
    TagLib::FLAC::File inputFLACTagFile(inputFLAC.toStdString().data());
#elif defined(Q_OS_WIN)
    TagLib::FLAC::File inputFLACTagFile(inputFLAC.toStdWString().data());
#endif

    // Prettier metadata to use for folder/filenames
    // MP3
    if(codec == "MP3") {
        if (preset == "245kbps VBR (V0)")      {preset = "V0";}
        else if (preset == "225kbps VBR (V1)") {preset = "V1";}
        else if (preset == "190kbps VBR (V2)") {preset = "V2";}
        else if (preset == "175kbps VBR (V3)") {preset = "V3";}
        else if (preset == "165kbps VBR (V4)") {preset = "V4";}
        else if (preset == "130kbps VBR (V5)") {preset = "V5";}
        else if (preset == "115kbps VBR (V6)") {preset = "V6";}
        else if (preset == "100kbps VBR (V7)") {preset = "V7";}
        else if (preset == "85kbps VBR (V8)")  {preset = "V8";}
        else if (preset == "65kbps VBR (V9)")  {preset = "V9";}
        else if (preset == "320kbps CBR")      {preset = "320";}
        else if (preset == "256kbps CBR")      {preset = "256";}
        else if (preset == "192kbps CBR")      {preset = "192";}
        else if (preset == "128kbps CBR")      {preset = "128";}
        else if (preset == "64kbps CBR")       {preset = "64";}
    }
    else if (codec == "Opus") {
        if (preset == "192kbps VBR")      {preset = "192";}
        else if (preset == "160kbps VBR") {preset = "160";}
        else if (preset == "128kbps VBR") {preset = "128";}
        else if (preset == "96kbps VBR")  {preset = "96";}
        else if (preset == "64kbps VBR")  {preset = "64";}
        else if (preset == "32kbps VBR")  {preset = "32";}
    }

    // Move through the input string, matching syntax, pushing its equivalent into a formatted string, then deleting the matched portion of the original and loop
    while(syntax.length() > 0) {
        // If we are at a % and there is a matching %
        if (syntax[0] == '%' && syntax.indexOf('%', 1) != -1) {
            // Find the matching percent marker, e.g. %artist"%"
            nextMarkerIndex = syntax.indexOf('%', 1);

            // Extract the tag contained within the two % symbols in lowercase
            currentElement = syntax.mid(1, nextMarkerIndex-1).toLower();

            // If the tag exists in the file
            if(inputFLACTagFile.properties().contains(currentElement.toStdWString())) {
                // Retrieve parsed tag from the file, cleaning it on the way
                parsedString = cleanString(TStringToQString(inputFLACTagFile.properties()[currentElement.toStdWString()].front()).trimmed());

                // Add parsed element to the eventual output string
                formattedString += parsedString;
            }

            // Remove the matched portion from the input string
            syntax.remove(0, nextMarkerIndex+1);
        }
        // If we are at a & and there is a matching &
        else if(syntax[0] == '&' && syntax.indexOf('&', 1) != -1) {
            // Find the matching ampersand, e.g. &codec"&"
            nextMarkerIndex = syntax.indexOf('&', 1);

            // Extract the tag contained within the two & symbols in lowercase
            currentElement = syntax.mid(1, nextMarkerIndex-1).toLower();

            // Bit-depth e.g. 16, 24
            if(currentElement == "bps") {
                // Manual BPS insertion
                if(futureBPS != -1) {
                    formattedString += QString::number(futureBPS);
                }
                // inputFLAC BPS insertion
                else {
                    formattedString += QString::number(inputFLACTagFile.audioProperties()->bitsPerSample());
                }
            }

            // Smartbit
            else if(currentElement == "smartbit") {
                // Lossless smartbit, uses bit-depth + "-" + a short sample rate, e.g. "16-44" for 16-bit 44100Hz FLAC or "24-96" for 24-bit 96000Hz FLAC
                if(codec == "FLAC") {
                    if(futureBPS != -1 && futureSampleRate != -1) {
                        formattedString += QString::number(futureBPS) + "-" + QString::number(futureSampleRate).mid(0, 2);
                    }
                    else if(futureBPS != -1) {
                        formattedString += QString::number(futureBPS) + "-" + QString::number(inputFLACTagFile.audioProperties()->sampleRate()).mid(0, 2);
                    }
                    else if(futureSampleRate != -1) {
                        formattedString += QString::number(inputFLACTagFile.audioProperties()->bitsPerSample()) + "-" + QString::number(futureSampleRate);
                    }
                    else {
                        formattedString += QString::number(inputFLACTagFile.audioProperties()->bitsPerSample()) + "-" + QString::number(inputFLACTagFile.audioProperties()->sampleRate()).mid(0, 2);
                    }
                }
                // Lossy smartbit, uses bitrate/preset, e.g. "320" for MP3 320 or "V0" for MP3 V0
                else {
                    formattedString += preset;
                }
            }

            // Sample rate e.g. 44100, 48000, 96000
            else if(currentElement == "samplerate") {
                // Manual sample rate insertion
                if (futureSampleRate != -1) {
                    formattedString += QString::number(futureSampleRate);
                }
                // inputFLAC sample rate insertion
                else {
                    formattedString += QString::number(inputFLACTagFile.audioProperties()->sampleRate());
                }
            }

            // Shortened sample rate, e.g. 44, 48, 96
            else if(currentElement == "short-samplerate") {
                // Manual sample rate insertion
                if (futureSampleRate != -1) {
                    formattedString += QString::number(futureSampleRate).mid(0, 2);
                }
                // inputFLAC sample rate insertion
                else {
                    formattedString += QString::number(inputFLACTagFile.audioProperties()->sampleRate()).mid(0, 2);
                }
            }

            // Codec
            else if(currentElement == "codec") {
                formattedString += codec;
            }

            // Bitrate
            else if(currentElement == "bitrate") {
                formattedString += preset;
            }

            // Padded track number logic, e.g. 01, 02, 03
            else if(currentElement == "paddedtracknumber") {
                // Retrieve parsed tag from the file, cleaning it on the way
                parsedString = cleanString(QString::number(inputFLACTagFile.xiphComment()->track()));

                // Remove any leading zeroes
                parsedString.remove(QRegExp("^[0]*"));

                // Pad the track number if it's 1-9
                if(parsedString.toInt() >= 1 && parsedString.toInt() <= 9) {
                    parsedString.insert(0, '0');
                }
                // If it's 0, set to "00"
                else if(parsedString == "" || parsedString.toInt() == 0) {
                    parsedString = "00";
                }

                formattedString += parsedString;
            }

            // Remove the matched portion from the string
            syntax.remove(0, nextMarkerIndex+1);
        }
        else {
            // If dealing with a folder
            if (syntax[0] == "/") {
                // Remove spaces and periods from the end of the folder name (so far); not allowed on Windows
                formattedString.replace(QRegExp("[.\\s]+$"), "");
            }

            // Pass non-matching characters into the formattedString
            formattedString += syntax[0];
            syntax.remove(0, 1);
        }
    }

    // Clean the formatted string of illegal file characters, and ignore folder delimiters in this process
    formattedString = cleanString(formattedString, "/");

    // Remove spaces and periods from the end of the folder/filename; not allowed on Windows
    formattedString.replace(QRegExp("[.\\s]+$"), "");

    return formattedString;
}

// Losslessly compresses a GIF using Gifsicle. Multi-threaded but can only accept one file at a time, so initialization costs for every file
void compressGIF(QString inputGIF) {
    // The name of the eventual compressed GIF output; used for efficiency
    QString compressedGIF = QFileInfo(inputGIF).dir().path() + "/" + QFileInfo(inputGIF).baseName() + "compressed" + ".gif";

    QProcess GifsicleProcess;
    QString programLocation = checkInstalledProgram("sDefaultGifsicleLocation", "gifsicle");
    if(programLocation == "") {
        return;
    }
    GifsicleProcess.setProgram(programLocation);

    // Gifsicle arguments
    // -O3: optimization level 3 (highest/slowest)
    // -j: number of threads to use
    // --no-comments: removes comment metadata
    // --no-names: removes name metadata
    // -o: output location
    QStringList arguments;
    arguments << "-O3" << "-j" + QString::number(QThread::idealThreadCount()) << "--no-comments" << "--no-names"
              << QDir::toNativeSeparators(inputGIF) << "-o" << QDir::toNativeSeparators(compressedGIF);
    GifsicleProcess.setArguments(arguments);

    // Start and wait
    GifsicleProcess.start();
    GifsicleProcess.waitForFinished(-1);

    // Gifsicle overwrites the original file even if the file it "compressed" ends up being larger, so we handle that here
    // If the compressed GIF is smaller than the original file
    if(QFileInfo(compressedGIF).size() < QFileInfo(inputGIF).size()) {
        // Remove original file
        QFile(inputGIF).remove();
        // Rename compressed GIF to the original file's name
        QFile(compressedGIF).rename(inputGIF);
    }
    // Else Gifsicle made a larger GIF
    else {
        // Remove the new GIF
        QFile(compressedGIF).remove();
    }
}

// Losslessly compresses and strips a JPG using JPEGOptim. Not multi-threaded so needs to be called multiple times via threads.
void compressJPG(QString inputJPG) {
    QProcess JPEGOptimProcess;
    QString programLocation = checkInstalledProgram("sDefaultJPEGOptimLocation", "jpegoptim");
    if(programLocation == "") {
        return;
    }
    JPEGOptimProcess.setProgram(programLocation);

    // JPEGOptim arguments
    // -q: quiet
    // --strip-com: strip comment data
    // --strip-exif: strip exif data
    // --strip-iptc: strip IPTC/Photoshop data
    // --strip-xmp: strip XMP data
    // --all-progressive: force progressive mode on all JPGs
    QStringList arguments;
    arguments << "-q" << "--strip-com" << "--strip-exif" << "--strip-iptc" << "--strip-xmp" << "--all-progressive" << QDir::toNativeSeparators(inputJPG);
    JPEGOptimProcess.setArguments(arguments);

    // Start and wait
    JPEGOptimProcess.start();
    JPEGOptimProcess.waitForFinished(-1);
}

// Losslessly compresses a list of PNGs using OxiPNG. Multi-threaded and handles all files at once so only one initialization cost.
void compressPNGs(QStringList inputPNGs) {
    QProcess OxiPNGProcess;
    QString programLocation = checkInstalledProgram("sDefaultOxiPNGLocation", "oxipng");
    if(programLocation == "") {
        return;
    }
    OxiPNGProcess.setProgram(programLocation);

    // OxiPNG arguments
    // -o 6: optimization level (highest/slowest)
    // --strip safe: strip all metadata that doesn't impact viewing of image
    // -t: number of threads
    QStringList arguments;
    arguments << "-o" << "6" << "--strip" << "safe" << "-t" << QString::number(QThread::idealThreadCount());
    foreach (QString currentPNG, inputPNGs) {
        arguments << QDir::toNativeSeparators(currentPNG);
    }
    OxiPNGProcess.setArguments(arguments);

    // Start and wait
    OxiPNGProcess.start();
    OxiPNGProcess.waitForFinished(-1);
}

// Returns the real format of an image by reading the bytes from its magic header
// This is to detect files that are named improperly before sending them to their respective compressors
QString getRealImageFormat(QString inputImage) {
    QString realFormat = "";

    // Open the image file in read-only mode
    QFile inputImageFile(inputImage);
    inputImageFile.open(QIODevice::ReadOnly);

    // Input validation
    if(!inputImageFile.exists()) {
        return "";
    }

    // Create a 4-byte header that contains the first 4 bytes of the input file
    QByteArray imageHeader(inputImageFile.read(4));

    inputImageFile.close();

    // If file has a magic BMP header ("BM")
    if (imageHeader.left(2) == QByteArray::fromHex("424d")) {
        realFormat = "BMP";
    }
    // Else if file has a magic GIF header ("GIF")
    else if (imageHeader.left(3) == QByteArray::fromHex("474946")) {
        realFormat = "GIF";
    }
    // Else if file has a magic JPG/JPEG header ("ÿØ")
    else if (imageHeader.left(2) == QByteArray::fromHex("ffd8")) {
        realFormat = "JPG";
    }
    // Else if file has a magic PNG header (".PNG")
    else if (imageHeader == QByteArray::fromHex("89504e47")) {
        realFormat = "PNG";
    }
    else {
        return "";
    }

    return realFormat;
}

// Handler function to send images of many formats to their proper compressors
void compressImages(QStringList inputFiles) {
    QSettings MIKSettings;
    QStringList pendingImages;

    // QStringList of images that reside in the passed-in path
    pendingImages += inputFiles.filter(QRegExp("^.*\\.bmp$", Qt::CaseInsensitive));
    pendingImages += inputFiles.filter(QRegExp("^.*\\.gif$", Qt::CaseInsensitive));
    pendingImages += inputFiles.filter(QRegExp("^.*\\.jpg$", Qt::CaseInsensitive));
    pendingImages += inputFiles.filter(QRegExp("^.*\\.jpeg$", Qt::CaseInsensitive));
    pendingImages += inputFiles.filter(QRegExp("^.*\\.png$", Qt::CaseInsensitive));

    // QStringLists of each filetype
    QStringList pendingBMP;
    QStringList pendingGIF;
    QStringList pendingJPG;
    QStringList pendingPNG;

    // Stores a QString of the magic-header-defined real filetype of a file, not its alleged extension
    QString realFormat;

    // Used for storing expensive calls so we don't need to call them multiple times
    QString tempRename;
    foreach (QString currentImage, pendingImages) {
        // Get the magic-header-defined real filetype of a file, not its alleged extension
        realFormat = getRealImageFormat(currentImage);

        // BMP
        if(realFormat == "BMP") {
            // If file ends with .bmp, add it to the BMP list
            if(QFileInfo(currentImage).suffix().toLower() == "bmp") {
                pendingBMP += currentImage;
            }
            // Else rename it to .bmp and add it to the BMP list
            else {
                tempRename = QFileInfo(currentImage).dir().path() + "/" + QFileInfo(currentImage).baseName() + ".bmp";
                QFile(currentImage).rename(tempRename);
                pendingBMP += tempRename;
            }
        }

        // GIF
        else if(realFormat == "GIF") {
            // If file ends with .gif, add it to the GIF list
            if(QFileInfo(currentImage).suffix().toLower() == "gif") {
                pendingGIF += currentImage;
             }
            // Else rename it to .gif and add it to the GIF list
            else {
                tempRename = QFileInfo(currentImage).dir().path() + "/" + QFileInfo(currentImage).baseName() + ".gif";
                QFile(currentImage).rename(tempRename);
                pendingGIF += tempRename;
            }
        }

        // JPG/JPEG
        else if(realFormat == "JPG") {
            // If file ends with .jpg, add it to the JPG list
            if(QFileInfo(currentImage).suffix().toLower() == "jpg") {
                pendingJPG += currentImage;
            }
            // Else rename it to .jpg (.jpegs included) and add it to the JPG list
            else {
                tempRename = QFileInfo(currentImage).dir().path() + "/" + QFileInfo(currentImage).baseName() + ".jpg";
                QFile(currentImage).rename(tempRename);
                pendingJPG += tempRename;
            }
        }

        // PNG
        else if(realFormat == "PNG") {
            // If file ends with .png, add it to the PNG list
            if(QFileInfo(currentImage).suffix().toLower() == "png") {
                pendingPNG += currentImage;
            }
            // Else rename it to .png and add it to the PNG list
            else {
                tempRename = QFileInfo(currentImage).dir().path() + "/" + QFileInfo(currentImage).baseName() + ".png";
                QFile(currentImage).rename(tempRename);
                pendingPNG += tempRename;
            }
        }
    }

    // BMP compression (compresses to PNG)
    // If the BMP list isn't empty, the user wants to compress BMPs, and a BMP compression program exists
    if(!pendingBMP.isEmpty() && MIKSettings.value("bDefaultCompressBMP", false).toBool() && checkInstalledProgram("sDefaultOxiPNGLocation", "oxipng") != "") {
        // For each BMP in the pendingBMP list
        foreach (QString currentBMP, pendingBMP) {
            // Store its eventual PNG name in a variable
            tempRename = QFileInfo(currentBMP).dir().path() + "/" + QFileInfo(currentBMP).baseName() + ".png";

            // Open the BMP and save it as a PNG
            QImage tempBitmap(QFile(currentBMP).fileName());
            tempBitmap.save(tempRename);

            // Add it to the PNG list
            pendingPNG += tempRename;

            // Delete the .bmp from the temp folder
            QFile(currentBMP).remove();
        }
    }

    // GIF compression
    // If the GIF list isn't empty, the user wants to compress GIFs, and a GIF compression program exists
    if(!pendingGIF.isEmpty() && MIKSettings.value("bDefaultCompressGIF", false).toBool() && checkInstalledProgram("sDefaultGifsicleLocation", "gifsicle") != "") {
        // Compress each GIF in the pendingGIF list
        foreach (QString currentGIF, pendingGIF) {
            compressGIF(currentGIF);
        }
    }

    // JPG compression
    // If the JPG list isn't empty, the user wants to compress JPGs, and a JPG compression program exists
    if(!pendingJPG.isEmpty() && MIKSettings.value("bDefaultCompressJPG", false).toBool() && checkInstalledProgram("sDefaultJPEGOptimLocation", "jpegoptim") != "") {
        QThreadPool compressJPGsPool;
        // Pass each JPG file into the compressJPG function in its own thread
        foreach (QString currentJPG, pendingJPG) {
            QtConcurrent::run(&compressJPGsPool, compressJPG, currentJPG);
        }
        // Wait for all JPGs to finish
        compressJPGsPool.waitForDone();
    }

    // PNG compression
    // If the PNG list isn't empty, the user wants to compress PNGs, and a PNG compression program exists
    if(!pendingPNG.isEmpty() && MIKSettings.value("bDefaultCompressPNG", false).toBool() && checkInstalledProgram("sDefaultOxiPNGLocation", "oxipng") != "") {
        // Send all PNGs into the compression program
        compressPNGs(pendingPNG);
    }

    return;
}

// Converts a WAV into a FLAC
void convertWAV(QString inputWAV) {
    QString outputFLAC = inputWAV;
    outputFLAC.replace(".wav", ".flac");

    QProcess FLACProcess;
    QString programLocation = checkInstalledProgram("sDefaultFLACLocation", "flac");
    if(programLocation == "") {
        return;
    }
    FLACProcess.setProgram(programLocation);

    // FLAC arguments
    // -f: force
    // -V: verify
    // -8: level 8 compression (highest)
    // -o: output location
    QStringList arguments;
    arguments << "-f" << "-V" << "-8" << QDir::toNativeSeparators(inputWAV) << "-o" << QDir::toNativeSeparators(outputFLAC);
    FLACProcess.setArguments(arguments);

    // Start and wait
    FLACProcess.start();
    FLACProcess.waitForFinished(-1);

    // Remove the original WAV
    QFile(inputWAV).remove();
}

// Converts a FLAC to a FLAC (re-FLACing)
QString convertToFLAC(QString inputFLAC, conversionParameters_t *conversionParameters, int futureBPS, int futureSampleRate) {
    QString outputFLAC = "";
    int inputFLACBPS = 0;
    int inputFLACBitrate = 0;

    // Open the firstFLAC with TagLib, read some important data, then immediately destroy it
    // Under Windows, TagLib cannot open the same file multiple times so must be completely destroyed before the next access
    {
#if defined(Q_OS_LINUX)
        TagLib::FLAC::File inputFLACTagFile(inputFLAC.toStdString().data());
#elif defined(Q_OS_WIN)
        TagLib::FLAC::File inputFLACTagFile(inputFLAC.toStdWString().data());
#endif
        inputFLACBPS = inputFLACTagFile.audioProperties()->bitsPerSample();
        inputFLACBitrate = inputFLACTagFile.audioProperties()->sampleRate();
    }

    // If the user wants a SoX-specific feature and the file actually needs it
    if((conversionParameters->presetInput == "Force 16-bit" || conversionParameters->presetInput == "Force 44.1kHz/48kHz" || conversionParameters->presetInput == "Force 16-bit and 44.1kHz/48kHz") &&
        (inputFLACBPS >= 24 || (inputFLACBitrate != 44100 && inputFLACBitrate != 48000))) {
        // Variables to hold dynamic tag-based filenames as defined by the user
        QString parsedFileSyntax = parseNamingSyntax(conversionParameters->syntaxInput, conversionParameters->codecInput, conversionParameters->presetInput, inputFLAC, futureBPS, futureSampleRate);
        QString parsedFolderSyntax = "";

        // If the output is going to be in a nested folder(s)
        if(parsedFileSyntax.contains('/')) {
            // Set the folder to everything except the filename
            parsedFolderSyntax = parsedFileSyntax.mid(0, parsedFileSyntax.lastIndexOf('/'));
        }

        // Eventual name of the SoX-manipulated output FLAC
        outputFLAC = QFileInfo(inputFLAC).dir().path() + "/" + QFileInfo(inputFLAC).baseName() + "downsampled" + ".flac";

        QProcess SoXProcess;
        QString programLocation = checkInstalledProgram("sDefaultSoXLocation", "sox");
        if(programLocation == "") {
            return "";
        }
        SoXProcess.setProgram(programLocation);

        // SoX arguments
        // -G: Guarding to protect against clipping
        // -b: bit-depth
        // rate: add the "rate" effect to the effect chain
        // -v: volume adjustment
        // -L: linear phase response
        // 44100/48000/etc: sample rate that SoX should resample to
        // dither: triangular dithering (default dithering method)
        QStringList arguments;
        arguments << QDir::toNativeSeparators(inputFLAC) << "-G";

        // If the user requests 16-bit
        if(conversionParameters->presetInput == "Force 16-bit" || conversionParameters->presetInput == "Force 16-bit and 44.1kHz/48kHz") {
            arguments << "-b" << "16";
        }

        arguments << QDir::toNativeSeparators(outputFLAC) << "rate" << "-v" << "-L";

        // If the user requests downsampling
        if(conversionParameters->presetInput == "Force 44.1kHz/48kHz" || conversionParameters->presetInput == "Force 16-bit and 44.1kHz/48kHz") {
            if(inputFLACBitrate % 44100 == 0) {
                arguments << "44100";
            }
            else if(inputFLACBitrate % 48000 == 0) {
                arguments << "48000";
            }
        }

        // If no downsampling should occur, use the file's original samplerate (required via SoX syntax)
        else if(conversionParameters->presetInput == "Force 16-bit") {
            arguments << QString::number(inputFLACBitrate);
        }

        arguments << "dither";

        SoXProcess.setArguments(arguments);

        // Start and wait
        SoXProcess.start();
        SoXProcess.waitForFinished(-1);

        // Make any necessary folders for the file to live in
        QDir().mkpath(conversionParameters->outputDir.path() + "/" + parsedFolderSyntax);

        // If the conversion was successful
        if(QFile(outputFLAC).exists()) {
            // Remove the original
            QFile(conversionParameters->outputDir.path() + "/" + parsedFileSyntax + ".flac").remove();
            // Renamed the converted file to the original
            QFile(outputFLAC).rename(conversionParameters->outputDir.path() + "/" + parsedFileSyntax + ".flac");
        }

        // Set the eventual return value to this FLAC
        outputFLAC = conversionParameters->outputDir.path() + "/" + parsedFileSyntax + ".flac";
    }
    else {
        // Variables to hold dynamic tag-based filenames as defined by the user
        QString parsedFileSyntax = parseNamingSyntax(conversionParameters->syntaxInput, conversionParameters->codecInput, conversionParameters->presetInput, inputFLAC, futureBPS, futureSampleRate);
        QString parsedFolderSyntax;

        // If the output is going to be in a nested folder(s)
        if(parsedFileSyntax.contains('/')) {
            // Set the folder to everything except the filename
            parsedFolderSyntax = parsedFileSyntax.mid(0, parsedFileSyntax.lastIndexOf('/'));
        }

        // Eventual name of the output FLAC
        outputFLAC = conversionParameters->outputDir.path() + "/" + parsedFileSyntax + ".flac";
        // Make any necessary folders for the file to live in
        QDir().mkpath(conversionParameters->outputDir.path() + "/" + parsedFolderSyntax);

        QProcess FLACProcess;
        QString programLocation = checkInstalledProgram("sDefaultFLACLocation", "flac");
        if(programLocation == "") {
            return "";
        }
        FLACProcess.setProgram(programLocation);

        // FLAC arguments
        // -f: force
        // -V: verify
        // -8: level 8 compression (highest)
        // -o: output location
        QStringList arguments;
        arguments << "-f" << "-V" << "-8" << QDir::toNativeSeparators(inputFLAC) << "-o" << QDir::toNativeSeparators(outputFLAC);
        FLACProcess.setArguments(arguments);
        // Start and wait
        FLACProcess.start();
        FLACProcess.waitForFinished(-1);

        // Set the eventual return value to this FLAC
        outputFLAC = conversionParameters->outputDir.path() + "/" + parsedFileSyntax + ".flac";
    }

    return outputFLAC;
}

// Converts a FLAC to a Opus
QString convertToOpus(QString inputFLAC, conversionParameters_t *conversionParameters) {
    // Variables to hold dynamic tag-based filenames as defined by the user
    QString parsedFileSyntax = parseNamingSyntax(conversionParameters->syntaxInput, conversionParameters->codecInput, conversionParameters->presetInput, inputFLAC);
    QString parsedFolderSyntax = "";

    // If the output is going to be in a nested folder(s)
    if(parsedFileSyntax.contains('/')) {
        // Set the folder to everything except the filename
        parsedFolderSyntax = parsedFileSyntax.mid(0, parsedFileSyntax.lastIndexOf('/'));
    }

    // Eventual name of the output Opus
    QString outputOpus = conversionParameters->outputDir.path() + "/" + parsedFileSyntax + ".opus";

    // Make any necessary folders for the file to live in
    QDir().mkpath(conversionParameters->outputDir.path() + "/" + parsedFolderSyntax);

    QProcess OpusProcess;
    QString programLocation = checkInstalledProgram("sDefaultOpusLocation", "opusenc");
    if(programLocation == "") {
        return "";
    }
    OpusProcess.setProgram(programLocation);

    // Opus arguments
    // --quiet: suppress output
    // --bitrate: bitrate in kbps (defaults to VBR mode)
    QStringList arguments;
    arguments << "--quiet";

    if(conversionParameters->presetInput == "192kbps VBR") {
        arguments << "--bitrate" << "192";
    }
    else if(conversionParameters->presetInput == "160kbps VBR") {
        arguments << "--bitrate" << "160";
    }
    else if(conversionParameters->presetInput == "128kbps VBR") {
        arguments << "--bitrate" << "128";
    }
    else if(conversionParameters->presetInput == "96kbps VBR") {
        arguments << "--bitrate" << "96";
    }
    else if(conversionParameters->presetInput == "64kbps VBR") {
        arguments << "--bitrate" << "64";
    }
    else if(conversionParameters->presetInput == "32kbps VBR") {
        arguments << "--bitrate" << "32";
    }

    arguments << QDir::toNativeSeparators(inputFLAC) << QDir::toNativeSeparators(outputOpus);
    OpusProcess.setArguments(arguments);

    // Start and wait
    OpusProcess.start();
    OpusProcess.waitForFinished(-1);

    // Remove the resultant "ENCODER" and "ENCODER_OPTIONS" tags from output Opus files
#if defined(Q_OS_LINUX)
    TagLib::Ogg::Opus::File outputOpusTagFile(outputOpus.toStdString().data());
#elif defined(Q_OS_WIN)
    TagLib::Ogg::Opus::File outputOpusTagFile(outputOpus.toStdWString().data());
#endif
    TagLib::PropertyMap outputOpusTagMap = outputOpusTagFile.properties();
    outputOpusTagMap.erase("ENCODER");
    outputOpusTagMap.erase("ENCODER_OPTIONS");
    outputOpusTagFile.setProperties(outputOpusTagMap);
    outputOpusTagFile.save();

    return outputOpus;
}

// Converts a FLAC to an MP3
QString convertToMP3(QString inputFLAC, conversionParameters_t *conversionParameters) {
    // Variables to hold dynamic tag-based filenames as defined by the user
    QString parsedFileSyntax = parseNamingSyntax(conversionParameters->syntaxInput, conversionParameters->codecInput, conversionParameters->presetInput, inputFLAC);
    QString parsedFolderSyntax = "";

    // If the output is going to be in a nested folder(s)
    if(parsedFileSyntax.contains('/')) {
        // Set the folder to everything except the filename
        parsedFolderSyntax = parsedFileSyntax.mid(0, parsedFileSyntax.lastIndexOf('/'));
    }

    // Eventual name of the output WAV and output MP3
    QString outputWAV = conversionParameters->outputDir.path() + "/" + parsedFileSyntax + ".wav";
    QString outputMP3 = conversionParameters->outputDir.path() + "/" + parsedFileSyntax + ".mp3";

    // Make any necessary folders for the files to live in
    QDir().mkpath(conversionParameters->outputDir.path() + "/" + parsedFolderSyntax);

    // Two QStringLists to be used as a pair for a tag and its data to live in (TRACKNUMBER == 01, YEAR == 2017, and so on)
    QStringList pendingTagNames;
    QStringList pendingTagData;

    // Initialize an iterator for traversal through the TagFile
#if defined(Q_OS_LINUX)
    TagLib::FLAC::File inputFLACTagFile(inputFLAC.toStdString().data());
#elif defined(Q_OS_WIN)
    TagLib::FLAC::File inputFLACTagFile(inputFLAC.toStdWString().data());
#endif
    TagLib::Map<TagLib::String, TagLib::StringList>::ConstIterator it = inputFLACTagFile.xiphComment()->fieldListMap().begin();

    // Loop through the inputFLACTagFile
    while(it != inputFLACTagFile.xiphComment()->fieldListMap().end()) {
        // For every tag in a certain field (ALBUMARTIST, TRACKNUMBER, etc)
        // Can have multiple tags on one field, e.g. multiple ARTISTs
        for(unsigned int i = 0; i < it->second.size(); i++) {
            // Add the tag (GENRE, DISCNUMBER, etc) to the respective list
            pendingTagNames += TStringToQString(it->first);
            // Add the data (Rock, 2, etc) to the respective list
            pendingTagData += TStringToQString(it->second[i]);
        }
        // Increase the iterator for the next pass
        it++;
    }

    QProcess deFLACProcess;
    QString programLocation = checkInstalledProgram("sDefaultFLACLocation", "flac");
    if(programLocation == "") {
        return "";
    }
    deFLACProcess.setProgram(programLocation);

    // deFLAC arguments
    // -d: decode to WAV
    // -o: output location
    QStringList arguments;
    arguments << "-d" << QDir::toNativeSeparators(inputFLAC) << "-o" << QDir::toNativeSeparators(outputWAV);
    deFLACProcess.setArguments(arguments);

    // Start and wait
    deFLACProcess.start();
    deFLACProcess.waitForFinished(-1);

    QProcess LAMEProcess;
    programLocation = checkInstalledProgram("sDefaultLAMELocation", "lame");
    if(programLocation == "") {
        return "";
    }
    LAMEProcess.setProgram(programLocation);

    // LAME arguments
    // -q 0: use highest quality/slowest algorithms
    // -V: variable bitrate mode (VBR)
    // -b: constant bitrate mode (CBR)
    arguments.clear();
    arguments << "-q" << "0";

    if(conversionParameters->presetInput == "245kbps VBR (V0)")      {arguments << "-V" << "0";}
    else if(conversionParameters->presetInput == "225kbps VBR (V1)") {arguments << "-V" << "1";}
    else if(conversionParameters->presetInput == "190kbps VBR (V2)") {arguments << "-V" << "2";}
    else if(conversionParameters->presetInput == "175kbps VBR (V3)") {arguments << "-V" << "3";}
    else if(conversionParameters->presetInput == "165kbps VBR (V4)") {arguments << "-V" << "4";}
    else if(conversionParameters->presetInput == "130kbps VBR (V5)") {arguments << "-V" << "5";}
    else if(conversionParameters->presetInput == "115kbps VBR (V6)") {arguments << "-V" << "6";}
    else if(conversionParameters->presetInput == "100kbps VBR (V7)") {arguments << "-V" << "7";}
    else if(conversionParameters->presetInput == "85kbps VBR (V8)")  {arguments << "-V" << "8";}
    else if(conversionParameters->presetInput == "65kbps VBR (V9)")  {arguments << "-V" << "9";}
    else if(conversionParameters->presetInput == "320kbps CBR")      {arguments << "-b" << "320";}
    else if(conversionParameters->presetInput == "256kbps CBR")      {arguments << "-b" << "256";}
    else if(conversionParameters->presetInput == "192kbps CBR")      {arguments << "-b" << "192";}
    else if(conversionParameters->presetInput == "128kbps CBR")      {arguments << "-b" << "128";}
    else if(conversionParameters->presetInput == "64kbps CBR")       {arguments << "-b" << "64";}

    arguments << QDir::toNativeSeparators(outputWAV) << QDir::toNativeSeparators(outputMP3);

    LAMEProcess.setArguments(arguments);

    // Start and wait
    LAMEProcess.start();
    LAMEProcess.waitForFinished(-1);

    // Remove the original WAV file
    QFile(outputWAV).remove();

    // Create a TagFile and a PropertyMap for the resultant MP3. This MP3 will not have any data in its property map yet so we create a new one
#if defined(Q_OS_LINUX)
    TagLib::MPEG::File outputMP3TagFile(outputMP3.toStdString().data());
#elif defined(Q_OS_WIN)
    TagLib::MPEG::File outputMP3TagFile(outputMP3.toStdWString().data());
#endif
    TagLib::PropertyMap outputMP3TagMap;

    // Storage variable for efficiency
    QString pendingTag;

    for(int i = 0; i < pendingTagNames.count(); i++) {
        // Set the storage variable to the current tag in lowercase
        pendingTag = pendingTagNames[i].toLower();

        // Special cases to handle, such as when the name doesn't exactly match the tag or when the tag can contain multiple values (use .insert instead of .replace)
        if(pendingTag == "albumartist" || pendingTag == "album artist") {
            outputMP3TagMap.insert("ALBUMARTIST", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "artist" || pendingTag == "performer") {
            outputMP3TagMap.insert("ARTIST", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "beatsperminute") {
            outputMP3TagMap.replace("BPM", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "description") {
            outputMP3TagMap.replace("COMMENT", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "composer") {
            outputMP3TagMap.insert("COMPOSER", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "disc") {
            outputMP3TagMap.replace("DISCNUMBER", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "genre") {
            outputMP3TagMap.insert("GENRE", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "musicbrainzartistid") {
            outputMP3TagMap.replace("MUSICBRAINZ_ARTISTID", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "musicbrainzdiscid") {
            outputMP3TagMap.replace("MUSICBRAINZ_DISCID", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "musicbrainzreleaseartistid") {
            outputMP3TagMap.replace("MUSICBRAINZ_ALBUMARTISTID", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "musicbrainzreleasecountry") {
            outputMP3TagMap.replace("RELEASECOUNTRY", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "musicbrainzreleaseid") {
            outputMP3TagMap.replace("MUSICBRAINZ_ALBUMID", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "musicbrainzreleasestatus") {
            outputMP3TagMap.replace("MUSICBRAINZ_ALBUMSTATUS", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "musicbrainzreleasetype") {
            outputMP3TagMap.replace("MUSICBRAINZ_ALBUMTYPE", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "musicbrainztrackid") {
            outputMP3TagMap.replace("MUSICBRAINZ_TRACKID", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "musicipid") {
            outputMP3TagMap.replace("MUSICIP_PUID", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "musicbrainzreleasestatus") {
            outputMP3TagMap.replace("MUSICBRAINZ_ALBUMSTATUS", QStringToTString(pendingTagData[i]));
        }
        else if(pendingTag == "date") {
            outputMP3TagMap.replace("YEAR", QStringToTString(pendingTagData[i]));
        }
        // Else handle it as a regular tag. This includes custom tags and tags that are named the same as their respective official names, such as "artist" mapping to "ARTIST"
        else {
            outputMP3TagMap.replace(QStringToTString(pendingTag.toUpper()), QStringToTString(pendingTagData[i]));
        }
    }

    // Manual picture data handling, which must be manipulated and added as frames
    // For every picture in the original FLAC file
    for(unsigned int i = 0; i < inputFLACTagFile.pictureList().size(); i++) {
        // Make a new picture frame
        TagLib::ID3v2::AttachedPictureFrame *tempPictureFrame = new TagLib::ID3v2::AttachedPictureFrame;
        // Set its mimetype to the original picture's
        tempPictureFrame->setMimeType(inputFLACTagFile.pictureList()[i]->mimeType());
        // Set its description to the original picture's
        tempPictureFrame->setDescription(inputFLACTagFile.pictureList()[i]->description());
        // Set its type to the original picture's (casted from FLAC's type to MPEG's type)
        tempPictureFrame->setType(static_cast<TagLib::ID3v2::AttachedPictureFrame::Type>(inputFLACTagFile.pictureList()[i]->type()));
        // Set its data to the original picture's
        tempPictureFrame->setPicture(inputFLACTagFile.pictureList()[i]->data());

        // Add the frame to the original File
        outputMP3TagFile.ID3v2Tag()->addFrame(tempPictureFrame);
    }

    // Set the MP3's tags to the propertyMap that we have been manipulating
    outputMP3TagFile.setProperties(outputMP3TagMap);
    // Save the MP3 file. Arguments in order: Save all tags, strip any tags that are not in the propertyMap, use id3v2.4, and don't save id3v1 tags)
    outputMP3TagFile.save(TagLib::MPEG::File::AllTags, true, 4, false);

    return outputMP3;
}
