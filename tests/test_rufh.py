import os
from urllib.parse import urlparse

import pytest
import requests

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


@pytest.fixture
def post(url):
    req = requests.Request('POST', url=url)
    req.headers[UPLOAD_DRAFT] = str(4)
    return req.prepare()


@pytest.fixture
def head(url):
    req = requests.Request('HEAD', url=url)
    req.headers[UPLOAD_DRAFT] = str(4)
    return req.prepare()


@pytest.fixture
def post_with_file(url, small_file):
    req = requests.Request('POST', url=url)
    req.headers[UPLOAD_DRAFT] = str(4)
    files = {'file': small_file}
    req.files = files
    return req.prepare()


@pytest.fixture
def session():
    return requests.Session()


class TestPost:

    def test_known_size_upload(self, post_with_file, session):
        post_with_file.headers[UPLOAD_COMPLETE] = TRUE

        r = session.send(post_with_file)
        assert r.status_code == 104, "104 status code was expected"
        assert LOCATION in r.headers, "Location header was expected"
        assert r.headers[LOCATION], "Non-empty Location header value was expected"

    def test_some_other_post(self, url):
        pass


class TestHead:
    def test_offset_retrieval(self, post_with_file, session, head):
        post_with_file.headers[UPLOAD_COMPLETE] = TRUE

        r = session.send(post_with_file)
        assert r.status_code == 104, "104 status code was expected"
        assert LOCATION in r.headers, "Location header was expected"
        assert r.headers[LOCATION], "Non-empty Location header value was expected"

        location = r.headers[LOCATION]
        # urlparse will throw an exception in case the location url is not correct
        # to pass the test, we don't expect the exception
        urlparse(location)
        head.url = location
        r = session.send(head)
        assert r.status_code == 204, "204 status code was expected"
        assert UPLOAD_OFFSET in r.headers
        assert r.headers[UPLOAD_OFFSET]
        assert int(r.headers[UPLOAD_OFFSET]) > 0
        assert UPLOAD_COMPLETE in r.headers
        assert r.headers[UPLOAD_COMPLETE] == TRUE

    def test_offset_retrieval_bad_head(self, post_with_file, session, head):
        post_with_file.headers[UPLOAD_COMPLETE] = TRUE

        r = session.send(post_with_file)
        assert r.status_code == 104, "104 status code was expected"
        assert LOCATION in r.headers, "Location header was expected"
        assert r.headers[LOCATION], "Non-empty Location header value was expected"

        location = r.headers[LOCATION]
        # urlparse will throw an exception in case the location url is not correct
        # to pass the test, we don't expect the exception
        urlparse(location)
        head.url = location
        # The request MUST NOT include an Upload-Offset
        head.headers[UPLOAD_OFFSET] = str(10)
        r = session.send(head)
        # The request MUST NOT include an Upload-Offset or Upload-Complete header field.
        # The server MUST reject requests with either of these fields by responding with
        # a 400 (Bad Request) status code.
        assert r.status_code == 400, "400 status code was expected"

    def test_offset_retrieval_bad_head2(self, post_with_file, session, head):
        post_with_file.headers[UPLOAD_COMPLETE] = TRUE

        r = session.send(post_with_file)
        assert r.status_code == 104, "104 status code was expected"
        assert LOCATION in r.headers, "Location header was expected"
        assert r.headers[LOCATION], "Non-empty Location header value was expected"

        location = r.headers[LOCATION]
        # urlparse will throw an exception in case the location url is not correct
        # to pass the test, we don't expect the exception
        urlparse(location)
        head.url = location
        # The request MUST NOT include an Upload-Complete
        head.headers[UPLOAD_COMPLETE] = FALSE
        r = session.send(head)
        # The request MUST NOT include an Upload-Offset or Upload-Complete header field.
        # The server MUST reject requests with either of these fields by responding with
        # a 400 (Bad Request) status code.
        assert r.status_code == 400, "400 status code was expected"

    def test_simple_head(self):
        pass


class TestPatch:
    def test_simple_patch(self):
        pass
