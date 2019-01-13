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
        try {
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
        } catch (const std::exception & e) {
            LOG(INFO) << "EXCEPTION in destroying processornode!";
        }
        LOG(INFO) << TAG << "..ProcessorNode destroyed.";
    }
    
    
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
                logAndShowUIMessage(sstream.str());
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
            std::stringstream sstream;
            sstream << "Reading data from host " << hostName;
            logAndShowUIMessage(sstream.str());
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
            std::stringstream sstream;
            sstream << "Sending data to " << hostName;
            showUIMessage(sstream.str());
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
            std::stringstream sstream;
            sstream << "Reading data from host " << hostName << ":" << portNumber;
            logAndShowUIMessage(sstream.str());
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
            std::stringstream sstream;
            sstream << "Sending data to host " << hostName << ":" << portNumber;
            logAndShowUIMessage(sstream.str());
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
    
    bool ProcessorNode::isRunning() const {
        return running;
    }
    
    
    /** Starts the Node. This includes starting the network reader and/or writer for
     communicating to other Nodes, starting the data handling thread, and looping in the
     main thread to handle user commands. The start loop runs until shutdown command is given
     from the keyboard or one arrives from the previous node. */
    void ProcessorNode::start() {
        if (running) return;
        
        try {
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
            
            LOG(INFO) << TAG << "Start the network receive handler thread...";
            running = true;
            if (netInput) {
                incomingHandlerThread = new std::thread(&ProcessorNode::threadFunc, this);
            }
            
            LOG(INFO) << "Starting io service thread.";
            ioServiceThread = new std::thread([this] {return io_service.run();} );
            
        } catch (const std::exception & e) {
            stop();
            std::stringstream sstream;
            sstream << "ERROR Something went wrong in starting the node's networking components: " << e.what();
            logAndShowUIMessage(sstream.str(), true);
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
        commandHandlerThread = new std::thread([this] {
            bool shutDownOrdered = false;
            while (running && ((netInput && netInput->isRunning()) || (netOutput && netOutput->isRunning())))
            {
                
                {
                    std::unique_lock<std::mutex> ulock(guard);
                    condition.wait(ulock, [this, &shutDownOrdered] {
                        std::string cmd = command;
                        if (cmd.length() > 0) {
                            LOG(INFO) << "Command received: " << cmd;
                            try {
                                Package p;
                                if (running) {
                                    if (cmd == "ping") {
                                        p.setType(Package::Control);
                                        p.setData(cmd);
                                        sendData(p);
                                        showUIMessage("Ping sent to next node (if any).");
                                    } else if (cmd == "readfile") {
                                        if (dataFileName.length() > 0) {
                                            LOG(INFO) << TAG << "Got a read command to read a data file. " << dataFileName;
                                            showUIMessage("Handling command to read a file " + dataFileName);
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
                                            logAndShowUIMessage("Sent the shutdown command to next node (if any).");
                                        }
                                        logAndShowUIMessage("Initiated quitting of this node...");
                                        running = false;
                                        shutDownOrdered = true;
                                        condition.notify_all();
                                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                    }
                                }
                            } catch (const std::exception & e) {
                                std::stringstream sstream;
                                sstream << "ERROR Something went wrong in node's command handling loop: " << e.what();
                                logAndShowUIMessage(sstream.str(), true);
                            }
                        }
                        
                        return !running;
                    });
                }
            }
            if (shutDownOrdered) {
                LOG(INFO) << "Got shutdown package so asking client app to shut down.";
                stop();
                initiateClientAppShutdown();
            }
        });
        LOG(INFO) << "Exiting the ProcessorNode::start().";
    }
    
    /** Stops the Node. This includes closing and destroying the network reader and/or writer
     and setting the running flag to false. These operations finishes the data handling thread
     as well as the main thread loop running in the start() method. */
    void ProcessorNode::stop() {
        showUIMessage("Stopping the node...");
        if (running) {
            running = false;
            LOG(INFO) << TAG << "Notify all";
            condition.notify_all();
            LOG(INFO) << TAG << "Stopped output, stopping io service...";
            io_service.stop();
            LOG(INFO) << TAG << "Stopping input...";
            if (netInput && netInput->isRunning()) {
                netInput->stop();
            }
            LOG(INFO) << TAG << "Stopped input, now stopping output...";
            // Close and destroy the sending network object
            if (netOutput && netOutput->isRunning()) {
                netOutput->stop();
            }
            // Pause the calling thread to allow node & network threads to finish their jobs.
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            LOG(INFO) << TAG << "Waiting for the worker threads...";
            incomingHandlerThread->join();
            commandHandlerThread->join();
            ioServiceThread->join();
            delete incomingHandlerThread; incomingHandlerThread = nullptr;
            delete commandHandlerThread; commandHandlerThread = nullptr;
            delete ioServiceThread; ioServiceThread = nullptr;
            LOG(INFO) << TAG << "...threads finished, exiting ProcessorNode::stop";
        }
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
                if (netInput) {
                    Package package = netInput->read();
                    while (!package.isEmpty() && running) {
                        LOG(INFO) << TAG << "Received a package!";
                        showUIMessage("Received data from previous node.");
                        LOG(INFO) << TAG << "Received package: " << boost::uuids::to_string(package.getUuid()) << " " << package.getTypeAsString() << ":" << package.getData();
                        if (package.getType() == Package::Control && package.getData() == "shutdown") {
                            showUIMessage("Got shutdown command, forwarding and initiating shutdown.");
                            sendData(package);
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                            command = "quit";
                            condition.notify_all();
                            // Do not handle possible remaining packages after shutdown message.
                            break;
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
            logAndShowUIMessage(sstream.str(), true);
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
    
    void ProcessorNode::errorInData(const std::string & what) {
        std::stringstream sstream;
        sstream << "ERROR in incoming data; discarded " << what;
        LOG(WARNING) << sstream.str();
        showUIMessage(sstream.str());
    }
    
    void ProcessorNode::showUIMessage(const std::string & message) {
        if (observer != nullptr) {
            observer->NodeEventHappened(ProcessorNodeObserver::EventType::UINotificationEvent, message);
        }
    }
    
    void ProcessorNode::logAndShowUIMessage(const std::string & message, bool isWarning) {
        if (isWarning) {
            LOG(WARNING) << message;
        } else {
            LOG(INFO) << message;
        }
        showUIMessage(message);
    }
    
    void ProcessorNode::initiateClientAppShutdown() {
        if (observer != nullptr) {
            observer->NodeEventHappened(ProcessorNodeObserver::EventType::ShutDownEvent, "Shutdown of node requested from network.");
        }
    }
} //namespace

