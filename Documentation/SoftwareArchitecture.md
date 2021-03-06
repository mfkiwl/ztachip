# Software architecture

![ztachip software architecture](images/ztachip_sw_architecture.png)

ztachip software are layered in the following way:

### **pcore programs**

pcore programs run on an array of VLIW processors called pcores.

They implement tensor operators that can be invoked by mcore programs below.

They are vector processors that capable of executing multiple instructions per clock.

pcore programs are files with suffix *.p

[Click here](https://github.com/ztachip/ztachip/blob/master/software/target/apps/nn/kernels/conv.p) for an example of a pcore program implementing convolution operator.

[Click here](https://github.com/ztachip/ztachip/blob/master/Documentation/pcore_programmer_guide.md) for more information on how to program pcore

### **mcore programs**

Program that runs on a MIPS based controller called mcore. 

mcore programs are C programs with special extensions (special extensions begin line with '>') to handle tensor memory operations such as tensor copy,resize,reshape,reordering...

Execution on tensors are invoked by calling tensor operators implemented by pcore programs.

mcore programs are files with suffix *.m

An important concept with mcore programs is that each mcore instructions are associated with a tensor process. There are 2 tensor processes available. There are also seperate PCORE memory space for each process. In a mcore program, there are main thread and child thread and each thread is responsible to dispatch tensor instructions to a seperate tensor process. This concept is important to overlay memory operation from one process with tensor operator execution from the other process. This concept is how ztachip achieves data hiding, for example while PCORE processors are executing a tensor operator using PCORE memory space from process A, tensor engine at the same time can transfer data from DDR to PCORE memory space for process B. And when tensor operator execution is completed for process A and PCORE processors ready to switch to process B, all the data required are already been transfered from DDR to PCORE process B's memory space. Tensor processes are used in all provided examples.

Pictures below illustrates mcore process concept...

MCORE's main thread is emitting tensor instructions for Process A.

MCORE's child thread is emitting tensor instructions for Process B.

Tensor operator execution operates on PCORE memory space for process A, while TensorEngine transfers DDR memory to/from PCORE memory space for Process B.

![mcore process](images/tensor_process0.png)

Now tensor operator execution operates on PCORE memory space for process B, while TensorEngine transfers DDR memory to/from PCORE memory space for Process A. The advantage now is that all memory required to execute tensor operator has already been transfered to PCORE memory space from previous step.

![mcore process](images/tensor_process1.png)


[Click here](https://github.com/ztachip/ztachip/blob/master/software/target/apps/nn/kernels/conv.m) for an example of mcore program implementing convolution operator.

Together mcore and pcore programs form the ztachip tensor programming paradym

[Click here](https://github.com/ztachip/ztachip/blob/master/Documentation/mcore_programmer_guide.md) for more information on how to program mcore.  

### **graph nodes**

These are C++ objects used by host processor to request executions of mcore+pcore programs above.

Executions are scheduled as a graph.

Graph nodes send requests to mcores as messages to a special hardware queue.

[Click here](https://github.com/ztachip/ztachip/blob/master/software/target/apps/resize/resize.cpp) for example of a graph node implementing interface to image_resize acceleration functions.

[Click here](https://github.com/ztachip/ztachip/blob/master/Documentation/app_programmer_guide.md) for more information on how to use graph.

### **User applications**

User applications use ztachip via graph nodes execution above

[Click here](https://github.com/ztachip/ztachip/blob/master/examples/classifier/classifier.cpp) for example of an application performing Mobinet's image classification using graph.

# CONCLUSION

ztachip software paradigm provides the DSA framework to accelerate a number of classes of applications.

The domains that can be mapped to ztachip are domains where its problems can be expressed in terms of tensor operations with the following characteristics.

- Tensor data operations can be expressed by MCORE instructions.

- Tensor computation operations (tensor operators) can be expressed by PCORE instructions.

- Data and computing need to be decoupled. This is key requirement to eliminate the need for cache, solve data latency issue.

Different domains can then be supported by changing the ALU implementation and PCORE instruction set. For example complex data types for SDR domain. Floating point for physics simulation domain. Special data type for GENOMICS.




