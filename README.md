# Concurrent Hash Table

A thread-safe hash table in C with reader–writer locks and strict priority-ordered
thread execution.

**Authors:** Johan Johnson, Jordan Khan
**Course:** COP 4600 — Programming Assignment #2

## Design

- **Reader–writer locks** for thread-safe hash table access — concurrent readers,
  exclusive writers.
- **Condition variables + mutex** for priority-based thread ordering: each command
  spawns a thread with an assigned priority, and threads execute in strict priority
  order via a wait/signal mechanism.
- **Jenkins one-at-a-time hash** for computing hash values.
- **Linked-list** buckets.
- **Separate mutex for the log file**, so concurrent writes don't interleave.

## Build and run

```bash
make
```

```bash
./chash
```

The program reads commands from `commands.txt` and produces:

1. Console output showing the result of each operation
2. `hash.log` with detailed timing and lock information
3. The final database state

To clean compiled output:

```bash
make clean
```

## Files

| File | Purpose |
|---|---|
| `chash.c` | Main program — thread management and command parsing |
| `hash_operations.c` | Hash table operations (insert, delete, update, search, print) |
| `hash_utils.c` | Utilities — Jenkins hash, timestamps |
| `chash.h` | Structure definitions and function declarations |
| `Makefile` | Build configuration |
| `commands.txt` | Input commands (editable) |

See [TESTING_GUIDE.txt](TESTING_GUIDE.txt) for test procedures.

## AI usage citation

I used Claude (Anthropic's AI assistant) to help me understand concepts and debug
issues while implementing this assignment.

**What I used AI for:**

- Understanding reader-writer lock implementation and proper usage patterns
- Clarifying how condition variables work for priority-based thread ordering
- Debugging synchronization issues and potential race conditions
- Getting help with the Jenkins one-at-a-time hash function implementation
- Understanding proper timestamp logging format and microsecond precision
- Clarifying the difference between nested functions and proper function scope in C
- Getting advice on how to structure the code into modular files
- Understanding how to properly sort linked list contents for the print command

**Prompts used:**

1. "Can you explain this concurrent hash table assignment to me?"
2. "How do reader-writer locks work and when should I use read vs write locks?"
3. "How can I make threads execute in priority order using condition variables?"
4. "I'm getting a segmentation fault with my nested comparison function, can you help?"
5. "What's the best way to organize this code into multiple files?"
6. "How does the Jenkins hash function work?"

**Approach:**

I read through the assignment requirements carefully and implemented the basic structure
myself. When I encountered concepts I didn't fully understand (like reader-writer locks
and condition variables), I asked Claude to explain them. I then applied those concepts
to my implementation. When I hit bugs or wasn't sure about the best approach, I consulted
Claude for debugging help and best practices.

The core logic and structure of the program are my own work, informed by the explanations
and guidance provided by the AI assistant.
