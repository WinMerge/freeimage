#!/usr/bin/env python3
"""Offline integration tests for release coverage, tables and lookup failures."""
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class CheckerTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.responses = {}
        self.names = set()
        script = self.root / '.github/scripts/check_upstream_versions.sh'
        script.parent.mkdir(parents=True)
        shutil.copy2(ROOT / '.github/scripts/check_upstream_versions.sh', script)
        self.script = script

        def header(path):
            source = ROOT / path
            target = self.root / path
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
            return source.read_text(errors='replace')

        def github(name, repo, version):
            tag = ('lcms' if name == 'lcms2' else 'v') + version
            self.responses[f'https://api.github.com/repos/{repo}/releases/latest'] = {
                'tag_name': tag, 'html_url': f'https://github.com/{repo}/releases/tag/{tag}'}
            self.names.add(name)

        for name, repo, path, macro in (
            ('OpenEXR', 'AcademySoftwareFoundation/openexr', 'Source/OpenEXR/OpenEXRConfig.h', 'OPENEXR_VERSION_STRING'),
            ('Imath', 'AcademySoftwareFoundation/Imath', 'Source/Imath/ImathConfig.h', 'IMATH_VERSION_STRING'),
            ('libdeflate', 'ebiggers/libdeflate', 'Source/LibDeflate/libdeflate.h', 'LIBDEFLATE_VERSION_STRING'),
            ('OpenJPEG', 'uclouvain/openjpeg', 'Source/LibOpenJPEG/opj_config_private.h', 'OPJ_PACKAGE_VERSION'),
            ('libpng', 'pnggroup/libpng', 'Source/LibPNG/png.h', 'PNG_LIBPNG_VER_STRING'),
            ('zlib', 'madler/zlib', 'Source/ZLib/zlib.h', 'ZLIB_VERSION'),
        ):
            version = re.search(r'\b' + macro + r'\s+"([\d.]+)"', header(path))[1]
            github(name, repo, version)
        for name, repo, path, prefix, last in (
            ('LibRaw', 'LibRaw/LibRaw', 'Source/LibRawLite/libraw/libraw_version.h', 'LIBRAW', 'PATCH'),
            ('Zstandard (OpenEXR)', 'facebook/zstd', 'Source/OpenEXR/Zstd/lib/zstd.h', 'ZSTD', 'RELEASE'),
            ('OpenJPH (OpenEXR)', 'aous72/OpenJPH', 'Source/OpenEXR/OpenJPH/openjph/ojph_version.h', 'OPENJPH', 'PATCH'),
        ):
            text = header(path)
            version = '.'.join(re.search((prefix + '_' + part + '_VERSION' if prefix == 'LIBRAW'
                                         else prefix + '_VERSION_' + part) + r'\s+(\d+)', text)[1]
                               for part in ('MAJOR', 'MINOR', last))
            github(name, repo, version)
        version = re.search(r'version\s+([\d.]+)', header('Source/LibWebP/NEWS'))[1]
        github('libwebp', 'webmproject/libwebp', version)
        version = re.search(r'TIFFLIB_VERSION_STR_MAJ_MIN_MIC\s+"([\d.]+)"', header('Source/LibTIFF4/tiffvers.h'))[1]
        self.responses['https://gitlab.com/api/v4/projects/libtiff%2Flibtiff/releases'] = [
            {'tag_name': 'v' + version, '_links': {'self': 'https://gitlab.com/libtiff/libtiff/-/releases/v' + version}}]
        self.names.add('libtiff')
        version = re.search(r'JVERSION\s+"(\d+)', header('Source/LibJPEG/jversion.h'))[1]
        self.responses['https://www.ijg.org/files/'] = f'jpegsrc.v{version}.tar.gz'
        self.names.add('libjpeg (IJG)')
        self.manifest = self.root / 'Source/ThirdParty/sources.json'
        self.manifest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(ROOT / 'Source/ThirdParty/sources.json', self.manifest)
        for item in json.loads(self.manifest.read_text()):
            (self.manifest.parent / item['archive']).touch()
            version = re.search(r'-(\d+(?:\.\d+)+)\.', item['archive'])[1]
            if item['name'] == 'libaom':
                self.responses['https://aomedia.googlesource.com/aom/+refs/tags?format=JSON'] = (
                    ")]}'\n" + json.dumps({'v' + version: {}, 'v99.0.0-rc1': {}, 'unrelated': {}}))
                self.names.add('libaom')
            else:
                repo = '/'.join(item['url'].split('/')[3:5])
                github(item['name'], repo, version)
        bindir = self.root / 'bin'
        bindir.mkdir()
        mock = bindir / 'curl'
        mock.write_text('''#!/usr/bin/env python3
import json, os, sys
responses = json.load(open(os.environ['CURL_FIXTURES']))
url = next(arg for arg in sys.argv[1:] if arg.startswith('https://'))
if url not in responses or responses[url] is None:
    sys.exit(22)
value = responses[url]
print(value if isinstance(value, str) else json.dumps(value))
''')
        mock.chmod(0o755)

    def run_checker(self, *args):
        fixture = self.root / 'responses.json'
        fixture.write_text(json.dumps(self.responses))
        summary = self.root / 'summary.md'
        env = {**os.environ, 'PATH': str(self.root / 'bin') + os.pathsep + os.environ['PATH'],
               'CURL_FIXTURES': str(fixture), 'GITHUB_STEP_SUMMARY': str(summary),
               'GH_TOKEN': '', 'GITHUB_TOKEN': '', 'GITHUB_REPOSITORY': ''}
        result = subprocess.run(['bash', str(self.script), *args], env=env, capture_output=True, text=True)
        report = summary.read_text()
        self.assertIn(report.strip(), result.stdout)
        return result, report

    def test_all_current_are_visible(self):
        result, report = self.run_checker()
        self.assertEqual(result.returncode, 0, result.stderr)
        current = report.split('### Up to date')[1].split('### Not checked')[0]
        self.assertEqual(len(self.names), 20)
        for name in self.names:
            self.assertIn('| ' + name + ' |', current)
        self.assertNotIn('(null)', report)
        self.assertIn('LibJXR:', report)

    def test_archive_updates_and_zip_release_prefix(self):
        for repo, tag in [('strukturag/libheif', 'v99.0.0'), ('google/brotli', 'v99.0.0'),
                          ('mm2/Little-CMS', 'lcms99.0.0')]:
            self.responses[f'https://api.github.com/repos/{repo}/releases/latest']['tag_name'] = tag
        result, report = self.run_checker()
        self.assertEqual(result.returncode, 0, result.stderr)
        updates, current = report.split('### Up to date')
        for name in ('libheif', 'brotli', 'lcms2'):
            self.assertIn('| ' + name + ' |', updates)
            self.assertNotIn('| ' + name + ' |', current)

    def test_failed_lookup_is_not_current_and_fails_the_job(self):
        self.responses['https://api.github.com/repos/google/brotli/releases/latest'] = None
        result, report = self.run_checker()
        self.assertEqual(result.returncode, 1)
        self.assertNotIn('| brotli |', report)
        self.assertIn('- brotli: upstream lookup failed', report)

    def test_tags_fallback_ignores_preview_and_nonversion_tags(self):
        latest = self.responses['https://api.github.com/repos/google/brotli/releases/latest']
        self.responses['https://api.github.com/repos/google/brotli/releases/latest'] = None
        self.responses['https://api.github.com/repos/google/brotli/tags?per_page=100'] = [
            {'name': tag} for tag in ('main', 'v99.0.0-rc1', latest['tag_name'])]
        result, report = self.run_checker()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotIn('99.0.0', report)

    def test_new_manifest_entry_is_automatically_checked(self):
        entries = json.loads(self.manifest.read_text())
        entries.append({'name': 'extra', 'archive': 'extra-1.0.0.zip',
                        'url': 'https://github.com/example/extra/releases/download/v1.0.0/extra-1.0.0.zip'})
        self.manifest.write_text(json.dumps(entries))
        (self.manifest.parent / 'extra-1.0.0.zip').touch()
        self.responses['https://api.github.com/repos/example/extra/releases/latest'] = {
            'tag_name': 'v1.0.1', 'html_url': 'https://github.com/example/extra/releases/tag/v1.0.1'}
        result, report = self.run_checker()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('| extra | 1.0.0 | [v1.0.1]', report)

    def test_missing_archive_is_reported(self):
        item = json.loads(self.manifest.read_text())[0]
        (self.manifest.parent / item['archive']).unlink()
        result, report = self.run_checker()
        self.assertEqual(result.returncode, 1)
        self.assertIn('vendored archive/version not found', report)


if __name__ == '__main__':
    unittest.main()
