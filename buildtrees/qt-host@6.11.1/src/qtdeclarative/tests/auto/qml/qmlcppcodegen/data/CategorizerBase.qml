import QtQml

QtObject {
    enum Parameters {
        Length = 32,
        Iterations = 2,

        Category0 = 0xf0f,
        Category1 = 0xf0f0,
        Category2 = 0xf0f0f,
        Maximum   = 0xf0f0f0,
        Mask      = 0xabcdef
    }

    function randomNumber() : int {
        return (Math.random() * CategorizerBase.Maximum);
    }

    property list<double> numbers: {
        var result = [];
        for (var i = 0; i < CategorizerBase.Length; ++i)
            result[i] = randomNumber();
        return result;
    }
}
