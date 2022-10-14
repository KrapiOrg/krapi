# Krapi

> Krapi is a simplistic blockchain to serve as a backend for a secure voting application
> and is defined by the krapi protocol.


## Protocol Description

### Definitions:
- Client: A non-contributing member that wishes to utilize the chain without storing or
mining.
- Node: A contributing member but not a mine that stores the entirety of the blockchain.
- Miner: A node that competes for transctions by polling transaction pools, if it provides
sufficient proof of work it gets rewarded.
- Transaction Pool: An endpoint that miners poll for transactions submitted by clients.
- Discovery: an endpoint that clients use to find transaction pools, nodes can use
this endpoint to find other nodes in the network

Krapi utilizes websockets for bidirectional communication between its various servers,
the flow goes like this:

1. Client sends a `pool discovery message` to a known discovery server,
2. The client gets a list of pools to connect to.
3. The client chooses one transaction pool and establishes a connection to it.
4. Clients create transcations that are sent to pools where miners will compete for those
transactions.
5. Once a miner provides sufficient PoW a block is added to the blockchain and the change
is propagted to other nodes/mines on the network.

NOTE: All servers are ideally hosted on port 8080 for production but one can change this 
through the config.json files.

## Building

### Requirements
1. CMake version >= 3.23.
2. C++20 capable compiler.

* This project uses CMake as the build system.
* This project utilizes git submodules.

```commandline
git clone https://github.com/KrapiOrg/krapi
cd krapi
git submodule update --init --recursive
cmake --build .
```

## Running
After building, an executable will be built for each server in the protocol outlined above.

To run a particular server lets say `discovery` do the following (the process should be similar
for all other servers)
1. Place a config file for `discovery` called xxx.json (where xxx is the name of the file)
2. The contents of the file should look like this
```json
{
  "server_port": 8080,
  "server_host": "127.0.0.1",
  "hosts": [
    "127.0.0.1:8080"
  ]
}
```
server_port: controls the port the `discovery` server will bind to
server_host: the host uri the server will run on
hosts: the list of known `transaction pool` known in the krapi network.
3. Run the command `discovery --config=xxx.json`

## QnA

> Why C++?

> C++ was chosen because it's the language I'm most comfortable in, this choice
> comes with the fact that C++ is not memory safe, but I felt that the rich
> and mature library ecosystem greatly outshines any problems that could arise due to 
> memory mismanagment espicially if I'm careful enough to avoid common pitfalls in that regard.
> > Hashem

> Why seemingly reinvent the wheel and not use a pre-existing blockchain?

Krapi was from the start envisoned as a learning tool and to be used in our final year
capstone project, maybe in the future it could be something more but for now it is what
it is.


# Libraries in use
[cryptopp](https://github.com/weidai11/cryptopp)


[cryptopp-cmake](https://github.com/abdes/cryptopp-cmake)


[cxxopts](https://github.com/jarro2783/cxxopts)


[fmtlib](https://github.com/fmtlib/fmt)


[nlohmann_json](https://github.com/nlohmann/json)


[spdlog](https://github.com/gabime/spdlog)


[ixwebsocket](https://github.com/machinezone/IXWebSocket)


[googletest](https://github.com/google/googletest)
