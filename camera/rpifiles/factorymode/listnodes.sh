#!/bin/bash
/usr/bin/avahi-browse -rptf _http._tcp | ./listnodes.py --pretty
