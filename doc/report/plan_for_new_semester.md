# status of the current framework

C++ Framework:

* DOM Parser, Type resolver Implementation Details missing
* Integrate testing and analyzing


# Todo list for PLDI paper
Must:

* finish C++ framework, debug, and do some initial tests(this week)
* general improvement
  - input and output selection
* run rate improvement
  - connect generation module with type resolve module(for array of complex type)
  - For system type that needs special function to initialize, e.g. p_thread_t, lock
    + the common init function
    + from the project code(difficult)
  - Handle file operation, network operation, job schedule operation, thread operation
    + use faked file, input, and faked thread.
    + This may read the input, so should be integrated into input generation module.
* build rate improvement
  - Dependence Code Sort: header file dependence is not sufficient for many projects
  - wrapper for some srcML bugs(enum, old style function parameter declaration)
    + *implement separate for easy add and remove*
  - address some special syntax appeared often in GNU projects:
    + DECLARE(uch, inbuf,  INBUFSIZ +INBUF_EXTRA);
    + *implement separate for easy add and remove*
  - Conditional compile: implement as a preprocess pass:
    + get all #ifndef tokens
    + recognize frequent ones, e.g. `__sun`, and determine the value(false).
    + for undecided ones,
      - if not many or not important, just randomly choose one.
      - make a separate version of the project(Not Desired)
* backend enhance
  - connect this module with type resolve module
* Change Experiment
  - better script to automatically selection of diff
* slicing
* big code equivalent experiment

Optional:

* further code transformation for structures
* fix build error
  - randomly mutate the programs (or apply the search algorithm)
  - machine learning
* support more language
  - C++

# Applications of the framework paper (future work)
The old ones:

* software redundancy detection and mining
* software change understanding, software evolution
* patch fix verification
* bug detection by invariants

New ones:

* API precondition/postcondition mining: for an API,
we search our corpus for the uses of the API,
build context and run tests.
Synthesize the pre/post-condition for *all* the uses.
* empirical computational complexity:
without our tool, the way to measure the complexity of a segment is,
running some workload for the whole function or program,
then calculate the size of workload and running time to the segment.
We can just apply the workload directly to the segment and its context.
* semantically decomposition of tangled change set:
for one change, increase the context to include another change, to see if invariants are the same.
If is, they are not belong to the same change group.

* API protocol mining:
a function should be called before another.
We can swap these two function, to see if it crashes.
* semantic mutation slicing:
delete a stmt, see if summary or invariants change.
* code clone detection:
use the existing token tree based method to get initial candidate,
then use our tool to analyze properties for better precision.
