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
#include <boost/algorithm/string.hpp>

#include <g3log/g3log.hpp>

#include <ProcessorNode/NetworkWriter.h>


namespace OHARBase {

const std::string NetworkWriter::TAG{"NetWriter "};

/**
 Constructor to create the writer with host name. See the
 constructor of Networker class about handling the parameters.
 @param hostName the host to send data to, including port number.
 @param io_s The boost asio io service.
 */
NetworkWriter::NetworkWriter(const std::string & hostName, boost::asio::io_service & io_s)
: Networker(hostName,io_s), threader(nullptr)
{
}

/**
 Constructor to create the writer with host name. See the
 constructor of Networker class about handling the parameters.
 @param hostName The host to send data to.
 @param portNumber The port to send data to.
 @param io_s The boost asio io service.
 */
NetworkWriter::NetworkWriter(const std::string & hostName, int portNumber, boost::asio::io_service & io_s)
: Networker(hostName, portNumber, io_s), threader(nullptr)
{
}

NetworkWriter::~NetworkWriter()
{
}



/** Thread function which does all the relevant work of sending data packages.
 start() method sets up the networking things, and then threadFunc() is waiting
 for data packages to arrive. When one arrives, it is notified of it (see write()), and then goes
 through a round of a loop. In the loop, a package is taken from the queue, packaged
 in a json data string and then sent over the network. Locks and synchronization are used
 to make sure the queue is handled by one thread at a time only. Function quits the loop
 and returns when the stop() is called and the running flag is set to false.
 */
void NetworkWriter::threadFunc() {
   /*
    What is happening here (can be used to draw an activity diagram)...
    - set the write's state to running.
    - check if we have an address to send data to
    - while writer is running
    - if there are messages in the queue
    read the message package from the queue
    convert the data from there to JSON
    determine the address to send data to
    send it ahead
    - if there are no messages in the queue
    wait for someone to awake the thread to send messages.
    - loop to while (above)
    */
   running = true;
   if (host.length() > 0 && port > 0) {
      LOG(INFO) << TAG << "Starting the write loop.";
      while (running) {
         guard.lock();
         if (!msgQueue.empty()) {
            LOG(INFO) << TAG << "Stuff in send queue! Reading Package";
            Package p = msgQueue.front();
            LOG(INFO) << TAG << "Package read. Now convert to json...";
            nlohmann::json j = p;
            currentlySending = j.dump();
            
            LOG(INFO) << TAG << "Sending: " << currentlySending;
            msgQueue.pop();
            LOG(INFO) << TAG << "Just popped, next unlock";
            guard.unlock();
            
            std::string tmpHost;
            int tmpPort;
            // If package has destination address, use it instead of node's configured destination address.
            tmpHost = host;
            tmpPort = port;
            if (p.hasDestination()) {
               LOG(INFO) << "Package specific destination exists.";
               std::vector<std::string> strs;
               boost::split(strs, p.destination(), boost::is_any_of(":"));
               if (strs.size() == 2) {
                  tmpHost = strs.at(0);
                  tmpPort = std::stoi(strs.at(1));
               }
            }
            LOG(INFO) << TAG << "Destination address is " << tmpHost << ":" << tmpPort;
            
            
            // TODO: Try to resolve the output address if specified as host name
//            boost::asio::ip::udp::endpoint destination;
//            boost::asio::ip::udp::resolver resolver(socket.get_executor());
//            resolver.async_resolve({tmpHost, tmpPort},
//                                   io_strand.wrap([this](const boost::system::error_code& ec,
//                                                         boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
//                                                  {
//               if (!ec)
//               {
//                  std::for_each(endpoint_iterator, {}, [this](auto& it) {
//                     destination.address(it.endpoint().address(), tmpPort);
//                  });
//               }
//               else
//               {
//                  //ec.message()?
//               }
//            }));
            
            
            LOG(INFO) << TAG << "Creating message...";
            boost::shared_ptr<std::string> message(new std::string(currentlySending));
            boost::asio::ip::udp::endpoint destination(boost::asio::ip::address::from_string(tmpHost), tmpPort);
            LOG(INFO) << TAG << "Now sending to address " << destination.address().to_string() << ":" << destination.port();
            socket.async_send_to(boost::asio::buffer(*message), destination,
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

/** This method is called when the boost async send finishes.
 @param error Result code of sending, might be an error.
 @param bytes_transferred How many bytes were sent. */
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
 notification of data put in the send queue by somebody (calling write()).
 */
void NetworkWriter::start() {
   // Create and run in thread...
   // Set up the socket
   // start working
   if (!running) {
      LOG(INFO) << TAG << "Starting NetworkWriter.";
      socket.open(boost::asio::ip::udp::v4());
      threader = new std::thread(&NetworkWriter::threadFunc, this);
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
      socket.cancel();
      socket.close();
      condition.notify_all();
      if (threader->joinable()) {
         threader->detach();
      }
      delete threader;
      threader = nullptr;
   }
   LOG(INFO) << TAG << "Exiting NetworkWriter::stop.";
}


/** Use write to send packages to the next ProcessorNode. The package is
 put into a queue of packages to send and will be sent when all the previous packages
 have been sent by the threadFunc().
 @param data The data package to send.
 */
void NetworkWriter::write(const Package & data)
{
   if (running) {
      LOG(INFO) << TAG << "Putting data to networkwriter's message queue.";
      guard.lock();
      msgQueue.push(data);
      LOG(INFO) << "METRICS packages in outgoing queue: " << msgQueue.size();
      guard.unlock();
      // Notify the writer thread there's something to send.
      condition.notify_one();
   }
}


} //namespace
