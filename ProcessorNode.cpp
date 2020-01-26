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

#include <ProcessorNode/ProcessorNode.h>
#include <ProcessorNode/NetworkReader.h>
#include <ProcessorNode/NetworkWriter.h>
#include <ProcessorNode/PingHandler.h>
#include <ProcessorNode/DataFileReader.h>
#include <ProcessorNode/NodeConfiguration.h>
#include <ProcessorNode/ConfigurationFileReader.h>
#include <ProcessorNode/ConfigurationHandler.h>

namespace OHARBase {

const std::string ProcessorNode::TAG{"PNode "};
/** Empty string, used when reference to string must be returned but no value is stored: return this one in those cases. */
const std::string KNullString{""};

/** Constructor for the processor node.
 @param obs The observer of the node who gets event and error notifications of activities in the node. */
ProcessorNode::ProcessorNode(ProcessorNodeObserver * obs)
: config(nullptr), networkReader(nullptr), networkWriter(nullptr), configReader(nullptr), configWriter(nullptr), running(false), nodeInitiatedShutdownStarted(false), incomingHandlerThread(nullptr), ioServiceThread(nullptr),
   commandHandlerThread(nullptr), hasIncoming(false), observer(obs)
{
   LOG(INFO) << TAG << "Creating ProcessorNode.";
   handlers.push_back(new PingHandler(*this));
   handlers.push_back(new ConfigurationHandler(*this));
}

/** Destructor cleans all the internal objects of the Node when it is destroyed. */
ProcessorNode::~ProcessorNode() {
   LOG(INFO) << TAG << "Destroying ProcessorNode...";
   try {
      while (!handlers.empty()) {
         delete handlers.front();
         handlers.pop_front();
      }
      delete config;
      // Close and destroy the sending network object
      if (networkWriter) {
         if (networkWriter->isRunning())
            networkWriter->stop();
         delete networkWriter;
      }
      if (networkReader) {
         if (networkReader->isRunning())
            networkReader->stop();
         delete networkReader;
      }
      if (configReader) {
         if (configReader->isRunning()) {
            configReader->stop();
         }
         delete configReader;
      }
      if (configWriter) {
         if (configWriter->isRunning()) {
            configWriter->stop();
         }
      }
      if (ioServiceThread && ioServiceThread->joinable()) {
         LOG(INFO) << TAG << "Waiting for the ioServiceThread thread...";
         ioServiceThread->detach();
         delete ioServiceThread; ioServiceThread = nullptr;
      }

   } catch (const std::exception & e) {
      LOG(INFO) << "EXCEPTION in destroying processornode!";
   }
   LOG(INFO) << TAG << "..ProcessorNode destroyed.";
}

// MARK: Configuration

/**
 Configures the node using the provided configuration file. For file details, see
 documentation of the project and ConfigurationFileReader class.
 @param configFile The name of the file where configuration is read from.
 @return True if configuration was done successfully, false otherwise.
 */
bool ProcessorNode::configure(const std::string & configFile) {
   bool success = false;
   if (configFile.length() > 0) {
      try {
         showUIMessage("------ > Configuring node...");
         delete config;
         config = nullptr;
         config = new NodeConfiguration();
         ConfigurationFileReader reader(*config);
         if (reader.read(configFile)) {
            showUIMessage("Configuration for node:");
            std::string cvalue = config->getValue(ConfigurationDataItem::CONF_INPUTADDR);
            setInputSource(cvalue);
            cvalue = config->getValue(ConfigurationDataItem::CONF_CONFINADDR);
            setConfigurationInputSource(cvalue);
            cvalue = config->getValue(ConfigurationDataItem::CONF_OUTPUTADDR);
            setOutputSink(cvalue);
            cvalue = config->getValue(ConfigurationDataItem::CONF_CONFOUTADDR);
            createConfigurationOutputWriter();
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
      } catch (const std::exception & e) {
         std::stringstream sstream;
         sstream << "ERROR Could not configure the node with config " << configFile << " because " << e.what();
         logAndShowUIMessage(sstream.str(), ProcessorNodeObserver::EventType::ErrorEvent);
      }
   }
   return success;
}

/**
 Gets a value for a configuration name.
 @param itemName Configuration name, for which a value is requested.
 @return The value for the configuration item.
 @throws If there is no configuration, throws a runtime error exception.
 */
std::string ProcessorNode::getConfigItemValue(const std::string & itemName) const {
   if (!config) {
      throw std::runtime_error("No configuration.");
   }
   return config->getValue(itemName);
}

/** Sets the address of the input source for the Node. This is the port where
 data is read. Node listens for arrivind data from this port and then handles it using the DataHandler objects.
 @param port The port number to listen to, e.g. "1234". */
void ProcessorNode::setInputSource(const std::string & port) {
   if (networkReader) {
      delete networkReader;
      networkReader = nullptr;
   }
   if (port.length() && port != "null") {
      std::stringstream sstream;
      sstream << "Reading data from port " << port;
      logAndShowUIMessage(sstream.str());
      int iPort = std::stoi(port);
      networkReader = new NetworkReader(iPort, *this, io_service);
   } else {
      showUIMessage("This node has no previous node to read data from.");
   }
}

/** Sets the address of the configuration source for the Node. This is the port where
 config data is read. Node listens for arrivind configuration messages from this port
 and then handles it using the ConfigurationHandler object.
 @param port The host name, e.g. "1234". */
void ProcessorNode::setConfigurationInputSource(const std::string & port) {
   if (configReader) {
      delete configReader;
      configReader = nullptr;
   }
   if (port.length() && port != "null") {
      std::stringstream sstream;
      sstream << "Reading configurations from port " << port;
      logAndShowUIMessage(sstream.str());
      int iPort = std::stoi(port);
      // Note the last param: configreader shares the listening port with other nodes,
      // because several nodes may run on the same machine and all nodes should listen
      // to the same port for config broadcast messages. Value tells the reader to reuse the port
      // sharing it with other nodes running possibly on the same machine.
      configReader = new NetworkReader(iPort, *this, io_service, true);
   } else {
      showUIMessage("This node has no configuration port to read config messages from.");
   }
}

/** Creates the configuration output writer object. The address to send data is not known and not needed at this point.
 Instead, the writer takes the Configurator address from the incoming configuration message and replies to
 that same address. That address is placed to the Package as origin address, and set to as the destination address of the package when sending it ahead. Thus, the NetworkWriter sees the package has a destination address and uses that, always.
 The confifguration output writer is not created if the "usual" writer is created, since then it can be used to send
 also the configuration responses to the Configurator. */
void ProcessorNode::createConfigurationOutputWriter() {
   // Check that we do not have the writer already created...
   if (!networkWriter) {
      // And that the configured output address does not exist (and writer not yet created)...
      std::string writerHost = getConfigItemValue(ConfigurationDataItem::CONF_OUTPUTADDR);
      if (writerHost.length() == 0 || writerHost == "null") {
         LOG(INFO) << "No usual networkWriter in this Node so creating a config writer.";
         // OK there should be no writer, so we need to create the config writer to be able to send config responses.
         if (configWriter) {
            delete configWriter;
            configWriter = nullptr;
         }
         
         // The host and port number does not matter since config Packages always must contain the address where to
         // send the configuration responses.
         configWriter = new NetworkWriter("localhost", 12345, io_service);
      }
   }
}

/**
 If you need to read the configuration information of the ProcessorNode, call this method.
 @return The node configuration.
 */
const NodeConfiguration & ProcessorNode::getConfiguration() const {
   return *config;
}

/** Sets the address of the output sink for the Node. This is the hostname and port where
 data is written to.
 @param hostName The host name, e.g. "127.0.0.1:1234" or "130.231.44.121:1234". */
void ProcessorNode::setOutputSink(const std::string & hostName) {
   // Create a new Network object for sending data to the datagram socket.
   if (networkWriter) {
      delete networkWriter;
      networkWriter = nullptr;
   }
   if (hostName.length() && hostName != "null") {
      std::stringstream sstream;
      sstream << "Sending data to " << hostName;
      showUIMessage(sstream.str());
      networkWriter = new NetworkWriter(hostName, io_service);
   } else {
      showUIMessage("This node has no next node to send data to.");
   }
}

/** Sets the address of the output sink for the Node. This is the hostname and port where
 data is written to.
 @param hostName The host name, e.g. "127.0.0.1:1234" or "130.231.44.121:1234".
 @param portNumber The port number to listen to, e.g. 1234. */
void ProcessorNode::setOutputSink(const std::string & hostName, int portNumber) {
   // Create a new Network object for sending data to the datagram socket.
   if (networkWriter) {
      delete networkWriter;
      networkWriter = nullptr;
   }
   if (hostName.length() && hostName != "null") {
      std::stringstream sstream;
      sstream << "Sending data to host " << hostName << ":" << portNumber;
      logAndShowUIMessage(sstream.str());
      networkWriter = new NetworkWriter(hostName, portNumber, io_service);
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


/** A Node can read input from a data file. The file is read by giving the command "readfile"
 in the Node's start loop.
 @param fileName The name of the file to read. */
void ProcessorNode::setDataFileName(const std::string & fileName) {
   dataFileName = fileName;
   std::stringstream sstream;
   if (dataFileName.length() > 0) {
      sstream << "Node uses local input data file: " << fileName;
   } else {
      sstream << "Node has no local data input file.";
   }
   logAndShowUIMessage(sstream.str());
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
   std::stringstream sstream;
   if (outputFileName.length() > 0) {
      sstream << "Node uses local output data file: " << fileName;
   } else {
      sstream << "Node has no local data output file.";
   }
   logAndShowUIMessage(sstream.str());
}


/** For getting the name of the data file Node is writing output data to.
 @return The file name to write data to. */
const std::string & ProcessorNode::getOutputFileName() const {
   return outputFileName;
}

/** Used to query if the node is running or not (start() has been successfully called).
 @return Returns true if node is running. */
bool ProcessorNode::isRunning() const {
   return running;
}

// MARK: Starting the node

/** Starts the Node. This includes starting the network reader and/or writer for
 communicating to other Nodes, starting the data handling thread, and looping in a separate
 thread to handle user commands. After successfully starting the node, method returns to
 caller and the ProcessorNode threads handle commands and incoming data processing. */
void ProcessorNode::start() {
   
   /*
    Steps made here to start the node (can be used to draw an UML activity diagram):
    - check if netinput exists
    if yes, start it
    - check if netoutput exist
    if yes, start it
    - node is now running
    - check again if netinput exist
    if yes, start the incoming handler thread  (runs the threadFunc method, check it out)
    - start the ioservice thread (needed in using boost::asio for networking)
    - start the command handling thread
    And that is it; node has been started.
    */
   if (running) return;
   
   nodeInitiatedShutdownStarted = false;
   
   try {
      running = true;
      // Start the listening network reader
      showUIMessage("------ > Starting the node " + config->getValue(ConfigurationDataItem::CONF_NODENAME));
      if (networkReader) {
         LOG(INFO) << TAG << "Start the input reader";
         networkReader->start();
      }
      if (configReader) {
         LOG(INFO) << "Start the configuration reader";
         configReader->start();
      }
      if (configWriter) {
         LOG(INFO) << "Start the config writer.";
         configWriter->start();
      }
      // Start the sending network object
      if (networkWriter) {
         LOG(INFO) << TAG << "Start the output writer";
         networkWriter->start();
      }
      if (networkReader) {
         LOG(INFO) << TAG << "Start the network receive handler thread...";
         incomingHandlerThread = new std::thread(&ProcessorNode::threadFunc, this);
      }
      
      LOG(INFO) << "Starting io service thread.";
      if (!ioServiceThread) {
         ioServiceThread = new std::thread([this]
         {
            boost::asio::io_context::count_type count = io_service.run();
            LOG(INFO) << "io_service run exited with count " << count;
            return count;
         } );
      }
   } catch (const std::exception & e) {
      stop();
      std::stringstream sstream;
      sstream << "ERROR Something went wrong in starting the node's networking components: " << e.what();
      logAndShowUIMessage(sstream.str(), ProcessorNodeObserver::EventType::ErrorEvent);
      return;
   }
   
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
   
   LOG(INFO) << "Starting command handling loop.";
   /*
    What happens here (can be used to draw the activity diagram):
    - while the node is running:
    - waits for commands; to be awoken (by someone calling condition.notify_all).
    - this may take time (minutes, hours, days,...) -- until a command arives from the user or network package.
    - when awoken, then check what the command was:
    - if it is "ping" command then
    create a package and send the package with the ping command to next node.
    show a message in the UI that a ping message came and was sent away too.
    - if it is "readfile" command then
    create a package and send the package with the readfile command to next node.
    pass the package with the command to handlers. Maybe one of them will handle it and do the actual file reading.
    - if it is "quit" or "shutdown" then
    - if it is "shutdown" then
    send the shutdown command ahead with a package to the next node.
    - end if
    - then stop the node from running and
    - notify the user that node will close and
    - notify all other threads that they should also start packing the whistles in bags
    - go to start (to while...)
    */
   //MARK: Command handler thread.
   commandHandlerThread = new std::thread([this] {
      while (running)
      {
         
         {
            std::unique_lock<std::mutex> ulock(guard);
            condition.wait(ulock, [this] {
               commandGuard.lock();
               std::string cmd = command;
               command = "";
               commandGuard.unlock();
               if (cmd.length() > 0) {
                  LOG(INFO) << "Command received: " << cmd;
                  try {
                     Package p;
                     if (running) {
                        if (cmd == "ping") {
                           p.setType(Package::Control);
                           p.setPayload(cmd);
                           sendData(p);
                           showUIMessage("Ping sent to next node (if any).");
                        } else if (cmd == "readfile") {
                           queuePackageCounts.clear();
                           if (dataFileName.length() > 0) {
                              LOG(INFO) << TAG << "Got a read command to read a data file. " << dataFileName;
                              showUIMessage("Handling command to read a file " + dataFileName);
                              p.setType(Package::Control);
                              p.setPayload(cmd);
                              passToHandlers(p);
                           } else {
                              showUIMessage("Readfile command came, but no data file specified for this node.");
                           }
                        } else if (cmd == "quit" || cmd == "shutdown") {
                           if (cmd == "shutdown") {
                              p.setType(Package::Control);
                              p.setPayload(cmd);
                              sendData(p);
                              logAndShowUIMessage("Sent the shutdown command to next node (if any).");
                           }
                           logAndShowUIMessage("Initiated quitting of this node...");
                           running = false;
                           nodeInitiatedShutdownStarted = true;
                           condition.notify_all();
                           std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                     }
                  } catch (const std::exception & e) {
                     std::stringstream sstream;
                     sstream << "ERROR Something went wrong in node's command handling loop: " << e.what();
                     logAndShowUIMessage(sstream.str(), ProcessorNodeObserver::EventType::ErrorEvent);
                  }
               }
               
               return command.length() > 0 || !running; // !running;
            });
         }
      }
      if (nodeInitiatedShutdownStarted) {
         LOG(INFO) << "Got shutdown package so asking client app to shut down.";
         stop();
      }
      LOG(INFO) << "Exiting command handler thread.";
   });
   LOG(INFO) << "Exiting the ProcessorNode::start().";
}

/** Stops the Node. This includes closing and destroying the network reader and/or writer
 and setting the running flag to false. These operations finishes the threads
 started in the start() method. */
void ProcessorNode::stop() {
   showUIMessage("Stopping the node...");
   running = false;
   LOG(INFO) << TAG << "Notify all";
   condition.notify_all();
//   LOG(INFO) << TAG << "Stopping io service...";
//   if (!io_service.stopped()) {
//      io_service.stop();
//   }
   if (networkReader && networkReader->isRunning()) {
      LOG(INFO) << TAG << "Stopping input...";
      networkReader->stop();
      LOG(INFO) << TAG << "Stopped input";
   }
   if (configReader && configReader->isRunning()) {
      LOG(INFO) << TAG << "Stopping config reader...";
      configReader->stop();
      LOG(INFO) << TAG << "Stopped config reader";
   }
   if (configWriter && configWriter->isRunning()) {
      LOG(INFO) << TAG << "Stopping config writer...";
      configWriter->stop();
      LOG(INFO) << TAG << "Stopped config writer";
   }
   // Close and destroy the sending network object
   if (networkWriter && networkWriter->isRunning()) {
      LOG(INFO) << TAG << "Stopping output...";
      networkWriter->stop();
      LOG(INFO) << TAG << "Stopped output";
   }
   LOG(INFO) << TAG << "Input & Output stopped, now a pause.";
   // Pause the calling thread to allow node & network threads to finish their jobs.
   std::this_thread::sleep_for(std::chrono::milliseconds(100));
   LOG(INFO) << TAG << "And after pause, detach from the threads and destroy them.";
   if (incomingHandlerThread && incomingHandlerThread->joinable()) {
      LOG(INFO) << TAG << "Waiting for the incomingHandlerThread thread...";
      incomingHandlerThread->detach();
      delete incomingHandlerThread; incomingHandlerThread = nullptr;
   }
   if (commandHandlerThread && commandHandlerThread->joinable()) {
      LOG(INFO) << TAG << "Waiting for the commandHandlerThread thread...";
      commandHandlerThread->detach();
      delete commandHandlerThread; commandHandlerThread = nullptr;
   }
   
   LOG(INFO) << TAG << "...threads finished, exiting ProcessorNode::stop";
   
   showUIMessage("...Node stopped.");
   if (nodeInitiatedShutdownStarted) {
      initiateClientAppShutdown();
   }
}

/** Handles a command from the user/app. Command handling is processed in a dedicated thread
 started in the start() method.
 @param aCommand The command received from the user/app. */
void ProcessorNode::handleCommand(const std::string & aCommand) {
   commandGuard.lock();
   command = aCommand;
   commandGuard.unlock();
   LOG(INFO) << "Received a command " << command;
   condition.notify_all();
   // Update send queue status here too since writer does not notify Node when sending has been done.
   if (networkWriter) {
      updatePackageCountInQueue("net-out", networkWriter->packagesInQueue());
   }
}

/** Method sends the data to the next node by using the NetworkWriter object.
 @param data The data package to send to the next Node. */
void ProcessorNode::sendData(const Package & data) {
   if (networkWriter) {
      showUIMessage("Sending a package of type " + data.getTypeAsString());
      LOG(INFO) << TAG << "Telling network writer to send a package.";
      networkWriter->write(data);
      updatePackageCountInQueue("net-out", networkWriter->packagesInQueue());
   // If node hasn't got the next node, it uses the config writer to send config packages (only).
   } else if (configWriter && data.getType() == Package::Configuration) {
      showUIMessage("Sending configuration response message to Configurator.");
      LOG(INFO) << "No networkWriter so using configWriter to send a response to Configurator";
      configWriter->write(data);
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
   // Find the current handler from the container.
   auto iter = std::find(std::begin(handlers), std::end(handlers), current);
   // if it was found, advance to the next handler.
   if (iter != handlers.end()) {
      iter++;
      // If there are more handlers, let them consume the package.
      if (iter != handlers.end()) {
         std::all_of(iter, std::end(handlers), [&](DataHandler * handler) {
            if (handler->consume(package)) {
               return false;  // Stop if the handler thinks it should not be handled by rest of the handlers.
            }
            return true; // Continue passing the Package to the next handler.
         });
      }
   }
}

/**
 Node's thread function runs in a loop and waits for incoming data packages from the
 NetworkReader. mutexes and condition variables are used to notify of such situation
 as well as guard the incoming message queue. As packages arrive, the function
 passes the package to Handlers. It also checks if the package is a shutdown control
 message and if that is so, shutdown message is first sent to the following node (if any),
 and the quit command is passsed to the command handling thread to shut down the node.
 */
//MARK: incomingHandlerThread
void ProcessorNode::threadFunc() {
   /*
    What is happening here in this thread...
    - while (1) the thread (node) is running:
    - wait for someone to wake the thread (someone calls condition.notify_all)...
    - waiting is over. Check if we are still running
    (someone might have set the running to false while waiting the thread to be awoken)
    - if running, then get the next package from the network reader
    - while (2) the package is not empty (and we are still running)
    if the package is a control package and the command is "shutdown"
    - send the shutdown command ahead
    - set the current command to be "quit"
    - awaken the other threads (especially command handler; it will then handle the quit command)
    - break; stop handling any more packages since we are closing the shop anyways.
    else
    - pass the package to handlers
    - get the next package
    loop to while (2) to check if the package was empty
    - loop to while (1) to wait for further notifications of incoming packags.
    */
   while (running) {
      LOG(INFO) << TAG << "Receive queue empty, waiting...";
      {
         // Wait until the condition variable is notified that something happened.
         std::unique_lock<std::mutex> ulock(guard);
         condition.wait(ulock, [this] { return this->hasIncoming || !running; });
      }
      // OK, something happened so if we are still running, check if something came from the network.
      if (running) {
         if (configReader) {
            handlePackagesFrom(*configReader);
         }
         if (networkReader) {
            handlePackagesFrom(*networkReader);
            updatePackageCountInQueue("net-in", networkReader->packagesInQueue());
         }
         hasIncoming = false;
      }
   }
   LOG(INFO) << TAG << "Exit incoming data handler thread in ProcessorNode!";
}

void ProcessorNode::handlePackagesFrom(NetworkReader & reader) {
   Package package = reader.read();
   // If package is empty, nothing came.
   while (!package.isEmpty() && running) {
      LOG(INFO) << TAG << "Received a package!";
      showUIMessage("Received a package.");
      LOG(INFO) << TAG << "Received package: " << boost::uuids::to_string(package.getUuid()) << " " << package.getTypeAsString() << ":" << package.getPayloadString();
      if (package.getType() == Package::Control && package.getPayloadString() == "shutdown") {
         showUIMessage("Got shutdown command, forwarding and initiating shutdown.");
         sendData(package);
         std::this_thread::sleep_for(std::chrono::milliseconds(200));
         commandGuard.lock();
         command = "quit";
         commandGuard.unlock();
         condition.notify_all();
         // Do not handle possible remaining packages after shutdown message.
         break;
      } else {
         if (package.getType() == Package::Control) {
            queuePackageCounts.clear();
            showUIMessage("Control package arrived with command " + package.getPayloadString());
         }
         // Package was either data, configuration or control, so let the handlers handle it.
         passToHandlers(package);
         // Check if there are more packages to handle; handle them all while we are here.
         package = reader.read();
      }
   }
}

/** This method takes the incoming data and passes it to be handled by the
 DataHandler objects in the Node. The data is given to all Handlers until one
 returns true, indicating that the package has been handled and should not be passed
 ahead to next handlers anymore. A Handler can of course handle the package and still return false,
 enabling multiple handlers for a single package.
 @param package The data package to handle. */
void ProcessorNode::passToHandlers(Package & package) {
   LOG(INFO) << TAG << "Passing a package to handlers, count: " << handlers.size();
   try {
      for (DataHandler * handler : handlers) {
         LOG(INFO) << TAG << "Offering data to next Handler...";
         if (handler->consume(package)) {
            LOG(INFO) << TAG << "Handler returned true, not offering forward anymore";
            break;
         }
      }
   } catch (const std::exception & e) {
      std::stringstream sstream;
      sstream << "ERROR Something went wrong in handling a package: " << e.what() << " with id " << boost::uuids::to_string(package.getUuid());
      logAndShowUIMessage(sstream.str(), ProcessorNodeObserver::EventType::ErrorEvent);
   }
}

/**
 This method takes care of handling the statistics related to the Packages in different queues in the Node.
 Each queue has a name, and number of packages currently in the queue. This is updated when Packages arrive and leave.
 Status of the queues can then be updated in the UI by calling showUIMessage, with event QueueStatusEvent.
 */
void ProcessorNode::updatePackageCountInQueue(const std::string & queueName, int packageCount)  {
   queue_package_type::iterator iter = queuePackageCounts.find(queueName);
   if (iter != queuePackageCounts.end()) {
      std::pair<int,int> counts = queuePackageCounts[queueName];
      counts.first = packageCount;
      counts.second = std::max(counts.second, packageCount);
      queuePackageCounts[queueName] = counts;
   } else {
      queuePackageCounts[queueName] = {packageCount, packageCount};
   }
   std::stringstream packageStream;
   auto save = [&packageStream](const std::pair<std::string,std::pair<int,int>> & entry) {
      packageStream << entry.first << ":" << entry.second.first << ":" << entry.second.second << " ";
   };
   std::for_each(queuePackageCounts.begin(), queuePackageCounts.end(), save);
   showUIMessage(packageStream.str(), ProcessorNodeObserver::EventType::QueueStatusEvent);
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
// From NetworkReaderObserver:
/** Called by the NetworkReader when it could not parse/handle the incoming data.
 Not much can be done about it, than to log and notify app/user. Let them see what was
 wrong and do something about it.
 @param what The error message. */
void ProcessorNode::errorInData(const std::string & what) {
   std::stringstream sstream;
   sstream << "ERROR in incoming data; discarded " << what;
   LOG(WARNING) << sstream.str();
   showUIMessage(sstream.str(), ProcessorNodeObserver::EventType::ErrorEvent);
}

/** Notifies the node observer (assuming it is a (G)UI) of something.
 @param message The message to the user.
 @param e Type of the event. */
void ProcessorNode::showUIMessage(const std::string & message, ProcessorNodeObserver::EventType e) {
   if (observer != nullptr) {
      observer->NodeEventHappened(e, message);
   }
}

/** Notifies the node observer (assuming it is a (G)UI) of something and log it too.
 @param message The message to show to user.
 @param e Type of the event. */
void ProcessorNode::logAndShowUIMessage(const std::string & message, ProcessorNodeObserver::EventType e) {
   if (e == ProcessorNodeObserver::EventType::WarningEvent || e == ProcessorNodeObserver::EventType::ErrorEvent) {
      LOG(WARNING) << message;
   } else {
      LOG(INFO) << message;
   }
   showUIMessage(message);
}

/** Node wants to shut itself down so notify also the client app/ui so that
 it can close the app. Node has already (or is in the process of) closed itself. */
void ProcessorNode::initiateClientAppShutdown() {
   if (observer != nullptr) {
      observer->NodeEventHappened(ProcessorNodeObserver::EventType::ShutDownEvent, "Shutdown of node requested from network.");
   }
}
} //namespace

