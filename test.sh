#!/usr/bin/env bash
set -xeuo pipefail

cmake ./
make
./Testing