# Profile migration safety tests

This standalone Qt Core/Test suite exercises the production helpers used by
`XMLConfigMigration` and `XMLConfigReader`. It checks complete XML copying,
preservation of parse errors, rejection of invalid converted XML, UTF-8 text,
file replacement failures, retries, and preservation of the original profile.

```sh
cmake -S tests/profilemigration -B build-profile-migration
cmake --build build-profile-migration --config Debug
ctest --test-dir build-profile-migration -C Debug --output-on-failure
```

The text log is `build-profile-migration/profile-migration-results.txt`. CI runs
the suite with Qt 5 and Qt 6 on Linux and Windows. These focused helper tests do
not exercise platform keyboard-code conversion or physical controllers.

Migration now rejects incomplete source XML before conversion and validates the
converted UTF-8 bytes before replacing the file. It uses a temporary file in the
destination directory and reports a failure if that cannot be created or committed.
Direct in-place overwrite is disabled. The previous profile remains in place on
validation or persistence failure; the device is not loaded from an uncommitted conversion.
