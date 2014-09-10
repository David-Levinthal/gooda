gooda
=====

pmu event analysis package


Gooda is a pmu event data analysis package that consists of some predefined data collection scripts to use perf record in a sensible manner, analyze the data utilizing a cycle accounting methodology and create the tables and dot/svg files needed for the gooda-visualizer package, which is found on gooda-visualizer
The most recent package available from this link now contains the visualizer and analyzer. So only a single invocation of git clone is now needed.

Modern superscalar, out-of-order microprocessors dominate large scale server computing. Monitoring their activity, during program execution, has become complicated due to the complexity of the microarchitectures and their IO interactions. Recent processors have thousands of performance monitoring events. These are required to actually provide coverage for all of the complex interactions and performance issues that can occur. Knowing which data to collect and how to interpret the results has become an unreasonable burden for code developers whose tasks are already hard enough.  It becomes the task of the analysis tool developer to bridge this gap.
To address this issue, a generic decomposition of how a microprocessor is using the consumed cycles allows code developers to quickly understand which of the myriad of microarchitectural complexities they are battling, without requiring a detailed knowledge of the microarchitecture. When this approach is intrinsically integrated into a performance data analysis tool, it enables software developers to take advantage of the microarchitectural methodology that has only been available to experts.
The Generic Optimization Data Analyzer (GOoDA) project integrates this expertise into a profiling tool in order to lower the required expertise of the user and, being designed from the ground up with large-scale object-oriented applications in mind, it will be particularly useful for large codebases.
There is a great deal of useful documentation on performance analysis and SW optimization available in gooda-analyzer/docs.

https://github.com/David-Levinthal/gooda/tree/master/gooda-analyzer/docs
https://github.com/David-Levinthal/gooda/blob/master/gooda-analyzer/docs/CycleAccountingandPerformanceAnalysis.pdf
https://github.com/David-Levinthal/gooda/blob/master/gooda-analyzer/docs/Driving_the_Gooda_visualizer.pdf
https://github.com/David-Levinthal/gooda/blob/master/gooda-analyzer/docs/Micro-architecture.pdf
https://github.com/David-Levinthal/gooda/blob/master/gooda-analyzer/docs/cycle_accounting_and_gooda.pdf

![Cycle Tree]
(https://github.com/David-Levinthal/gooda/blob/master/gooda-visualizer/images/cycleTree.png)

