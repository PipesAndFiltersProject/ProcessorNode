//
//  NetworkWriter.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 20.9.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#pragma once

#include <queue>
#include <condition_variable>

#include <ProcessorNode/Networker.h>
#include <ProcessorNode/Package.h>

namespace OHARBase {
	
   //TODO: 1..n destination addresses instead of only one.
   //TODO: Rules for specifying to which output address to send the package to.
	/** NetworkWriter handles the sending of the data packages to the next node.
	 It contains a queue of data packages to send. Sending happens in a separate thread
	 in order to keep the main thread responsive to user actions as well
	 as to enable handling and receiving the data from other nodes separately.
	 @author Antti Juustila
	 */
	class NetworkWriter : public Networker {
	public:
		NetworkWriter(const std::string & hostName, boost::asio::io_service & io_s);
		NetworkWriter(const std::string & hostName, int portNumber, boost::asio::io_service & io_s);
		~NetworkWriter();
		
		virtual void start() override;
		virtual void stop() override;
		
		void write(const Package & data);
		
	private:
		NetworkWriter() = delete;
		NetworkWriter(const NetworkWriter &) = delete;
		const NetworkWriter & operator =(const NetworkWriter &) = delete;
		
		void threadFunc();
		
		void handleSend(const boost::system::error_code& error,
							 std::size_t bytes_transferred);
	private:
		
      /** Has the resolved endpoint to use for sending data. */
      boost::asio::ip::udp::endpoint resolvedEndpoint;
		/** Contains the data which is currenty being sent, in a string. */
		std::string currentlySending;
		/** The condition variable used to signal the sending thread that new data is available
		 in the queue. */
		std::condition_variable condition;
      /** The thread where networking code runs in. */
		std::thread * threader;
		/** Logging tag. */
		static const std::string TAG;
		
	};
	
}


#endif /* defined(__PipesAndFiltersFramework__NetworkWriter__) */
