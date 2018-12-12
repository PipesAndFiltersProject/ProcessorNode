//
//  ProcessorNode.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 19.9.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include <sstream>
#include <iostream>

#include <boost/uuid/uuid_io.hpp>

#include <g3log/g3log.hpp>

#include <OHARBaseLayer/ProcessorNode.h>
#include <OHARBaseLayer/NetworkReader.h>
#include <OHARBaseLayer/NetworkWriter.h>
#include <OHARBaseLayer/PingHandler.h>
#include <OHARBaseLayer/DataFileReader.h>
#include <OHARBaseLayer/NodeConfiguration.h>
#include <OHARBaseLayer/ConfigurationFileReader.h>
#include <OHARBaseLayer/ProcessorNodeObserver.h>

namespace OHARBase {
   
   /** Constructor for the processor node.
    @param aName The name of the processor node. */
   ProcessorNode::ProcessorNode(const std::string & aName, ProcessorNodeObserver * o)
   : config(0), name(aName), netInput(0), netOutput(0), running(false), hasIncoming(false), TAG("PNode "), observer(o)
   {
      LOG(INFO) << TAG << "Creating ProcessorNode.";
      handlers.push_back(new PingHandler(*this));
   }
   
   /** Destructor cleans all the internal objects of the Node when it is destroyed. */
   ProcessorNode::~ProcessorNode() {
      LOG(INFO) << TAG << "Destroying ProcessorNode...";
      while (!handlers.empty()) {
         delete handlers.front();
         handlers.pop_front();
      }
      delete config;
      // Close and destroy the sending network object
      if (netOutput) {
         if (netOutput->isRunning())
            netOutput->stop();
         delete netOutput;
      }
      if (netInput) {
         if (netInput->isRunning())
            netInput->stop();
         delete netInput;
      }
      LOG(INFO) << TAG << "..ProcessorNode destroyed.";
   }
   
   
   bool ProcessorNode::configure(const std::string & configFile) {
      bool success = false;
      if (configFile.length() > 0) {
         showUIMessage("------ > Configuring node...");
         delete config;
         config = nullptr;
         config = new NodeConfiguration();
         ConfigurationFileReader reader(*config);
         if (reader.read(configFile)) {
            std::string cvalue = config->getValue(ConfigurationDataItem::CONF_INPUTADDR);
            setInputSource(cvalue);
            cvalue = config->getValue(ConfigurationDataItem::CONF_OUTPUTADDR);
            setOutputSink(cvalue);
            cvalue = config->getValue(ConfigurationDataItem::CONF_INPUTFILE);
            setDataFileName(cvalue);
            cvalue = config->getValue(ConfigurationDataItem::CONF_OUTPUTFILE);
            setOutputFileName(cvalue);
            showUIMessage("------ > Configured");
            success = true;
         } else {
            delete config;
            config = nullptr;
         }
      }
      return success;
   }
   
   /** Sets the address of the input source for the Node. This is the hostname and port where
    data is read. In practice, the host is always the local host, 127.0.0.1. Node listens
    for arrivind data from this port and then handles it using the DataHandler objects.
    @param hostName The host name, e.g. "127.0.0.1:1234". */
   void ProcessorNode::setInputSource(const std::string & hostName) {
      if (netInput) {
         delete netInput;
         netInput = nullptr;
      }
      if (hostName.length() && hostName != "null") {
         LOG(INFO) << TAG << "Setting input: " << hostName;
         std::string msg = "Reading data from " + hostName;
         showUIMessage(msg);
         netInput = new NetworkReader(hostName, *this, io_service);
      } else {
         showUIMessage("This node has no previous node to read data from.");
      }
   }
   
   /** Sets the address of the output sink for the Node. This is the hostname and port where
    data is written to.
    @param hostName The host name, e.g. "127.0.0.1:1234" or "130.231.44.121:1234". */
   void ProcessorNode::setOutputSink(const std::string & hostName) {
      // Create a new Network object for sending data to the datagram socket.
      if (netOutput) {
         delete netOutput;
         netOutput = nullptr;
      }
      if (hostName.length() && hostName != "null") {
         LOG(INFO) << TAG << "Setting output: " << hostName;
         std::string msg = "Sending data to " + hostName;
         showUIMessage(msg);
         netOutput = new NetworkWriter(hostName, io_service);
      } else {
         showUIMessage("This node has no next node to send data to.");
      }
   }
   
   
   /** Sets the address of the input source for the Node. This is the hostname and port where
    data is read. In practice, the host is always the local host, 127.0.0.1. Node listens
    for arrivind data from this port and then handles it using the DataHandler objects.
    @param hostName The host name, e.g. "127.0.0.1".
    @param portNumber The port number to listen to, e.g. 1234. */
   void ProcessorNode::setInputSource(const std::string & hostName, int portNumber) {
      if (netInput) {
         delete netInput;
         netInput = nullptr;
      }
      if (hostName.length() && hostName != "null") {
         LOG(INFO) << TAG << "Setting input: " << hostName << ":" << portNumber;
         std::string msg = "Reading data from " + hostName;
         showUIMessage(msg);
         netInput = new NetworkReader(hostName, portNumber, *this, io_service);
      } else {
         showUIMessage("This node has no previous node to read data from.");
      }
   }
   
   /** Sets the address of the output sink for the Node. This is the hostname and port where
    data is written to.
    @param hostName The host name, e.g. "127.0.0.1:1234" or "130.231.44.121:1234".
    @param portNumber The port number to listen to, e.g. 1234. */
   void ProcessorNode::setOutputSink(const std::string & hostName, int portNumber) {
      // Create a new Network object for sending data to the datagram socket.
      if (netOutput) {
         delete netOutput;
         netOutput = nullptr;
      }
      if (hostName.length() && hostName != "null") {
         LOG(INFO) << TAG << "Setting output: " << hostName << ":" << portNumber;
         std::string msg = "Sending data to " + hostName;
         showUIMessage(msg);
         netOutput = new NetworkWriter(hostName, portNumber, io_service);
      } else {
         showUIMessage("This node has no next node to send data to.");
      }
   }
   
   
   /** The node can be configured to do various activities by implementing different kinds
    of DataHandlers. These handlers can then be added to the Node, usually at the startup of
    the application, in the main() function.
    @param h The DataHandler to add to the Node. */
   void ProcessorNode::addHandler(DataHandler * h) {
      handlers.push_back(h);
   }
   
   
   /** For querying the name of the Node.
    @return The name of the Node. */
   const std::string & ProcessorNode::getName() const {
      return name;
   }
   
   /** A Node can read input from a data file. The file is read by giving the command "readfile"
    in the Node's start loop.
    @param fileName The name of the file to read. */
   void ProcessorNode::setDataFileName(const std::string & fileName) {
      dataFileName = fileName;
      if (dataFileName.length() > 0) {
         showUIMessage("Node uses local input data file: " + fileName);
      } else {
         showUIMessage("Node has no local data input file.");
      }
   }
   
   
   /** For getting the name of the data file Node is reading input data from.
    @return The file name to read data from. */
   const std::string & ProcessorNode::getDataFileName() const {
      return dataFileName;
   }
   
   /** A Node can write output to a data file. The file is written to the file usually
    by a DataHandler who is is handling incoming data, often combined to data read from a data file.
    param fileName The name of the file to write to. */
   void ProcessorNode::setOutputFileName(const std::string & fileName) {
      outputFileName = fileName;
      showUIMessage("Node writes to local output data file: " + fileName);
   }
   
   
   /** For getting the name of the data file Node is writing output data to.
    @return The file name to write data to. */
   const std::string & ProcessorNode::getOutputFileName() const {
      return outputFileName;
   }
   
   bool ProcessorNode::isRunning() const {
      return running;
   }
   
   
   /** Starts the Node. This includes starting the network reader and/or writer for
    communicating to other Nodes, starting the data handling thread, and looping in the
    main thread to handle user commands. The start loop runs until shutdown command is given
    from the keyboard or one arrives from the previous node. */
   void ProcessorNode::start() {
      // Start the listening network reader
      showUIMessage("------ > Starting the node " + name);
      if (netInput) {
         LOG(INFO) << TAG << "Start the input reader";
         netInput->start();
      }
      // Start the sending network object
      if (netOutput) {
         LOG(INFO) << TAG << "Start the output writer";
         netOutput->start();
      }
      LOG(INFO) << TAG << "Start the receive handler thread...";
      running = true;
      if (netInput) {
         threader = std::thread(&ProcessorNode::threadFunc, this);
      }
      
      showUIMessage("Starting io service thread.");
      new std::thread([this] {return io_service.run();} );
      
      //      showUIMessage("Starting console thread.");
      //      new std::thread([this] {
      //         while (running) {
      //            showUIMessage("Enter command (ping, readfile, quit or shutdown) > ");
      //            getline(std::cin, command);
      //            LOG(INFO) << TAG << "------ > User command: " << command;
      //            condition.notify_all();
      //            if (command == "quit") {
      //               return; // running = false;
      //            }
      //         }
      //      });
      
      showUIMessage("Starting command handling loop.");
      new std::thread([this] {
         while (running && ((netInput && netInput->isRunning()) || (netOutput && netOutput->isRunning())))
         {
            
            {
               std::unique_lock<std::mutex> ulock(guard);
               condition.wait(ulock, [this] {
                  std::string cmd = command;
                  Package p;
                  if (running) {
                     if (cmd == "ping") {
                        p.setType(Package::Control);
                        p.setData(cmd);
                        sendData(p);
                        showUIMessage("Ping sent to next node (if any).");
                     } else if (cmd == "readfile") {
                        if (dataFileName.length() > 0) {
                           LOG(INFO) << TAG << "Reading data file.. " << dataFileName;
                           showUIMessage("Reading file " + dataFileName);
                           p.setType(Package::Control);
                           p.setData(cmd);
                           passToHandlers(p);
                        } else {
                           showUIMessage("No data file specified in configuration for this node.");
                        }
                     } else if (cmd == "quit" || cmd == "shutdown") {
                        if (cmd == "shutdown") {
                           p.setType(Package::Control);
                           p.setData(cmd);
                           sendData(p);
                           showUIMessage("Sent the shutdown command to next node (if any).");
                        }
                        showUIMessage("Initiated quitting of this node...");
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        running = false;
                        condition.notify_all();
                        // stop();
                     }
                  }
                  return !running;
               });
            }
         }
      });
      // stop();
   }
   
   /** Stops the Node. This includes closing and destroying the network reader and/or writer
    and setting the running flag to false. These operations finishes the data handling thread
    as well as the main thread loop running in the start() method. */
   void ProcessorNode::stop() {
      running = false;
      showUIMessage("Stopping the node...");
      LOG(INFO) << TAG << "Stopping input...";
      if (netInput && netInput->isRunning()) {
         netInput->stop();
      }
      LOG(INFO) << TAG << "Stopped input, now stopping output...";
      // Close and destroy the sending network object
      if (netOutput && netOutput->isRunning()) {
         netOutput->stop();
      }
      LOG(INFO) << TAG << "Stopped output, stopping io service...";
      io_service.stop();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      LOG(INFO) << TAG << "Notify all";
      condition.notify_all();
      LOG(INFO) << TAG << "...exiting ProcessorNode::stop";
      showUIMessage("...Node stopped.");
   }
   
   void ProcessorNode::handleCommand(const std::string & aCommand) {
      command = aCommand;
      condition.notify_all();
   }
   
   /** Method sends the data to the next node by using the NetworkWriter object.
    @param data The data package to send to the next Node. */
   void ProcessorNode::sendData(const Package & data) {
      if (netOutput) {
         showUIMessage("Sending a package of type " + data.getTypeAsString());
         LOG(INFO) << TAG << "Telling network writer to send a package.";
         netOutput->write(data);
      }
   }
   
   /** Some handlers in Node need to pass packages they handled to the <strong>next</strong>
    handlers in the list of handlers. This includes handlers that read items from a file,
    and the data items read need to be forwarded to the next handlers.<p>
    The implicit assumption is that previous handlers do not do anything relevant to the content
    read by the handler in question, but the next ones do. See documentation of the handlers member
    variable for details.
    @param current The current handler, after which come the handlers that are offered this package.
    @param package The Package to offer to the following Handlers.
    */
   void ProcessorNode::passToNextHandlers(const DataHandler * current, Package & package) {
      bool found = false;
      for (std::list<DataHandler*>::iterator iter = handlers.begin(); iter != handlers.end(); iter++) {
         LOG(INFO) << TAG << "Offering data to next Handler...";
         if (!found && current == *iter) {
            found = true;
            LOG(INFO) << TAG << "Found current handler....";
            continue;
         }
         if (found) {
            LOG(INFO) << TAG << "..so offering the package to the rest.";
            if ((*iter)->consume(package)) {
               LOG(INFO) << TAG << "Consumer returned true, not offering forward anymore";
               break;
            }
         }
      }
   }
   
   /**
    Node's thread function runs in a loop and waits for incoming data packages from the
    NetworkReader. mutexes and condition variables are used to notify of such situation
    as well as guard the incoming message queue. As packages arrive, the function
    passes the package to Handlers. It also checks if the package is a shutdown control
    message and if that is so, the run loop is stopped.
    */
   void ProcessorNode::threadFunc() {
      while (running) {
         LOG(INFO) << TAG << "Receive queue empty, waiting...";
         
         {
            std::unique_lock<std::mutex> ulock(guard);
            condition.wait(ulock, [this] { return this->hasIncoming || !running; });
         }
         
         if (running) {
            LOG(INFO) << TAG << "Received a package!";
            showUIMessage("Received data from previous node.");
            if (netInput) {
               guard.lock();
               Package package = netInput->read();
               guard.unlock();
               showUIMessage("Package id: " + boost::uuids::to_string(package.getUuid()));
               LOG(INFO) << TAG << "Received package: " << package.getTypeAsString() << ":" << package.getData();
               while (!package.isEmpty() && running) {
                  showUIMessage("Package type was " + package.getTypeAsString());
                  if (package.getType() == Package::Control && package.getData() == "shutdown") {
                     showUIMessage("Got shutdown command, forwarding and initiating shutdown.");
                     sendData(package);
                     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                     // TODO check if code in comments (modified?) would work without sig abort 6, as stop() does cause it.
                     //							running = false;
                     //							condition.notify_all();
                     //							break;
                     command = "quit";
                     condition.notify_all();
                     break;
                     // stop();
                  } else {
                     if (package.getType() == Package::Control) {
                        showUIMessage("Control package arrived with command " + package.getData());
                     }
                     passToHandlers(package);
                     if (netInput) {
                        package = netInput->read();
                     }
                  }
               }
            }
            hasIncoming = false;
         }
      }
      LOG(INFO) << TAG << "Exit incoming data handler thread in ProcessorNode!";
   }
   
   /** This method takes the incoming data and passes it to be handled by the
    DataHandler objects in the Node. The data is given to all Handlers until one
    returns true, indicating that the package has been handled and should not be passed
    ahead anymore. A Handler can of course handle the package and still return false,
    enabling multiple handlers for a single package.
    @param package The data package to handle. */
   void ProcessorNode::passToHandlers(Package & package) {
      LOG(INFO) << TAG << "Handlers count: " << handlers.size();
      for (std::list<DataHandler*>::iterator iter = handlers.begin(); iter != handlers.end(); iter++) {
         LOG(INFO) << TAG << "Offering data to next Handler...";
         if ((*iter)->consume(package)) {
            LOG(INFO) << TAG << "Handler returned true, not offering forward anymore";
            break;
         }
      }
   }
   
   
   // From NetworkReaderObserver:
   /** Implements the NetworkReaderObserver interface. NetworkReader calls this interface
    method when data has been received from the previous Node. The Node then notifies the
    data handling thread (running the threadFunc()) which executes and handles the incoming
    data. */
   void ProcessorNode::receivedData() {
      LOG(INFO) << TAG << "Processor has incoming data!";
      hasIncoming = true;
      condition.notify_all();
   }
   
   void ProcessorNode::showUIMessage(const std::string & message) {
      std::cout << message << std::endl;
      if (observer != nullptr) {
         observer->NodeEventHappened(ProcessorNodeObserver::EventType::UINotification, message);
      }
   }
   
} //namespace

