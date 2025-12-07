# A simplicity in mind modern simulation engine.

glsim is a high performance simulation engine that has simplicity in mind.

## Building

glsim exposes C++ into python and building is quite straight forward thanks to *scikit-build*

### Release Build

```bash
pip install .
```

### Debug Builds

```bash
cmake --preset debug
cmake --build --preset build-debug
```

## Running Tests

```bash
export PYTHONPATH=$PYTHONPATH:/path/to/glsim # for debug builds
python tests/test_all.py
```
