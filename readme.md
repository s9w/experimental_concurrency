# Experimental concurrency

This repository contains code to test and benchmark a bunch of C++ concurrency features. It's mostly concerned with the latency of different synchronization primitive operations, which is a crucial metric for time-critical applications. This readme contains an analysis and summary of the data obtained from two CPUs.

## Statistical prelude; setup
Performance measurements must be done repeatedly to properly do statistics with them. With concurrency this is especially important as the properties of the operating system's scheduling are out of the control of an application developer and can't be assumed to be well-behaved.

When values are distrubuted [normally](https://en.wikipedia.org/wiki/Normal_distribution), averages and standard deviations offer a sufficient characterization of a value. This is almost never the case for concurrency performance measurements where outliers are quite common. Therefore, all measurements are plotted as a cumulative distributions ([CDF](https://en.wikipedia.org/wiki/Cumulative_distribution_function)).

Histograms also visualize arbitrary distributions well. But with performance numbers it's essential to know how likely certain bad cases can be. That's especially important in concurrency as an unusually slow response time for a single thread will often slow down the entire operation, depending on the design.

This image summarizes how the same data is visualized with histograms and as a CDF:

![cdf_illustration](https://user-images.githubusercontent.com/6044318/148369119-c2839993-01ad-47e5-a950-ecaff75d5542.png)

In that context there are often percentiles reported and plotted. The 99th percentile for example is the time that 99% of all cases take less to run. So only 1% of cases would take longer. The 50th percentile is also known as the median.

The tests were run on Windows 10. The two CPUs featured here are an Intel i7 7700 (released 2017) and and AMD Ryzen 3800X (released 2019, Zen 2 Architecture). Note that this is not a "fair" comparison at all - they're vastly different architectures from different generations. They're simply the CPUs I have at hand here.

## Thread creation
One of the most basic operation is the creation of a thread.

```c++
auto thread_function() -> void
{
   // time_point C
}

// time_point A
std::jthread t{thread_function};
// time_point B
```

Of interest here is
1) how long the creating thread (often the "main" thread) is blocked (the "cost", A to B) and
2) how long it takes for the thread to be ready (the "latency", A to C).

Results:

![thread_start_ryzen](https://user-images.githubusercontent.com/6044318/148402387-28e70eaf-9b2f-4c65-a6d0-172a6ea79ce6.png)

![thread_start_7700](https://user-images.githubusercontent.com/6044318/148346393-11dac0d5-9956-438d-8469-be2f40969b3d.png)

For both architectures it takes quite a while longer for the new thread to be ready than for the creation function to return.

Note that the distribution has a pretty long tail. If we look at the 99.9th percentile (i.e., the worst time out of 1000 runs), that value is about 5 times as high as the median. For the Intel CPU, that is over 300µs - which is a lot in almost all real-time scenarios. The chance of these slow cases to occur naturally increases with the more often you try your luck. I would say the general consensus that thread creation should be avoided where possible is justified.

## Latencies of synchronization primitives
After starting threads, you often want to communicate between them. There are lots of so-called synchronization primitives in the C++ standard library that might be used for that. 

- There's [`std::atomic_flag`](https://en.cppreference.com/w/cpp/atomic/atomic_flag) where the time between calling `atomic_flag.test_and_set(); atomic_flag.notify_one();` in one thread and and `atomic_flag.wait(false)` returning in another thread is important.
- For mutexes, we can look at the time between one thread calling `mutex.unlock()` and another thread acquiring that lock again. We use the modern wrapper [`std::scoped_lock`](https://en.cppreference.com/w/cpp/thread/scoped_lock).
- For a [`std::counting_semaphore`](https://en.cppreference.com/w/cpp/thread/counting_semaphore) we can look at the time between `semaphore.release()` in one thread and `semaphore.acquire()` returning in another thread.
- And to compare, a brute-force [spinlock](https://en.wikipedia.org/wiki/Spinlock) is expected to outperform everything in terms of pure latency but comes with the known drawbacks.


**Results**: Under "lab" conditions with an idle system and the measurements carefully done one after another, the latencies of all the proper primitives (`std::atomic_flag`, `std::semaphore`, `std::mutex`) perform almost identical with mutex slightly slower:

![latency_comparison_primitives_7700](https://user-images.githubusercontent.com/6044318/148345307-95520f86-c7c3-4bbe-bab5-63a5639bb024.png)

If a spinlock is added to the mix however, it vastly outperforms them in terms of latency. The significant drawbacks of spinlocks should be kept in mind.

![latency_comparison_spinlock_7700](https://user-images.githubusercontent.com/6044318/148348235-c336d088-6250-4112-86ba-b2f0968bcd3d.png)

For The Ryzen CPU, latencies are quite a bit quicker. All in one image:

![latency_comparison_all_ryzen](https://user-images.githubusercontent.com/6044318/148403063-7c824b18-ea84-42fd-916a-be9084ecc785.png)

Here are typical values for the Ryzen CPU:

|                        | 50th percentile (median) [µs] | 99th percentile [µs] | 99.9th percentile [µs]
-------------------------|------------------------------:|---------------------:|----------------------:
scoped_lock latency      | 1.00                          | 5.00                 | 15.70
atomic_flag_test latency | 0.80                          | 4.60                 | 16.10
semaphore latency        | 0.80                          | 4.70                 | 16.60
spinlock latency         | 0.20                          | 0.50                 |  6.41


## Contention
It's one thing to measure primitive communication between two threads in an idle system. A better model for real-life behavior is when multiple threads try to access the same resource. This is measured with another test that creates multiple threads which all try to increment a shared integer. The contenders are:

- A `std::mutex` being locked during increment
- A `std::atomic_flag` being used as a signal for other threads to increment via `.notify_one()`
- And as a bonus, a `std::atomic_int` being incremented directly via `.fetch_add(1)`

With such a setup, a clear performance difference can now be observed with atomics now outperforming a mutex much more clearly:

![contention_7700](https://user-images.githubusercontent.com/6044318/148349790-5a70d063-f48e-4436-ad14-cfe7f010ab9a.png)

![contention_ryzen](https://user-images.githubusercontent.com/6044318/148403763-d8827515-640d-4fc4-bf57-2a3f4054e303.png)

Note that the absolute values of this measurements aren't too meaningful. In a scenario like this there are long stretches of a single core incrementing without any intervention from otheres. That explains the much lower latencies compared to the isolated measurements above. The important part is the relation between the compared approaches.

If we dive into the code of `std::mutex::lock()` in MSVC, we end up at `AcquireSRWLockExclusive()` while the atomic `.wait()` function ends up at [`WaitOnAddress()`](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitonaddress). That `WaitOnAddress()` performs better, although it suffers from some restrictions (only available on Windows8+; limited to 1,2,4 or 8 byte values; can return [spuriously](https://en.wikipedia.org/wiki/Spurious_wakeup)). While `std::atomic<T>::wait` is guaranteed to not return spuriously, it does suffer from the [ABA problem](https://en.wikipedia.org/wiki/ABA_problem) unlike other synchronization primitives. These restrictions directly mean that `std::atomic<>` of types bigger than 8 bytes can't use that efficient function. And indeed with larger types the `.wait()` regresses to [`SleepConditionVariableSRW()`](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-sleepconditionvariablesrw).

The atomic `.fetch_add()` performs even better since it uses system intrinsics for that particular operation. That again limits the range of scenarios where it can be used. In particular it's only defined for primitive integer and floating point types.

## Differences between hardware cores
In some of the measurements above, you might have noticed a curious pleateau in the CDF for the Ryzen CPU. That direcly relates to a bimodal distrubution of observed latencies. The reason for this is actually pretty cool. The CPU in question (3800X) has 8 Cores / 16 threads. But it's built from two identical packages ("CCX") of [4 cores / 8 threads each](https://en.wikipedia.org/wiki/Zen_2#Design). There's a fast interconnect but it's not quite as fast as communication within a single CCX.

To measure this effect, we can run another latency test and set the thread affinity explicitly to limit communication between two threads. Doing that for all combinations of the 16 threads results in 256 combinations. We can then visualize the median latencies in a grid:

![heatmap_ryzen](https://user-images.githubusercontent.com/6044318/148236028-d5783fdf-249c-4404-b0b6-c1668a4da0de.png)

The diagonal is removed as communication within a core is not interesting. It's clear that communication between cores within a physical package (core 0-7 and cores 8-15) is faster than between them, proving this hypothesis. The difference measured is about a factor of 2.

There was another interesting phenomenom discovered during development of this test. Communication with a particular core (number 12 on my system) always seemed slower than all others. It turned out that core 12 is the fastest core in my system, usually clocking slightly higher than the rest. It was therefore popular with the scheduler so the main thread (which starts the measurements, progress output etc.) was almost always running on that core, distorting the measurement. Only after explicitly ensuring that the two measured cores aren't also running the main thread did that effect disappear.

## Summary
- Creating a thread is very costly compared to all other thread operations
- Latencies for inter-thread communication via synchronization primitives is in the range of one or a few microseconds
- Latency of a spinlock is about an order of magnitude faster
- All synchroniation primitives exhibited almost identical latency times in uncontended workload
- Under contention, more specialized operations (atomics) performed better
- If low latencies are *very* important, be mindful of your CPU architecture and topology
