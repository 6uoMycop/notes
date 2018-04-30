## Programming with RDDs
### RDD Basics
An RDD in Spark is simply an immutable distributed collection of objects. Each RDD is split into multiple *partitions*, which may be computed on different nodes of the cluster. RDDs can contain any type of Python, Java, or Scala objects, including user-defined classes.

Users create RDDs in two ways: by loading an external dataset, or by distributing a collection of objects (e.g., a list or set) in their driver program. We have already seen loading a text file as an RDD of strings using `SparkContext.textFile()`, as shown in Example 3-1.

```
Example 3-1. Creating an RDD of strings with textFile() in Python

>>> lines = sc.textFile("README.md")
```
Once created, RDDs offer two types of operations: *transformations* and *actions*. *Transformations* construct a new RDD from a previous one. For example, one common transformation is filtering data that matches a predicate. In our text file example, we can use this to create a new RDD holding just the strings that contain the word *Python*, as shown in Example 3-2.

```
Example 3-2. Calling the filter() transformation

>>> pythonLines = lines.filter(lambda line: "Python" in line)
```
*Actions*, on the other hand, compute a result based on an RDD, and either return it to the driver program or save it to an external storage system (e.g., HDFS). One example of an action we called earlier is `first()`, which returns the first element in an RDD and is demonstrated in Example 3-3.
```
Example 3-3. Calling the first() action

>>> pythonLines.first()
u'## Interactive Python Shell'
```
Transformations and actions are different because of the way Spark computes RDDs. Although you can define new RDDs any time, Spark computes them only in a *lazy* fashion - that is, the first time they are used in an action. This approach might seem unusual at first, but makes a lot of sense when you are working with Big Data. For instance, consider Example 3-2 and Example 3-3, where we defined a text file and then filtered the lines that include *Python*. If Spark were to load and store all the lines in the file as soon as we wrote `lines = sc.textFile(...)`, it would waste a lot of storage space, given that we then immediately filter out many lines. Instead, once Spark sees the whole chain of transformations, it can compute just the data needed for its result. In fact, for the `first()` action, Spark scans the file only until it finds the first matching line; it doesn't even read the whole file.

Finally, Spark's RDDs are by default recomputed each time you run an action on them. If you would like to reuse an RDD in multiple actions, you can ask Spark to persist it using `RDD.persist()`. We can ask Spark to persist our data in a number of different places, which will be covered in Table 3-6. After computing it the first time, Spark will store the RDD contents in memory (partitioned across the machines in your cluster), and reuse them in future actions. Persisting RDDs on disk instead of memory is also possible. The behavior of not persisting by default may again seem unusual, but it makes a lot of sense for big datasets: if you will not reuse the RDD, there's no reason to waste storage space when Spark could instead stream through the data once and just compute the result.

In practice, you will often use `persist()` to load a subset of your data into memory and query it repeatedly. For example, if we knew that we wanted to compute multiple results about the README lines that contain *Python*, we could write the script shown in Example 3-4.
```
Example 3-4. Persisting an RDD in memory

>>> pythonLines.persist

>>> pythonLines.count()
2

>>> pythonLines.first()
u'## Interactive Python Shell'
```
To summarize, every Spark program and shell session will work as follows:
1. Create some input RDDs from external data.
2. Transform them to define new RDDs using transformations like `filter()`.
3. Ask Spark to `persist()` any intermediate RDDs that will need to be reused.
4. Launch actions such as `count()` and `first()` to kick off a parallel computation, which is then optimized and executed by Spark.

### Creating RDDs
Spark provides two ways to create RDDs: loading an external dataset and parallelizing a collection in your driver program.

The simplest way to create RDDs is to take an existing collection in your program and pass it to SparkContext's `parallelize()` method, as shown in Examples 3-5 through 3-7. This approach is very useful when you are learning spark, since you can quickly create your own RDDs in the shell and perform actions on them. Keep in mind, however, that outside of prototyping and testing, this is not widely used since it requires that you have your entire dataset in memory on one machine.
```
Example 3-5. parallelize() method in Python

lines = sc.parallelize(["pandas", "i like pandas"])
```

```
Example 3-6. parallelize() method in Scala

val lines = sc.parallelize(List("pandas", "i like pandas"))
```

A more common way to create RDDs is to load data from external storage. Loading external datasets is covered in detail in Chapter 5. However, we already saw one method that loads a text file as an RDD of strings, `SparkContext.textFile()` , which is shown in Examples 3-8 through 3-10.

```
Example 3-8. textFile() method in Python

lines = sc.textFile("/path/to/README.md")
```

```
Example 3-9. textFile() method in Scala

val lines = sc.textFile("/path/to/README.md")
```

### Common Transformations and Actions
#### Basic RDDs
**Element-wise transformations**

The most two common transformations you will likely be using are `map()` and `filter()`. The `map()` function takes in a function and applies it to each element in the RDD with the result of the function being the new value of each element in the resulting RDD. The `filter()` transformation takes in a function and returns an RDD that only has elements that pass the `filter()` function.

Let's look at a basic example of `map()` that squares all of the numbers in an RDD (Examples 3-26 through 3-28).
```
Example 3-27. Scala squaring the values in an RDD

val input = sc.parallelize(List(1, 2, 3, 4))
val result = input.map(x => x * x)
println(result.collect().mkString(","))
```

**Actions**

The most common action on basic RDDs you will likely use is `reduce()`, which takes a function that operates on two elements of the type in your RDD and returns a new element of the same type. A simple example of such a function is `+`, which we can use to sum our RDD. With `reduce()`, we can easily sum the elements of our RDD, count the number of elements, and perform other types of aggregations (see Examples 3-32 through 3-34).

```
Example 3-33. reduce() in Scala

val sum = rdd.reduce((x, y) => x + y)
```

## Advanced Spark Programming
### Broadcast Variables
As an example, say that we wanted to write a Spark program that looks up countries by their call signs by prefix matching in an array. This is useful for ham radio call signs since each country gets its own prefix, although the prefixes are not uniform in length. If we wrote this natively in Spark, the code might look like Example 6-6.
```Python
# Look up the locations of the call signs on the
# RDD contactCounts. We load a list of call sign
# prefixes to country code to support this lookup.
signPrefixes = loadCallSignTable()

def processSignCount(sign_count, signPrefixes):
    country = lookupCountry(sign_count[0], signPrefixes)
    count = sign_count[1]
    return (country, count)

countryContactCounts = (contactCounts
                        .map(processSignCount)
                        .reduceByKey((lambda x, y: x + y)))
```
This program would run, but if we had a larger table (say, with IP addresses instead of call signs), the `signPrefixes` could easily be several megabytes in size, making it expensive to send that `Array` from the master alongside each task. In addition, if we used the same `signPrefixes` object later (maybe we next we ran the same code on *file2.txt*), it would be sent *again* to each node.

We can fix this by making `signPrefixes` a broadcast variable.

Using broadcast variables, our previous example looks like Example 6-7 through 6-9.
```Python
# Look up the locations of the call signs on the
# RDD contactCounts. We load a list of call sign
# prefixes to country code to support this lookup.
signPrefixes = sc.broadcast(loadCallSignTable())

def processSignCount(sign_count, signPrefixes):
    country = lookupCountry(sign_count[0], signPrefixes.value)
    count = sign_count[1]
    return (country, count)

countryContactCounts = (contactCounts
                        .map(processSignCount)
                        .reduceByKey((lambda x, y: x + y)))

countryContactCounts.saveAsTextFile(outputDir + "/countries.txt")
```

```Scala
# Look up the locations of the call signs on the
# RDD contactCounts. We load a list of call sign
# prefixes to country code to support this lookup.
val signPrefixes = sc.broadcast(loadCallSignTable())
val countryContactCounts = contactCounts.map{case (sign, count) =>
  val country = lookupInArray(sign, signPrefixes.value)
  (country, count)
}.reduceByKey((x, y) => x + y)
countryContactCounts.saveAsTextFile(outputDir + "/countries.txt")
```
As shown in these examples, the process of using broadcast variable is simple:
1. Create a `Broadcast[T]` by calling `SparkContext.broadcast` on an object of type `T`. Any type works as long as it is also `Serializable`.
2. Access its value with the `value` property (or `value()` method in Java).
3. The variable will be sent to each node only once, and should be treated as read-only (updates will *not* be propagated to other nodes).

## Running on a Cluster
### Spark Runtime Architecture
Before we dive into the specifics of running Spark on a cluster, it's helpful to understand the architecture of Spark in a distributed mode (illustrated in Figure 7-1).

In distributed mode, Spark uses a master/slave architecture with one central coordinator and many distributed workers. The central coordinator is called the *driver*. The driver communicates with a potentially large number of distributed workers called *executors*. The driver runs in its own process and each executor is a separate Java process. A driver and its executors are together termed a Spark *application*.

```
                        -------------------
                        |   Spark Driver  |
                        -------------------
                                |
                        -------------------
                        | Cluster Master  |
                        | Mesos, YARN, or |
                        |    Standalone   |
                        -------------------
                                |
          ---------------------------------------------
          |                     |                     |
  ------------------    ------------------    ------------------
  | Cluster Worker |    | Cluster Worker |    | Cluster Worker |
  |----------------|    |----------------|    |----------------|
  |    Executor    |    |    Executor    |    |    Executor    |
  ------------------    ------------------    ------------------
```

A Spark application is launched on a set of machines using an external service called a *cluster manager*. As noted, Spark is packaged with a built-in cluster manager called the Standalone cluster manager. Spark also works with Hadoop YARN and Apache Mesos, two popular open source cluster managers.

#### Summary
To summarize the concepts in this section, let's walk through the exact steps that occur when you run a Spark application on a cluster:

1. The user submits an application using `spark-submit`.
2. `spark-submit` launches the driver program and invokes the `main()` method specified by the user.
3. The driver program contacts the cluster manager to ask for resources to launch executors.
4. The cluster manager launches executors on behalf of the driver program.
5. The driver process runs through the user application. Based on the RDD actions and transformations in the program, the driver sends work to executors in the form of tasks.
6. Tasks are run on executor processes to compute and save results.
7. If the driver's `main()` method exits or it calls `SparkContext.stop()`, it will terminate the executors and release resources from the cluster manager.

### Deploying Applications with spark-submit

Value | Explanation
------------ | -------------
`spark://host:port` | Connect to a Spark Standalone cluster at the specified port. By default Spark Standalone masters use port 7077.
`local[N]` | Run in local mode with N cores.

Apart from a cluster URL, `spark-submit` provides a variety of options that let you control specific details about a particular run of your application. These options fall roughly into two categories. The first is scheduling information, such as the amount of resources you'd like to request for your job (as shown in Example 7-2). The second is information about the runtime dependencies of your application, such as libraries or files you want to deploy to all worker machines.

The general format for spark-submit is shown in Example 7-3.
```
bin/spark-submit [options] <app jar | python file> [app options]
```
`<app jar | python file>` refers to the JAR or Python script containing the entry point into your application.

Flag | Explanation
------------ | -------------
`--class` | The "main" class of your application if you're running a Java or Scala program.

```
# Submitting a Java application to Standalone cluster mode
$ ./bin/spark-submit \
  --master spark://hostname:7077 \
  --deploy-mode cluster \
  --class com.databricks.examples.SparkExample \
  --name "Example Program" \
  --jars dep1.jar,dep2.jar,dep3.jar \
  --total-executor-cores 300 \
  --executor-memory 10g \
  myApp.jar "options" "to your application" "go here"
```
### Cluster Managers
#### Standalone Cluster Manager
**Launching the Standalone cluster manager**

You can start the Standalone cluster manager either by starting a master and workers by hand, or by using launch scripts in Spark's *sbin* directory. The launch scripts are the simplest option to use, but require SSH access between your machines and are currently (as of Spark 1.1) available only on Mac OS X and Linux. We will cover these first, then show how to launch a cluster manually on other platforms.

To use the cluster launch scripts, follow these steps:

1. Copy a compiled version of Spark to the same location on all your machines - for example, */home/yourname/spark*.
2. Set up password-less SSH access from your master machine to the others. This requires having the same user account on all the machines, creating a private SSH key for it on the master via `ssh-keygen`, and adding this key to the *.ssh/authorized_keys* file of all the workers. If you have not set this up before, you can follow these commands:
   ```
   # On master: run ssh-keygen accepting default options
   $ ssh-keygen -t dsa
   Enter file in which to save the key (/home/you/.ssh/id_dsa): [ENTER]
   Enter passphrase (empty for no passphrase): [EMPTY]
   Enter same passphrase again: [EMPTY]
   
   # On workers:
   # copy ~/.ssh/id_dsa.pub from your master to the worker, then use:
   $ cat ~/.ssh/id_dsa.pub >> ~/.ssh/authorized_keys
   $ chmod 644 ~/.ssh/authorized_keys
   ```
3. Edit the *conf/slaves* file on your master and fill in the workers' hostnames.
4. To start the cluster, run `sbin/start-all.sh` on your master (it is important to run it there rather than on a worker). If everything started, you should get no prompts for a password, and the cluster manager's web UI should appear at http://masternode:8080 and show all your workers.
5. To stop the cluster, run `bin/stop-all.sh` on your master node.

If you are not on a UNIX system or would like to launch the cluster manually, you can also start the master and workers by hand, using the `spark-class` script in Spark's *bin/* directory. On your master, type:
```
bin/spark-class org.apache.spark.deploy.master.Master
```
Then on workers:
```
bin/spark-class org.apache.spark.deploy.worker.Worker spark://masternode:7077
```
(where `masternode` is the hostname of your master).

**High availability**

When running in production settings, you will want your Standalone cluster to be available to accept applications even if individual nodes in your cluster go down. Out of the box, the Standalone mode will gracefully support the failure of worker nodes. If you also want the master of the cluster to be high available, Spark supports using Apache ZooKeeper (a distributed coordination system) to keep multiple standby masters and switch to a new one when any of them fails.

## Tunning and Debugging Spark

### Components of Execution: Jobs, Tasks, and Stages

To demonstrate Spark's phases of execution, we'll walk through an example application and see how user code compiles down to a lower-level execution plan. The application we'll consider is a simple bit of log analysis in the Spark shell. For input data, we'll use a text file that consists of log messages of varying degrees of severity, along with some blank lines interspersed.
```
## input.txt ##
INFO This is a message with content
INFO This is some other content
(empty line)
INFO Here are more messages
WARN This is a warning
(empty line)
ERROR Something bad happened
WARN More details on the bad thing
INFO back to normal messages
```
We want to open this file in the Spark shell and compute how many log messages appear at each level of severity. First let's create a few RDDs that will help us answer this question, as shown in Example 8-7.

```
// Read input file
scala> val input = sc.textfile("input.txt")
// Split into words and remove empty lines
scala> val tokenized = input.
     |   map(line => line.split(" ")).
     |   filter(words => words.size > 0)
// Extract the first word from each line (the log level) and do a count
scala> val counts = tokenized.
     |   map(words => words(0), 1)
     | reduceByKey{ (a, b) => a + b }
```
This sequence of commands results in an RDD, `counts`, that will contain the number of log entries at each level of severity. After executing these lines in the shell, the program has not performed any actions. Instead, it has implicitly defined a directed acyclic graph (DAG) of RDD objects that will be used later once an action occurs. Each RDD maintains a pointer to one or more parents along with metadata about which type of relationship they have. For instance, when you call `val b = a.map()` on an RDD, the RDD `b` keeps a reference to its parent `a`. These pointers allow an RDD to be traced to all of its ancestors.

Before we perform an action, these RDDs simply store metadata that will help us compute them later. To trigger computation, let's call an action on the `counts` RDD and `collect()` it to the driver, as shown in Example 8-9.
```
scala> counts.collect()
res86: Array[(String, Int)] = Array((ERROR,1), (INFO,4), (WARN,2))
```
Spark's scheduler creates a physical execution plan to compute the RDDs needed for performing the action. Here when we call `collect()` on the RDD, every partition of RDD must be materialized and then transferred to the driver program. Spark's scheduler starts at the final RDD being computed (in this case, `counts`) and works backward to find what it must compute. It visits that RDD's parents, its parents' parent, and so on, recursively to develop a physical plan necessary to compute all ancestor RDDs. In the simplest case, the scheduler outputs a computation *stage* for each RDD in this graph where the stage has *tasks* for each partition in that RDD. Those stages are then executed in reverse order to compute the final required RDD.

```
                       RDD Graph                      Physical Plan

                                                  ---------------------
                                                  |                   |
                    ---------------               |  ---------------  |
                    |             |               |  |             |  |
             -------| HadoopRDD   |               |  |  HadoopRDD  |  |
             |      |             |               |  |             |  |
             |      ---------------               |  ---------------  |
sc.textFile -|             |                      |         |         |
             |      ---------------               |  ---------------  |
             |      |             |               |  |             |  |
             -------|  MappedRDD  |               |  |  MappedRDD  |  |
                    |             |               |  |             |  |
                    ---------------               |  ---------------  |
                           |                      |         |         |
                    ---------------   ======>     |  ---------------  |   Stage 1
                    |             |               |  |             |  |
  .map(...)         |  MappedRDD  |               |  |  MappedRDD  |  |
                    |             |               |  |             |  |
                    ---------------               |  ---------------  |
                           |                      |         |         |
                    ---------------               |  ---------------  |
                    |             |               |  |             |  |
  .filter(...)      | FilteredRDD |               |  | FilteredRDD |  |
                    |             |               |  |             |  |
                    ---------------               |  ---------------  |
                           |                      |         |         |
                    ---------------               ---------------------
                    |             |               ----------|----------
  .reduceByKey(...) | ShuffuleRDD |               |  ---------------  |
                    |             |               |  |             |  |
                    ---------------               |  | ShuffuleRDD |  |   Stage 2
                                                  |  |             |  |
                                                  |  ---------------  |
                                                  ---------------------
```

> To understand what happens during the shuffle we can consider the example of the reduceByKey operation. The reduceByKey operation generates a new RDD where all values for a single key are combined into a tuple - the key and the result of executing a reduce function against all values associated with that key. The challenge is that not all values for a single key necessarily reside on the same partition, even the same machine, but they must be co-located to compute the result.
>
> In Spark, data is generally not distributed across partitions to be in the necessary place for a specific operation. During computations, a single task will operate on a single partition - thus, to organize all the data for a single reduceByKey reduce task to execute, Spark needs to perform an all-to-all operation. It must read from all partitions to find all the values for all keys, and then bring together values across partitions to compute the final result for each key - this is called the shuffle.

### Finding Information
#### Driver and Executor Logs

By default Spark outputs a healthy amount of logging information. It is also possible to customize the logging behavior to change the logging level or log output to non-standard locations. Spark's logging subsystem is based on log4j,a widely used Java logging library, and uses log4j's configuration format. An example log4j configuration file is bundled with Spark at *conf/log4j.properties.template*. To customize Spark's logging, first copy the example to a file called *log4j.properties*. You can then modify behavior such as the root logging level (the threshold for logging output). By default, it is `INFO`. For less log output, it can be set to `WARN` or `ERROR`.
