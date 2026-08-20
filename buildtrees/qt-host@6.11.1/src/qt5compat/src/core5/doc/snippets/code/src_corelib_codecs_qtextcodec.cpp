// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:insignificant reason:docs

//! [0]
QByteArray encodedString = "...";
QTextCodec *codec = QTextCodec::codecForName("KOI8-R");
QString string = codec->toUnicode(encodedString);
//! [0]


//! [1]
QString string = "...";
QTextCodec *codec = QTextCodec::codecForName("KOI8-R");
QByteArray encodedString = codec->fromUnicode(string);
//! [1]


//! [2]
QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
QTextDecoder *decoder = codec->makeDecoder();

QString string;
while (new_data_available()) {
    QByteArray chunk = get_new_data();
    string += decoder->toUnicode(chunk);
}
delete decoder;
//! [2]
