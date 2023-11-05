import requests
import pytest
import os

# some constants used frequently in tests
FALSE = '?0'
TRUE = '?1'
UPLOAD_COMPLETE = 'Upload-Complete'
UPLOAD_OFFSET = 'Upload-Offset'
LOCATION = 'Location'
UPLOAD_DRAFT = 'upload-draft-interop-version'


@pytest.fixture
def small_file(request):
    base = os.path.basename(request.config.rootdir)
    if base == "tests":
        return open('test.txt', 'rb')
    return open(os.path.join('tests', 'test.txt'), 'rb')


class TestPost:
    @pytest.fixture
    def post(self, url):
        req = requests.Request('POST', url=url)
        req.headers[UPLOAD_DRAFT] = str(4)
        return req.prepare()

    @pytest.fixture
    def post_with_file(self, url, small_file):
        req = requests.Request('POST', url=url)
        req.headers[UPLOAD_DRAFT] = str(4)
        files = {'file': small_file}
        req.files = files
        return req.prepare()

    @pytest.fixture
    def session(self):
        return requests.Session()

    def test_known_size_upload(self, post_with_file, session):
        post_with_file.headers[UPLOAD_COMPLETE] = TRUE

        r = session.send(post_with_file)
        assert r.status_code == 104, "104 status code was expected"
        assert LOCATION in r.headers, "Location header was expected"
        assert r.headers[LOCATION], "Non-empty Location header value was expected"

    def test_some_other_post(self, url):
        pass


class TestHead:
    def test_simple_head(self):
        pass


class TestPatch:
    def test_simple_patch(self):
        pass
