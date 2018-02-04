# Java Programming Tutorial
## Multithreading & Concurrent Programming

### The Infamous "Unresponsive User Interface"

The infamous *Unresponsive User Interface (UI) problem* is best illustrated by the following Swing program with a counting-loop.

The GUI program has two buttons. Pushing the "Start Counting" button starts the counting. Pushing the "Stop Counting" button is *supposed* to stop (pause) the counting. The two button-handlers communicate via a `boolean` flag called `stop`. The stop-button handler sets the `stop` flag; while the start-button handler checks if `stop` flag has been set before continuing the next count.

You should write the program under Eclipse/NetBeans so that we could trace the threads.

#### Example 1: Unresponsive UI

```java
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
 
/** Illustrate Unresponsive UI problem caused by "busy" Event-Dispatching Thread */
public class UnresponsiveUI extends JFrame {
   private boolean stop = false;  // start or stop the counter
   private JTextField tfCount;
   private int count = 1;
 
   /** Constructor to setup the GUI components */
   public UnresponsiveUI() {
      Container cp = this.getContentPane();
      cp.setLayout(new FlowLayout(FlowLayout.CENTER, 10, 10));
      cp.add(new JLabel("Counter"));
      tfCount = new JTextField(count + "", 10);
      tfCount.setEditable(false);
      cp.add(tfCount);
 
      JButton btnStart = new JButton("Start Counting");
      cp.add(btnStart);
      btnStart.addActionListener(new ActionListener() {
         @Override
         public void actionPerformed(ActionEvent evt) {
            stop = false;
            for (int i = 0; i < 100000; ++i) {
               if (stop) break;  // check if STOP button has been pushed,
                                 //  which changes the stop flag to true
               tfCount.setText(count + "");
               ++count;
            }
         }
      });
      JButton btnStop = new JButton("Stop Counting");
      cp.add(btnStop);
      btnStop.addActionListener(new ActionListener() {
         @Override
         public void actionPerformed(ActionEvent evt) {
            stop = true;  // set the stop flag
         }
      });
 
      setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      setTitle("Counter");
      setSize(300, 120);
      setVisible(true);
   }
 
   /** The entry main method */
   public static void main(String[] args) {
      // Run GUI codes in Event-Dispatching thread for thread safety
      SwingUtilities.invokeLater(new Runnable() {
         public void run() {
            new UnresponsiveUI();  // Let the constructor do the job
         }
      });
   }
}
```

However, once the START button is pushed, the UI is *frozen* – the counter value is not updated on the display (i.e., the display is not *refreshed*), and the user interface is not responding to the clicking of the STOP button, or any other user interaction.

##### Tracing the threads (Advanced)
From the program trace (via Eclipse/NetBeans), we observe:

1. The `main()` method is started in the "main" thread.
2. The JRE's windowing subsystem, via `SwingUtilities.invokeLater()`, starts 3 threads: "AWT-Windows" (daemon thread), "AWT-Shutdown" and "AWT-EventQueue-0". The "AWT-EventQueue-0" is known as the *Event-Dispatching Thread (EDT)*, which is the one and only thread responsible for handling all the events (such as clicking of buttons) and refreshing the display to ensure thread safety in GUI operations and manipulating GUI components. The constructor `UnresponsiveUI()` is scheduled to run on the Event-Dispatching thread (via `invokeLater()`), after all the existing events have been processed. The "main" thread exits after the `main()` method completes. A new thread called "DestroyJavaVM" is created.
3. When you click the START button, the `actionPerformed()` is run on the EDT. The EDT is now fully-occupied with the compute-intensive counting-loop. In other words, while the counting is taking place, the EDT is *busy* and unable to process any event (e.g., clicking the STOP button or the window-close button) and refresh the display - until the counting completes and EDT becomes available. As the result, the display freezes until the counting-loop completes.

It is recommended to run the GUI construction codes on the EDT via the `invokeLater()`. This is because many of the GUI components are not guaranteed to be thread-safe. Channeling all accesses to GUI components in a single thread ensure thread safety. Suppose that we run the constructor directly on the `main()` method (under the "main" thread), as follow:

```java
public static void main(String[] args) {
   new UnresponsiveUI();
}
```

The trace shows that:

1. The `main()` method starts in the "main" thread.
2. A new thread "AWT-Windows" (Daemon thread) is started when we step-into the constructor "`new UnresponsiveUI()`" (because of the "`extends JFrame`").
3. After executing "`setVisible(true)`", another two threads are created - "AWT-Shutdown" and "AWT-EventQueue-0" (i.e., the EDT).
4. The "main" thread exits after the `main()` method completes. A new thread called "DestroyJavaVM" is created.
5. At this point, there are 4 threads running - "AWT-Windows", "AWT-Shutdown" and "AWT-EventQueue-0 (EDT)" and "DestroyJavaVM".
6. Clicking the START button invokes the `actionPerformed()` in the EDT.

In the earlier case, the EDT is started via the `invokeLater()`; while in the later case, the EDT starts after `setVisible()`.

#### Example 2: Still Unresponsive UI with Thread

Instead of using the *event-dispatching thread* to do the compute-intensive counting, let's create a new thread to do the counting, instead of using the EDT, as follows:

```java
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/** Illustrate the Unresponsive UI problem caused by "starved" event-dispatching thread */
public class UnresponsiveUIwThread extends JFrame {
   private boolean stop = false;
   private JTextField tfCount;
   private int count = 1;
 
   /** Constructor to setup the GUI components */
   public UnresponsiveUIwThread() {
      Container cp = getContentPane();
      cp.setLayout(new FlowLayout(FlowLayout.CENTER, 10, 10));
      cp.add(new JLabel("Counter"));
      tfCount = new JTextField(count + "", 10);
      tfCount.setEditable(false);
      cp.add(tfCount);
 
      JButton btnStart = new JButton("Start Counting");
      cp.add(btnStart);
      btnStart.addActionListener(new ActionListener() {
         @Override
         public void actionPerformed(ActionEvent evt) {
            stop = false;
            // Create our own Thread to do the counting
            Thread t = new Thread() {
               @Override
               public void run() {  // override the run() to specify the running behavior
                  for (int i = 0; i < 100000; ++i) {
                     if (stop) break;
                     tfCount.setText(count + "");
                     ++count;
                  }
               }
            };
            t.start();  // call back run()
         }
      });
 
      JButton btnStop = new JButton("Stop Counting");
      cp.add(btnStop);
      btnStop.addActionListener(new ActionListener() {
         @Override
         public void actionPerformed(ActionEvent evt) {
            stop = true;
         }
      });
 
      setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      setTitle("Counter");
      setSize(300, 120);
      setVisible(true);
   }
 
   /** The entry main method */
   public static void main(String[] args) {
      // Run GUI codes in Event-Dispatching thread for thread safety
      javax.swing.SwingUtilities.invokeLater(new Runnable() {
         @Override
         public void run() {
            new UnresponsiveUIwThread();  // Let the constructor do the job
         }
      });
   }
}
```
A new thread is created by sub-classing the `Thread` class, with an anonymous inner class. We override the `run()` method to specify the running behavior of the thread, which performs the compute-intensive counting. An instance is created. Invoking the `start()` method of the instance causes the `run()` to execute on its own thread. (The details on creating new thread will be explained later.)

The responsiveness improves slightly. But the proper counter value is still not shown, and there is a delay in response to the "STOP" button. (You may not see the difference running with a dual-core processor.)

This is because the counting thread does not voluntarily yield control to the EDT. The "starved" EDT is unable to update the display and response to the "STOP" button. Nonetheless, the JVM may force the counting thread to yield control according to the scheduling algorithm, which results in delay on updating the display (TODO: not sure about this).

##### Tracing the Threads (Advanced)
When the "START" button is clicked, a new thread called "Thread-*n*" (n is a running number) is created to run the compute-intensive counting-loop. However, this thread is not programmed to yield control to other threads, in particular, the event-dispatching thread.

This program is, however, slightly better than the previous program. The display is updated, and the clicking of "STOP" button has its effect after some delays.

#### Example 3: Responsive UI with Thread

Let's modify the program by making a call to the counting-thread's `sleep()` method, which requests the counting-thread to yield control to the event-dispatching thread to update the display and response to the "STOP" button. The counting program now works as desired. The `sleep()` method also provides the necessary delay needed.
```java
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
 
/** Resolve the unresponsive UI problem by running the compute-intensive task
    in this own thread, which yields control to the EDT regularly */
public class UnresponsiveUIwThreadSleep extends JFrame {
   private boolean stop = false;
   private JTextField tfCount;
   private int count = 1;
 
   /** Constructor to setup the GUI components */
   public UnresponsiveUIwThreadSleep() {
      Container cp = getContentPane();
      cp.setLayout(new FlowLayout(FlowLayout.CENTER, 10, 10));
      cp.add(new JLabel("Counter"));
      tfCount = new JTextField(count + "", 10);
      tfCount.setEditable(false);
      cp.add(tfCount);
 
      JButton btnStart = new JButton("Start Counting");
      cp.add(btnStart);
      btnStart.addActionListener(new ActionListener() {
         @Override
         public void actionPerformed(ActionEvent evt) {
            stop = false;
            // Create a new Thread to do the counting
            Thread t = new Thread() {
               @Override
               public void run() {  // override the run() for the running behaviors
                  for (int i = 0; i < 100000; ++i) {
                     if (stop) break;
                     tfCount.setText(count + "");
                     ++count;
                     // Suspend this thread via sleep() and yield control to other threads.
                     // Also provide the necessary delay.
                     try {
                        sleep(10);  // milliseconds
                     } catch (InterruptedException ex) {}
                  }
               }
            };
            t.start();  // call back run()
         }
      });
 
      JButton btnStop = new JButton("Stop Counting");
      cp.add(btnStop);
      btnStop.addActionListener(new ActionListener() {
         @Override
         public void actionPerformed(ActionEvent evt) {
            stop = true;
         }
      });
 
      setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      setTitle("Counter");
      setSize(300, 120);
      setVisible(true);
   }
 
   /** The entry main method */
   public static void main(String[] args) {
      // Run GUI codes in Event-Dispatching thread for thread safety
      javax.swing.SwingUtilities.invokeLater(new Runnable() {
         @Override
         public void run() {
            new UnresponsiveUIwThreadSleep();  // Let the constructor do the job
         }
      });
   }
}
```
The `sleep()` method suspends the current thread and put it into the waiting state for the specified number of milliseconds. Another thread can begin execution (in a single CPU environment). (The `sleep()` can be interrupted by invoking the `interrupt()` method of this thread, which triggers an `InterruptedException` - this is unusual!)

In this case, the thread created to do the counting ("Thread-n") yields control *voluntarily* to other threads after every count (via the "`sleep(10)`"). This allows the event-dispatching thread to refresh the display as well as processing the "STOP" button after *each* count.

#### Example 4: SwingWorker

JDK 1.6 provides a new `javax.swing.SwingWorker` class, which can be used to run compute-intensive tasks in background threads, and passes the final result or intermediate results back to methods that run on the event-dispatching thread. We shall discuss `SwingWorker` in the later section.
