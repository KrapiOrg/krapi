# Krapi

> Krapi is a simplistic blockchain to serve as a backend for a voting application frontend, built as a capstone project for my
Bachelors Degree in Computer Engineer's requirement for graduation.

### Requirements
1. CMake version >= 3.23.
2. CLang15+ compiler, this dependency is related to the usage of concurrencpp and the needed support for coroutines
make sure the build of CLang you use supports rrti and exceptions.

## Tested with the following configurations

1. You can directly use IntelliJ's CLion IDE and build the required targets as you go.
2. VSCode with the following extensions:
      - Better C++ Syntax by Jeff Hykin
      - Clang-Format by Xaver Hellauer
      - Clang-Tidy by notskm
      - clangd by the LLVM foundation
      - CodeLLDB by Vadim Chungunov
      - CMake Tools by Microsoft

* This project uses CMake as the build system generator.
* This project utilizes git submodules.

## Building

```commandline
git clone https://github.com/KrapiOrg/krapi
cd krapi
git submodule update --init --recursive
cmake --build .
```
## Documentation

This code is highly undocumented and I am unlikely to support it going further nonetheless this project
supports DyOxgen and you can get valuable insight by looking at the generated docs.

To generate the project docs run the following command.

```commandline
doxygen Doxyfile
```

You can then host an http sever to view the docs, for example here is how to do it with python:

* Make sure you have generated the docs with doxygen beforehand.

```commandline
cd docs
cd html
python3 -m http.server
```

## Motivation

Building a blockchain from scratch is no easy feet and I learned this the hard way.

I wanted to work on a project that is suffienctly complex yet with enough learning is feasible,
working on krapi I gained ALOT of experience in a multitude of areas I wouldn't have been exposed to otherwise.

I learned alot about concurrent programming, event-driven programming, the network stack, WebRTC, WebSockets, Proxies,
C++20 type constraints (concepts), C++20 coroutines, CMake, git & github, 
the C++ library ecosystem and many more little C++ knowledge nuggets.

I also want to note that this project doesn't follow those clickbaity articles spread all over the Internet
that claim to teach you how to program a blockchain in 50 lines of code, this code tried to emulate to the best of my ability
how other true blockchains work, this isn't quite the real deal but I would say it's a step above what's currently on the Internet.


## Running

1. Build the signaling target and run it
2. Build the node target and run 2 of them
3. Build the lightnode target, to run a lightnode do
```commandline
./lightnode <name>
```
4. To run elections you need to build and run a control_node target and build vote_application
where ```<name>``` is the name you wish to give to the node.

# Architecture

## Picking a programming language

- Dart?

  At first I gave Dart a thought, it seemed to me that having my entire stack  in Dart would be beneficial as I had decided long ago that the front-end would be written in Flutter, after doing a small prototype it was clear that Dart was not a good choice because it had lackluster support for server-side applications (You can also find the prototype on my Github)

- Python, JavaScript || Go?

  All of these languages have excellent support for server-side applications,
  I decided against any of them because python/javascript lack proper
  concurrency/parallelism and I had absolutely no experience with golang plus all of these languages had less to offer when it comes to Object Oriented
  Design/Programming (Ended up not using alot of those concepts anyway, Go was probably a good choice)

- Java || C#?

  Any of the two would have been excellent choices; java has the Spring framework and
  C# has dotnet for the back-end, they also offer excellent support of OOP, excellent support for
  concurrency and parallelism, at the end I didn’t choose either, while I had experience in C# it was
  nowhere near the level that would allow me to complete my objectives, the same story goes for Java, I
  didn’t want to spend a lot of time debugging and wanted to focus on quick prototyping.

- Rust || C++?

  Rust is a multi-paradigm, high-level, general-purpose
  programming language. Rust emphasizes performance, type safety, and concurrency, what enticed me for
  rust was that a lot of blockchain & blockchain-adjacent projects use it so I thought it could be beneficial for me to
  follow in their footsteps, even though I had little experience with rust I started learning it, I went
  quite far in that path, but soon I hit a wall and couldn’t progress any further due to the steep learning
  curve rust entails, I had to quickly pivot to working with C++.


## Why C++?

- Experience; I had worked with C++ for most of my university tenure, it was
familiar, standardized, fast and didn’t get in the way with things like the burrow checker like in rust or with a garbage collector like in C#/Java and the others (Oh man I've come to regret this, I wish I had a garbage  collector or a burrow checker holding my hands; C++ is just full of little things that can easily cause UB and unsafe memory accesses)

- Modern versions of C++ (especially C++20) added many powerful and useful
constructs to the language like standardized threading, stackless coroutines, concepts,
structured bindings, universal references, variant types, etc…, I ended up using
these features a lot throughout my codebase.


- A large ecosystem of well maintained, documented and battle tested libraries, I also had access to one of the largest communities in the programming world to ask questions in case anything went awry, I had no shortage of great educational videos and learning materials too.

- Libraries, compiler and other infrastructure


As mentioned earlier C++ offers one of the richest library ecosystems in the programming
world, the C++ standard library often had what I needed but there were times when I just couldn’t
make do with what’s available by default, in this section we’ll be talking about the build system I used,
the libraries I included and we’ll be delving into greater detail about how each library served a particular
purpose.


### CMake


- CMake is an open-source, cross-platform family of tools designed to build, test and
package software. CMake is able to generate UNIX MakeFiles, these files are then used by
the GNU Make build system to compile my back-end code, CMake is the defacto standard in
the C++ sphere it is widely used by most libraries, I used it extensively because it allowed
the easy inclusion of external libraries into my project.

### LLVM/Clang


- I built Clang with RTTI support (Run-Time Type Information), using clang allowing me
to use tools such as clang-tidy, which is a static analyser, clang-format to maintain a
consistent style across my codebase, the clangd language server to provide errors and
warnings and lldb as my debugger of choice, I specifically used version 16 of clang I.e.
clang16 as it included support for C++20 coroutines, which we’ll talk more about when
talking about concurrencpp.

I think at the end the compiler choice didn't matter this much as I thought I just didn't know how to properly configure GCC ;=;

### LevelDB

- LevelDB is a fast key-value storage library written at Google that provides an
ordered mapping from string keys to string values, I used LevelDB extensively in my
back-end code, I used it store the miner’s local copy of the blockchain, I also used it to store spent transactions and to store messages that needed to be retried.

### WebRTC


- Our networking architecture is based on a P2P model, for this I required
an implementation of the WebRTC protocol. WebRTC (Web Real Time Communication)
is a protocol developed by Google that enables peer-to-peer communication. It allows for
the transmission of data without the need for an intermediary server (except in my case I
use a signaling server as mentioned before). This can be useful in a variety of contexts,
including in the context of blockchain technology. WebRTC can be used to facilitate the
exchange of messages between peers, messages relayed can about new transactions and the
current state of the blockchain. To use WebRTC in my backend I used the well
established libdatachannel library, which is a standalone implementation of WebRTC
Data Channels, WebRTC Media Transport, and WebSockets in C++.

Ehhh, Learning WebRTC for DataChannels (which are essentially WebSockets) is too heavy and looking back now, I think going with something like libuv or ZeroMQ would have been better.


### Cryptography & UUIDs


- In my project I rely heavily on hashing and UUID (Universally Unique Identifiers) generation,
they are critical to get right for me to succeed in my objectives, they are mission critical and they represent
a major security hazard as reinventing the wheel causes big headaches, so for hashing I went with the
reliable cryptopp library and for uuid generation I used the stduuid library, messages passed around the
network use uuids as tags to identify messages.


### Utilities


- Every project needs a good logger, providing good output logs was an instrumental part of
making sure my code was doing what it was intended to do, I used the famous logging library spdlog,
its API was easy to use and it provided 3 levels of logging by default (info, warning and error) these were
more than enough for my needs. For text formatting I used libfmt, which is intended as a modern
replacement for traditional string formatting through the C functions printf and the like. We needed a
human-readable codec for the data and messages I were passing around the network. We settled on the
JSON format whose support in C++ was achieved with the nlohmann_json library



![Krapi Event Loop](https://I.imgur.com/ubZcMaj.png)

The above diragram gives a little bit of insight about how the event loop in this design works.

## Demo [Arabic]

[![Krapi Demo](https://I.imgur.com/zB8Qbyj.png)](https://youtu.be/NvcLwSh_mIw "Krapi Demo")

## Remarks

I think I've learned quite a few valuable lessons, but these are the ones that are atop my head right now:

- I need to get into the habit of writing tests; I lothed writing a new feature for it to mess with previous work causing it to produce wrong output.

- I need to also get into the habit of writing descriptive commits; this repo is just filled to the brim with sweeping changes being labeled as "Checkpoint" or "Milestone".

- I need to pick a stack and stick to it; I changed core libraries probably 4-5 times forcing me to rewrite massive portions of the project every time.

- C++ is a beautiful language, it's that for someone like me who's moving from an intermediate understanding of the language to a more nuanced/advanced one, things can get messy, I thought I had a pretty good understanding of memory ownership until I wrote a piece of code that made me pull my hair until I discovered that lambda caputred by value shared_ptrs go out of scope when a coroutine suspends, in handsight though, alot of these quirky moments could have been avoided had I read more or perhaps written tests like any good programmer-person but alas, all of us have to start somewhere.

- CMake is just *p__p emoji*

- nlohmann_json rocks

- I should probably research things more instead of diving head first into a project ;=;

# Notes

- Box.h is based on [polymorphic_value by jbcoe](https://github.com/jbcoe/polymorphic_value)

- I intend to keep this repository and project as is, it's highly unlikely that'll update/refactor it; the code here holds sentimental value and I want this to serve as a sort of "milestone" in my career, I am proud of it but I recognize I need to move on onto other projects that I feel are more aligned with what I want to do in life.