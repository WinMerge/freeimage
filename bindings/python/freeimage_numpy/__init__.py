"""Owned, top-down RGB/RGBA NumPy arrays; no FreeImage bitmap escapes a call."""
from __future__ import annotations
import ctypes as C
import ctypes.util
import os
import threading
import numpy as np

__all__ = ["load", "loadf", "save", "load_bytes", "save_bytes", "version"]
_lock = threading.RLock()
_library = None


def _lib():
    global _library
    with _lock:
        if _library is not None:
            return _library
        path = os.environ.get("FREEIMAGE_LIBRARY")
        if not path:
            path = ctypes.util.find_library("freeimage") or ctypes.util.find_library("FreeImage")
        if not path:
            raise OSError("FreeImage shared library not found; set FREEIMAGE_LIBRARY to its full path")
        lib = (C.WinDLL if os.name == "nt" else C.CDLL)(path)
        ptr, uint, integer = C.c_void_p, C.c_uint, C.c_int
        signatures = {
            "Initialise": (None, [integer]), "GetVersion": (C.c_char_p, []),
            "HasPixels": (integer, [ptr]), "Unload": (None, [ptr]), "GetWidth": (uint, [ptr]), "GetHeight": (uint, [ptr]),
            "GetBPP": (uint, [ptr]), "GetImageType": (integer, [ptr]), "GetColorType": (integer, [ptr]),
            "GetRedMask": (uint, [ptr]), "IsTransparent": (integer, [ptr]),
            "GetScanLine": (ptr, [ptr, integer]), "ConvertTo24Bits": (ptr, [ptr]), "ConvertTo32Bits": (ptr, [ptr]),
            "AllocateT": (ptr, [integer, integer, integer, integer, uint, uint, uint]), "GetPalette": (ptr, [ptr]),
            "GetFIFFromFormat": (integer, [C.c_char_p]),
            "GetFileType": (integer, [C.c_char_p, integer]), "GetFIFFromFilename": (integer, [C.c_char_p]),
            "Load": (ptr, [integer, C.c_char_p, integer]), "Save": (integer, [integer, ptr, C.c_char_p, integer]),
            "OpenMemory": (ptr, [ptr, uint]), "CloseMemory": (None, [ptr]),
            "GetFileTypeFromMemory": (integer, [ptr, integer]), "LoadFromMemory": (ptr, [integer, ptr, integer]),
            "SaveToMemory": (integer, [integer, ptr, ptr, integer]), "AcquireMemory": (integer, [ptr, C.POINTER(ptr), C.POINTER(uint)]),
        }
        if os.name == "nt":
            signatures.update({"GetFileTypeU": (integer, [C.c_wchar_p, integer]), "GetFIFFromFilenameU": (integer, [C.c_wchar_p]),
                               "LoadU": (ptr, [integer, C.c_wchar_p, integer]), "SaveU": (integer, [integer, ptr, C.c_wchar_p, integer])})
        for name, (result, args) in signatures.items():
            fn = getattr(lib, "FreeImage_" + name)
            fn.restype, fn.argtypes = result, args
        lib.FreeImage_Initialise(0)
        # Keep the library initialized for process lifetime. Do not shut down
        # other extensions sharing it, and never return views into its memory.
        _library = lib
        return lib


def version():
    return _lib().FreeImage_GetVersion().decode("ascii")


def _path(path):
    return os.fsdecode(path) if os.name == "nt" else os.fsencode(path)


def _file_fn(name):
    return getattr(_lib(), "FreeImage_" + name + ("U" if os.name == "nt" else ""))


def _format(value):
    result = _lib().FreeImage_GetFIFFromFormat(value.upper().encode("ascii")) if isinstance(value, str) else int(value)
    if result < 0:
        raise ValueError("Unknown image format")
    return result


def _bgr(bitmap):
    if np.little_endian:
        return _lib().FreeImage_GetRedMask(bitmap) == 0x00FF0000
    raise NotImplementedError("Bitmap byte order on big-endian hosts is not yet supported")


def _to_array(bitmap):
    lib, converted = _lib(), None
    try:
        kind = lib.FreeImage_GetImageType(bitmap)
        if not lib.FreeImage_HasPixels(bitmap):
            raise ValueError("Header-only images cannot be returned as pixel arrays")
        if lib.FreeImage_GetColorType(bitmap) == 5:
            raise TypeError("CMYK arrays are not supported; load with RGB conversion")
        gray8 = lib.FreeImage_GetBPP(bitmap) == 8 and lib.FreeImage_GetColorType(bitmap) == 1 and not lib.FreeImage_IsTransparent(bitmap)
        if kind == 1 and lib.FreeImage_GetBPP(bitmap) not in (24, 32) and not gray8:
            # Resolve palettes, including indexed transparency, into actual colours.
            converted = (lib.FreeImage_ConvertTo32Bits if lib.FreeImage_IsTransparent(bitmap) else lib.FreeImage_ConvertTo24Bits)(bitmap)
            if not converted:
                raise OSError("Cannot convert indexed bitmap")
            bitmap = converted
        width, height, bpp = lib.FreeImage_GetWidth(bitmap), lib.FreeImage_GetHeight(bitmap), lib.FreeImage_GetBPP(bitmap)
        layouts = {1: (np.uint8, bpp // 8), 2: (np.uint16, 1), 6: (np.float32, 1), 7: (np.float64, 1),
                   9: (np.uint16, 3), 10: (np.uint16, 4), 11: (np.float32, 3), 12: (np.float32, 4)}
        if kind not in layouts:
            raise TypeError(f"Unsupported FreeImage pixel type {kind}")
        dtype, channels = layouts[kind]
        result = np.empty((height, width, channels), dtype=dtype)
        row_bytes = width * channels * result.dtype.itemsize
        for y in range(height):
            address = lib.FreeImage_GetScanLine(bitmap, height - 1 - y)
            result[y] = np.frombuffer(C.string_at(address, row_bytes), dtype=dtype).reshape(width, channels)
        if kind == 1 and channels >= 3 and _bgr(bitmap):
            result[..., :3] = result[..., [2, 1, 0]]
        return result[..., 0].copy() if channels == 1 else result
    finally:
        if converted:
            lib.FreeImage_Unload(converted)


def loadf(path, flags=0):
    """Return (owned array, detected format ID); arrays use top-down RGB order."""
    filename = _path(path)
    kind = _file_fn("GetFileType")(filename, 0)
    if kind < 0:
        kind = _file_fn("GetFIFFromFilename")(filename)
    bitmap = _file_fn("Load")(kind, filename, int(flags))
    if not bitmap:
        raise OSError(f"Cannot load image: {os.fsdecode(path)}")
    try:
        return _to_array(bitmap), kind
    finally:
        _lib().FreeImage_Unload(bitmap)


def load(path, flags=0):
    return loadf(path, flags)[0]


def _from_array(image):
    lib, image = _lib(), np.asarray(image)
    if image.ndim not in (2, 3) or not image.size:
        raise ValueError("Expected a nonempty H×W or H×W×{1,3,4} array")
    channels = 1 if image.ndim == 2 else image.shape[2]
    if channels not in (1, 3, 4) or image.shape[0] > 0x7FFFFFFF or image.shape[1] > 0x7FFFFFFF:
        raise ValueError("Unsupported image shape")
    kinds = {np.dtype("uint8"): {1: 1, 3: 1, 4: 1}, np.dtype("uint16"): {1: 2, 3: 9, 4: 10},
             np.dtype("float32"): {1: 6, 3: 11, 4: 12}, np.dtype("float64"): {1: 7}}
    if image.dtype not in kinds or channels not in kinds[image.dtype]:
        raise TypeError("Use native-endian uint8, uint16, float32, or scalar float64")
    bitmap = lib.FreeImage_AllocateT(kinds[image.dtype][channels], image.shape[1], image.shape[0], image.dtype.itemsize * channels * 8, 0, 0, 0)
    if not bitmap:
        raise MemoryError("FreeImage allocation failed")
    try:
        pixels = image.reshape(image.shape[0], image.shape[1], channels)
        if image.dtype == np.uint8 and channels >= 3 and _bgr(bitmap):
            pixels = pixels[..., [2, 1, 0, 3] if channels == 4 else [2, 1, 0]]
        pixels = np.ascontiguousarray(pixels)
        if image.dtype == np.uint8 and channels == 1:
            palette = np.zeros((256, 4), dtype=np.uint8)
            palette[:, :3] = np.arange(256, dtype=np.uint8)[:, None]
            C.memmove(lib.FreeImage_GetPalette(bitmap), palette.ctypes.data, palette.nbytes)
        for y in range(image.shape[0]):
            C.memmove(lib.FreeImage_GetScanLine(bitmap, image.shape[0] - 1 - y), pixels[y].ctypes.data, pixels[y].nbytes)
        return bitmap
    except BaseException:
        lib.FreeImage_Unload(bitmap)
        raise


def save(path, image, format=None, flags=0):
    filename = _path(path)
    kind = _format(format) if format is not None else _file_fn("GetFIFFromFilename")(filename)
    if kind < 0:
        raise ValueError("Cannot infer image format")
    bitmap = _from_array(image)
    try:
        if not _file_fn("Save")(kind, bitmap, filename, int(flags)):
            raise OSError(f"Cannot save image: {os.fsdecode(path)}")
    finally:
        _lib().FreeImage_Unload(bitmap)


def load_bytes(data, flags=0):
    data = bytes(data)
    if not data or len(data) > 0xFFFFFFFF:
        raise ValueError("Encoded data must contain 1..2**32-1 bytes")
    buffer = C.create_string_buffer(data)
    memory = _lib().FreeImage_OpenMemory(buffer, len(data))
    if not memory:
        raise MemoryError("FreeImage memory stream allocation failed")
    try:
        kind = _lib().FreeImage_GetFileTypeFromMemory(memory, 0)
        bitmap = _lib().FreeImage_LoadFromMemory(kind, memory, int(flags))
        if not bitmap:
            raise OSError("Cannot decode image bytes")
        try:
            return _to_array(bitmap)
        finally:
            _lib().FreeImage_Unload(bitmap)
    finally:
        _lib().FreeImage_CloseMemory(memory)


def save_bytes(image, format, flags=0):
    kind, bitmap = _format(format), _from_array(image)
    memory = _lib().FreeImage_OpenMemory(None, 0)
    try:
        if not memory:
            raise MemoryError("FreeImage memory stream allocation failed")
        if not _lib().FreeImage_SaveToMemory(kind, bitmap, memory, int(flags)):
            raise OSError("Cannot encode image bytes")
        data, size = C.c_void_p(), C.c_uint()
        if not _lib().FreeImage_AcquireMemory(memory, C.byref(data), C.byref(size)):
            raise OSError("Cannot acquire encoded image")
        return C.string_at(data, size.value)
    finally:
        if memory:
            _lib().FreeImage_CloseMemory(memory)
        _lib().FreeImage_Unload(bitmap)
