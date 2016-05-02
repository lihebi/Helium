Slicer Program built on CodeSurfer
=================================

Bulid
-----

```
make
```

Csurf license
-------------

```
license-manager start
```

Test
----

### prepare benchmark

Inside `test/` folder, change the path in `slicing-criteria.txt` to **Absolute** path or **Relative path**.
One path per line. It will do slicing for every line.

`make clean` if necessary.
Then `make`. Then uncomment and change the last line in `myproj.csconf` to:

```
PRESET_BUILD_OPTIONS = highest
```

Then `make` again.

### run Slicer
Go back to this folder, run `make test`. The output slice detail is in `test/result.txt`.
