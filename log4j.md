## Understanding Apache log4j
### Overview of the log4j Architecture
In essence, log4j consists of three types of primary objects:
- `Logger`: The `Logger` object is responsible for capturing logging information.
- `Appender`: The `Appender` object is responsible for publishing logging information to various preferred destinations.
- `Layout`: The `Layout` object is used to format logging information in different styles. `Layout` objects play an important role in publishing logging information in a way that is human-readable and reusable.

Next, let's go over the principal auxiliary components in the log4j framework that play a vital role in the logging framework:
- `Level`: The `Level` object defines the granularity and priority of any logging information. The `Level` values can be arranged in an ascending manner:
   ```
   DEBUG<INFO<WARN<ERROR<FATAL
   ```

Now that you have seen the log4j core components, it is time to briefly discuss how they interact with each other.
- The central part of the log4j framework is the `Logger` object.
- An application instantiates a named `Logger` instance and passes various logging information to it.
- A `Logger` object has a designated `Level` object associated with it.
- A `Logger` logs only the messages with `Level` objects equal to or greater than its assigned `Level` object, or else it rejects the logging request.
- Once the `Level` condition has been met, the `Logger` object passes the logging information to all its associated `Appender` objects.
- Similar to `Logger` objects, `Appender` objects can also have threshold Levels attached to them. The logging information is validated against the threshold `Level` attached to the `Appender`. If the log message has a `Level` equal to or greater than the threshold `Level`, the logging message is passed to the next stage.
- The `Appender` utilizes any `Layout` object associated with it to format the message, and finally it publishes the logging information to the preferred destination.
### Configuring log4j
#### What We Can Configure
Let's look at a sample configuration file.
```
#set the level of the root logger to DEBUG and set its appender
#as an appender named testAppender
log4j.rootLogger = DEBUG, testAppender
#define a named logger
log4j.logger.dataAccessLogger = com.apress.logging.logger

#set the appender named testAppender to be a console appender
log4j.appender.testAppender=org.apache.log4j.ConsoleAppender

#set the layout for the appender testAppender
log4j.appender.testAppender.layout=org.apache.log4j.PatternLayout
log4j.appender.testAppender.layout.conversionPattern=%m%n
```
The preceding configuration defines the level of the root logger as `DEBUG` and specifies the appender to use as `testAppender`. Next, we define the appender `testAppender` as referencing the `org.apache.log4j.ConsoleAppender` object and specify the layout of the appender as `org.apache.log4j.PatternLayout`.

A more complex configuration can attach multiple appenders to a particular logger. Each appender, in turn, can have a different layout, and that layout can have a conversion pattern associated with it.
```
# define the root logger with two appenders writing to console and file
log4j.rootLogger = DEBUG, CONSOLE, FILE

#define your own logger named com.foo
#and assign level and appender to your own logger

log4j.logger.com.foo=DEBUG,FILE
#define the appender named FILE
log4j.appender.FILE=org.apache.log4j.FileAppender
log4j.appender.FILE.File=${user.home}/log.out

#define the appender named CONSOLE
log4j.appender.CONSOLE=org.apache.log4j.ConsoleAppender
log4j.appender.CONSOLE.conversionPattern=%m%n
```
This configuration file defines the root logger as having level `DEBUG` and attaches two appenders named `CONSOLE` and `FILE` to it. We define one of our own custom loggers with the name `com.foo` and the level `DEBUG` and attach an appender named `FILE` to the custom logger. The appender `FILE` is defined as `org.apache.log4j.FileAppender`. The `FILE` appender writes to a file named `log.out` located in the `user.home` system path.