# RUFH testing CLI

This is a basic CLI for testing RUFH protocol which was done during IETF 118 Hackathon.

It is mostly a boilerplate code written in `python`, utilizing `pytest` for writing
test cases to check whether server under test complies with RUFH protocol specs.

## Installation

### Retrieve the code from got repository and create python virtual environment
1. `git clone [the repo]`
1. `cd ietf-hackathon`
1. `python -m venv venv`

### Activate the venv and install dependencies
1. `source venv/bin/activate`
1. `pip install -r tests/requirements.txt`

## Using the CLI

### Run the CLI providing server URL of the initial POST
1. `pytest --url http://127.0.0.1:5000/files`

### Running only a batch of tests
It is possible to run only part of tests by using `pytest -k` option, e.g.:
```shell
pytest -k TestPost --url http://127.0.0.1:5000/files
pytest -k TestHead --url http://127.0.0.1:5000/files
```
