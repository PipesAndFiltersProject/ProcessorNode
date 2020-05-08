//
//  ProcessorNode.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 19.9.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#pragma once

#include <string>
#include <vector>
#include <list>
#include <thread>

#include <boost/asio.hpp>

#include <ProcessorNode/NetworkReaderObserver.h>
#include <ProcessorNode/Package.h>
#include <ProcessorNode/ProcessorNodeObserver.h>

/** \mainpage
 
 This documentation describes a library to develop distributed system of Nodes.
 
 The library needs the following tools and uses the following other external libraries:
 
 -# A C++ compiler supporting C++ 17 and STL.
 -# cmake for building the library.
 -# boost 1.70.0 or newer. You need to build system library from boost. Build using c++17 (e.g. b2 cxxflags="-std=c++17".). For more information, see boost getting started guide.
 -# nlohmann::json for parsing and creating JSON.
 -# g3logger for logging events in all components of this software.
 
 You can build the library like this (in the root dir of the lib):
 
 -# mkdir build
 -# cd build
 -# cmake ..
 -# make
 -# sudo make install
 
 The last step copies the library binary to /usr/local/lib and headers to /usr/local/include in *nix based environments.
 
 Then you need to implement another layer on top for applicaton specific functionality and/or an app that utilises the library, communicating with other apps over the net. For an example, see StudentLayer library and BasicInfoGUI node and a sample configuration/installation in the StudentPassing system.
 
 See also README.md (generated with doxygen to Related pages by doxygen).
 
 (c) Antti Juustila, 2014-2020.
 
 */


/** @brief Namespace OHARBase contains basic classes for
 building distributed applications based on the Pipes & Filters
 architectural style.<p>
 OHARBase namespace contains the basic classes for
 building distributed applications based on the Pipes & Filters
 architectural style.<p>In this implementation of this style,
 Filters (a.k.a. Nodes, ProcessorNodes) receive data from
 the network using datagrams, process the data (including reading
 data from a file) and send the processed data to the next Node,
 again using datagrams. A Node may or may not have all these three
 elements (network input, file, network output), but may have some of
 them. Usually a Node has either input or output or both, and possibly
 a file also. A Node can only have one of each of these three, currently.
 Though since datagrams are used, it would be relatively simple to add support
 for receiving datagrams from multiple sources and send broadcast messages to
 larger set of recipients.
 */
namespace OHARBase {
   
   // Forward declarations
   class NetworkReader;
   class NetworkWriter;
   class DataHandler;
   class DataItem;
   class NodeConfiguration;
   
   /**
    ProcessorNode is a central class in the architecture. It acts as the Filter
    in the Pipes & Filters architectural style. The UDP datagrams are the pipe from
    one Filter to the next. In the code, the words Node, ProcessorNode and Filter are
    synonyms.<br />
    Node holds all the other important objects in the Filter's internal architecture:<br />
    <ul>
    <li>NetworkReader object, waiting for data from other ProcessorNode</li>
    <li>NetworkWriter object, sending data to other ProcessorNode</li>
    <li>Several DataHandler objects, which handle the data received from other ProcessorNodes, and/or creates and
    sends data to other ProcessorNodes, possibly reading and/or writing data to/from files also.</li>
    <li>ProcessorNode configuration data, which is held in the NodeConfiguration member variable.</li>
    </ul>
    When implementing an application following this Pipes&Filters architecture, one usually has several
    Filters (a.k.a ProcessorNodes). Each filter application as a main() function, which instantiates one ProcessorNode and
    configures it. These nodes can run on different machines, communicating with each other by sending datagram
    messages. One could install the nodes within a same machine, of course, and communicate within the same
    machine, for testing purposes.<p>
    Configuring a node means usually setting up the reader and/or writer. A node may or may
    not have a reader or writer. Usually the first node in the chain of nodes does not have a reader since
    it does not receive data, only sends it. Accordingly, the last node does not have a writer, since
    it only received data from previous node, handles it and then the data is ready, finished. At this
    point the result of the processing can be shown to the user and/or saved into a file, for example.<p>
    ProcessorNode itself manages only the setting of a specific node; which object it contains. The actual
    work is done by the networking objects and the handlers. Thus it is quite simple to set up a filter.
    Just create the reader and/or writer, and create the necessary handler objects and add them into the
    ProcessorNode, and start running the processor.<p>
    Nodes can be configured by using a configuration file or remotely by sending configuration messages to the Node in JSON
    format.
    @author Antti Juustila
    */
   class ProcessorNode final : public NetworkReaderObserver {
   public:
      // MARK: - Setup
      ProcessorNode(ProcessorNodeObserver * obs);
      virtual ~ProcessorNode();
      
      bool configure(const std::string & configFile);
      
      void setInputSource(const std::string & port);
      void setConfigurationInputSource(const std::string & port);
      void createConfigurationOutputWriter();
      void setOutputSink(const std::string & hostName);
      void setOutputSink(const std::string & hostName, int portNumber);
      
      void setDataFileName(const std::string & fileName);
      void setOutputFileName(const std::string & fileName);
      
      const std::string & getDataFileName() const;
      const std::string & getOutputFileName() const;
      
      void addHandler(DataHandler * h);
      
      // MARK: -      
      void start();
      void stop();
      bool isRunning() const;
      void handleCommand(const std::string & aCommand);
      
      virtual void receivedData() override;
      virtual void errorInData(const std::string & what) override;
      
      void sendData(Package & data);
      
      void passToHandlers(Package & package);
      
      void passToNextHandlers(const DataHandler * current, Package & data);
      
      void updatePackageCountInQueue(const std::string & queueName, int packageCount);
      
      void showUIMessage(const std::string & message, ProcessorNodeObserver::EventType e = ProcessorNodeObserver::EventType::NotificationEvent);
      void logAndShowUIMessage(const std::string & message, ProcessorNodeObserver::EventType e = ProcessorNodeObserver::EventType::NotificationEvent);
      
      std::string getConfigItemValue(const std::string & itemName) const;
      
      const NodeConfiguration & getConfiguration() const;
      
   private:
      /** The thread function where the handling of the data received by the NetworkReader is done.
       This thread is started in the Node's start() method. */
      void threadFunc();
      
      /** There is no need to copy ProcessorNodes so delete copy constructor. */
      ProcessorNode(const ProcessorNode &) = delete;
      /** There is no need to copy ProcessorNodes so delete assignment operator. */
      const ProcessorNode & operator = (const ProcessorNode &) = delete;
      
      void initiateClientAppShutdown();
      
      void handlePackagesFrom(NetworkReader & reader);
      
      std::string listeningPort() const;
      
   protected:
      
      /** The file name of the data file Node uses to read input data. */
      std::string dataFileName;
      /** The file name of the data file Node uses to write output data. */
      std::string outputFileName;
      
      /** The configuration object for the node. Contains information about the config
       of the node, including IP addresses to use when sending and receiving data. */
      NodeConfiguration * config;
      
   private:
      
      /** The boost io_service is needed for boost async network operations. */
      boost::asio::io_service io_service;
      
      /** The reader for receiving data from the previous Node. May be null. */
      NetworkReader * networkReader;
      /** The writer for sending data to the next Node. May be null. */
      NetworkWriter * networkWriter;
      /** The configuration reader for reading configuration messages. */
      NetworkReader * configReader;
      /** The configuration writer for sendng config responses. Created only if there is no networkWriter (output config
       element is nonexistent or null. Otherwise networkWriter is used also for sending config responses. */
      NetworkWriter * configWriter;
      
      /** The list of DataHandlers to process the incoming data packages.
       The assumption is that the handlers are put in the list in a following manner:<br />
       <ul><li>Handlers in the beginning of the list either create, receive or read data from file,</li>
       <li>or handle commands,</li>
       <li>the ones in the middle manipulate, add to or do other modifications to the data and</li>
       <li>the ones at the end pack data for sending, send it, or format and save the data into a file.</li></ul>
       */
      std::list<DataHandler*> handlers;
      /** Flag for indicating is the Node running or not. */
      std::atomic<bool> running;
      /** When node initiates shutdown (not the user/UI), this
       is set to true by the commandHandlerThread. Then, when
       stop() is called, observer is notified about the shutdown as the last step. */
      bool nodeInitiatedShutdownStarted;
      /** Thread for handling the incoming data. */
      std::thread * incomingHandlerThread;
      /** Thread for handling boost io_service */
      std::thread * ioServiceThread;
      /** Thread for handling commands from user and network */
      std::thread * commandHandlerThread;
      /** Mutex for guarding access to elements shared between threads. */
      std::mutex guard;
      /** Condition variable for notifications between threads to indicate processing is
       needed in the other thread. */
      std::condition_variable condition;
      /** Flag which is used to wait for incoming data. */
      bool hasIncoming;
      
      /** Command entered by the user or received from the previous node. */
      std::string command;
      /**  Command can be changed by man thread (user) and netinput handler thread. Must use mutex to guard it. */
      std::mutex commandGuard;
      
      // A container to keep track on which queues hold how many packages now, how many packages at max during batch run.
      using queue_package_type = std::map<std::string, std::pair<int,int>>;
      queue_package_type queuePackageCounts;
      
      /** Logging tag. */
      static const std::string TAG;
      
      /** Observer to whom events in the node are notified to. */
      ProcessorNodeObserver * observer;
   };
   
   
} //namespace

