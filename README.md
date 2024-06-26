# ProcessorNode

This library is part of a educational project aiming at teaching software architectures.

## Introduction

ProcessorNode is a C++ library for building distributed systems composed of Nodes. Typically one Node is deployed into one machine, communicating over the network with other Nodes in an installation.

Nodes can be arranged in *chains*:

```
N1 ----> N2 ----> N3 ----> N4
```

Where N1 sends data to N2, where it is sent to N3 and finally to N4. The chain can be used to *collect* data through the Nodes or process or *refine* data step by step along the chain. At the end, some refined or complete data is then available for other use.

Another way to arrange the Nodes is to collect data from various leaf nodes, aggregate it along the Node branches, *fan-in* style:

```
N1 ---\ 
       \
N2 ------> N5 ------\
                     ---> N7
N3 --------> N6 ----/
            /
N4 --------/
```

Here the leaf Nodes N1-4 collect data. N1 and N2 pass it along to N5, which delivers data combining it ahead to N7. Similarily, N3 and N4 deliver their data to N6, which sends it ahead to N7. Obviously the Nodes in between can also collect their own data, aggregate it with incoming data and then send it ahead.

An example of such kind of an arragement would be an environmental observation scenario. Nodes would be deployed in small devices (e.g. Raspberry Pi with Wi-Fi), collecting environmental data (temperature, humidity, air pressure, water level in rivers or lakes, etc.). Data would then be collected from large areas combined and delivered to a central location where data could be used for statistics, visualization or altering humans when environmental conditions exceed some threshold limits.

## Implementation

ProcessorNode has been implemented in standard C++ version 17. Implementation is compiler and OS independent. Implementation is inside C++ namespace `OHARBase`.

### Communication and Packages

Communication between the Nodes happens over UDP protocol, using JSON messages called *packages*. JSON payload is application specific. Payload must be included in a package:

```JSON
{ 
"package" : "123e4567-e89b-12d3-a456-426655440000",
"type" : "command" | "data" | "configuration",
"payload" : "command value" | "payload in json"
}
```
Each package has a globally unique id, which can be used to track how packages are delivered through the Nodes in an installation. 

For more detailed documentation of the JSON structures, see [JSONDocs](JSONDocs.md).

ProcessorNode takes care of parsing and creating the "package" id (UUID value) and "type" elements. 

Application specific code must take care of handling the "payload". Application specific playload can be text, but usually it is JSON. 

The only exception is the "type":"configuration" packages -- then the payload contains node configuration data, and ProcessorNode takes care of handling configuration request and responses. This enables *remote configuration* of the nodes. All other payload contents must be handled by the application specific code outside of ProcessorNode library. Again see [JSONDocs](JSONDocs.md) for details on configuration messages.

### Configuration 

As implied above, the Nodes can be *configured*. Configuration can be done by sending configuration packages to Nodes, but also (and more simply) using configuration files.

Configuration file for a Node should include at least these *key-value* pairs, separated by tab character:

* `name` -- the name of the Node,
* `input` -- the input port used by the node for reading incoming packages (optional, can be left out or "null"),
* `output` -- the output IP address of the next Node to send packages to, including the port (no host names, just numeric IP addresses),
* `filein` -- the optional input data file a Node can read to handle data in batches. Data file format is tsv, but the contents is application specific,
* `fileout` -- the optional output data file a Node can write data to. No special formatting requirements exist.

Optional configuration items include encryption:

* `encrypt` -- With the value `rot13` supported currently. When this configuration value is in the config file, Node will encrypt the payload of the JSON package if the type of the package is `data`, using [rot13](https://en.wikipedia.org/wiki/ROT13) before sending the JSON to the next node. Incoming JSON payload will be also decrypted using rot13.

If using encryption, obviously all the nodes in the same Pipes & Filters installation **must** use the same encryption setting. Note that only the payload of the JSON packages of type `data` is encrypted, other parts of the JSON message are *not* encrypted. Nor the payload of the `control` or `configuration` messages. See [JSONDocs](JSONDocs.md) for details on JSON messages and their types.

If the application wants to use the ProcessorNode remote configuration features, configuration file should additionally include:

* `config-in` -- A port listening for configuration read request messages (UDP broadcast messages) from a remote Configurator app
    * this port should be the same for all Nodes in the installation, since Configurator app does not have information on which machines Nodes are installed and which and ports Nodes are listening to. So a same port number should be configured for all Nodes.
* `config-out` with value "yes" -- if the Node does not have an output to the Next node, which can also be used to send responses to configuration requests, a configuration output should be created.

Additionally, configuration file may include application specific configuration items, as seen in the DirWatcher example app (discussed below).

An example of a configuration file looks like this (key-value pairs on a line *must* be tab separated):

```
nodeconfiguration
name        Exercise Information
config-in   10001
input       50002
output      192.168.1.165:50003
filein      /Users/juustila/StudentPassing/exercise-info.txt
fileout     /Users/juustila/StudentPassing/some-reporting-data.txt
```
First line of a configuration file must always have the word `nodeconfiguration`, and nothing else.

Then, the sample file is describing a Node:
* named "Exercise information", 
* having a port listening to config broadcast messages in port 10001, 
* receiving data packages from previous Node from port 50002, 
* sending packages to next Node at 192.168.1.165:50003, 
* readind data from tsv file "exercise-info.txt" and also 
* writing some reporting data to a file named.

If no output is configured and remote configuration support is required, one should specify `config-out` with value "yes". No other packages would be sent using this writer. If the Node has `output`, the same socket is used for sending "usual" packages and also configuration responses.


## Dependencies

ProcessorNode requires the following external components:

| Component | Version        | Purpose |
| --------------|---------------|-----------|
| [Boost](https://boost.org)          | 1.70.0+        | Networking (Boost::asio), string algorithms, uuid's |
| [g3logger](https://github.com/KjellKod/g3log)     | 1.3+             | Logging actions in the library |
| [nlohmann::json](https://github.com/nlohmann/json) | 3.2+       | For parsing and creating JSON from/to objects |

Download the Boost library and make sure the boost headers are available for the building of ProcessorNode library. The usual steps are:

* run the `bootstrap.sh` / .bat (in Windows) and then
* run the b2 file. Specify C++17 as the standard to use: `b2 cxxflags="-std=c++17"`
* `b2 install` installs the headers to a system specific location where ProcessorNode build can find them.

Use `git clone` to retrieve g3logger and nlohmann::json. Both can be build using cmake. See instructions from documentation of these libraries.

The g3logger *must* be initialized from the application level code by creating the logger object and specifying the logging method and logging file to write to. For example:

```C++
int main(int argc, const char * argv[])
{
  std::unique_ptr<g3::LogWorker> logworker{ g3::LogWorker::createLogWorker() };
  auto defaultHandler = logworker->addDefaultLogger(argv[0], "./");
  g3::initializeLogging(logworker.get());

  if (argc < 2) {
    LOG(WARNING) << "DDirWatcher " << "No config file specified! Give config file name as startup parameter.";
// ...
```
## Building

After building and installing Boost, g3logger and nlohmann::json, build and install ProcessorNode from the project root directory:

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```
If all goes well, the library has been build and installed. Headers needed for building apps on top of ProcessorNode can be found from the system header location (e.g. `/usr/local/include/ProcessorNode`) and the static library can be found from system lib directory (e.g. `/usr/local/lib/libProcessorNode.a`) for linking to applications.

Also a cmake package file is exported during the build. You can then include ProcessorNode to your own app by including the following in your CMakeLists.txt:

```
find_package(ProcessorNode REQUIRED)
...
target_include_directories(AppName PRIVATE ${ProcessorNode_INCLUDE_DIRS}  ...)
...
target_link_libraries(AppName ProcessorNode::ProcessorNode ...)

```

Then include the necessary headers from the lib into your app code, use the implementation and build and link. 

## Usage and example app

The basic usage of ProcessorNode is as follows:

1. Implementing application specific extension(s) of `DataItem` classes by subclassing `DataItem`. `DataItem` is an abstraction of *data* handled by the Nodes.
2. Implementing application specific *data handlers* by extending the `DataHandler` class,
3. Configuring the Node using a *configuration file*, including configuration items known by ProcessorNode, as well as application specific configurations,
2. Creating a `ProcessorNode` object instance,
3. Calling ProcessorNode's `configure` passing the configuration file as a parameter,
4. Adding *handlers* implemented in step 2 to handle application specific data items implemented in step 1.
    * see `DataHandler` and extensions of it from DirWatcher as examples, 
5. Calling ProcessorNode's `start()` to begin processing incoming/outgoing data packages according to application specific implementation.

An example app build on top of ProcessorNode can be found in the [DirWatcher](https://github.com/PipesAndFiltersProject/DirWatcher) project. It follows the fan-in style of architecture explained above so that there is one last Node receiving packages from leaf Nodes (no intermediate Nodes in between). DirWatcher does not support remote configuration currently.

```
LeafNode ---\ 
             \
LeafNode ------> LastNode
             /
LeafNode ----
           / 
LeafNode --
```

DirWatcher Leaf Nodes (with Leaf Node configuration) observe certain directories on the installed computer. When a change occurs in the directory (file is edited, created, renamed or deleted), the Node will send a JSON package ahead to the next Node, containing information about the file system event. The last Node (with Last Node configuration) then exports the change information into an xml or json file (depending on configuration), and a web page can display the events, as they are happening, to the user(s).


## Developer documentation

The project includes a `doxyfile.in`, a configuration file for [Doxygen](http://www.doxygen.nl). Using it you can create a more detailed developer documentation of the library. Document generation is integrated to CMake. After running cmake, create the docs:

```
make doc
```
or, if using Ninja: `ninja doc`. A `docs` subdirectory is created in the build directory. Generated HTML documentation can be found there. You can edit the doxyfile.in to create LaTeX, docbook, RTF, man pages or whatever if you prefer something else than HTML.

Document generation requires that Doxygen is installed. If also GraphViz is installed, generated documentation includes class and dependency diagrams.

To create component diagrams with graphviz and cmake do this (using Ninja as build system):

```
cmake -GNinja --graphviz=processornode.dot ..
dot -Tpng -oprocessornode.png processornode.dot
```

The result will be a png image file depicting the components of the software with dependency information:

![ProcessorNode dependencies](processornode.png)


## Who do I talk to?

Repo is owned and maintained by Antti Juustila (antti.juustila at oulu.fi)

## License

(c) Antti Juustila 2014-2020. This software is licensed in [GPL version 2 or later](https://opensource.org/licenses/gpl-2.0.php).

