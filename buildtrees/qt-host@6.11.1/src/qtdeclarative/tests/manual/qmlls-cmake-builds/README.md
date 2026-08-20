# Testing the automatic qmlls CMake type registration

First of all, setup qmlls in your favorite editor (see https://www.qt.io/blog/whats-new-in-qml-language-server-qmlls-shipped-with-qt-6.6 for instructions).

## Steps

### Scenario 1: automatic update of the linting suggestions

1. Open the manual test using the `CMakeLists.txt` (for QtC, for example) or the folder in which the `CMakeLists.txt` lies (for VS Code, for example) in the editor prepared for qmlls.
2. Make sure the project is configured and built.
3. In your editor, open the `Main.qml` and `helloworld.h`.
4. Modify the `helloworld.h` file by commenting the existing `Q_PROPERTY myPPP` out and save `helloworld.h`.
5. Wait for the Main.qml file to automatically update its diagnostic to show `Could not find property "myPPP"`.
It should also not propose myPPP as autocompletion in `HelloWorld` anymore.
6. Repeat steps 4 + 5 with your own modifications and check that the modification in the `helloworld.h` can be seen in
the `Main.qml` file, without having to rebuild the project yourself.

### Scenario 2: non-blocking completion

1. Open the manual test using the `CMakeLists.txt` (for QtC, for example) or the folder in which the `CMakeLists.txt` lies (for VS Code, for example) in the editor prepared for qmlls.
2. Make sure the project is configured and built.
3. In your editor, open the `Main.qml` and `helloworld.h`.
4. Modify the `helloworld.h` file by commenting the existing `Q_PROPERTY myPPP` out and save `helloworld.h`.
5. Remove `myPPP: 55` from `Main.qml` and replace it with `myP`: it should get autocompleted to `myPPP`, without waiting for the CMake build process to finish.

### Scenario 3: Automatic CMake build on WS opening

1. Open the manual test using the `CMakeLists.txt` (for QtC, for example) or the folder in which the `CMakeLists.txt` lies (for VS Code, for example) in the editor where qmlls is **disabled**.
2. Make sure the project build folder is configured and cleaned.
3. Enable qmlls in your editor, and open `Main.qml`
4. You should see lots of linting warnings about the missing qmltypes files of the AutoGenCMake module.
5. After few seconds, qmlls should have built the project in the background, and retriggered a new linting pass, without any user interaction. The warnings about the missing qmltypes file of the AutoGenCMake module should disappear by themselves. Also make sure that qmlls didn't crash while doing that.
