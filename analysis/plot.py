import matplotlib.pyplot as plt
import numpy as np
import json

j = json.load(open("json_out.txt"))


def get_title(filename):
    lookup_map = {
        "thread_start_latency": "thread start latency",
        "thread_start_cost": "thread start cost",
        "semaphore_latency": "Semaphore latency",
        "raw_mutex_lock_latency": "Mutex latency",
        "condition_variable_latency": "std::condition_variable.wait() latency",
        "atomic_flag_test_latency": "std::atomic_flag test_and_set() latency",
        "atomic_flag_clear_latency": "std::atomic_flag clear() latency",
        "spinlock_latency": "Spinlock latency",
        "contention_atomic_flag": "std::atomic_flag",
        "contention_atomic_add": "std::atomic add",
        "contention_mutex": "std::mutex"
    }
    if filename not in lookup_map:
        return filename
    return lookup_map[filename]


def nuke_axes(ax):
    ax.spines["bottom"].set_visible(False)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.spines["left"].set_visible(False)
    ax.axes.get_yaxis().set_visible(False)


def print_metrics(in_fn, data_us):
    avg = np.average(data_us)
    perc_99 = np.percentile(data_us, 99.0)
    perc_999 = np.percentile(data_us, 99.9)
    print(
        "{:<28} avg {:>8.2f} 99th {:>8.2f} ({:>4.1f}x), 99.9th {:>8.2f} ({:>4.1f}x) med: {:.1f}".format(
            in_fn, avg, perc_99, perc_99/avg, perc_999, perc_999/avg, np.median(data_us)
        )
    )


def get_cdf(sorted_us, max_x_range):
    x = sorted_us
    y = 100.0 * (np.arange(len(sorted_us))+1) / float(len(sorted_us))

    # x values are many times in the data, make things unique
    new_x, indices, counts = np.unique(x, return_index=True, return_counts=True)
    indices = indices+(counts-1) # get the last indices (ie biggest values)
    new_y = y[indices]

    new_x = np.insert(new_x, 0, 0.0)
    new_y = np.insert(new_y, 0, 0.0)
    if max_x_range > new_x[-1]:
        new_x = np.insert(new_x, len(new_x)-1, max_x_range)
        new_y = np.insert(new_y, len(new_y)-1, 100.0)
    return new_x, new_y
    

def stacked_hist(filenames, fn=None, max_x_range=None, xlabel="Time"):
    if type(filenames) is not list:
        filenames = [filenames]
        if fn is None:
            fn = filenames[0]
    else:
        if fn is None:
            raise Exception("test123")
    fig = plt.figure(figsize=(5, 3))
    ax = fig.add_subplot(111)

    sorted_us_datas = [np.sort(j[in_fn][1:])/1000.0 for in_fn in filenames]
    max_x_range = max_x_range or np.max(np.percentile(sorted_us_datas, 99.0, interpolation="higher"))

    for in_fn, data_us in zip(filenames, sorted_us_datas):
        x, y = get_cdf(data_us, max_x_range)
        ax.plot(x, y, label=get_title(in_fn))
        print_metrics(in_fn, data_us)
    
    ax.set_yticks([0.0, 50.0, 90.0, 95.0, 100.0])
    ax.set_yticklabels([str(label)+"%" for label in ax.get_yticks().tolist()])

    ax.set_xlabel("{} [µs]".format(xlabel))
    ax.set_ylabel("Probability of values ≤ x")
    ax.set_xlim(0, max_x_range)
    ax.set_ylim(-2, 105.0)
    ax.spines["top"].set_visible(False)
    ax.spines["bottom"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.grid(axis="y")
    if len(filenames) > 1:
        ax.legend()

    fig.tight_layout()
    out_fn = fn + ".png"
    fig.savefig(out_fn, dpi=100)


def plot_heatmap():
    fig = plt.figure(figsize=(5, 4))
    ax = fig.add_subplot(111)
    core_count = int(np.sqrt(len(j["thread_map"])))
    m = np.zeros(shape=(core_count, core_count))

    for i, ns in enumerate(j["thread_map"]):
        y, x = divmod(i, core_count)
        m[y, x] = ns/1000.0
    m = np.maximum(m, np.transpose(m))
    axes_image = ax.imshow(m, cmap ='Greens')

    ax.imshow(np.identity(core_count), cmap ='cool', alpha=np.identity(core_count))

    ax.set_xticks(range(core_count))
    ax.set_yticks(range(core_count))
    ax.xaxis.set_label_position('top') 

    ax.set_xlabel("Core A")
    ax.set_ylabel("Core B")
    ax.xaxis.tick_top()

    plt.colorbar(axes_image, label="Atomic latency between core A and core B [µs]")
    fig.tight_layout()
    fig.savefig("heatmap.png", dpi=100)


# stacked_hist(["thread_start_cost", "thread_start_latency"], fn="thread_start")

stacked_hist(["spinlock_latency", "semaphore_latency"], fn="latency_comparison")
# stacked_hist("spinlock_latency")
stacked_hist("semaphore_latency")

# stacked_hist(["contention_atomic_add", "contention_atomic_flag", "contention_mutex"], fn="contention")


# stacked_hist("raw_mutex_lock_latency")
# stacked_hist("mutex_lock_unlock_latency_st")
# stacked_hist("scoped_lock_latency")
# stacked_hist("atomic_flag_test_latency")
# stacked_hist("atomic_flag_clear_latency")



# plot_heatmap()

# stacked_hist("minimum_sleep", max_x_range=5500)

# Print the single-value entries
for key, value in j.items():
    if len(value) == 1:
        print(key, value)