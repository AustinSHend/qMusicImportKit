# qMusicImportKit <a href="url"><img src="https://user-images.githubusercontent.com/2212907/70855912-361db400-1e98-11ea-9ee5-990acecfd763.png" align="left"></a>

Designed for power users who take lineage and data integrity seriously. Integrates many powerful tools into a natural workflow, and takes extra measures to make sure conversions are done the *right* way. Created due to my frustration with using many programs and conversion scripts in a slow and disjointed workflow.

This is a fully-rewritten port of [MusicImportKit](https://github.com/AustinSHend/MusicImportKit) in Qt and C++, focusing on cross-platform support for Linux and a refinement of the original MusicImportKit's features.

## Includes

* Parallel conversion to FLAC (-V8 re-FLACing), MP3, and Opus.

* Proper downsampling (e.g. 96kHz -> 48kHz) and bit-depth reduction (e.g. 24-bit -> 16-bit) using SoX (with a VHQ triangular dither filter, guarding, and 44.1/48 sample-rate detection).

* Genuine LAME header info is preserved by exporting all tags from a .flac, decoding to .wav (destroying all tags in the process), encoding the .wav to .mp3 through LAME, and reapplying original tags to the .mp3 (including preserving unlimited custom tags through TXXX frame manipulation).

* Loudgain-powered ReplayGain data on all formats, using the ITU-R BS.1770 algorithm with RG 2.0 (-18 dB) reference loudness and true peak calculation.

* Quicklinks to Discogs and MusicBrainz using automatic artist+album metadata from the input files.

* Integration with AlbumArtDownloader (multi-source album art fetching), PuddleTag/Mp3tag (powerful tagging software), and Spek (spectral analysis).

* Full custom parsing syntax, able to read any tag enclosed by "%" and several audio properties (codec, bitrate, sample-rate, bit-depth, etc). Includes several popular default syntaxes.

* Copy custom files from the input folder (and nested folders) into the output folder, with full regex+wildcards support.

* Optionally strip metadata and compress .bmps, .gifs, .jpegs, and .pngs, reducing filesize and bloat.

* Impossible to make bad (Lossy->Lossless, Lossy->Lossy) transcodes, ensuring that data stays artifact-free.

* Robust codebase, currently tested on **108** albums of all shapes and sizes (including a few [witch.house](https://user-images.githubusercontent.com/2212907/70855973-1935b080-1e99-11ea-8d0b-b25ea975d8b3.png) albums for good measure). All features have been double and triple-checked against proper traditional methods to make sure the output files match.

* All features operate as fast as possible while still maintaining proper output. This program will always trade speed for accuracy. Check "Necessary Limitations/Quirks" below for inconvenient aspects of that decision.

<div align="center">
    <a href="url"><img src="https://user-images.githubusercontent.com/2212907/89350881-72479400-d676-11ea-8666-55a5ac0430ad.png"></a>
</div>

## Basic Usage

1. Choose input folder: Pick a folder that contains .wavs or .flacs that you want to convert from (e.g. after unzipping an album from Bandcamp). Files in this folder will not be changed/touched.

2. Choose temp folder: Create a transient folder that exists as a working space while you prepare to convert (e.g. tagging and downloading art). Primarily created through the "Copy" button above it, but can also be pointed at any folder verbatim.

3. Guess metadata: Upon confirming a temp folder (through copy or otherwise), these boxes will be autofilled based on the first available .flac's metadata (but can be changed if the metadata is incorrect).

4. Use Discogs/MusicBrainz links: These buttons will search Discogs and MusicBrainz, using the Artist/Album textboxes above.

5. Use AlbumArtDownloader/PuddleTag/Mp3Tag/Spek:
    * AlbumArtDownloader will use the Artist/Album textboxes above for its query, and save files to the temp folder.
    * PuddleTag/Mp3Tag will open with the temp folder as its target, and you can freely edit tags.
    * The Spek button will open every .flac in the temp folder sequentially in Spek. Spek can be used to detect files which have been "upconverted" or "transcoded" (usually used in a negative context).
        * Converting from a lossy (MP3, Opus) file to a lossless (FLAC, WAV) file does not increase its quality, and you may find that Bandcamp artists that don't know better are just transcoding their MP3s to FLAC to upload to Bandcamp. This means you're not really getting lossless files; you're getting bloated MP3s.
        * True lossless files will extend to the very top of the spectral with no shelves visible
        * 320kbps CBR MP3s that have been transcoded to FLAC will have a "cut-off" at 20.5kHz and a barely visible "shelf" at 16kHz
        * 256kbps CBR MP3s that have been transcoded to FLAC will have a "cut-off" at 20kHz and a clearly visible "shelf" at 16kHz
        * 245kbps VBR (aka V0) MP3s that have been transcoded to FLAC will have a "cut-off" at 19.5kHz and a visible "shelf" at 16kHz
        * 192kbps CBR MP3s that have been transcoded to FLAC will have a "cut-off" at 19kHz and a clearly visible "shelf" at 16kHz
        * 190kbps VBR (aka V2) MP3s that have been transcoded to FLAC will have a "cut-off" at 18.5kHz and a visible "shelf" at 16kHz
        * 128kbps CBR MP3s that have been transcoded to FLAC will have a "cut-off" at 16kHz

6. Choose output folder: Pick a base folder that you want to send the converted files to. This folder path will be combined with your preferred syntax to create directories and files as desired.

7. Create preferred syntax: Create a syntax to specify what your folders and files are going to be named. You can send files directly to the output folder with something like "%tracknumber%. %title%" or send them to a folder with something like "%albumartist% - %album%\\%tracknumber%. %title%"

8. Choose options: Most options are straightforward.
    * Copy specific filetypes will copy all matching files in the temp folder to the output folder. Regex and wildcards are supported.

9. Choose conversion option:
    * FLAC:
        * FLAC encodes require `flac` (Linux) or `flac.exe` (Windows)
        * All FLAC encodes use V8 (highest) compression. There is never a reason to use less than V8.
        * All FLAC conversions will re-encode your temp .flacs. Useful for forcing V8 compression, easy renaming and moving, ReplayGain, and other included features.
        * Forcing 16-bit will reduce 24-bit FLACs to 16-bit FLACs. This massively decreases the filesize, but drops genuine inaudible sound data. This feature requires `sox` (Linux) or `sox.exe` (Windows)
        * Forcing 44.1kHz/48kHz will reduce a FLAC's sample rate to 44.1kHz or 48kHz, depending on its original sample rate. This will massively decrease the filesize, but drops genuine inaudible sound data. This feature requires `sox` (Linux) or `sox.exe` (Windows)
        * Both 16-bit and 44.1/48 forcing will only occur if a file needs it.

    * MP3:
        * MP3 conversions require both `lame` and `flac` (Linux) or `lame.exe` and `flac.exe` (Windows)
        * CBR and VBR are supported. VBR options are superior and recommended, but CBR options are included for compatibility with certain hardware.
        * V0 is considered transparent, or indistinguishable from the original FLAC file. This is the recommended setting for high quality MP3 audio.
        * Other recommended encoder settings can be found [here](https://wiki.hydrogenaud.io/index.php?title=LAME#Recommended_encoder_settings).

    * Opus:
        * Opus conversions require `opusenc` (Linux) or `opusenc.exe` (Windows)
        * 192kbps VBR is considered transparent, or indistinguishable from the original FLAC file. This is the recommended setting for high quality Opus audio.
        * Other recommended encoder settings can be found [here](https://wiki.hydrogenaud.io/index.php?title=Opus#Music_encoding_quality) and [here](https://wiki.xiph.org/Opus_Recommended_Settings#Recommended_Bitrates).


## Plugins

* [AlbumArt (WINE)](https://hydrogenaud.io/index.php?topic=57392.msg984669#msg984669) (Linux) or [AlbumArt.exe](https://sourceforge.net/projects/album-art/) (Windows)
    * Opens AlbumArtDownloader with the artist+album filled out (from the "guessed" textboxes above) and pointed at the temp folder
    * AlbumArtDownloader works under Linux with specific configurations. See the linked hydrogenaud.io thread for more details.
    * When asked for a custom command in qMIK, the format `WINEPREFIX=/home/user/.wineAAD wine /home/user/.wineAAD/drive_c/Program\ Files/AlbumArtDownloader/AlbumArt.exe` is confirmed working on my machine. Adjust for your own install locations and username.

* `loudgain` (Linux) or [Loudgain (WSL)](https://github.com/Moonbase59/loudgain) (Windows)
	* Scans ReplayGain data for tracks.
	* Loudgain only works under WSL on Windows. Installation instructions are available via their Github page.
	* qMIK will automatically use your default WSL distro's `loudgain` installation, so make sure it is callable there if you want it to be detected.

* `gifsicle` (Linux) or [gifsicle.exe](https://github.com/kohler/gifsicle) ([Unofficial binaries](https://eternallybored.org/misc/gifsicle/)) (Windows)
	* Compresses and strips metadata from .gifs

* `jpegoptim` (Linux) or [jpegoptim.exe](https://github.com/tjko/jpegoptim) ([Unofficial binaries](https://github.com/XhmikosR/jpegoptim-windows)) (Windows)
	* Compresses and strips metadata from .jpgs

* `flac` (Linux) or [flac.exe](https://xiph.org/flac/) (Windows)
    * Encode input .wavs to .flac
    * Re-encode input .flacs to .flac
    * Decode .flac to .wav, for feeding into LAME

* `lame` (Linux) or [lame.exe](http://lame.sourceforge.net/) ([Unofficial binaries](http://rarewares.org/mp3-lame-bundle.php)) (Windows)
    * Convert .wav to .mp3 (automatically gets .wavs from `flac`/`flac.exe`, which is also required for MP3 conversions)

* `oxipng` (Linux) or [oxipng.exe](https://github.com/shssoichiro/oxipng) (Windows)
	* Compresses and strips metadata from .pngs and .bmps

* `puddletag` (Linux) or [Mp3Tag.exe](https://www.mp3tag.de/en/) (Windows)
    * Highly recommended to pair with GrammarTron for [Puddletag](https://gist.github.com/AustinSHend/7ca8522d3f70a19d25596a773584236d) or [Mp3Tag](https://community.mp3tag.de/t/case-conversion/11684)
    * Opens the temp folder for tag editing

* `opusenc` (Linux) or [opusenc.exe](https://opus-codec.org/downloads/) (Windows)
    * Convert .flac into .opus (`flac`/`flac.exe` not required)

* `sox` (Linux) or [sox.exe](http://sox.sourceforge.net/) (Windows)
    * Resample .flacs
    * Reduce bit-depths of .flacs

* `spek` (Linux) or [spek.exe](http://spek.cc/) (Windows)
    * Opens the temp folder in Spek for spectral analysis


## Necessary Limitations/Quirks

* MP3 Conversions:
    * Simpler methods of MP3 conversion (e.g. FFmpeg, which uses LAME as well) strip the LAME header info from the output MP3 and thus there is no (easy) way to tell if an unknown MP3 file that you find used LAME in its creation or an inferior tool (such as FhG). For being courteous to others (and our future selves), we take extra steps to preserve this data. Manual decoding to .wav and encoding to .mp3 is actually faster than using an FFmpeg implementation, but destroys tags in the process so we handle that manually.

* ReplayGain:
    * ReplayGain album-mode calculation cannot be multithreaded and includes relatively intensive true peak calculation. This means the ReplayGain process takes a frustratingly *large* portion of the overall conversion process time. Disable ReplayGain if you don't need it or speed is a priority.

## Compilation Dependencies

* QT5 >= 5.10

* TagLib > 1.9.1

## Credits

* Uses [TagLib](https://taglib.org/) to assist with tag reading.
