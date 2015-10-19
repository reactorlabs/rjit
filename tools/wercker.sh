#!/bin/bash

set -e
SLACK_USER=''

case $WERCKER_STARTED_BY in 
    romantsegelskyi)
    SLACK_USER=romantsegelskyi
    ;;
    o--)
    SLACK_USER=o-
    ;;
    peta)
    SLACK_USER=peta
    ;;
    pales)
    SLACK_USER=paley
    ;;
    janvitek)
    SLACK_USER=j.vitek
    ;;
esac


