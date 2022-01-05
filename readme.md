# Experimental concurrency

This repository contains code to test and benchmark a bunch of C++ concurrency features. It's mostly concerned with the latency of different synchronization primitive operations, which is a crucial metric for time-critical applications.

## Statistical prelude
Performance measurements must be done repeatedly to properly do statistics with them. With concurrency this is especially crucial as the properties of the operating systems scheduling are out of the control of an application developer and can't be assumed to be well-behaved.

When values are distrubuted [normally](https://en.wikipedia.org/wiki/Normal_distribution), averages and standard deviations offer a sufficient characterization of a value. This is almost never the case for concurrency performance measurements where outliers are quite common. Therefore all measurements are plotted as a cumulative distributions ([CDF](https://en.wikipedia.org/wiki/Cumulative_distribution_function)).

Histograms also visualize distributions well. But with performance numbers it's essential to know how likely certain bad cases can be. That's especially important in concurrency as an unusually time for a single thread to respond will often slow down the entire operation, depending on the design.

In that context there are often percentiles reported and plotted. The 99th percentile for example is the time that 99% of all cases take less to run. So only 1% of cases would take longer.

The tests were run on Windows 10. The two CPUs featured here are an Intel i7 7700 (2017) and and AMD Ryzen 3800X (2019, Zen 2 Architecture). Note that this is not a "fair" comparison at all - they're vastly different architectures from different generations. They're simply the CPUs I have at hand here.

## Thread creation
One of the most basic operation is the creation of a thread.

```c++
auto thread_function(){
   // time_point C
}

// time_point A
std::jthread t(thread_function);
// time_point B
```

Interesting here is how long the spawning thread (often the "main" thread) is blocked (the "cost", A to B) and how long it takes for the thread to be ready (the "latency", A to C). These two times are *not* identical.

This is the results for the AMD Ryzen:

![](analysis/thread_start_ryzen.png)

And for the Intel:

![](analysis/thread_start_7700.png)

For both architectures, it takes quite a while longer for the new thread to be ready than for the creation function to return. If those numbers are high depends on the use-case.

Note that the distribution has quite a long tail. If we look at the 99.9th percentile (i.e. the worst time out of 1000 runs), that value is about 5 times as high as the median. For the Intel CPU for example, that is over 300µs - which is a lot in almost all real-time scenarios. The chance of these slow cases to occur naturally increases with the more often you try your luck. I would say the general consensus that thread creation is a bad idea is justified.

## Latencies of synchronization primitives
Besides starting threads, you often want to communicate between them. There are lots of so-called synchronization primitives in the C++ standard library that might be used for that. 

- There's `std::atomic_flag` where the time between calling `atomic_flag.test_and_set(); atomic_flag.notify_one();` in one thread and and `atomic_flag.wait(false)` returning in another thread is important.
- For mutexes, we can look at the time between one thread calling `mutex.unlock()` and another thread acquiring that lock again.
- For a `std::semaphore` we can look at the time between `semaphore.release()` in one thread and `semaphore.acquire()` returning in another thread.
- And to compare, a brute-force [spinlock](https://en.wikipedia.org/wiki/Spinlock) is expected to outperform everything in terms of pure latency, even with the known drawbacks.


**Results**: Under "lab" conditions with an idle system and the measurements carefully done one after another, the latencies of all the proper primitives (`std::atomic_flag`, `std::semaphore`, `std::mutex`) perform **identical**. Namely on the Intel CPU they have a latency of about 4µs, and ~0.8µs on the AMD. A spinlock blows them both out of the water with 1/10th of that latency. 

**AMD**:

![](analysis/latency_comparison_ryzen.png)

**Intel**:

![](analysis/latency_comparison_7700.png)

TODO other latencies

If you're like me you might wonder how come that all these different primitives result in the same latencies. It's a bit more complex then that, see below.

## Contention
It's one thing to measure primitive communication between two threads in an idle system. A better model for real-life behavior is when multiple threads try to access the same resource. This is measured with another test that spawns multiple threads which all try to increment a shared integer. The contenders are:

- A `std::mutex` being locked during increment
- A `std::atomic_flag` being used as a signal for other threads to increment via `.notify_one()`
- And as a bonus, a `std::atomic_int` being incremented directly via `.fetch_add(1)`

With such a setup, a clear performance difference can now be observed:

**Intel**:

![](analysis/contention_7700.png)

If we drive into the code of `std::mutex::lock()` in MSVC, we end up at `AcquireSRWLockExclusive()` while the atomic `.wait()` function ends up at [`WaitOnAddress()`](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitonaddress). That `WaitOnAddress()` performs better, although it suffers from some restrictions (only available on Windows8+; limited to 1,2,4 or 8 byte values; can return spuriously).

The atomic [`.fetch_add()`](https://en.cppreference.com/w/cpp/atomic/atomic/fetch_add) performs even better since it uses system intrinsics for that particular operation. That again limits the range of scenarios where it can be used.

## Differences between hardware cores
In some of the measurements above, you might have noticed a curious pleateau in the CDF for the Ryzen CPU. That direcly relates to a bimodal distrubution of observed latencies. The reason for this is actually pretty cool. The CPU in question (3800X) has 8 Cores / 16 threads. But it's build from two identical packages ("CCX") of [4 cores / 8 threads each](https://en.wikipedia.org/wiki/Zen_2#Design). There's a fast interconnect but it's not quite as fast as communication within a single CCX.

To measure this effect, we can run another latency test and set the thread affinity explicitly to limit communication between two threads. Doing that for all combinations of the 16 threads makes for 256 combinations. We can then visualize the median latencies in a grid:

![](analysis/heatmap_ryzen.png)

The diagonal is removed as communication within a core is not interesting. It's clear that communication between cores within a physical package (core 0-7 and cores 8-15) is faster than between them, proving this hypothesis. The difference measured is about a factor of 2.

There was another interesting phenomenom discovered during development of this test. Communication with a particular core (number 12 on my system) always seemed slower than all others. It turned out that core 12 is the fastest core in my system, usually clocking slightly higher than the rest. It was therefore popular with the scheduler so the main thread (which starts the measurements, progress output etc.) was almost always running on that core, distorting the measurement. Only after explicitly ensuring that the two measured cores aren't also running the main thread did that effect disappear.

## Summary
- Creating a thread is very costly compared to all other thread operations
- Latencies for inter-thread communication via synchronization primitives is in the range of one or a few microseconds
- Latency of a spinlock is about an order of magnitude faster
- All synchroniation primitives exhibited identical latency times in uncontended workload
- Under contention, more specialized operations (atomics) performed better
- If latencies are very important, be mindful of your CPU architecture
