// ==========================================================
// Input/Output functions
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
//
// This file is part of FreeImage 3
//
// Copyright (C) 2003-2026 The FreeImage Project
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================

// Select wide POSIX stdio offsets in this translation unit for legacy make
// builds as well as CMake. Feature macros must precede all system headers.
#if !defined(_WIN32) && !defined(_FILE_OFFSET_BITS)
#define _FILE_OFFSET_BITS 64
#endif

#include "FreeImage.h"
#include "Utilities.h"
#include <stdint.h>
#include <limits.h>
#include <errno.h>
#ifndef _WIN32
#include <sys/types.h>
#endif
#include "FreeImageIO.h"

// =====================================================================
// File IO functions
// =====================================================================

unsigned DLL_CALLCONV 
_ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
	return (unsigned)fread(buffer, size, count, (FILE *)handle);
}

unsigned DLL_CALLCONV 
_WriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
	return (unsigned)fwrite(buffer, size, count, (FILE *)handle);
}

int DLL_CALLCONV
_SeekProc(fi_handle handle, long offset, int origin) {
	return fseek((FILE *)handle, offset, origin);
}

long DLL_CALLCONV
_TellProc(fi_handle handle) {
	return ftell((FILE *)handle);
}

static uint64_t DLL_CALLCONV
_ReadProc64(void *buffer, uint64_t size, uint64_t count, fi_handle handle) {
	if (size == 0 || count == 0 || size > SIZE_MAX || count > SIZE_MAX / size) return 0;
	return (uint64_t)fread(buffer, (size_t)size, (size_t)count, (FILE *)handle);
}

static uint64_t DLL_CALLCONV
_WriteProc64(void *buffer, uint64_t size, uint64_t count, fi_handle handle) {
	if (size == 0 || count == 0 || size > SIZE_MAX || count > SIZE_MAX / size) return 0;
	return (uint64_t)fwrite(buffer, (size_t)size, (size_t)count, (FILE *)handle);
}

static int DLL_CALLCONV
_SeekProc64(fi_handle handle, int64_t offset, int origin) {
#ifdef _WIN32
	return _fseeki64((FILE *)handle, offset, origin);
#else
	// Legacy make/toolchains may still expose a narrower off_t. Never truncate.
	if ((int64_t)(off_t)offset != offset) {
		errno = EOVERFLOW;
		return -1;
	}
	return fseeko((FILE *)handle, (off_t)offset, origin);
#endif
}

static int64_t DLL_CALLCONV
_TellProc64(fi_handle handle) {
#ifdef _WIN32
	return _ftelli64((FILE *)handle);
#else
	return (int64_t)ftello((FILE *)handle);
#endif
}

// ----------------------------------------------------------

void
SetDefaultIO(FreeImageIO *io) {
	io->read_proc  = _ReadProc;
	io->seek_proc  = _SeekProc;
	io->tell_proc  = _TellProc;
	io->write_proc = _WriteProc;
}

void
SetDefaultIO64(FreeImageIO64 *io) {
	io->read_proc  = _ReadProc64;
	io->seek_proc  = _SeekProc64;
	io->tell_proc  = _TellProc64;
	io->write_proc = _WriteProc64;
}

// =====================================================================
// Memory IO functions
// =====================================================================

/**
The _MemoryReadProc function reads up to count items of size bytes from the input stream and stores them in buffer.
_MemoryReadProc returns the number of full items actually read, 
which may be less than count if an error occurs or if the end of the file is encountered before reaching count.
If size or count is 0, _MemoryReadProc returns 0 and the buffer contents are unchanged

@param buffer
@param size
@param count
@param handle
@return
*/
unsigned DLL_CALLCONV
_MemoryReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
	if (!handle || !buffer || (size == 0) || (count == 0)) {
		return 0;
	}

	FIMEMORYHEADER *mem_header = (FIMEMORYHEADER*)(((FIMEMORY*)handle)->data);

	// NB: size and count come straight from file-driven callers (e.g. height
	// * pitch), so the total must be checked without overflowing: the int
	// product introduced by the 3.19 rewrite wraps for huge inputs
	// (CVE-2023-47992 bug class), bypassing the bounds check below and
	// handing memcpy a giant length. A size_t product is still not enough:
	// it wraps exactly like before on ILP32 targets (MinGW32, Win32 MSVC,
	// armv7, wasm32). Compare with division instead, which does not depend
	// on the width of size_t.
	const int remaining_bytes = mem_header->file_length - mem_header->current_position;
	if (remaining_bytes > 0) {
		const size_t remaining = (size_t)remaining_bytes;
		// size != 0 here (guarded above), so the division is safe.
		if (count <= remaining / size) {
			const size_t required_bytes = (size_t)size * count; // cannot overflow: <= remaining
			// copy size bytes count times
			memcpy(buffer, (char*)mem_header->data + mem_header->current_position, required_bytes);
			mem_header->current_position += (int)required_bytes;
			return count;
		}
		else {
			// if there isn't required_bytes bytes left to read, set pos to eof and return a short count
			memcpy(buffer, (char*)mem_header->data + mem_header->current_position, remaining);
			mem_header->current_position = mem_header->file_length;
			return (unsigned)(remaining / size);
		}
	}

	// if size or count is 0, _MemoryReadProc returns 0 and the buffer contents are unchanged.
	return 0;
}

unsigned DLL_CALLCONV 
_MemoryWriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
	if (!handle || !buffer) {
		return 0;
	}

	FIMEMORYHEADER *mem_header = (FIMEMORYHEADER*)(((FIMEMORY*)handle)->data);

	// NB: the old (long)(size * count) wrapped in 32-bit unsigned before
	// the cast (same bug class as the _MemoryReadProc total above), so a
	// huge total became tiny and the copy claimed more items than were
	// written. Compute the product wide and refuse totals the int stream
	// positions (max 2 GiB data_length) cannot represent. Zero-size
	// behavior is unchanged (required_bytes == 0, same as before).
	const unsigned long long required_total = (unsigned long long)size * (unsigned long long)count;
	if (required_total > 0x7FFFFFFFull) {
		return 0;
	}
	const long required_bytes = (long)required_total;

	// NB: long is only 32 bits on Windows (LLP64), so current_position +
	// required_bytes below is signed overflow there once the stream has
	// been seeked near INT_MAX (e.g. seek to INT_MAX - 7, write 16 bytes):
	// the wrapped sum skips the growth loop and memcpy writes far past the
	// buffer. Refuse any write whose end the int stream positions cannot
	// represent before forming that sum. current_position is never negative
	// (_MemorySeekProc keeps it in [0, INT_MAX]), so the subtraction is safe.
	if (required_bytes > (long)(std::numeric_limits<int>::max() - mem_header->current_position)) {
		return 0;
	}

	// double the data block size if we need to
	while( (mem_header->current_position + required_bytes) >= mem_header->data_length ) {
		long newdatalen = 0;

		// if we are at or above 1G, we cant double without going negative
		if( mem_header->data_length & 0x40000000 ) {
			// max 2G
			if( mem_header->data_length == 0x7FFFFFFF ) {
				return 0;
			}
			newdatalen = 0x7FFFFFFF;
		} else if( mem_header->data_length == 0 ) {
			// default to 4K if nothing yet
			newdatalen = 4096;
		} else {
			// double size
			newdatalen = mem_header->data_length << 1;
		}
		void *newdata = realloc(mem_header->data, newdatalen);
		if(!newdata) {
			return 0;
		}
		mem_header->data = newdata;
		mem_header->data_length = (int)newdatalen;
	}

	memcpy((char *)mem_header->data + mem_header->current_position, buffer, required_bytes);
	mem_header->current_position += required_bytes;

	if( mem_header->current_position > mem_header->file_length ) {
		mem_header->file_length = mem_header->current_position;
	}

	return count;
}

/**
The _MemorySeekProc function moves the file pointer (if any) associated with stream to a new location that is offset bytes from origin. 
The next operation on the stream takes place at the new location. On a stream open for update, the next operation can be either a read or a write.
The argument origin must be one of the following constants, defined in STDIO.H:
	SEEK_CUR	Current position of file pointer.
	SEEK_END	End of file.
	SEEK_SET	Beginning of file.
You can use _MemorySeekProc to reposition the pointer anywhere in a file. 
The pointer can also be positioned beyond the end of the file. 

@param handle
@param offset
@param origin
@return If successful, returns 0. Otherwise, returns -1.
*/
int DLL_CALLCONV 
_MemorySeekProc(fi_handle handle, long offset, int origin) {
	if (!handle) {
		return -1;
	}

	FIMEMORYHEADER *mem_header = (FIMEMORYHEADER*)(((FIMEMORY*)handle)->data);

	// you can use _MemorySeekProc to reposition the pointer anywhere in a file
	// the pointer can also be positioned beyond the end of the file

	switch (origin) { //0 to filelen-1 are 'inside' the file
		default:
		case SEEK_SET: //can fseek() to 0-7FFFFFFF always
			if ((offset >= 0) && (offset <= std::numeric_limits<int>::max())) {
				// the 64-bit long offset may overflow when casted to int
				mem_header->current_position = (int)offset;
				return 0;
			}
			break;

		case SEEK_CUR:
			if (((mem_header->current_position + offset) >= 0) && ((mem_header->current_position + offset) <= std::numeric_limits<int>::max())) {
				// the 64-bit result may overflow when casted to int
				mem_header->current_position = (int)(mem_header->current_position + offset);
				return 0;
			}
			break;

		case SEEK_END:
			if (((mem_header->file_length + offset) >= 0) && ((mem_header->file_length + offset) <= std::numeric_limits<int>::max())) {
				// the 64-bit result may overflow when casted to int
				mem_header->current_position = (int)(mem_header->file_length + offset);
				return 0;
			}
			break;
	}

	return -1;
}

long DLL_CALLCONV 
_MemoryTellProc(fi_handle handle) {
	if (!handle) {
		return -1;
	}
	FIMEMORYHEADER *mem_header = (FIMEMORYHEADER*)(((FIMEMORY*)handle)->data);
	return mem_header->current_position;
}

// ----------------------------------------------------------

void
SetMemoryIO(FreeImageIO *io) {
	io->read_proc  = _MemoryReadProc;
	io->seek_proc  = _MemorySeekProc;
	io->tell_proc  = _MemoryTellProc;
	io->write_proc = _MemoryWriteProc;
}
