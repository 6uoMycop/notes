* Protect access to concurrent resources
* Simple solution: use mutex/lock
* TLS: each thread has its own object (no need to lock)
   * "just" add `__thread` to variables
   ```
   __thread int value = 1;
   
   ```
* Drawback: increased memory usage and small CPU penalty, only simple data types for static/global variables can be made TLS