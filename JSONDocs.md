
# StudentPassing JSON data structures

This document describes the structure and usage of the various JSON data structures used in intercomponent communication in the StudentPassing system.

There are Data packages and Configuration packages, described below in separate chapters.


## Data packages sent between Nodes

Packages are sent to network and received from network. General strucure is as follows:

```JSON
{ 
"package" : "123e4567-e89b-12d3-a456-426655440000",
"type" : "command" | "data" | "configuration",
"payload" : "command value" | "payload in json"
}
```
For details, see the ProcessorNode documentation. Package types related to configuration are described also in the ProcessorNode docs.

Payload contains the actual application specific data. For example, in StudentPassing, student information:

```JSON
{
"id" : "12345",
"name" : "Test Student",
"studyprogram" : "TOL",
"exampoints" : 15,
"exercisepoints" : [8,10,3,1,8,0,11],
"courseprojectpoints" : 16,
"grade" : 3
}
```
And combined example (data sent/received) as a Package:

```JSON
{ 
"package" : "123e4567-e89b-12d3-a456-426655440000",
"type" : "data",
"payload" :
{
"id" : "12345",
"name" : "Test Student",
"studyprogram" : "TOL",
"exampoints" : 15,
"exercisepoints" : [8,10,3,1,8,0,11],
"courseprojectpoints" : 16,
"grade" : 3
}
}
```

Note that in StudentPassing, the data payload for student data, only student id is compulsory. Different nodes provide different data items, and combine data they received from previous node(s) to the data the node itself produces. This combined data is then sent ahead to the next node(s).

## Command packages

Command packages can be sent from Nodes to other nodes, or from other external components (like the Configurator).

An example of a command:

```JSON
{
"package" : "123e4567-e89b-12d3-a456-426655440000",
"type" : "command",
"payload" : "shutdown"
}
```
Which will handled by the Nodes so that 1) the shutdown message is passed to next Node, if any, and 2) the Node shuts itself down.

Other existing commands are documented in the ProcessorNode docs. Note that applications may create and implement other commands and handling for those.

* readfile, which causes the Node to read an input file and handle the data from there (if node has an input file)
* quit, which causes the Node to quit (without sending shutdown command to next Nodes).
* ping, which causes the Node to show ping somehow to user and forward that ping command to next node(s). This command can be used to test if the node connections are working.

## Remote Node Configuration

Node configuration data may be fetched over the network from a Node by sending a configuration message to the Node:

```JSON
{
"package" : "123e4567-e89b-12d3-a456-426655440000",
"type" : "configuration",
"payload" : "read"
}
```

Node then replies with Node configuration data:

```JSON
{
"package" : "123e4567-e89b-12d3-a456-426655440000",
"type" : "configuration",
"payload" : 
{
"operation" : "info",
"configitems" : [
{"name" : "node name"},
{"config-item" : "item-value"},
{"config-item2" : "item-value"}
]
}
}
```

If a Node or Nodes needs to be reconfigured remotely, first the remote configurator sends the `configuration - operation : read` message. Node then replies to the sender with `configuration - operation : info` message. Remote configurator then changes the configuration, and sends the `configuration - operation : set` message (below) to the Node. Node will chage the configuration and continues to process other Packages according to the new configuration.

```JSON
{
"package" : "123e4567-e89b-12d3-a456-426655440000",
"type" : "configuration",
"payload" : 
{
"operation" : "set",
"configitems" : [
{"name" : "node name"},
{"config-item" : "item-value"},
{"config-item2" : "item-value"}
]
}
}
```
Example of a Node configuration payload contents:

```
"payload" : {
"operation" : "info",
"configitems" : [
{"name" : "Exam points handling"},
{"input" : 25400 },
{"config" : 10000 },
{"output" : "130.231.44.321:25600"},
{"filein" : "/Users/juustila/StudentPassing/exam-info.txt"},
{"fileout" : "/Users/juustila/StudentPassing/Examp-report.txt"}
]
}
```
Where

- name is the node's name visible to end users,
- input is the input UDP port the Node listens to for incoming Packanges,
- output is the IP address of the Next node where packages are sent from this Node,
- filein is the input file Node is reading data from,
- fileout is the output file Node is writing data to.

Additional application specific configuration items can be listed in the items array.


## Who do I talk to? ##

Repo is owned and maintained by Antti Juustila (antti.juustila at oulu.fi)

## License ##

(c) Antti Juustila 2014-2020. This software is licensed in [GPL version 2 or later](https://opensource.org/licenses/gpl-2.0.php).

