#!/usr/bin/env python3
'''
Rotating file package
'''

from datetime import datetime
from glob import glob
import logging
import os
from subprocess import check_call


class RotatingFile():
    def __init__(self, dir, filename, maxNumFiles, maxFileSize, compressDir=None, append=False):
        self.indx = 0
        self.directory = dir
        self.fileName = filename
        self.fileBasename, self.fileExtension = os.path.splitext(filename)
        self.filePath = os.path.join(dir, filename)
        self.fileBasePath = os.path.join(dir, self.fileBasename)
        self.maxNumFiles = maxNumFiles
        self.maxFileSize = maxFileSize
        if compressDir:
            if not os.path.isdir(compressDir):
                logging.info(f"Directory for storing compressed files doesn't exist, creating {compressDir}")
                os.mkdir(compressDir, 0o755)
        self.compressDir = compressDir
        self.append = append

        self.lastFilePath = f"{self.fileBasePath}_{maxNumFiles:06d}{self.fileExtension}"

        self.fd = None

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, execType, execValue, execTb):
        if isinstance(execValue, Exception):
            logging.error(f"Exception in block: {execType}")
            logging.error(f"Exception message: {execValue}")
            return True

    def _rotate(self, force=False):
        """
          Always write the the named base file, when that gets full, push all
           saved files down to the next larger index, and if a compressDir is
           given, compress the oldest one, giving it a name with the time that
           it is was compressed and moving it to the compress directory.
          The old base file is given the index '01' and a new base file is
           created.
          The force option is needed to rotate before a write that would
           overfill the file.
        """
        fn = lambda i: f"{self.fileBasePath}_{i:06d}{self.fileExtension}"
        if force or (os.path.getsize(self.filePath) >= self.maxFileSize):
            logging.debug(f"Rotating: closing old '{self.filePath}")
            self.close()
            numFiles = min(len(glob(f"{self.fileBasePath}_*{self.fileExtension}")), self.maxNumFiles)
            print("numFiles:", numFiles)
            for i in reversed(range(1, numFiles + 1)):
                logging.debug(f"Renaming '{fn(i)}' to '{fn(i + 1)}'")
                os.rename(fn(i), fn(i + 1))
            logging.debug(f"Renaming '{self.filePath}' to '{fn(1)}'")
            os.rename(self.filePath, fn(1))
            if os.path.exists(self.lastFilePath):
                logging.debug(f"Dealing with last file: {self.lastFilePath}")
                if self.compressDir:
                    # compress the last file, move it to the compress dir, rename with current time
                    check_call(['gzip', self.lastFilePath])
                    compressFileName = f"{self.fileBasename}_{datetime.now().isoformat()}{self.fileExtension}.gz"
                    compressFilePath = os.path.join(self.compressDir, compressFileName)
                    logging.debug(f"Moving compressed {self.lastFilePath} to {compressFilePath}")
                    os.rename(f"{self.lastFilePath}.gz", compressFilePath)
                else:
                    logging.debug(f"Dropping oldest file: {self.lastFilePath}")
                    os.remove(self.lastFilePath)
            logging.debug(f"Reopening new '{self.fileBasePath}")
            self.open()

    def open(self):
        if self.append:
            mode = 'a'
        else:
            mode = 'w'
        self.fd = open(self.filePath, mode)

    def write(self, msg, flushOnWrite=True):
        """Write string to file
          Test if the msg will fit before writing, and if so, rotate the file
           first -- never write a file bigger than the max size.
          Flush before checking file size to allow asynch write to happen.

          Inputs
            msg: string to be written to file
            flushOnWrite: optional boolean that cause a flush after each write
        """
        logging.debug(f"Writing {len(msg)} bytes to {self.filePath}")
        self.fd.flush()
        if os.path.getsize(self.filePath) + len(msg) > self.maxFileSize:
            self._rotate(True)
        self.fd.write(msg)
        if flushOnWrite:
            self.fd.flush()

    def close(self):
        self.fd.close()

'''
    @property
    def filenameTemplate(self):
        return os.path.join(self.dir, self.filename + f"_{self.indx:06d}")
'''
    

#
# TESTING
#
if __name__ == '__main__':
    FILES_DIR = "/tmp"
    FILE_NAME = "foo.log"
    FILE_BASE = "foo"
    FILE_EXT = ".log"
    NUM_FILES = 3
    FILE_SIZE = 100
    LOG_LEVEL = "DEBUG"

    logging.basicConfig(level=LOG_LEVEL,
                        format='%(asctime)s %(levelname)-8s %(message)s',
                        datefmt='%Y-%m-%d %H:%M:%S')

    myFile = RotatingFile(FILES_DIR, FILE_NAME, NUM_FILES, FILE_SIZE)
    myFile.open()
    for _ in range((2 * NUM_FILES) + 1):
        for _ in range(FILE_SIZE):
            myFile.write("0123456789")  # N.B. integer multiple of FILE_SIZE
    myFile.close()
    sizes = []
    for name in glob(f"{FILES_DIR}/{FILE_BASE}*{FILE_EXT}"):
#        size = os.stat(name).st_size
        size = os.path.getsize(name)
        sizes.append(size)
    assert len(sizes) == NUM_FILES and sum(sizes) == FILE_SIZE * NUM_FILES, f"Failed to write {NUM_FILES} files of {FILE_SIZE} bytes: {sizes}"
    print("Test1: Success")

    with RotatingFile(FILES_DIR, FILE_NAME, NUM_FILES, FILE_SIZE, "/tmp/bar") as f:
        for i in range(4 * NUM_FILES * FILE_SIZE):
            msg = f"test  {i:03}\n"
            f.write(msg)
    for name in glob(f"{FILES_DIR}/{FILE_NAME}*"):
        print(name, os.stat(name).st_size)
        #### TODO test if there are NUM_FILES full of 'i' ... 'i - NUM_FILES'
    print("Test2: Success")
