# Adf7z
7-zip plugin to extract Amiga OFS or FFS files from ADF images.

## Limitations
Limitations include but are not limited to:
* Cannot create or modify ADF-files. Only extract files.
* No support for links.
* File attributes are ignored.

## Credits
* Uses code from 7-Zip by Igor Pavlov (https://www.7-zip.org/)
* Uses github mirror of 7-Zip repository by kornelski (https://github.com/kornelski/7z)
* Based on information from "The .ADF (Amiga Disk File) format FAQ" by Laurent Clévy (http://lclevy.free.fr/adflib/adf_info.html)

## Install
Create a folder 'Formats' in your 7-zip install directory and copy ADFHandler.dll into it.

Example: "C:\Program Files\7-Zip\Formats\ADFHandler.dll"
