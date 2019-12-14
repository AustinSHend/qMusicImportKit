#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <iomanip>

#include <QDir>
#include <QProcess>
#include <QSettings>
#include <QtConcurrent/QtConcurrentRun>
#include <QThreadPool>

#include <attachedpictureframe.h>
#include <flacfile.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <opusfile.h>
#include <tpropertymap.h>

#include "xlsxdocument.h"

struct conversionParameters_t {
    QStringList inputFLACs;
    QDir outputDir;
    QString presetInput;
    QString syntaxInput;
    QString codecInput;
};

void getBashPATH();
bool removeDir(const QString &dirName);
QDir getNearestParent(QDir pathDir);
QString cleanString(QString input, QString ignoredChars = "");
QStringList findFiles(QDir rootDir, QStringList patternList = {"*.*"});
QString checkInstalledProgram(QString location, QString programName = "", bool useSettingsKey = true);
void openSpekWorker(QStringList inputFLACs);
QString parseNamingSyntax(QString syntax, QString codec, QString preset, QString filename, int futureBPS = -1, int futureSampleRate = -1);
void compressGIF(QString inputGIF);
void compressJPG(QString inputJPG);
void compressPNGs(QStringList inputPNGs);
QString getRealImageFormat(QString inputImage);
void compressImages(QStringList inputFiles);
void convertWAV(QString inputWAV);
QString convertToFLAC(QString inputFLAC, conversionParameters_t *conversionParameters, int futureBPS, int futureSampleRate);
QString convertToOpus(QString inputFLAC, conversionParameters_t *conversionParameters);
QString convertToMP3(QString inputFLAC, conversionParameters_t *conversionParameters);
bool insertInWorkbook (QString lastFolder, QString logScore, QString notes);

#endif // HELPER_H
