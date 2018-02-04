## Tutor 7: How to integrate you own Java project

### Tutorial: Hello World with Apache Ant
#### Preparing the project
We have to create only the `src` directory. (Because I am working on Windows, here is the win-syntax - translate to your shell):
```
md src
```
The following simple Java class just prints a fixed message out to STDOUT, so just write this code into `src\oata\HelloWorld.java`.
```
package oata;

public class HelloWorld {
    public static void main(String[] args) {
        System.out.println("Hello World");
    }
}
```
Now just try to compile and run that:
```
md build\classes
javac -sourcepath src -d build\classes src\oata\HelloWorld.java
java -cp build\classes oata.HelloWorld
```
which will result in 
```
Hello World
```
Creating a jar-file is not very difficult. But creating a *startable* jar-file needs more steps: create a manifest-file containing the start class, creating the target directory and archiving the files.
```
echo Main-Class: oata.HelloWorld>myManifest
md build\jar
jar cfm build\jar\HelloWorld.jar myManifest -C build\classes .
java -jar build\jar\HelloWorld.jar
```

#### Four steps to a running application
After finishing the java-only step we *have* to think about our build process. We have to compile our code, otherwise we couldn't start the program. Oh - "start" - yes, we could provide a target for that. We should package our application. Now it's only one class - but if you want to provide a download, no one would download several hundreds files ... (think about a complex Swing GUI - so let us create a jar file. A startable jar file would be nice ... And it's a good practise to have a "clean" target, which deletes all the generated stuff. Many failures could be solved just by a "clean build".

By default Ant uses `build.xml` as the name for a buildfile, so our `.\build.xml` would be:
```
<project>

    <target name="clean">
        <delete dir="build"/>
    </target>

    <target name="compile">
        <mkdir dir="build/classes"/>
        <javac srcdir="src" destdir="build/classes"/>
    </target>

    <target name="jar">
        <mkdir dir="build/jar"/>
        <jar destfile="build/jar/HelloWorld.jar" basedir="build/classes">
            <manifest>
                <attribute name="Main-Class" value="oata.HelloWorld"/>
            </manifest>
        </jar>
    </target>

    <target name="run">
        <java jar="build/jar/HelloWorld.jar" fork="true"/>
    </target>

</project>
```
Now you can compile, package and run the application via
```
ant compile
ant jar
ant run
```

#### Enhance the build file
Now we have a working buildfile we could do some enhancements: many time you are referencing the same directories, main-class and jar-name are hard coded, and while invocation you have to remember the right order of build steps.

The first and second point would be addressed with *properties*, the third with a special property - an attribute of the \<project\>-tag and the fourth problem can be solved using dependencies.
```
<project name="HelloWorld" basedir="." default="main">

    <property name="src.dir"     value="src"/>

    <property name="build.dir"   value="build"/>
    <property name="classes.dir" value="${build.dir}/classes"/>
    <property name="jar.dir"     value="${build.dir}/jar"/>

    <property name="main-class"  value="oata.HelloWorld"/>



    <target name="clean">
        <delete dir="${build.dir}"/>
    </target>

    <target name="compile">
        <mkdir dir="${classes.dir}"/>
        <javac srcdir="${src.dir}" destdir="${classes.dir}"/>
    </target>

    <target name="jar" depends="compile">
        <mkdir dir="${jar.dir}"/>
        <jar destfile="${jar.dir}/${ant.project.name}.jar" basedir="${classes.dir}">
            <manifest>
                <attribute name="Main-Class" value="${main-class}"/>
            </manifest>
        </jar>
    </target>

    <target name="run" depends="jar">
        <java jar="${jar.dir}/${ant.project.name}.jar" fork="true"/>
    </target>

    <target name="clean-build" depends="clean,jar"/>

    <target name="main" depends="clean,run"/>

</project>
```
Now it's easier, just do a `ant` and you will get
```
Buildfile: build.xml

clean:

compile:
    [mkdir] Created dir: C:\...\build\classes
    [javac] Compiling 1 source file to C:\...\build\classes

jar:
    [mkdir] Created dir: C:\...\build\jar
      [jar] Building jar: C:\...\build\jar\HelloWorld.jar

run:
     [java] Hello World

main:

BUILD SUCCESSFUL
```
