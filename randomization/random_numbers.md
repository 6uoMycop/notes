# Random Numbers On a Computer
First of all, it is not really possible (nor desirable) to have real random numbers. What we want is a repeatable sequence of seemingly random numbers that satisfy certain properties, such as the average value of a list of random numbers between say, 0 and 1000, should be 500. Other properties, such as no (predictable) relation between the current random number and the next random number is desirable.

In any program where random values are necessary (for example, most simulations) the programming language must provide us with a random number generator. This will be a function that will, when called, provide us with a single random number... when called again, provide us with another random number.

## "Pseudo" random number sequences
Generating a single random number is easy. 27. There!

Generating a **sequence of random numbers** is quite difficult because we want certain assumptions about this sequence to be true!

Note that "Random" number generators produce what are called "pseudo" random numbers, meaning that the numbers only "approximate" real randomness. 

Some Goals:

1. Repeatable: Why?
2. Speed (Fast to Compute): Why?
3. Average value is average of range.
4. Odd followed by an Even as Likely as Even followed by an Odd.
5. When choosing 1000 numbers between 0 and 1000 we will not hit every number.
6. When choosing a numbers between 0 and 1000 we are equally likely to get any number.
7. When choosing 10000 numbers between 0 and 1000 we are likely to get roughly 10 of each number.

Some Possible Techniques to Create Random Sequence:

1. Time - Use the computers clock
2. Radiation - Install some radiation in the computer and compute how often the atoms decay... (uggh)
3. Math - use a formula (see below)

The advantage of using mathematics to generate a random number (sequence) is that it is **REPEATABLE**. This is important for the following reason: debugging a program. Imagine the problems you already have finding errors in your code. What would happen if the "path" (or program flow) was different (random) every time? Debugging would be a nightmare! At the end of the day, the programmer wants the same sequence of random numbers every time, but the user may require a different sequence. To generate a different sequence of random numbers we use a "seeding" function. For the purposes of this course, you will most likley not need to "seed" your random number generator. 
