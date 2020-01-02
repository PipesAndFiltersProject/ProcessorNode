//
//  NetworkReader.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 20.9.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#ifndef __PipesAndFiltersFramework__NetworkReader__
#define __PipesAndFiltersFramework__NetworkReader__

#include <sys/socket.h>

#include <OHARBaseLayer/Networker.h>

namespace OHARBase {
	
	// Forward declaration.
	class NetworkReaderObserver;
	
	/**
	 This class handles receiving of data to this ProcessorNode from another ProcessorNode.
	 Implementation is based on the Networker class which defines the common functionality of
	 reading and writing of data from/to another ProcessorNode.
	 @author Antti Juustila
	 */
	class NetworkReader : public Networker {
	public:
		NetworkReader(const std::string & hostName, NetworkReaderObserver & obs, boost::asio::io_service & io_s);
		NetworkReader(const std::string & hostName, int portNumber, NetworkReaderObserver & obs, boost::asio::io_service & io_s);
		~NetworkReader();
		
		virtual void start() override;
		virtual void stop() override;
		
		Package read();
		
	private:
		NetworkReader() = delete;
		NetworkReader(const NetworkReader &) = delete;
		NetworkReader(const NetworkReader &&) = delete;
		const NetworkReader & operator =(const NetworkReader &) = delete;
		
		void handleReceive(const boost::system::error_code & error, std::size_t bytes_transferred);
		
	private:
				
		/** The observer of the reader. When the reader receives data from the network,
		 it will notify the reader about the data. The observer can then retrieve the data by using
		 read() and handle it. */
		NetworkReaderObserver & observer;
		
      /** The address of the sender whose packages we are handling. */
      boost::asio::ip::udp::endpoint remote_endpoint;
      
		/** Tag for logging. */
		static const std::string TAG;
		
	};
	
	
} //namespace

#endif /* defined(__PipesAndFiltersFramework__NetworkReader__) */
