// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import examples.Hangman

Item {
    PageHeader {
        id: header
        title: "Hangman Store"
    }

    Column {
        anchors.top: header.bottom
        anchors.bottom: restoreButton.top
        anchors.margins: topLevel.globalMargin
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: topLevel.globalMargin
        // ![2]
        StoreItem {
            product: product100Vowels
            width: parent.width
        }

        StoreItem {
            product: productUnlockVowels
            width: parent.width
        }
        // ![2]
    }

    SimpleButton {
        id: restoreButton
        anchors.bottom: parent.bottom
        anchors.margins: topLevel.globalMargin
        anchors.horizontalCenter: parent.horizontalCenter
        height: topLevel.buttonHeight
        width: parent.width * .5
        text: "Restore Purchases"
        onClicked: {
            console.log("restoring...");
            iapStore.restorePurchases();
        }
    }

    // ![0]
    Product {
        id: product100Vowels
        store: iapStore
        type: Product.Consumable
        identifier: "qt.io.demo.hangman.100vowels"

        onPurchaseSucceeded: {
            console.log(identifier + " purchase successful");
            //Add 100 Vowels
            applicationData.vowelsAvailable += 100;
            transaction.finalize();
            pageStack.pop();
        }

        onPurchaseFailed: {
            console.log(identifier + " purchase failed");
            console.log("reason: "
                        + transaction.failureReason === Transaction.CanceledByUser ? "Canceled" : transaction.errorString);
            transaction.finalize();
        }
    }
    // ![0]
    // ![1]
    Product {
        id: productUnlockVowels
        type: Product.Unlockable
        store: iapStore
        identifier: "qt.io.demo.hangman.unlockvowels"

        onPurchaseSucceeded: {
            console.log(identifier + " purchase successful");
            applicationData.vowelsUnlocked = true;
            transaction.finalize();
            pageStack.pop();
        }

        onPurchaseFailed: {
            console.log(identifier + " purchase failed");
            console.log("reason: "
                        + transaction.failureReason === Transaction.CanceledByUser ? "Canceled" : transaction.errorString);
            transaction.finalize();
        }

        onPurchaseRestored: {
            console.log(identifier + " purchase restored");
            applicationData.vowelsUnlocked = true;
            console.log("timestamp: " + transaction.timestamp);
            transaction.finalize();
            pageStack.pop();
        }
    }
    // ![1]

}
