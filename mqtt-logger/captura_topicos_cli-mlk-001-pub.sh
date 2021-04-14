#!/bin/sh
python mqtt-data-logger.py -b mqtt.eclipseprojects.io -t cli-mlk-001-pub/temperature -t cli-mlk-001-pub/humidity -t cli-mlk-001-pub/luminosity
