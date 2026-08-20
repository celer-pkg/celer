// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include "mainwindow.h"
#include "imageanalyzer.h"

#include <QWebFrame>
#include <QWebElementCollection>
#include <QNetworkDiskCache>

/*
 * Default Constructor
 */
//! [MainWindow - constructor]
MainWin::MainWin(QWidget * parent) : QWebView(parent)
{
    m_network = new QNetworkAccessManager(this);
    m_cache = new QNetworkDiskCache(this);
    m_cache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/imageanalyzer");
    m_cache->setMaximumCacheSize(1000000); //set the cache to 10megs
    m_network->setCache(m_cache);
    page()->setNetworkAccessManager(m_network);

    //! The object we will expose to JavaScript engine:
    m_analyzer = new ImageAnalyzer(m_cache, this);

    // Signal is emitted before frame loads any web content:
    QObject::connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
                     this, SLOT(addJSObject()));

    // qrc:// URLs refer to resources. See imagenalayzer.qrc
    QUrl startURL = QUrl("qrc:/index.html");

    // Load web content now!
    setUrl(startURL);
}
//! [MainWindow - constructor]

//! [MainWindow - addJSObject]
void MainWin::addJSObject() {
    // Add pAnalyzer to JavaScript Frame as member "imageAnalyzer".
    page()->mainFrame()->addToJavaScriptWindowObject(QString("imageAnalyzer"), m_analyzer);
}
//! [MainWindow - addJSObject]
