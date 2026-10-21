# Profile file-access regression tests

These 14 cases compile the production `XMLConfigReader` and test missing/deleted
profiles, absent input, directory paths, file selection, repeated reads, error
recovery and release of the previous file object. QtTest also reports two
setup/teardown checks, for 16 passes per run.

```sh
cmake -S tests/profilereader -B build-profile-reader
cmake --build build-profile-reader --config Debug
ctest --test-dir build-profile-reader -C Debug --output-on-failure
```

Qt Core and Test are sufficient. To select a Qt installation, pass
`-DQT_DIR=/path/to/Qt5-or-Qt6/lib/cmake/Qt5-or-Qt6` at configuration time.
The workflow runs Qt 5 and 6 on both Linux and native Windows and prints the
retained text report on Windows.

The reader's historical boolean convention is preserved: `read()` returns true
on failure. GUI callers inspect `hasError()`, so both surfaces must agree.
Failed file access must not invoke device parsing or reload the previous file.
A successful retry must clear the previous error.

Device XML parsing and legacy migration are controlled collaborators in this
suite, linked in place of their SDL-dependent implementations. The reader has
no test-only branches. These tests do not validate mappings, migration,
physical devices or GUI interactions. Additional local checks with the real
reader, SDL virtual joystick and device parser reproduce the missing-file,
retry and stale-selection failures on the original code and pass with the fix.
