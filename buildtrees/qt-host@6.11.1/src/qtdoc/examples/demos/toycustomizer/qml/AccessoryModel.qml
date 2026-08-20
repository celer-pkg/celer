// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

ListModel {
    property int totalSelectedAccessory: 0

    function resetAllAccessories() {
        for (let i = 0; i < count; ++i) {
            const item = get(i)
            const isDefaultEyes = (item.group === "eyes" && item.name === "Small Eyes")
            set(i, { selected: isDefaultEyes, color: "" })
            setAccessoryVisibility(item.key, isDefaultEyes)
        }
        totalSelectedAccessory = 0
    }

    function setAccessoryVisibility(key, vis) {
        if (key === "bracletsVisible") {
            AccessoryState["metalBracelet_RightVisible"] = vis
            AccessoryState["metalBracelet_LeftVisible"] = vis
            return
        }
        AccessoryState[key] = vis
    }

    function updateTotalSelectedAccessory() {
        let total = 0
        for (let i = 0; i < count; ++i) {
            const item = get(i)
            if (item.selected) {
                if (item.group === "adjectives" || item.group === "noun")
                    continue
                if (item.name === "Small Eyes")
                    continue
                ++total
            }
        }
        totalSelectedAccessory = total
    }

    function groups() {
        return ["headwear", "eyewear", "eyes", "items"]
    }

    function colorOf(name: string): color {
        for (let i = 0; i < count; ++i) {
            const data = get(i)
            if (data.group === "noun" || data.group === "adjectives")
                continue
            if (data.name === name)
                return data.color
        }
        return ""
    }

    ListElement {
        group: "headwear"
        name: "Beanie"
        newPrice: 450
        oldPrice: 900
        image: "images/HeadwearImages/BeanieImage1.png"
        selected: false
        color: ""
        key: "beanieVisible"
        modelRating: 3.77
    }

    ListElement {
        group: "headwear"
        name: "Cap"
        newPrice: 275
        oldPrice: 430
        image: "images/HeadwearImages/CapImage1.png"
        selected: false
        color: ""
        key: "capVisible"
        modelRating: 2.69
    }

    ListElement {
        group: "headwear"
        name: "Party Hat"
        newPrice: 375
        oldPrice: 610
        image: "images/HeadwearImages/PartyHatImage1.png"
        selected: false
        color: ""
        key: "partyHatVisible"
        modelRating: 3.31
    }

    ListElement {
        group: "headwear"
        name: "Headphones"
        newPrice: 450
        oldPrice: 840
        image: "images/HeadwearImages/HeadphonesImage1.png"
        selected: false
        color: ""
        key: "headphonesVisible"
        modelRating: 4.56
    }

    ListElement {
        group: "headwear"
        name: "Wizard Hat"
        newPrice: 475
        oldPrice: 890
        image: "images/HeadwearImages/WizardHatImage1.png"
        selected: false
        color: ""
        key: "wizardHatVisible"
        modelRating: 3.92
    }

    ListElement {
        group: "headwear"
        name: "Whiskers"
        newPrice: 325
        oldPrice: 520
        image: "images/HeadwearImages/WhiskersImage1.png"
        selected: false
        color: ""
        key: "whiskersVisible"
        modelRating: 2.87
    }

    ListElement {
        group: "headwear"
        name: "Bandana Hat"
        newPrice: 650
        oldPrice: 1430
        image: "images/HeadwearImages/BandanaHatImage1.png"
        selected: false
        color: ""
        key: "bandanaVisible"
        modelRating: 4.12
    }

    ListElement {
        group: "eyewear"
        name: "EyePatch"
        newPrice: 300
        oldPrice: 660
        image: "images/EyewearImages/EyepatchImage1.png"
        selected: false
        color: ""
        key: "eyePatchVisible"
        modelRating: 3.17
    }

    ListElement {
        group: "eyewear"
        name: "Incognito"
        newPrice: 650
        oldPrice: 1150
        image: "images/EyewearImages/IncognitoImage1.png"
        selected: false
        color: ""
        key: "incognitoVisible"
        modelRating: 1.98
    }

    ListElement {
        group: "eyewear"
        name: "Monacle"
        newPrice: 250
        oldPrice: 685
        image: "images/EyewearImages/MonacleImage1.png"
        selected: false
        color: ""
        key: "monacleVisible"
        modelRating: 2.34
    }

    ListElement {
        group: "eyewear"
        name: "Night Vision Goggles"
        newPrice: 375
        oldPrice: 800
        image: "images/EyewearImages/NightVisionGogglesImage1.png"
        selected: false
        color: ""
        key: "nvGogglesVisible"
        modelRating: 1.58
    }

    ListElement {
        group: "eyewear"
        name: "Sunglasses"
        newPrice: 325
        oldPrice: 590
        image: "images/EyewearImages/SunglassesImage1.png"
        selected: false
        color: ""
        key: "sunglassesVisible"
        modelRating: 5.00
    }

    ListElement {
        group: "eyewear"
        name: "Round Glasses"
        newPrice: 250
        oldPrice: 580
        image: "images/EyewearImages/GlassesImage1.png"
        selected: false
        color: ""
        key: "roundGlassesVisible"
        modelRating: 4.85
    }

    ListElement {
        group: "eyes"
        name: "Small Eyes"
        newPrice: 0
        oldPrice: 0
        image: "images/EyesImages/SmallEyesImage1.png"
        selected: true
        color: ""
        key: "smallEyesVisible"
        modelRating: 3.07
    }

    ListElement {
        group: "eyes"
        name: "Cute Eyes"
        newPrice: 225
        oldPrice: 610
        image: "images/EyesImages/CuteEyesImage1.png"
        selected: false
        color: ""
        key: "cuteEyesVisible"
        modelRating: 4.89
    }

    ListElement {
        group: "eyes"
        name: "Annoyed Eyes"
        newPrice: 200
        oldPrice: 360
        image: "images/EyesImages/AnnoyedEyesImage1.png"
        selected: false
        color: ""
        key: "annoyedEyesVisible"
        modelRating: 2.45
    }

    ListElement {
        group: "eyes"
        name: "Surprised Eyes"
        newPrice: 200
        oldPrice: 330
        image: "images/EyesImages/SurprisedEyesImage1.png"
        selected: false
        color: ""
        key: "surprisedEyesVisible"
        modelRating: 1.73
    }

    ListElement {
        group: "eyes"
        name: "Confused Eyes"
        newPrice: 225
        oldPrice: 390
        image: "images/EyesImages/ConfusedEyesImage1.png"
        selected: false
        color: ""
        key: "confusedEyesVisible"
        modelRating: 4.02
    }

    ListElement {
        group: "eyes"
        name: "Power Puff"
        newPrice: 250
        oldPrice: 530
        image: "images/EyesImages/PowerPuffEyesImage1.png"
        selected: false
        color: ""
        key: "powerpuffEyesVisible"
        modelRating: 4.52
    }

    ListElement {
        group: "eyes"
        name: "Wide Eyes"
        newPrice: 150
        oldPrice: 260
        image: "images/EyesImages/WideEyesImage1.png"
        selected: false
        color: ""
        key: "wideEyesVisible"
        modelRating: 1.92
    }

    ListElement {
        group: "items"
        name: "Butterfly Wings"
        newPrice: 525
        oldPrice: 1250
        image: "images/ItemsImages/ButterflyWingsImage1.png"
        selected: false
        color: ""
        key: "butterflyWingsVisible"
        modelRating: 4.23
    }

    ListElement {
        group: "items"
        name: "Angel Wings"
        newPrice: 550
        oldPrice: 1075
        image: "images/ItemsImages/AngelWingsImage1.png"
        selected: false
        color: ""
        key: "angelWingsVisible"
        modelRating: 4.38
    }

    ListElement {
        group: "items"
        name: "Bowtie"
        newPrice: 400
        oldPrice: 930
        image: "images/ItemsImages/BowtieImage1.png"
        selected: false
        color: ""
        key: "bowtieVisible"
        modelRating: 4.46
    }

    ListElement {
        group: "items"
        name: "Backpack"
        newPrice: 475
        oldPrice: 990
        image: "images/ItemsImages/BackpackImage1.png"
        selected: false
        color: ""
        key: "backpackVisible"
        modelRating: 3.92
    }

    ListElement {
        group: "items"
        name: "Necktie"
        newPrice: 400
        oldPrice: 660
        image: "images/ItemsImages/NecktieImage1.png"
        selected: false
        color: ""
        key: "necktieVisible"
        modelRating: 3.46
    }

    ListElement {
        group: "items"
        name: "Braclets"
        newPrice: 450
        oldPrice: 790
        image: "images/ItemsImages/BracletsImage1.png"
        selected: false
        color: ""
        key: "bracletsVisible"
        modelRating: 2.68
    }

    // Adjectives
    ListElement {
        group: "adjectives"
        name: "Wobbly"
        selected: false
    }

    ListElement {
        group: "adjectives"
        name: "Clumsy"
        selected: false
    }

    ListElement {
        group: "adjectives"
        name: "Bubbly"
        selected: false
    }

    ListElement {
        group: "adjectives"
        name: "Snuggly"
        selected: false
    }

    ListElement {
        group: "adjectives"
        name: "Zippy"
        selected: false
    }

    ListElement {
        group: "adjectives"
        name: "Sparkly"
        selected: false
    }

    ListElement {
        group: "adjectives"
        name: "Grumpy"
        selected: false
    }

    ListElement {
        group: "adjectives"
        name: "Sleepy"
        selected: false
    }

    ListElement {
        group: "adjectives"
        name: "Jolly"
        selected: false
    }

    ListElement {
        group: "adjectives"
        name: "Witty"
        selected: false
    }

    ListElement {
        group: "adjectives"
        name: "Sassy"
        selected: false
    }

    ListElement {
        group: "adjectives"
        name: "Silly"
        selected: false
    }

    ListElement {
        group: "adjectives"
        name: "Spunky"
        selected: false
    }

    // Nouns
    ListElement {
        group: "noun"
        name: "Puff"
        selected: false
    }

    ListElement {
        group: "noun"
        name: "Sprout"
        selected: false
    }

    ListElement {
        group: "noun"
        name: "Greg"
        selected: false
    }

    ListElement {
        group: "noun"
        name: "Nibbles"
        selected: false
    }

    ListElement {
        group: "noun"
        name: "Hops"
        selected: false
    }

    ListElement {
        group: "noun"
        name: "Whiskers"
        selected: false
    }

    ListElement {
        group: "noun"
        name: "Breeze"
        selected: false
    }

    ListElement {
        group: "noun"
        name: "Wolf"
        selected: false
    }

    ListElement {
        group: "noun"
        name: "Cloud"
        selected: false
    }

    ListElement {
        group: "noun"
        name: "Blossom"
        selected: false
    }

    ListElement {
        group: "noun"
        name: "Feather"
        selected: false
    }

    ListElement {
        group: "noun"
        name: "Spark"
        selected: false
    }

    ListElement {
        group: "noun"
        name: "Oak"
        selected: false
    }
}
