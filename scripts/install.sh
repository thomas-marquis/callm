#!/bin/bash

set -e

pip install torch --index-url https://download.pytorch.org/whl/cpu
pip install -r requirements_dev.txt
