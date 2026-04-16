# NoteFlow
Desktop application that allows multiple users to collaboratively review and annotate a text/code file in real time collaboratively.

Server URL: wss://noteflow-production-cd83.up.railway.app/

Build instructions — step by step:

1.  Clone the repo
2.  Install QScintilla https://www.riverbankcomputing.com/static/Docs/QScintilla/
3.  Open Qt Creator
4.  Open ui/CMakeLists.txt, configure with Qt 6 MinGW kit
6.  Also on CMakeLists update the library path to QScintilla to the library on your own system
7.  Run NoteFlow.exe or click Run in QtCreator
