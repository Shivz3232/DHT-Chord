# Student

- Name: Shivu Donmardi Gorva

# How to run

- `make build`
- `docker compose -f <docker compose file> up`

- If it is not possible to use make, each application will need to build separately by visiting the respective directory and running: `docker build . -t <application>`
- Added testcase 6 to check if a previously stored value can be successfully retrieved.
- All testcases can be modified in client/client.c LINE 54.

# Configuration Parameters

- All configuration parameters can be found in `config/config.c`
