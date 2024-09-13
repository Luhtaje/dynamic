# Ring Buffer Library

This project is an engineering thesis project conducted for Metropolia University of Applied Sciences in collaboration with Rightware Oy.

The primary focus of the project is a C++ templated dynamic ring buffer class library. This library implements both First-In-First-Out (FIFO) and Last-In-First-Out (LIFO) capabilities. Importantly, it follows the standard requirements of both a container and sequence container. This design ensures compatibility with the standard container adapters like stack, queue, and priority queue. A unique feature of this buffer is its FIFO and LIFO capabilities along with dynamic memory allocation feature: when full, it automatically allocates more memory instead of overwriting existing elements.

## Project Structure

The project is structured as follows:

- `docs/`: This directory contains documentation generated by Doxygen. You can find both HTML and Latex formats.
  - `html/index.html`
  - `latex/`

- `googletest/`: This directory contains the GoogleTest framework files. Note that GoogleTest is included as a GitHub submodule.

- `include/`: This directory houses all the header(s) for the ring buffer library.

- `test/`: This directory contains the test files for the ring buffer library.

## Building and Testing

The project is configured to create a GoogleTest executable that tests the functionality of the buffer and its iterators. The project can be built using Cmake.

The googletest submodule needs to be updated before building the project. To do this either clone and update directly:
```bash
git clone --recurse-submodules <repository-url>
```
or in the root directory of the project before building
```
git submodule init
git submodule update
```
Then build the project:
```bash
mkdir build
cd build
cmake ..
```
This will create a Visual Studio project. Set the project "RunTests" as startup project and hit run to run the tests.


## Documentation

Doxygen is used to generate project documentation. You can find the output in the `docs/` directory, in both HTML and Latex formats.
