// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

function restart() {
    for (var i = 0; i < initState.length; i++)
        currentState[i] = initState[i].slice();
    undoStack = [];
}

function isValidPosition() {
    var x = _event.data.x;
    var y = _event.data.y;
    if (x < 0 || x >= initState.length)
        return false;
    if (y < 0 || y >= initState.length)
        return false;
    if (initState[x][y] !== 0)
        return false;
    return true;
}

function calculateCurrentState() {
    if (isValidPosition() === false)
        return;
    var x = _event.data.x;
    var y = _event.data.y;
    var currentValue = currentState[x][y];
    if (currentValue === initState.length)
        currentValue = 0;
    else
        currentValue += 1;
    currentState[x][y] = currentValue;
    undoStack.push([x, y]);
}

function isOK(numbers) {
    var temp = [];
    for (var i = 0; i < numbers.length; i++) {
        var currentValue = numbers[i];
        if (currentValue === 0)
            return false;
        if (temp.indexOf(currentValue) >= 0)
            return false;
        temp.push(currentValue);
    }
    return true;
}

function isSolved() {
    for (var i = 0; i < currentState.length; i++) {
        if (!isOK(currentState[i]))
            return false;

        var column = [];
        var square = [];
        for (var j = 0; j < currentState[i].length; j++) {
            column.push(currentState[j][i]);
            square.push(currentState[Math.floor(i / 3) * 3 + Math.floor(j / 3)]
                                    [i % 3 * 3 + j % 3]);
        }

        if (!isOK(column))
            return false;
        if (!isOK(square))
            return false;
    }
    return true;
}

function undo() {
    if (!undoStack.length)
        return;

    var lastMove = undoStack.pop();
    var x = lastMove[0];
    var y = lastMove[1];
    var currentValue = currentState[x][y];
    if (currentValue === 0)
        currentValue = initState.length;
    else
        currentValue -= 1;
    currentState[x][y] = currentValue;
}
