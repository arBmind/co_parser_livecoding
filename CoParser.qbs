import qbs

QtApplication {
    cpp.cxxLanguageVersion: "c++20"
    consoleApplication: true
    files: [
        "Ast.h",
        "Ast.ostream.h",
        "CoFiber.h",
        "Scope.h",
        "Task.h",
        "Token.h",
        "Token.ostream.h",
        "UniqueCoroutineHandle.h",
        "coroutine.cpp",
        "coroutine.h",
        "main.cpp",
    ]
}
