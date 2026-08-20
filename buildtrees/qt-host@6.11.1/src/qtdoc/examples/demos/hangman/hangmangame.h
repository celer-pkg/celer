// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef HANGMANGAME_H
#define HANGMANGAME_H

#include <QObject>
#include <QStringList>
#include <QMutex>
#include <QSettings>
#include <QtQml/qqml.h>

#include "purchasing/inapp/inappproduct.h"
#include "purchasing/inapp/inapppurchasebackend.h"
#include "purchasing/inapp/inappstore.h"
#include "purchasing/inapp/inapptransaction.h"
#include "purchasing/qmltypes/inappproductqmltype.h"
#include "purchasing/qmltypes/inappstoreqmltype.h"

class HangmanGame : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString word READ word NOTIFY wordChanged)
    Q_PROPERTY(QString lettersOwned READ lettersOwned NOTIFY lettersOwnedChanged)
    Q_PROPERTY(QString vowels READ vowels CONSTANT)
    Q_PROPERTY(QString consonants READ consonants CONSTANT)
    Q_PROPERTY(int errorCount READ errorCount NOTIFY errorCountChanged)
    Q_PROPERTY(bool vowelsUnlocked READ vowelsUnlocked WRITE setVowelsUnlocked NOTIFY vowelsUnlockedChanged)
    Q_PROPERTY(int vowelsAvailable READ vowelsAvailable WRITE setVowelsAvailable NOTIFY vowelsAvailableChanged)
    Q_PROPERTY(int wordsGiven READ wordsGiven WRITE setWordsGiven NOTIFY wordsGivenChanged)
    Q_PROPERTY(int  wordsGuessedCorrectly READ wordsGuessedCorrectly WRITE setWordsGuessedCorrectly NOTIFY wordsGuessedCorrectlyChanged)
    Q_PROPERTY(int score READ score WRITE setScore NOTIFY scoreChanged)
    QML_NAMED_ELEMENT(Hangman)

public:
    explicit HangmanGame(QObject *parent = 0);
    Q_INVOKABLE void reset();
    Q_INVOKABLE void reveal();
    Q_INVOKABLE void gameOverReveal();
    Q_INVOKABLE void requestLetter(const QString &letterString);
    Q_INVOKABLE void guessWord(const QString &word);
    Q_INVOKABLE bool isVowel(const QString &letter);
    Q_INVOKABLE void setVowelsAvailable(int count);
    Q_INVOKABLE void setWordsGiven(int count);
    Q_INVOKABLE void setWordsGuessedCorrectly(int count);
    Q_INVOKABLE void setScore(int score);

    QString word() const { return m_word; }
    QString lettersOwned() const { return m_lettersOwned; }
    QString vowels() const;
    QString consonants() const;
    int errorCount() const;
    bool vowelsUnlocked() const;
    void setVowelsUnlocked(bool vowelsUnlocked);
    int vowelsAvailable() const;
    int wordsGiven() const;
    int wordsGuessedCorrectly() const;
    int score() const;

signals:
    void wordChanged();
    void lettersOwnedChanged();
    void errorCountChanged();
    void vowelBought(const QChar &vowel);
    void purchaseWasSuccessful(bool wasSuccessful);
    void vowelsUnlockedChanged(bool unlocked);
    void vowelsAvailableChanged(int arg);
    void wordsGivenChanged(int arg);
    void wordsGuessedCorrectlyChanged(int arg);
    void scoreChanged(int arg);

private slots:
    void registerLetterBought(const QChar &letter);

private:
    void chooseRandomWord();
    void initWordList();
    int calculateEarnedVowels();
    int calculateEarnedPoints();

    QString m_word;
    QString m_lettersOwned;
    QStringList m_wordList;
    QRecursiveMutex m_lock;
    bool m_vowelsUnlocked;
    QSettings m_persistentSettings;
    int m_vowelsAvailable;
    int m_wordsGiven;
    int m_wordsGuessedCorrectly;
    int m_score;
};

#endif // HANGMANGAME_H
