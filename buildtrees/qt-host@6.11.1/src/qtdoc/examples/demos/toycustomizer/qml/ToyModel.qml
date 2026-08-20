// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton
import QtQuick

ListModel {
    id: toyModel

    ListElement {
        name: qsTr("Teddy Bear")
        image: "images/Bear.png"
        originalPrice: 900
        discountPercent: 50
        rating: 4.85
        reviews: 451
        description: qsTr("A classic companion with a fluffy hug and gentle eyes, this teddy brings comfort and warmth to every moment.")
    }
    ListElement {
        name: qsTr("Koala")
        image: "images/Koala.png"
        originalPrice: 450
        discountPercent: 0
        rating: 4.75
        reviews: 301
        description: qsTr("This cuddly koala loves cozy naps and gentle hugs, always ready to bring a smile and soft comfort to your day.")
    }
    ListElement {
        name: qsTr("Lion")
        image: "images/Lion.png"
        originalPrice: 900
        discountPercent: 25
        rating: 4.85
        reviews: 315
        description: qsTr("With a golden mane and a brave heart, this lion is always ready for big adventures and warm cuddles.")
    }
    ListElement {
        name: qsTr("Monkey")
        image: "images/Monkey.png"
        originalPrice: 450
        discountPercent: 50
        rating: 4.75
        reviews: 301
        description: qsTr("Playful and full of mischief, this cheeky monkey loves to swing, giggle, and share endless fun with friends.")
    }
    ListElement {
        name: qsTr("Cat")
        image: "images/Cat.png"
        originalPrice: 900
        discountPercent: 50
        rating: 4.85
        reviews: 315
        description: qsTr("Soft and curious, this cuddly cat loves gentle snuggles and quiet moments filled with purrs and charm.")
    }
    ListElement {
        name: qsTr("Reindeer")
        image: "images/Deer.png"
        originalPrice: 450
        discountPercent: 0
        rating: 4.75
        reviews: 212
        description: qsTr("Soft and sweet, this little reindeer brings calm charm and cozy friendship everywhere it goes.")
    }
    ListElement {
        name: qsTr("Panda")
        image: "images/Panda.png"
        originalPrice: 900
        discountPercent: 0
        rating: 4.85
        reviews: 301
        description: qsTr("Gentle and round, this panda brings peaceful hugs and quiet joy wherever it goes.")
    }
    ListElement {
        name: qsTr("Pig")
        image: "images/Pig.png"
        originalPrice: 450
        discountPercent: 0
        rating: 4.75
        reviews: 315
        description: qsTr("A sweet piglet with a round belly, always ready to play and make friends.")
    }
    ListElement {
        name: qsTr("Sloth")
        image: "images/Sloth.png"
        originalPrice: 900
        discountPercent: 50
        rating: 4.85
        reviews: 451
        description: qsTr("A gentle sloth with a sleepy smile, perfect for slow cuddles and quiet moments.")
    }
    ListElement {
        name: qsTr("Rabbit")
        image: "images/Rabbit.png"
        originalPrice: 450
        discountPercent: 50
        rating: 4.75
        reviews: 376
        description: qsTr("With a soft coat and a friendly smile, this bunny is made for fun and friendship.")
    }
    ListElement {
        name: qsTr("Raccoon")
        image: "images/Raccoon.png"
        originalPrice: 450
        discountPercent: 50
        rating: 4.75
        reviews: 315
        description: qsTr("A playful raccoon with a soft mask, perfect for cuddles and little adventures.")
    }
    ListElement {
        name: qsTr("Sheep")
        image: "images/Sheep.png"
        originalPrice: 900
        rating: 4.85
        reviews: 289
        description: qsTr("A fluffy sheep with soft wool, always ready for cozy cuddles and gentle play.")
    }
    ListElement {
        name: qsTr("Tiger")
        image: "images/Tiger.png"
        originalPrice: 450
        rating: 4.75
        reviews: 212
        description: qsTr("A bold tiger with dark stripes, ready for wild games and big adventures.")
    }
    ListElement {
        name: qsTr("Squirrel")
        image: "images/Squirrel.png"
        originalPrice: 900
        rating: 4.85
        reviews: 376
        description: qsTr("A lively squirrel with a bushy tail, always ready to scurry and stash treats for later.")
    }
}
