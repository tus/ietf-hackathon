from urllib.parse import urlparse

import pytest


def pytest_addoption(parser):
    parser.addoption(
        "--url", action="store", default="http://127.0.0.1",
        help="url: initial post url to be tested, e.g. "
             "'https://test.server.org/uploads' or "
             "'http://127.0.0.1:8080/uploads'",
        type=url_type_checker
    )


def url_type_checker(value):
    try:
        urlparse(value)
    except:
        msg = f"passed url {value} was not parsed as valid url!"
        raise pytest.UsageError(msg)
    return value


@pytest.fixture
def url(request):
    return request.config.getoption("--url")
