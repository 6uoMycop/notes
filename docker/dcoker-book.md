# The Docker Book
## Chapter 1 Introduction
### 1.2 Docker components

Let's look at the core components that compose the Docker Community Edition:

- The Docker client and server, also called the Docker engine 
- Docker Images
- Registries
- Docker Containers

#### 1.2.1 DOCKER CLIENT AND SERVER
Docker is a client-server application. The Docker client talks to the Docker server or daemon, which, in turn, does all the work. You'll also sometimes see the Docker daemon called the Docker Engine. Docker ships with a command line client binary, `docker`, as well as a full RESTful API to interact with the daemon: `dockerd`.

#### 1.2.2 DOCKER IMAGES
Images are the building blocks of the Docker world, You launch your containers from images.

You can consider images to be the "source code" for your containers.

#### 1.2.4 CONTAINERS
Docker helps you build and deploy containers inside of which you can package your applications and services. As we've just learned, containers are launched from images and can contain one or more running processes. You can think about images as the building block or packing aspect of Docker and the containers as the running or execution aspect of Docker.

## Chapter 3 Getting Started with Docker
### 3.3 Working with our first container
You can keep playing with the container for as long as you like. When you're done, type `exit`, and you'll return to the command prompt of your Ubuntu host.

So what's happened to our container? Well, it has now stopped running. The container only runs for as long as the command we specified, `/bin/bash`, is running. Once we exited the container, that command ended, and the container was stopped.

The container still exists; we can show a list of current containers using the `docker ps -a` command.

```
CONTAINER ID IMAGE   COMMAND     CREATED  STATUS PORTS NAMES
1cd57c2cdf7f ubuntu  "/bin/bash" A minute Exited       gray_cat
```
By default, when we run just `docker ps`, we will only see the running containers. When we specify the `-a` flag, the `docker ps` command will show us all containers, both stopped and running.

### 3.5 Starting a stopped container
Well, if we want, we can restart a stopped container like so:
```
$ sudo docker start bob_the_container
```
### 3.6 Attaching to a container
We can reattach to that session using the `docker attach` command.
```
$ sudo docker attach bob_the_container
```

### 3.16 Deleting a container
If you are finished with a container, you can just delete it using the `docker rm` command.
```
$ sudo docker rm 80430f8d0921
80430f8d0921
```

## Chapter 4 Working with Docker images and repositories
### 4.2 Listing Docker images
Let's get started with Docker images by looking at what images are available to us on our Docker host. We can do this using the `docker images` command.

```
$ sudo docker images
REPOSITORY TAG     IMAGE ID     CREATED     VIRTUAL SIZE
ubuntu     latest  c4ff7513909d 6 days ago  225.4 MB
```

Inside Docker Hub (or on a Docker registry you run yourself), images are stored in repositories. You can think of an image repository as being much like a Git repository. It contains images, layers, and metadata about those images.

Each repository can contain multiple images (e.g., the `ubuntu` repository contains images for Ubuntu 12.04, 12.10, 13.04, 13.10, 14.04, 16.04). Let's get another image from the `ubuntu` repository now.
```
$ sudo docker pull ubuntu:16.04
16.04: Pulling from library/ubuntu
Digest: sha256:c6674c44c6439673bf56536c1a15916639c47ea04c3d6296c5df938add67b54b
Status: Downloaded newer image for ubuntu:16.04
```
Here we've use the `docker pull` command to pull down the Ubuntu 16.04 image from the `ubuntu` repository.

We identify each image inside that repository by what Docker calls tags. Each image is being listed by the tags applied to it, so, for example, `12.04`, `12.10`, `quantal`, or `precise` and so on.

We can refer to a specific image inside a repository by suffixing the repository name with a colon and a tag name, for example:
```
$ sudo docker run -t -i --name new_container ubuntu:16.04 /bin/bash
root@79e36bff89b4:/#
```
This launches a container from the `ubuntu:16.04` image, which is an Ubuntu 16.04 operating system.

Alternatively, a top-level repository only has a repository name like `ubuntu`. The top-level repositories are managed by Docker Inc and by selected vendors who provide curated base images that you can build upon (e.g., the Fedora team provides a `fedora` image).

### 4.3 Pulling images
When we run a container from images with the `docker run` command, if the image isn't present locally already then Docker will download it from the Docker Hub. By default, if you don't specify a specific tag, Docker will download the `latest` tag, for example:
```
$ sudo docker run -t -i --name next_container ubuntu /bin/bash
root@23a42cee91c3:/#
```
Will download the `ubuntu:latest` image if it isn't already present on the host.

Alternatively, we can use the `docker pull` command to pull images down ourselves preemptively. Using `docker pull` saves us some time launching a container from new image. Let's see that now by pulling down the fedora:21 base image.
```
$ sudo docker pull fedora:21
21: Pulling from library/fedora
d60b4509ad7d: Pull complete
Digest: sha256:4328c03e6cafef1676db038269fc9a4c3528700d04ca1572e706b4a0aa320000
Status: Downloaded newer image for fedora:21
```

### 4.5 Building our own images
#### 4.5.1 CREATING A DOCKER HUB ACCOUNT
Now let's test our new account from Docker. To sign into the Docker Hub you can use the `docker login` command.

```
$ sudo docker login
Login with your Docker ID to push and pull images from Docker Hub. If you don't have a Docker ID, head over to https://hub.docker.com to create one.
Username (jamtur01): jamtur01
Password:
Login Succeeded
```
This command will log you into the Docker Hub and store your credentials for future use. You can use the `docker logout` command to log out from a registry server.
#### 4.5.2 USING DOCKER COMMIT TO CREATE IMAGES
The first method of creating images uses the `docker commit` command. You can think about this method as much like making a commit in a version control system. We create a container, make changes to that container as you would change code, and then commit those changes to a new image.

Let's start by creating a container from the `ubuntu` image we've used in the past.
```
$ sudo docker run -i -t ubuntu /bin/bash
root@4aab3ce3cb76:/#
```
Next, we'll install Apache into our container.
```
root@4aab3ce3cb76:/# apt-get -yqq update
. . .
root@4aab3ce3cb76:/# apt-get -y install apache2
. . .
```
We've launched our container and then installed Apache within it. We're going to use this container as a web server, so we'll want to save it in its current state. That will save us from having to rebuild it with Apache every time we create a new container. To do this we exit from the container, using the `exit` command, and use the `docker commit` command.
```
$ sudo docker commit 4aab3ce3cb76 jamtur01/apache2
8ce0ea7a1528
```
You can see we've used the `docker commit` command and specified the ID of the container we've just changed (to find that ID you could use the `docker ps -l -q` command to return the ID of the last created container) as well as a target repository and image name, here `jamtur01/apache2`. Of note is that the `docker commit` command only commits the differences between the image the container was created from and the current state of the container. This means updates are lightweight.

#### 4.5.4 BUILDING THE IMAGE FROM OUR DOCKERFILE
We've used the `docker build` command to build our new image. We've specified the `-t` option to mark our resulting image with a repository and a name, here the `jamtur01` repository and the image name `static_web`.

You can also tag images during the build process by suffixing the tag after the image name with a colon, for example:
```
$ sudo docker build -t="jamtur01/static_web:v1" .
```

#### 4.5.9 LAUNCHING A CONTAINER FROM OUR NEW IMAGE
Let's launch a new container using our new image and see if what we've built has worked.
```
$ sudo docker run -d -p 80 --name static_web jamtur01/static_web nginx -g "daemon off;"
6751b94bb5c001a650c918e9a7f9683985c3eb2b026c2f1776e61190669494a8
```
Here I've launched a new container called `static_web` using the `docker run` command and the name of the image we've just cerated. We've specified the `-d` option, which tells Docker to run detached in the background. This allows us to run long-running processes like the Nginx daemon.

We've also specified a new flag, `-p`. The `-p` flag manages which network ports Docker publishes at runtime. When you run a container, Docker has two methods of assigning ports on the Docker host:
- Docker can randomly assign a high port from the range 32768 to 61000 on the Docker host that maps to port 80 on the container.
- You can specify a specific port on the Docker host that maps to port 80 on the container.

Let's look at what port has been assigned using the `docker ps` command.
```
$ sudo docker ps -l
CONTAINER ID  IMAGE                      ... PORTS                  NAMES
6751b94bb5c0  jamtur01/static_web:latest ... 0.0.0.0:49154->80/tcp  static_web
```
We see that port 49154 is mapped to the container port of 80.

The `-p` option also allows us to be flexible about how a port is published to the host. For example, we can specify that Docker bind the port to a specific port:
```
$ sudo docker run -d -p 80:80 --name static_web_80 jamtur01/static_web nginx -g "daemon off;"
```
This will bind port 80 on the container to port 80 on the local host. It's important to be wary of this direct binding: if you're running multiple containers, only one container can bind a specific port on the local host. 

### 4.6 Pushing images to the Docker Hub
Once we've got an image, we can upload it to the Docker Hub. This allows us to make it available for others to use. For example, we could share it with others in our organization or make it publicly available.

We push images to the Docker Hub using the `docker push` command.

Let's build an image without a user prefix and try and push it now.

```
$ cd static_web
$ sudo docker build --no-cache -t="static_web" .
. . .
Successfully built a312a2ed58c7
$ sudo docker push static_web
The push refers to a repository [docker.io/library/static_web]
c0121fc36460: Preparing
8591faa9900d: Preparing
9a39129ae0ac: Preparing
98305c1a8f5e: Preparing
0185b3091e8e: Preparing
ea9f151abb7e: Waiting
unauthorized: authentication required
```
What's gone wrong here? We've tried to push our image to the repository `static_web`, but Docker knows this is a root repository. Root repositories are managed only by the Docker, Inc., team and will reject our attempt to write to them as authorized. Let's try again, rebuilding our image with a user prefix and then pushing it.
```
$ sudo docker build --no-cache -t="jamtur01/static_web" .
$ sudo docker push jamtur01/static_web
The push refers to a repository [jamtur01/static_web] (len: 1)
Processing checksums
Sending image list
Pushing repository jamtur01/static_web to registry-1.docker.io (1 tags)
. . .
```
This time, our push has worked, and we've written to a user repository, `jamtur01/static_web`. We would write to your own user ID, which we created earlier, and to an appropriately named image (e.g., `youruser/yourimage`).

We can now see our uploaded image on the Docker Hub.

## Chapter 7 Docker Orchestration and Service Discovery

### 7.3 Docker Swarm

A swarm is made up of manager and worker nodes. Manager do the dispatching and organizing of work on the swarm. Each unit of work is called a task.

Worker nodes run the tasks dispatched from manager nodes. Out of the box, every node, managers and workers, will run tasks. You can instead configure a swarm manager node to only perform management activities and not run tasks.

#### 7.3.3 SETTING UP SWARM
We're going to create two cluster workers and a manager on our three hosts.

Host | IP Address | Role
------------ | ------------- | -------------
larry | 162.243.167.159 | Manager
curly | 162.243.170.66 | Worker
moe | 159.203.191.16 | Worker

We also need to make sure some ports are open between all our nodes. We need to consider the following access:

Host | IP Address
------------ | -------------
2377 | Cluster Management
7946 + udp | Node communication 
4789 + udp | Overlay network

We're going to start with registering a Swarm on our larry node and use this host as our Swarm manager. We're again going to need larry's public IP address. Let's make sure it's still assigned to an environment variable.

```
larry$ PUBLIC_IP="$(ifconfig eth0 | awk -F ' *|:' '/inet addr/{print $4}')"
larry$ echo $PUBLIC_IP
162.243.167.159
```
Now let's initialize a swarm on larry using this address.
```
$ sudo docker swarm init --advertise-addr $PUBLIC_IP
Swarm initialized: current node (bu84wfix0h0x31aut8qlpbi9x) is now a manager.

To add a worker to this swarm, run the following command:
    docker swarm join \
    --token SWMTKN-1-2mk0wnb9m9cdwhheoysr3pt8orxku8c7k3x3kjjsxatc5ua72v-776lg9r60gigwb32q329m0dli \    162.243.167.159:2377
```
You can see we've run a `docker` command: `swarm`. We've then used the `init` option to initialize a swarm and the `--advertise-addr` flag to specify the management IP of the new swarm.

We can see the swarm has been started, assigning `larry` as the swarm manager.

Let's add our `curly` and `moe` hosts to the swarm as workers. We can use the command emitted when we initialized the swarm.
```
curly$ sudo docker swarm join \
--token SWMTKN-1-2mk0wnb9m9cdwhheoysr3pt8orxku8c7k3x3kjjsxatc5ua72v-776lg9r60gigwb32q329m0dli \
162.243.167.159:2377
This node joined a swarm as a worker.
```

#### 7.3.4 RUNNING A SERVICE ON YOUR SWARM
With the swarm running, we can now start to run services on it. Remember services are a container image and commands that will be executed on our swarm nodes. Let's create a simple replica service now. Remember that replica services run the number of tasks you specify.

```
$ sudo docker service create --replicas 2 --name heyworld ubuntu /bin/sh -c "while true; do echo hey world; sleep 1; done"
8bl7yw1z3gzir0rmcvnrktqol
```
We've used the `docker service` command with the `create` keyword. This creates services on our swarm. We've used the `--name` flag to call the service: `heyworld`. The `heyworld` runs the `ubuntu` image and a `while loop` that echoes `hey world`. The `--replicas` flag controls how many tasks are run on the swarm. In this case we're running two tasks.

Let's look at our service using the `docker service ls` command.

```
$ sudo docker service ls
ID            NAME      REPLICAS  IMAGE   COMMAND
8bl7yw1z3gzi  heyworld  2/2       ubuntu  /bin/sh -c while true; do echo hey world; sleep 1; done
```
This command lists all services in the swarm. We can see that our `heyworld` service is running on two replicas.

But we still don't know where the service running. Let's look at another command: `docker service ps`.
```
$ sudo docker service ps heyworld
ID      NAME       IMAGE  NODE  DESIRED STATE CURRENT STATE
103q... heyworld.1 ubuntu larry Running       Running about a minute ago
6ztf... heyworld.2 ubuntu moe   Running       Running about a minute ago
```
We can see each task. suffixed with the task number, and the node it is running on.

If we want to stop a service we can run the `docker service rm` command.
```
$ sudo docker service rm heyworld
heyworld
```
