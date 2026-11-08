#include "FreeImage.h"

#include <climits>
#include <cstdio>
#include <cstring>

// _MemoryWriteProc used to add current_position and the write size in a
// long, which is 32 bits on Windows: a seek to INT_MAX - 7 followed by a
// 16-byte write overflowed and wrote past the buffer.

static bool fail(const char *message) {
	std::fprintf(stderr, "Memory write position test failed: %s\n", message);
	return false;
}

static bool run() {
	FIMEMORY *memory = FreeImage_OpenMemory();
	if(memory == NULL) {
		return fail("could not open memory stream");
	}

	bool ok = true;
	BYTE data[16];
	std::memset(data, 0xA5, sizeof(data));

	const long near_end = INT_MAX - 7;
	if(!FreeImage_SeekMemory(memory, near_end, SEEK_SET)) {
		ok = fail("seek to INT_MAX - 7 was rejected");
	} else if(FreeImage_WriteMemory(data, 1, sizeof(data), memory) != 0) {
		ok = fail("write ending past INT_MAX was accepted");
	} else if(FreeImage_TellMemory(memory) != near_end) {
		ok = fail("rejected write moved the stream position");
	}

	// The stream must stay usable for ordinary writes.
	if(ok) {
		BYTE *buffer = NULL;
		DWORD size = 0;
		if(!FreeImage_SeekMemory(memory, 0, SEEK_SET)) {
			ok = fail("seek back to 0 was rejected");
		} else if(FreeImage_WriteMemory(data, 1, sizeof(data), memory) != sizeof(data)) {
			ok = fail("ordinary write after a rejected write failed");
		} else if(!FreeImage_AcquireMemory(memory, &buffer, &size) || size != sizeof(data) ||
			std::memcmp(buffer, data, sizeof(data)) != 0) {
			ok = fail("ordinary write produced the wrong contents");
		}
	}

	FreeImage_CloseMemory(memory);
	return ok;
}

int main() {
	FreeImage_Initialise();
	const bool ok = run();
	FreeImage_DeInitialise();
	return ok ? 0 : 1;
}
