# CaLLM

**!Important!**: Allways apply ALL the principles described in this document.

CaLLM is a LLM runtime: a program that implements all the necessary computations to make a LLM to work.
This project uses pretrained models weights from HuggingFace.

## Core principles

### Inference-only

CaLLM only supports inference.

### CPU-only

The runtime is supposed to work perfectly on a single - but dedicated - CPU. No GPU support.

### For everyone machine

CaLLM must be able to run on any machine. Even on my Grandma's machine, that have only 16GB of RAM and 0 GPU.

## Workflow

### The spec folder

The `specs` directory contains information about the project and is a place to keep track of the work in progress.

Each subdirectory is dedicated to a feature and named according to the pattern: `XXX_<feature_name>`, where `XXX` is
the feature number.

**!Important!**: Don't edit the content located between `<!-- HUMAN-START -->` and `<!-- HUMAN-END -->` (case
unsensitive).

**!Important!**: only focus on the specified feature and don't touch anything else.

Each feature directory MUST be structured this way:

- `specs.md`: the global specification, no technical details. Expected information:
    - A complete rephrasing of the human-written
- `plan.md`: complete technical specification.
- `tasks.md`: the detailed taks list for implementation.

### Workflow's steps

Process one step at a time and only when you get asked to do so.
The steps are:

**1/ Specification**

Update only the `specs.md` file. Ask question

What you need to do:

- Clarify the human-written part, ask question when necessary
- Parse the existing project's structure and documentation to understand it better
- When everything is clear enough,
    - write a more detailed functional specification
    - write a part about the possible risks and limitations
- Don't update the code

**2/ Plan**

Update the `plan.md` file. Use the feature's `specs.md` file as a reference.

What you need to do:

- Architecture
- Technical requirements
- Expected folder structure
- If some of the existing code needed to be updated and how
- The testing strategy
- Technical risks and limitations
- The main implementation steps (but not the details)
- Don't update the code

**3/ Tasks**

Update the `tasks.md` file.

What you need to do:

- List all the tasks that need to be done to implement the plan
- Use the format `- [ ] ` for each task, you can use indentation to group tasks
- Read and analyse the project's code as well as the file `plan.md` as input to create the task list. Don't make up
- If you don't know how to do something, just stop and ask me first
- Don't update the code

**4/ Implementation**

Use the `tasks.md` file to implement the tasks.

What you need to do:

- Implement the listed tasks one by one
- Once a task is finished, mark it as done with `- [x] ` in the file `tasks.md`, then, switch to the next one
- Don't take any initiative, just follow the plan and the task list. If you come across an unplaned issue, just stop and
  ask me first
- When possible, run the tests
- When possible, run the code itself

## Technical principles

### Stack

- Main language: C
- Build system: CMake

### Coding conventions and patterns

**Object-like approach**

A struct can be used to represent a class.
Example:

```c
typedef struct Point Point;

Point *
Point_new(int x, int y);

void
Pont_free(Point *p);

int
Point_distance(Point *p1, Point *p2);
```

**Method name must start with the struct type name**

**Constructor and destructor**

Both methods `X_new` and `X_free` are required.

**Documentation**

Write each function and struct documentation in the header file.

**Private members**

The header file should not contain any private members.
In most of the cases, the header file only declares the type of the struct and the c file implements it.

## Main parts

### Safetensors parser

**Role**: Parse and load matrices from safetensors files.

A Safetensors is a specific file format that contains the model's weights.
Each one is a number encoded in a specific format.
The parser is responsible for extracting the matrices from the safetensors file and converting the number into a format
supported by the runtime.

**Challenges**:

- Most of the CPUs do not support the bfloat16 format by default
- A safetensors file can be large (>10GB)

### Tokenizer

**Role**: According to a token list, the tokenizer implements the algorithm - typically byte-pair encoding - to split
the input text into a list of token ids.

### Linear algebra library

**Role**: Provide the functions and utilities to perform linear algebra operations required by the runtime. This
component is also responsible for the memory management.

Efficiency is a key factor in the performance of the runtime.

**Challenges**:

- The CPU does not support bfloat16
- Memory management is not trivial
- The memory is split into two parts:
    - Static part: the model's weights
    - Dynamic part: the input and all intermediate results
- The linear algebra must be designed with the runtime's constraints in mind

### Runtime implementation

**Role**: Implements all the computations required by the LLM.

The runtime relies on the linear algebra library to perform the linear algebra operations.
It takes the model's weights and the token ids as input and returns the output.

**Challenges**:

- KV cache management
- Efficiency: memory and computation

### Inference engine

**Role**: Orchestrate the inference process by calling the tokenizer and the runtime.

The engine needs to loop over the inference runtime to generate the complete text until either the maximum number of
tokens
token number is reached or the termination token has been generated.

