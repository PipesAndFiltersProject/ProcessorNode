//
//  NetworkWriter.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 20.9.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include <g3log/g3log.hpp>

#include <OHARBaseLayer/NetworkWriter.h>


namespace OHARBase {
	
	/**
	 Constructor to create the writer with host name. See the
	 constructor of Networker class about handling the parameters.
	 @param hostName the host to send data to, including port number.
    @param io_s The boost asio io service.
	 */
	NetworkWriter::NetworkWriter(const std::string & hostName, boost::asio::io_service & io_s)
	: Networker(hostName,io_s), TAG("NetWriter ")
	{
		socket = std::unique_ptr<boost::asio::ip::udp::socket>(new boost::asio::ip::udp::socket(io_s));
	}
	
	/**
	 Constructor to create the writer with host name. See the
	 constructor of Networker class about handling the parameters.
	 @param hostName The host to send data to.
	 @param portNumber The port to send data to.
    @param io_s The boost asio io service.
	 */
	NetworkWriter::NetworkWriter(const std::string & hostName, int portNumber, boost::asio::io_service & io_s)
	: Networker(hostName, portNumber, io_s), TAG("NetWriter ")
	{
		socket = std::unique_ptr<boost::asio::ip::udp::socket>(new boost::asio::ip::udp::socket(io_s));
	}
	
	NetworkWriter::~NetworkWriter()
	{
	}
	
	
	
	/** Thread function which does all the relevant work of sending data packages.
	 First, the function sets up the networking things, and then runs in a loop, waiting
	 for data packages to arrive. When one arrives, it is notified of it, and then goes
	 through a round of a loop. There, the package is taken from the queue, packaged
	 in a data string and then sent over the network. Locks and synchronization are used
	 to make sure the queueu is handled by one thread at a time only. Function quits the loop
	 and returns when the stop() is called and the running flag is set to false.
	 @todo Reimplement using std lib socket implementation.
	 */
	void NetworkWriter::threadFunc() {
		if (host.length() > 0 && port > 0) {
			LOG(INFO) << TAG << "Starting the write loop.";
			while (running) {
				guard.lock();
				if (!msgQueue.empty()) {
					LOG(INFO) << TAG << "Stuff in send queue! Reading Package";
					Package p = msgQueue.front();
               LOG(INFO) << TAG << "Package read. Now convert to json...";
               nlohmann::json j = p;
               LOG(INFO) << TAG << "...and put to a string for sending: " << j.dump();
               currentlySending = j.dump();
               
					LOG(INFO) << TAG << "Sending: " << currentlySending;
					msgQueue.pop();
               LOG(INFO) << TAG << "Just popped, next unlock";
					guard.unlock();
               LOG(INFO) << TAG << "Determining destination address for " << host << ":" << port;
					boost::asio::ip::udp::endpoint destination(boost::asio::ip::address::from_string(host), port);
               LOG(INFO) << TAG << "Creating message...";
					boost::shared_ptr<std::string> message(new std::string(currentlySending));
               LOG(INFO) << TAG << "Now sending through socket " << destination.address().to_string() << ":" << destination.port();
					socket->async_send_to(boost::asio::buffer(*message), destination,
												 boost::bind(&NetworkWriter::handleSend, this,
																 boost::asio::placeholders::error,
																 boost::asio::placeholders::bytes_transferred));
					LOG(INFO) << TAG << "Async send delivered";
				} else {
					guard.unlock();
					LOG(INFO) << TAG << "Send queue empty, waiting...";
					std::unique_lock<std::mutex> ulock(guard);
					condition.wait(ulock, [this] {return !msgQueue.empty() || !running; } );
				}
			}
			LOG(INFO) << TAG << "Shutting down the network writer thread.";
		}
	}
	
	void NetworkWriter::handleSend(const boost::system::error_code& error,
						  std::size_t bytes_transferred)
	{
      if (error != boost::system::errc::success) {
         LOG(WARNING) << TAG << "Cannot send data to next node! " << error.value();
      } else {
         LOGF(INFO, "Sent %lu bytes through socket.", bytes_transferred);
      }
	}
	
	/**
	 Starts the network writer.
	 Basically starting the writer starts the thread which is waiting for
	 notification of data arriving in the send queue in a separate thread.
	 */
	void NetworkWriter::start() {
		// Create and run in thread...
		// Set up the socket
		// start working
		if (!running) {
         LOG(INFO) << TAG << "Starting NetworkWriter.";
			running = true;
			socket->open(boost::asio::ip::udp::v4());
			threader = std::thread(&NetworkWriter::threadFunc, this);
		}
	}
	
	/** Stops the writer. In practise this ends the loop in the send thread. */
	void NetworkWriter::stop() {
		LOG(INFO) << TAG << "Beginning NetworkWriter::stop.";
		if (running) {
			running = false;
         while (!msgQueue.empty()) {
            msgQueue.pop();
         }
			socket->cancel();
			socket->close();
			condition.notify_all();
         threader.join();
		}
      LOG(INFO) << TAG << "Exiting NetworkWriter::stop.";
	}
	
	
	/** Use write to send packages to the next ProcessorNode. The package is
	 put into a queue of packages to send and will be sent when all the previous packages
	 have been sent.
	 @param data The data package to send.
	 */
	void NetworkWriter::write(const Package & data)
	{
      LOG(INFO) << TAG << "Putting data to networkwriter's message queue.";
		guard.lock();
		msgQueue.push(data);
		guard.unlock();
		// Notify the writer thread there's something to send.
		condition.notify_one();
	}
	
	
} //namespace
