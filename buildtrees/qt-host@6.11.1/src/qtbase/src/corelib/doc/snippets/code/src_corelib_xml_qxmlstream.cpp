// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

void examples()
{
    {
        //! [0]
        QXmlStreamReader xml;
        //...
        while (!xml.atEnd()) {
            xml.readNext();
            //... do processing
        }
        if (xml.hasError()) {
            //... do error handling
        }
        //! [0]
    }

    QXmlStreamWriter stream;
    QString qualifiedName, namespaceUri, name, text;

    {
        //! [1]
        stream.writeStartElement(qualifiedName);
        stream.writeCharacters(text);
        stream.writeEndElement();
        //! [1]
    }

    {
        //! [2]
        stream.writeStartElement(namespaceUri, name);
        stream.writeCharacters(text);
        stream.writeEndElement();
        //! [2]
    }
}
