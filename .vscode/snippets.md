# Useful snippets

Check source of signal

```cpp
    QObject *obj = this->sender();
    if (obj != nullptr)
        qInfo() << "objName ::" << obj->metaObject()->className() << endl
                << "method ::" << obj->metaObject()->method(senderSignalIndex()).methodSignature();
```

To make antimicrox crash when warrning happens use command:

`QT_FATAL_WARNINGS=1 ./build/bin/antimicrox`

(Useful when used with debugger)