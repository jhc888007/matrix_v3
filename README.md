Python:
    python setup.py install
    python test_writer.py output.idx output.data 10000 < test_input.txt
    python test_reader.py output.idx output.data < test_input.txt

C++:
    g++ matrixwriter.cpp -o matrix_builder
    ./matrix_builder output.idx output.data < test_input.txt

