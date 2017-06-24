# Mesos
## Understanding Mesos Internals
### The Mesos architecture
#### Communication
Mesos currently uses an HTTP-like wire protocol to communicate with the Mesos components. Mesos uses the `libprocess` library to implement the communication that is located in `3rdparty/libprocess`. The libprocess library provides asynchronous communication between processes. Mesos communication happens along the following APIs:
- Scheduler API: This is used to communicate with the framework scheduler and master.
- Executor API: This is used to communicate with an executor and the Mesos slave.
## Developing Frameworks on Mesos
### The Mesos API
#### The scheduler API
The `org.apache.mesos.Scheduler` interface defines the interface that the framework must implement to
receive callbacks from the Mesos master.
- `registered(SchedulerDriver, FrameworkID, MasterInfo)`: This method is invoked when the scheduler registers with the Mesos master.
- `resourceOffers(SchedulerDriver, List<Offer>)`: This is the meat of most framework schedulers. `resourceOffers()` is invoked when the master offers resources to the framework. Each offer contains resources from a single slave. The scheduler should either accept the offer and launch tasks using
`offerIds` or reject the offers.
- `statusUpdate(SchedulerDriver, TaskStatus)`: This method is called whenever Mesos has a message for the framework. It is called when the status of the task has changed, such as when the task finishes or the task is lost because the slave it was running on is lost, and so on.

#### The SchedulerDriver API
SchedulerDriver interface defines scheduler life cycle methods and methods to interact with Mesos. It is responsible for invoking framework scheduler callbacks.
- `start()`: The `start()` method starts the scheduler and is required to be executed before any other methods can be executed.
- `join()`: The `join()` method waits for the driver to exit and may block indefinitely.
- `run()`: This is a convenient method to start and join the driver.
- `launchTasks(Collection<OfferID> offerIds, Collection<TaskInfo> tasks)`: This launches a set of tasks on a set of offers.

#### The executor API
- `launchTask(ExecutorDriver, TaskInfo)`: This method is the meat of the executor implementation. `launchTask()` is invoked when a task is launched on this executor. Note that launchTask also blocks the call, like all other methods in the executor, and no other callbacks will be invoked on this executor until this callback returns. Thus, if we require any time-consuming computation, we should do it in a separate thread. The task can be realized with a thread or process.

#### The ExecutorDriver API
- `run()`: This method starts the driver and blocks immediately by calling `join()`
- `sendStatusUpdate(TaskStatus status)`: This is used to send the status update message to the scheduler.

We will use it to invoke executor callbacks for communicating with the Mesos slave process.
### Developing a Mesos framework
#### Setting up the development environment
We will use Java API to write our framework. We will call our framework `MonteCarloArea`.
#### Adding the framework scheduler
`org.packt.mesos.MonteCarloScheduler` is our scheduler class. We have to implement the scheduler interface. We will just print the messages without adding any logic to the callbacks for now. We will increasingly add logic to our scheduler and executor to perform the calculation:
```java
public class MonteCarloScheduler implements Scheduler {

    public MonteCarloScheduler(){
    }

    @Override
    public void registered(SchedulerDriver schedulerDriver, Protos.FrameworkID frameworkID, Protos.MasterInfo masterInfo) {
        System.out.println("Scheduler registered with id " + frameworkID.getValue());
    }

    @Override
    public void reregistered(SchedulerDriver schedulerDriver, Protos.MasterInfo masterInfo) {
        System.out.println("Scheduler re-registered");
    }

    @Override
    public void resourceOffers(SchedulerDriver schedulerDriver, List<Protos.Offer> offers) {
        System.out.println("Scheduler received offers " +offers.size());
    }

    public void offerRescinded(SchedulerDriver schedulerDriver, Protos.OfferID offerID) {

    }

    public void statusUpdate(SchedulerDriver schedulerDriver, Protos.TaskStatus taskStatus) {
        System.out.println("Status update: task "+taskStatus.getTaskId().getValue()+" state is "+taskStatus.getState());
    }

    public void frameworkMessage(SchedulerDriver schedulerDriver, Protos.ExecutorID executorID, Protos.SlaveID slaveID, byte[] bytes) {

    }

    public void disconnected(SchedulerDriver schedulerDriver) {

    }

    public void slaveLost(SchedulerDriver schedulerDriver, Protos.SlaveID slaveID) {

    }

    public void executorLost(SchedulerDriver schedulerDriver, Protos.ExecutorID executorID, Protos.SlaveID slaveID, int i) {

    }

    public void error(SchedulerDriver schedulerDriver, String message) {
    }
}
```
`MonteCarloScheduler.resourceOffers()` is the primary method in which the scheduler responds to the offers received from the Mesos master. Currently, the scheduler does nothing useful.

#### Adding the framework launcher
We will call our launcher class `App` that will have only the main method:
```java
public class App {
    public static void main(String[] args) {
        System.out.println("Starting the MonteCarloArea from" + JAR_PATH + "  on Mesos with master " + args[0]);
        Protos.FrameworkInfo frameworkInfo = Protos.FrameworkInfo.newBuilder()
                .setName("MonteCarloArea")
                .setUser("")
                .build();
        MesosSchedulerDriver schedulerDriver = new MesosSchedulerDriver(new MonteCarloScheduler(), frameworkInfo, args[0]);
        schedulerDriver.run();
    }
}
```
The Mesos master address can be specified in any format accepted by Mesos, such as `host:port`, `zk://host1:port1/path/to/mesos/` or `file://path/to/file/containing/master/URI`. Finally, we start and block the driver using the `run()` method.
### Building our framework
Our framework is a very simple framework used to calculate the area under the curve in the given axis-parallel rectangle.
```java
class Term{
    double coefficient;
    double exponent;

    Term(){
        coefficient=exponent=0;
    }

    Term(double coefficient, double exponent){
        this.coefficient=coefficient;
        this.exponent=exponent;
    }

    public static Term fromString(String term){
        double coefficient=1;
        double exponent=0;
        String[] splits=term.split("x",-1);

        if(splits.length>0) {
            String coefficientString=splits[0].trim();
            if(!coefficientString.isEmpty()) {
                coefficient = Double.parseDouble(coefficientString);
            }
        }

        if (splits.length>1) {
            exponent=1;
            String exponentString = splits[1].trim();
            if (!exponentString.isEmpty()) {
                exponent = Double.parseDouble(exponentString);
            }
        }
        return new Term(coefficient, exponent);
    }

    @Override
    public String toString() {
        return coefficient+"x^"+exponent;
    }

    public double evaluate(double x){
        return coefficient*Math.pow(x,exponent);
    }
}

public class Expression {
    List<Term> terms;

    public Expression(){
        terms=new ArrayList<Term>();
    }

    public Expression(List<Term> terms){
        this.terms=terms;
    }

    public boolean addTerm(Term term){
        return terms.add(term);
    }

    public double evaluate(double x){
        double value=0;
        for (Term term : terms) {
            value+=term.evaluate(x);
        }
        return value;
    }

    public static Expression fromString(String s){
        Expression expression=new Expression();
        String[] terms = s.split("\\+");
        for (String term : terms) {
            expression.addTerm(Term.fromString(term));
        }
        return expression;
    }

    @Override
    public String toString() {
        StringBuilder builder=new StringBuilder();
        int i;
        for (i=0; i<terms.size()-1; i++) {
            builder.append(terms.get(i)).append(" + ");
        }
        builder.append(terms.get(i));
        return builder.toString();
    }
}
```
#### Adding an executor to our framework
```java
public class MonteCarloExecutor implements Executor{
    Expression expression;
    double xLow;
    double xHigh;
    double yLow;
    double yHigh;
    int n;

    public MonteCarloExecutor(Expression expression, double xLow, double xHigh, double yLow, double yHigh, int n) {
        this.expression = expression;
        this.xLow = xLow;
        this.xHigh = xHigh;
        this.yLow = yLow;
        this.yHigh = yHigh;
        this.n=n;
    }

    @Override
    public void registered(ExecutorDriver executorDriver, Protos.ExecutorInfo executorInfo, Protos.FrameworkInfo frameworkInfo, Protos.SlaveInfo slaveInfo) {
        System.out.println("Registered an executor on slave " + slaveInfo.getHostname());
}

    public void reregistered(ExecutorDriver executorDriver, Protos.SlaveInfo slaveInfo) {
        System.out.println("Re-Registered an executor on slave " + slaveInfo.getHostname());
    }

    public void disconnected(ExecutorDriver executorDriver) {
        System.out.println("Re-Disconnected the executor on slave");
    }

    public void launchTask(final ExecutorDriver executorDriver, final Protos.TaskInfo taskInfo) {
        System.out.println("Launching task "+taskInfo.getTaskId().getValue());
        Thread thread = new Thread() {
            @Override
            public void run(){
                //Notify the status as running
                Protos.TaskStatus status = Protos.TaskStatus.newBuilder()
                                                   .setTaskId(taskInfo.getTaskId())
                                                   .setState(Protos.TaskState.TASK_RUNNING)
                                                   .build();
                executorDriver.sendStatusUpdate(status);
                System.out.println("Running task "+taskInfo.getTaskId().getValue());
                double pointsUnderCurve=0;
                double totalPoints=0;

                for(double x=xLow;x<=xHigh;x+=(xHigh-xLow)/n){
                    for (double y=yLow;y<=yHigh;y+=(yHigh-yLow)/n) {
                        double value=expression.evaluate(x);
                        if (value >= y) {
                            pointsUnderCurve++;
                        }
                        totalPoints++;
                    }
                }
                double area=(xHigh - xLow)*(yHigh - yLow) * pointsUnderCurve/totalPoints; // Area of Rectangle * fraction of points under curve
                //Notify the status as finish
                status = Protos.TaskStatus.newBuilder()
                                 .setTaskId(taskInfo.getTaskId())
                                 .setState(Protos.TaskState.TASK_FINISHED)
                                 .setData(ByteString.copyFrom(Double.toString(area).getBytes()))
                                 .build();
                executorDriver.sendStatusUpdate(status);
                System.out.println("Finished task "+taskInfo.getTaskId().getValue()+ " with area : "+area);
            }
        };

        thread.start();
    }

    public void killTask(ExecutorDriver executorDriver, Protos.TaskID taskID) {
        System.out.println("Killing task " + taskID);
    }

    @Override
    public void frameworkMessage(ExecutorDriver executorDriver, byte[] bytes) {
    }

    @Override
    public void shutdown(ExecutorDriver executorDriver) {
        System.out.println("Shutting down the executor");
    }

    @Override
    public void error(ExecutorDriver executorDriver, String s) {

    }

    public static void main(String[] args) {
        if(args.length<6){
            System.err.println("Usage: MonteCarloExecutor <Expression> <xLow> <xHigh> <yLow> <yHigh> <Number of Points>");
        }
        MesosExecutorDriver driver = new MesosExecutorDriver(new MonteCarloExecutor(Expression.fromString(args[0]),Double.parseDouble(args[1]), Double.parseDouble(args[2]), Double.parseDouble(args[3]), Double.parseDouble(args[4]), Integer.parseInt(args[5])));
        Protos.Status status = driver.run();
        System.out.println("Driver exited with status "+status);
    }
}
```
Most of the callback methods have only print statements. The AUC calculation happens in `launchTask()`, and we need to add most of the code. `launchTask` should create a separate thread and do the following:
1. Notify Mesos about the status of the task to be run.
2. Run the task.
3. Notify Mesos about the status of the task when finished.

It first sends the status message that the task is running to Mesos. At the end, it updates the status to FINISHED and sends the result.

#### Updating our framework scheduler
We will add a Boolean flag `taskDone` to ensure that we only calculate the area once.

If the task is not done, `resourceOffers()` will accept the first offer, and launch a task with `MonteCarloExecutor`:
```java
    public void resourceOffers(SchedulerDriver schedulerDriver, List<Protos.Offer> offers) {
        if(tasks.size()>0 && !taskDone) {
            Protos.Offer offer = offers.get(0);

            Protos.TaskID taskID = Protos.TaskID.newBuilder().setValue("1").build();
            System.out.println("Launching task " + taskID.getValue()+" on slave "+offer.getSlaveId().getValue()+" with "+task);
            Protos.ExecutorInfo executor = Protos.ExecutorInfo.newBuilder()
                    .setExecutorId(Protos.ExecutorID.newBuilder().setValue("default"))
                    .setCommand(EXECUTOR_CMD+args)
                    .setName("Test Executor (Java)")
                    .build();

            Protos.TaskInfo taskInfo = Protos.TaskInfo.newBuilder()
                    .setName("MonteCarloTask-" + taskID.getValue())
                    .setTaskId(taskID)
                    .setExecutor(Protos.ExecutorInfo.newBuilder(executor))
                    .addResources(Protos.Resource.newBuilder()
                            .setName("cpus")
                            .setType(Protos.Value.Type.SCALAR)
                            .setScalar(Protos.Value.Scalar.newBuilder()
                                    .setValue(1)))
                    .addResources(Protos.Resource.newBuilder()
                            .setName("mem")
                            .setType(Protos.Value.Type.SCALAR)
                            .setScalar(Protos.Value.Scalar.newBuilder()
                                    .setValue(128)))
                    .setSlaveId(offer.getSlaveId())
                    .build();
            schedulerDriver.launchTasks(Collections.singletonList(offer.getId()), Collections.singletonList(taskInfo));
            taskDone=true;
        }
    }
```
Here, the executor instance is built and holds the command to launch `MonteCarloExecutor`. For the sake of simplification, we have set `EXECUTOR_CMD` to `"java -cp MonteCarloArea.jar:mesos-0.20.1-shaded-protobuf.jar -Djava.library.path=./libs org.packt.mesos.MonteCarloExecutor"` but we should read from the external environment.

`Task` holds the resource requirement of the task and specifies which executor to use. It also specifies where to launch the task by setting the `slaveId` that it receives from the offer. Here, we launch a task with one CPU and 128 MBs memory. Finally, it calls `launchTasks()` on the driver.

The `statusUpdate()` method now checks for the `TASK_FINISHED` update, and, on receiving the update, prints the AUC value passed by the executor:
```java
    public void statusUpdate(SchedulerDriver schedulerDriver, Protos.TaskStatus taskStatus) {
        System.out.println("Status update: task "+taskStatus.getTaskId().getValue()+" state is "+taskStatus.getState());
        if (taskStatus.getState().equals(Protos.TaskState.TASK_FINISHED)){
            double area = Double.parseDouble(taskStatus.getData().toStringUtf8());
            System.out.println("Task "+taskStatus.getTaskId().getValue()+" finished with area : "+area);
        } else {
            System.out.println("Task "+taskStatus.getTaskId().getValue()+" has message "+taskStatus.getMessage());
        }
    }
```

## Advanced Topics in Mesos
### The Consistency Model
#### How Is Slave Failure Handled?
#### How Is Master Failure handled?
After the current master crashes, a new leading master is elected. Gradually the slaves will discover this information through the course of their regular communication with the masters. Since the slaves are heartbeating periodically, they'll see their heartbeats fail to connect or rejected. The slaves will query ZooKeeper again to learn of the new master. At this point a slave will use different RPC to connect to the new master. When the new leader receives this RPC, it checks the registry to confirm that this slave is indeed live. If it is, it accepts the slave's registration; however, if the slave not in the registry, then it will tell the slave to kill itself so that it can be revived with new slave ID and rejoin the cluster as new slave without any baggage.

Now, let's consider what happens if we remove the registry. Suppose that a slave stops heartbeating due to a network partition, and the old leader decides that slave is dead, meaning all tasks on it are considered `LOST`. Then, the master fail over to a new leader, and at that time the slave, which was previously not sending heartbeats, miraculously manages to start sending heartbeats again. Now, the new leader will receive the slave's reregistration and accept it. But wait! The old leader thought that slave was dead, and sent the scheduler `LOST` message about all of the tasks on that slave. Now we've gotten into an unfortunate situation where tasks in `LOST` state seem to magically resurrect; this is a big bug because `LOST` is a terminal state for the tasks. The registry allows us to ensure that tasks can never move out of the `LOST` state (or any terminal state) back into a nonterminal state: the old leader marks the slave dead in the registry so that the new leader will know how to reject the reregistration.

If the Mesos master disconnects from ZooKeeper, it will enter into a leaderless state.
- If the Mesos master was a leader, it aborts itself.
- If the Mesos master was a standby, it simply waits for a reconnection to ZooKeeper.