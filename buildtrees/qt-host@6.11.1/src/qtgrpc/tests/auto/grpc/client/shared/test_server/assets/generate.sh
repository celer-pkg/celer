#!/bin/bash

openssl req -x509 -newkey rsa:4096 -days 365 -nodes -keyout key.pem -out cert.pem -subj "/C=DE/ST=Berlin/L=Berlin/O=The Qt Company GmbH/OU=RnD/CN=localhost/emailAddress=alexey.edelev@qt.io"
