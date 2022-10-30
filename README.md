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

Each server requires a different configuration file with a different format.
Configuration file paths can be customized through command line options for each respective server.

### Discovery
Nodes use this to find other nodes in the network, a configuration file looks like this.
This server is used by all other nodes/servers in the network as an entry point to the network.

```json
{
  "discovery_host": [
    "127.0.0.1",
    7005
  ],
  "identity_host": [
    "127.0.0.1",
    5000
  ],
  "network_hosts": [
    [
      "127.0.0.1",
      8000
    ],
    [
      "127.0.0.1",
      8001
    ]
  ]
}
```
### Identity
This is used so that when a node sends a message it gets routed to the correct destination.
Every node connects to it after a discovery request to find its ipaddress.
```json
[
  "127.0.0.1",
  5000
]
```

### Node
This is the critical part of the network, each node manages connections to other nodes in the network,
connections to clients i.e. it hosts a WebSocketServer and an HttpServer, and it self-manages WebSocket instances and HttpClients.

```json
{
  "ws_server_host": [
    "127.0.0.1",
    8000
  ],
  "http_server_host": [
    "127.0.0.1",
    8100
  ],
  "discovery_host": [
    "127.0.0.1",
    7005
  ]
}
```
Programatically a "ServerHost" is just a tuple consisting of the IPAddress and the port the server is running on,
So `["ssda.efsdf.sd",2423904]` is a ServerHost in json fomat.


For the network to properly start, start the individual servers in the following order
Discovery/Identity/Node1/Node2/...NodeN

### Example: Running a discovery server
use the following command`discovery --config=path/to/config_fille.json`

```


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
