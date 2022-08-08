# Useful snippets

Check source of signal

```cpp
    QObject *obj = this->sender();
    if (obj != nullptr)
        qInfo() << "objName ::" << obj->metaObject()->className() << endl
                << "method ::" << obj->metaObject()->method(senderSignalIndex()).methodSignature();
```
