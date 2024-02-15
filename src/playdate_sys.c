//
// Created by Fabian Hartmann.
// https://github.com/yesnocancel
//

#include "playdate_sys.h"

void registerPlaydateSysFunctions(void) {
    doom_set_print(playdate_printfn);
    doom_set_malloc(playdate_malloc, playdate_free);
    doom_set_file_io(playdate_fopen, playdate_fclose, playdate_read, playdate_write, playdate_seek, playdate_tell,
                     playdate_eof);
    doom_set_gettime(playdate_get_time);
    doom_set_exit(playdate_exit);
    doom_set_getenv(playdate_getenv);
}

void playdate_printfn(const char* str) {
    playdate->system->logToConsole(str);
}

void* playdate_malloc(int size) {
    if (size == 0) {
        size = 1;
    }
    return playdate->system->realloc(NULL, size);
}

void playdate_free(void* ptr) {
    playdate->system->realloc(ptr, 0);
}

SDFile* playdate_fopen(const char* filename, const char* mode) {
    FileOptions opt = 0;
    if (strcmp((mode), "r") == 0 || strcmp((mode), "rb") == 0) {
        opt = kFileRead | kFileReadData;
    } else if (strcmp((mode), "w") == 0 || strcmp((mode), "wb") == 0) {
        opt = kFileWrite;
    } else {
        playdate_printfn("Unknown file mode!!");
        // no idea, keep it NULL for now and let it crash
    }

    return playdate->file->open(filename, opt);
}

void playdate_fclose(SDFile* handle) {
    playdate->file->close(handle);
}

void playdate_exit(int code) {
    while (1) { asm("nop"); };
}

char* playdate_getenv(const char* var) {
    return ".";
}

int playdate_read(SDFile* handle, void* buf, int count) {
    return playdate->file->read(handle, buf, count);
}

int playdate_write(SDFile* handle, const void* buf, int count) {
    return playdate->file->write(handle, buf, count);
}

int playdate_seek(SDFile* handle, int offset, doom_seek_t origin) {
    int whence = 0;
    switch (origin) {
        case DOOM_SEEK_CUR:
            whence = SEEK_CUR;
            break;
        case DOOM_SEEK_END:
            whence = SEEK_END;
            break;
        case DOOM_SEEK_SET:
            whence = SEEK_SET;
            break;
    }

    return playdate->file->seek(handle, offset, whence);
}

int playdate_tell(SDFile* handle) {
    return playdate->file->tell(handle);
}

int playdate_eof(SDFile* handle) {
    int current_offset = playdate->file->tell(handle);

    // get file size
    int filesize = playdate->file->seek(handle, 0, SEEK_END);

    // return to original offset
    playdate->file->seek(handle, current_offset, SEEK_SET);

    return current_offset >= filesize;
}

void playdate_get_time(int* sec, int* usec) {
    float time = playdate->system->getElapsedTime();

    int timeSec = (int) time;
    int timeUsec = (int) ((time - timeSec) * 1000 * 1000);

    *sec = timeSec;
    *usec = timeUsec;
}



