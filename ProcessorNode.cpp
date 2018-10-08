//
//  ProcessorNode.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 19.9.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include <sstream>

#include <OHARBaseLayer/ProcessorNode.h>
#include <OHARBaseLayer/NetworkReader.h>
#include <OHARBaseLayer/NetworkWriter.h>
#include <OHARBaseLayer/PingHandler.h>
#include <OHARBaseLayer/DataFileReader.h>
#include <OHARBaseLayer/Log.h>
#include <OHARBaseLayer/NodeConfiguration.h>
#include <OHARBaseLayer/ConfigurationFileReader.h>

namespace OHARBase {
	
	/** Constructor for the processor node.
	 @param aName The name of the processor node. */
	ProcessorNode::ProcessorNode(const std::string & aName)
	: config(0), name(aName), netInput(0), netOutput(0), running(false), hasIncoming(false), TAG("PNode")
	{
		Log::get().entry(TAG, "Creating ProcessorNode.");
		handlers.push_back(new PingHandler(*this));
	}
	
	/** Destructor cleans all the internal objects of the Node when it is destroyed. */
	ProcessorNode::~ProcessorNode() {
      
      //TODO: should call stop instead, then here destroy the objects?
      
		Log::get().entry(TAG, "Destroying ProcessorNode.");
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
		Log::get().entry(TAG, "Destroyed ProcessorNode.");
	}
	
	
	void ProcessorNode::configure(const std::string & configFile) {
		if (configFile.length() > 0) {
			delete config;
			config = nullptr;
			config = new NodeConfiguration();
			ConfigurationFileReader reader(*config);
			reader.read(configFile);
			std::string cvalue = config->getValue(ConfigurationDataItem::CONF_INPUTADDR);
			setInputSource(cvalue);
			cvalue = config->getValue(ConfigurationDataItem::CONF_OUTPUTADDR);
			setOutputSink(cvalue);
			cvalue = config->getValue(ConfigurationDataItem::CONF_INPUTFILE);
			setDataFileName(cvalue);
			cvalue = config->getValue(ConfigurationDataItem::CONF_OUTPUTFILE);
			setOutputFileName(cvalue);
		}
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
			Log::get().entry(TAG, "Setting input: %s", hostName.c_str());
			netInput = new NetworkReader(hostName, *this, io_service);
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
			Log::get().entry(TAG, "Setting output: %s", hostName.c_str());
			netOutput = new NetworkWriter(hostName, io_service);
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
			Log::get().entry(TAG, "Setting input: %s:%d", hostName.c_str(), portNumber);
			netInput = new NetworkReader(hostName, portNumber, *this, io_service);
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
			Log::get().entry(TAG, "Setting output: %s:%d", hostName.c_str(), portNumber);
			netOutput = new NetworkWriter(hostName, portNumber, io_service);
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
	}
	
	
	/** For getting the name of the data file Node is writing output data to.
	 @return The file name to write data to. */
	const std::string & ProcessorNode::getOutputFileName() const {
		return outputFileName;
	}
	
	/** Starts the Node. This includes starting the network reader and/or writer for
	 communicating to other Nodes, starting the data handling thread, and looping in the
	 main thread to handle user commands. The start loop runs until shutdown command is given
	 from the keyboard or one arrives from the previous node. */
	void ProcessorNode::start() {
		// Start the listening network reader
		if (netInput) {
			Log::get().entry(TAG, "Start the input reader");
			netInput->start();
		}
		// Start the sending network object
		if (netOutput) {
			Log::get().entry(TAG, "Start the output writer");
			netOutput->start();
		}
		Log::get().entry(TAG, "Start the receive handler thread...");
		running = true;
      if (netInput) {
         threader = std::thread(&ProcessorNode::threadFunc, this);
      }
		
		new std::thread([this] {return io_service.run();} );
		
		new std::thread([this] {
			while (running) {
				std::cout << "Enter command > ";
				getline(std::cin, command);
            condition.notify_all();
				if (command == "quit") {
               return; // running = false;
				}
			}
		});
		
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
                  } else if (cmd == "readfile") {
                     if (dataFileName.length() > 0) {
                        Log::get().entry(TAG, "Reading data file.. %s", dataFileName.c_str());
                        p.setType(Package::Control);
                        p.setData(cmd);
                        passToHandlers(p);
                     }
                  } else if (cmd == "quit") {
                     p.setType(Package::Control);
                     p.setData(cmd);
                     sendData(p);
                     std::this_thread::sleep_for(std::chrono::milliseconds(500));
                     stop();
                  }
               }
               return !running;
            });
			}
		}
	}
	
	/** Stops the Node. This includes closing and destroying the network reader and/or writer
	 and setting the running flag to false. These operations finishes the data handling thread
	 as well as the main thread loop running in the start() method. */
	void ProcessorNode::stop() {
		running = false;
      Log::get().entry(TAG, "Stopping input...");
		if (netInput) {
			netInput->stop();
		}
      Log::get().entry(TAG, "Stopped input, now stopping output...");
		// Close and destroy the sending network object
		if (netOutput) {
			netOutput->stop();
		}
      Log::get().entry(TAG, "Stopped output, stopping io service...");
		io_service.stop();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
      Log::get().entry(TAG, "Notify all");
		condition.notify_all();
      Log::get().entry(TAG, "...exiting ProcessorNode::stop");
	}
	
	
	
	/** Method sends the data to the next node by using the NetworkWriter object.
	 @param data The data package to send to the next Node. */
	void ProcessorNode::sendData(const Package & data) {
		if (netOutput) {
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
			Log::get().entry(TAG, "Offering data to next Handler...");
			if (!found && current == *iter) {
				found = true;
				Log::get().entry(TAG, "Found current handler....");
				continue;
			}
			if (found) {
				Log::get().entry(TAG, "..so offering the package to the rest.");
				if ((*iter)->consume(package)) {
					Log::get().entry(TAG, "Consumer returned true, not offering forward anymore");
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
			Log::get().entry(TAG, "Receive queue empty, waiting...");

			{
				std::unique_lock<std::mutex> ulock(guard);
				condition.wait(ulock, [this] { return this->hasIncoming || !running; });
			}
			
			if (running) {
				Log::get().entry(TAG, "Received data!");
				if (netInput) {
					guard.lock();
					Package package = netInput->read();
					guard.unlock();
					Log::get().entry(TAG, "Received package %s : %s", package.getTypeAsString().c_str(), package.getData().c_str());
					while (!package.isEmpty()) {
						if (package.getType() == Package::Control && package.getData() == "quit") {
							sendData(package);
							std::this_thread::sleep_for(std::chrono::milliseconds(500));
//							running = false;
//							condition.notify_all();
//							break;
							stop();
						} else {
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
		Log::get().entry(TAG, "Exit thread in ProcessorNode!");
	}
	
	/** This method takes the incoming data and passes it to be handled by the
	 DataHandler objects in the Node. The data is given to all Handlers until one
	 returns true, indicating that the package has been handled and should not be passed
	 ahead anymore. A Handler can of course handle the package and still return false,
	 enabling multiple handlers for a single package.
	 @param package The data package to handle. */
	void ProcessorNode::passToHandlers(Package & package) {
		Log::get().entry(TAG, "Handlers count: %d", handlers.size());
		for (std::list<DataHandler*>::iterator iter = handlers.begin(); iter != handlers.end(); iter++) {
			Log::get().entry(TAG, "Offering data to next Handler...");
			if ((*iter)->consume(package)) {
				Log::get().entry(TAG, "Consumer returned true, not offering forward anymore");
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
		Log::get().entry(TAG, "Processor has incoming!");
		hasIncoming = true;
		condition.notify_all();
	}
	
	
	
} //namespace

