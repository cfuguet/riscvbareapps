# RaiderStream for the riscvbarelib runtime

## Compilatiion

```sh
make STREAM_KERNEL=<kernel> \
     STREAM_ARRAY_SIZE=<size> \
     CYCLES_PER_SEC=<freqHz>
```

## Parameters

* STREAM_KERNEL: name of the RaiderStream kernel to run.
  Possible values are indicated in file ../../RSOpts.cpp.
  You could use the value 'all' to run all kernels.

* STREAM_ARRAY_SIZE: Number of elements of the test arrays

* CYCLES_PER_SEC: clock frequency of the platform. Indicated as the number of
  cycles per second.
