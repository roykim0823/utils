### Simple pybind11 example
```
# set python virtual env via uv
uv venv --python 3.12
source .venv/bin/activate
uv pip install numpy  # required for running the test_example.py
uv pip install .
python test_example.py
```

The output will be
```
8
6
5
[2. 4. 6.]
```