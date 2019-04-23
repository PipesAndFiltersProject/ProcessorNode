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

namespace OHARBase {
   
   const std::string ProcessorNode::TAG{"PNode "};
                                         
   /** Constructor for the processor node.
    @param aName The name of the processor node.
    @param obs The observer of the node who gets event and error notifications of activities in the node. */
   ProcessorNode::ProcessorNode(const std::string & aName, ProcessorNodeObserver * obs)
   : config(nullptr), name(aName), networkReader(nullptr), networkWriter(nullptr), running(false),
   nodeInitiatedShutdownStarted(false), incomingHandlerThread(nullptr), ioServiceThread(nullptr),
   commandHandlerThread(nullptr), hasIncoming(false), observer(obs)
   {
      LOG(INFO) << TAG << "Creating ProcessorNode.";
      handlers.push_back(new PingHandler(*this));
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
      } catch (const std::exception & e) {
         LOG(INFO) << "EXCEPTION in destroying processornode!";
      }
      LOG(INFO) << TAG << "..ProcessorNode destroyed.";
   }
   
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
         } catch (const std::exception & e) {
            std::stringstream sstream;
            sstream << "ERROR Could not configure the node with config " << configFile << " because " << e.what();
            logAndShowUIMessage(sstream.str(), ProcessorNodeObserver::EventType::ErrorEvent);
         }
      }
      return success;
   }
   
   std::string ProcessorNode::getConfigItemValue(const std::string & itemName) const {
      if (!config) {
         throw std::runtime_error("No configuration.");
      }
      return config->getValue(itemName);
   }
   
   /** Sets the address of the input source for the Node. This is the hostname and port where
    data is read. In practice, the host is always the local host, 127.0.0.1. Node listens
    for arrivind data from this port and then handles it using the DataHandler objects.
    @param hostName The host name, e.g. "127.0.0.1:1234". */
   void ProcessorNode::setInputSource(const std::string & hostName) {
      if (networkReader) {
         delete networkReader;
         networkReader = nullptr;
      }
      if (hostName.length() && hostName != "null") {
         std::stringstream sstream;
         sstream << "Reading data from host " << hostName;
         logAndShowUIMessage(sstream.str());
         networkReader = new NetworkReader(hostName, *this, io_service);
      } else {
         showUIMessage("This node has no previous node to read data from.");
      }
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
   
   
   /** Sets the address of the input source for the Node. This is the hostname and port where
    data is read. In practice, the host is always the local host, 127.0.0.1. Node listens
    for arrivind data from this port and then handles it using the DataHandler objects.
    @param hostName The host name, e.g. "127.0.0.1".
    @param portNumber The port number to listen to, e.g. 1234. */
   void ProcessorNode::setInputSource(const std::string & hostName, int portNumber) {
      if (networkReader) {
         delete networkReader;
         networkReader = nullptr;
      }
      if (hostName.length() && hostName != "null") {
         std::stringstream sstream;
         sstream << "Reading data from host " << hostName << ":" << portNumber;
         logAndShowUIMessage(sstream.str());
         networkReader = new NetworkReader(hostName, portNumber, *this, io_service);
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
         // Start the listening network reader
         showUIMessage("------ > Starting the node " + name);
         if (networkReader) {
            LOG(INFO) << TAG << "Start the input reader";
            networkReader->start();
         }
         // Start the sending network object
         if (networkWriter) {
            LOG(INFO) << TAG << "Start the output writer";
            networkWriter->start();
         }
         
         running = true;
         if (networkReader) {
            LOG(INFO) << TAG << "Start the network receive handler thread...";
            incomingHandlerThread = new std::thread(&ProcessorNode::threadFunc, this);
         }
         
         LOG(INFO) << "Starting io service thread.";
         ioServiceThread = new std::thread([this] {return io_service.run();} );
         
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
         while (running && ((networkReader && networkReader->isRunning()) || (networkWriter && networkWriter->isRunning())))
         {
            
            {
               std::unique_lock<std::mutex> ulock(guard);
               condition.wait(ulock, [this] {
                  commandGuard.lock();
                  std::string cmd = command;
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
                  
                  return !running;
               });
            }
         }
         if (nodeInitiatedShutdownStarted) {
            LOG(INFO) << "Got shutdown package so asking client app to shut down.";
            stop();
         }
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
      LOG(INFO) << TAG << "Stopping io service...";
      if (!io_service.stopped()) {
         io_service.stop();
      }
      LOG(INFO) << TAG << "Stopping input...";
      if (networkReader && networkReader->isRunning()) {
         networkReader->stop();
      }
      LOG(INFO) << TAG << "Stopped input, now stopping output...";
      // Close and destroy the sending network object
      if (networkWriter && networkWriter->isRunning()) {
         networkWriter->stop();
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
      if (ioServiceThread && ioServiceThread->joinable()) {
         LOG(INFO) << TAG << "Waiting for the ioServiceThread thread...";
         ioServiceThread->detach();
         delete ioServiceThread; ioServiceThread = nullptr;
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
   }
   
   /** Method sends the data to the next node by using the NetworkWriter object.
    @param data The data package to send to the next Node. */
   void ProcessorNode::sendData(const Package & data) {
      if (networkWriter) {
         showUIMessage("Sending a package of type " + data.getTypeAsString());
         LOG(INFO) << TAG << "Telling network writer to send a package.";
         networkWriter->write(data);
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
            // Wait for the condition variable to be notified of something happening.
            std::unique_lock<std::mutex> ulock(guard);
            condition.wait(ulock, [this] { return this->hasIncoming || !running; });
         }
         // OK, something happened so if we are still running, check if something came from the network.
         if (running) {
            if (networkReader) {
               Package package = networkReader->read();
               // If package is empty, nothing came.
               while (!package.isEmpty() && running) {
                  LOG(INFO) << TAG << "Received a package!";
                  showUIMessage("Received data from previous node.");
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
                        showUIMessage("Control package arrived with command " + package.getPayloadString());
                     }
                     // Package was either data or control, so let the handlers handle it.
                     passToHandlers(package);
                     if (networkReader) {
                        // Check if there are more packages to handle; handle them all while we are here.
                        package = networkReader->read();
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
    ahead to next handlers anymore. A Handler can of course handle the package and still return false,
    enabling multiple handlers for a single package.
    TODO: Investigate if it is ok to multiple threads to call this method simultaneously. This happens now,
    since main thread (user), command handler as well as incoming data handler threads do all call it.
    @param package The data package to handle. */
   void ProcessorNode::passToHandlers(Package & package) {
      LOG(INFO) << TAG << "Passing a package to handlers, count: " << handlers.size();
      try {
         for (std::list<DataHandler*>::iterator iter = handlers.begin(); iter != handlers.end(); iter++) {
            LOG(INFO) << TAG << "Offering data to next Handler...";
            if ((*iter)->consume(package)) {
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

