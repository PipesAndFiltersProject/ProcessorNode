//
//  Networker.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 19.9.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#pragma once

#include <string>
#include <thread>
#include <queue>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <ProcessorNode/Package.h>

namespace OHARBase {
	
	/** An abstract base class for implementing networking in the architecture.
	 This class contains the common features needed in sending and receiving of
	 data packages to/from other ProcessorNodes in the architecture.
	 Both sending and receiving of data happens in a separate thread. Thus, the
	 networking objects need to initialize and configure the threads and the networking.
	 The initialization happens in the constructors (where the addresses are set) and in
	 start() method.
	 @author Antti Juustila
	 @todo Support for IPv6 and secure sockets / secure (encrypted) data.
	 */
	class Networker {
	public:
		Networker(const std::string & hostName, boost::asio::io_service & io_s);
		Networker(const std::string & hostName, int portNumber, boost::asio::io_service & io_s);
		virtual ~Networker();
		
		const std::string getHost() const;
		int getPort() const;
		
		/**
		 Starts the writing or reading networker, depending on the subclass implementation.
		 Note that here this is an abstract method.
		 */
		virtual void start(bool useAcknowledgements) = 0;
		/**
		 Stops the writing or reading networker, depending on the subclass implementation.
		 Note that here this is an abstract method.
		 */
		virtual void stop() = 0;
		
      /** How many data packages are there in the sending/receiving queue.
       @return Number of data packages in the queue. */
      int packagesInQueue() const;

      bool isRunning();
      
		
	private:
		Networker() = delete;
		Networker(const Networker &) = delete;
		const Networker & operator =(const Networker &) = delete;

		void setHost(const std::string & hostName);
		void setPort(int p);

	protected:
		/** Host name of the networker. If this is an object receiving data in the
		 ProcessorNode, this is the local IP address of the machine. If this is a sending
		 address, it is the IP address of the remote computer to send data to. */
		std::string host;
		/** Port number where data is read from (local machine, receiving data) or send to. */
		int port;
		
		/** If true, the object is running:<br />
		 <ul><li>NetworkReader is waiting for incoming packages from the network and
		 handling them as they arrive.</li>
		 <li>NetworkWriter is waiting for outbound packages and writing (sending) them to the remote
		 address (other ProcessorNode).</li>
		 </ul>
		 */
		bool running;

      /** A queue containing the data as Packages, received from the network.
       As more data could be received as this node could handle at a time, a queue is necessary to hold
       the data so that the node can handle them without loosing any data. */
      std::queue<Package> msgQueue;
      /** A mutex guards the access to the queue so that many threads do not manipulate the
       queue simultaneously. */
      std::mutex guard;

		/** The boost socket to use when sending or receiving data. */
		boost::asio::ip::udp::socket socket;
		/** The size of the buffer where incoming or outgoing data is stored into. */
      static const int BufferSize = 4096;
      /** The buffer where incoming or outgoing data is stored into. Automatically deallocated when hosting object dies. */
		std::shared_ptr<boost::array<char, BufferSize>> buffer;
	};
	
	
} //namespace

