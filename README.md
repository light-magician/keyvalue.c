# keyvalue.c

A tiny key value store with asynchronous handling in pure C.

### Design

- TCP Server
- GET, PUT, DELETE protocols
- multithreaded request handling
- file I/O on underlying file system

### Container

This code was developed in a linux docker container on an
arm64 macbook pro.

```bash
docker build -t keyvalue
docker run -v $(pwd):/code --name keyvalue.c -it keyvalue
```

### compiling

```bash
make # build
make run # build and run
make clean # remove build files
```

### Testing

in the `src/` directory
compile and link files for a test, then run the output binary

```bash
gcc thread_model_test.c thread_model.c -o thread_model_test.o -lpthread
./thread_model_test.o
```

Currently Active test compile and run commands:

```bash
gcc thread_model_test.c thread_model.c -o thread_model_test.o -lpthread
./thread_model_test.o
```
