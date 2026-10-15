import tempfile
import unittest
from pathlib import Path
import numpy as np
import freeimage_numpy as fi

class Arrays(unittest.TestCase):
    def test_rgb_rgba_and_noncontiguous(self):
        for channels in (3, 4):
            source = np.arange(9*17*channels, dtype=np.uint8).reshape(9,17,channels)[::-1,::2]
            encoded = fi.save_bytes(source, 'PNG')
            result = fi.load_bytes(encoded)
            np.testing.assert_array_equal(source, result)
            self.assertTrue(result.flags.owndata)
            result[:] = 0  # remains writable after the FreeImage allocation is freed
    def test_gray_and_hdr(self):
        for dtype in (np.uint8, np.uint16, np.float32, np.float64):
            source = np.arange(35, dtype=dtype).reshape(5,7)
            np.testing.assert_array_equal(source, fi.load_bytes(fi.save_bytes(source, 'TIFF')))
    def test_unicode_path(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'image-α.png'
            source = np.zeros((5,7,3), dtype=np.uint8); source[...,0]=230
            fi.save(path, source)
            result, kind = fi.loadf(path)
            np.testing.assert_array_equal(source, result)
            self.assertGreaterEqual(kind,0)
    def test_errors(self):
        with self.assertRaises(OSError): fi.load_bytes(b'not an image')
        with self.assertRaises(TypeError): fi.save_bytes(np.zeros((4,4),dtype=np.int64),'PNG')
        with self.assertRaises(ValueError): fi.save_bytes(np.zeros((4,4,2),dtype=np.uint8),'PNG')
        with self.assertRaises(ValueError): fi.save_bytes(np.zeros((4,4),dtype=np.uint8),'invalid')
        with self.assertRaises(OSError): fi.load('/nonexistent/freeimage.png')
        encoded=fi.save_bytes(np.zeros((4,4,3),dtype=np.uint8),'PNG')
        with self.assertRaises(ValueError): fi.load_bytes(encoded,0x8000)

if __name__=='__main__': unittest.main()
