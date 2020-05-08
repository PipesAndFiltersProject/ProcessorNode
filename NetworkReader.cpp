//
//  NetworkReader.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 20.9.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <g3log/g3log.hpp>

#include <nlohmann/json.hpp>

#include <ProcessorNode/NetworkReader.h>
#include <ProcessorNode/NetworkReaderObserver.h>


namespace OHARBase {
   
   const std::string NetworkReader::TAG{"NetReader "};
   
   /**
    Constructor to create the reader with a port to listen to.
    @param port the port for reading data.
    @param obs The observer who gets notified of network events.
    @param io_s The Boost asynchronous io service.
    @param reuseAddress Whether to reuse the port number, allowing several sockets to read from the same port. Used only in broadcast messaging in handling configuration query messages when Nodes are running on a same host.
    */
   NetworkReader::NetworkReader(int port,
                              NetworkReaderObserver & obs,
                              boost::asio::io_service & io_s, bool reuseAddress)
   :		Networker("", port, io_s), observer(obs), doReuseAddress(reuseAddress)
   {
   }
   
   
   NetworkReader::~NetworkReader() {
   }
   
   
   
   /** Starts the reader. Open the socket for reading data from the specified port. */
   void NetworkReader::start(bool useAcknowledgements) {
      LOG(INFO) << TAG << "Start reading for data from port: " << port;
      running = true;
      sendAckMessages = useAcknowledgements;
      
      buffer->fill(0);

      using namespace boost::asio::ip;
      remote_endpoint.address(address_v4::any());
      remote_endpoint.port(port);
      
      socket.open(remote_endpoint.protocol());
      if (doReuseAddress) {
         socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
      }
      socket.bind(remote_endpoint);
      readSocket();
   }

   /**
   Reads data from the opened socket asyncronously.
    */
   void NetworkReader::readSocket() {
      socket.async_receive_from(boost::asio::buffer(*buffer),
                                remote_endpoint,
                                boost::bind(&NetworkReader::handleReceive, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
      LOG(INFO) << TAG << "Async recv ongoing";
   }

   /** Handles the incoming data and possible errors. Places finally another read
    to the socket to handle more incoming data.
    @param error Error code
    @param bytes_transferred How many bytes came in. */
   void NetworkReader::handleReceive(const boost::system::error_code & error, std::size_t bytes_transferred) {
      LOG(INFO) << TAG << "Async recv finished code: " << error.value() << " " << error.message();
      LOG_IF(WARNING, (error != boost::system::errc::success)) << "Receive failed in reader " << error.value();
      if (!error || error == boost::asio::error::message_size)
      {
         if (buffer->data()) {
            std::string buf;
            buf.resize(bytes_transferred);
            buf.assign(buffer->begin(), bytes_transferred);
            LOG(INFO) << TAG << "Received " << bytes_transferred << " bytes: " << buf << " from " << remote_endpoint.address() << ":" << remote_endpoint.port();
            if (buf.length()>0) {
               try {
                  nlohmann::json j = nlohmann::json::parse(buf);
                  Package p = j.get<OHARBase::Package>();
                  std::stringstream stream;
                  stream << remote_endpoint.address() << ":";
                  std::string port = p.getPackageOriginsListeningPort();
                  if (port.length() > 0) {
                     stream << port;
                  } else {
                     stream << remote_endpoint.port();
                  }
                  p.setOrigin(stream.str());
                  LOG(INFO) << "Received package from origin " << p.origin();
                  guard.lock();
                  msgQueue.push(p);
                  if (sendAckMessages && p.getType() == Package::Data) {
                     Package ackMessage;
                     ackMessage.setType(Package::Type::Acknowledgement);
                     ackMessage.setPayload("ack");
                     ackMessage.setDestination(p.origin());
                     ackMessage.setUuid(p.getUuid());
                     LOG(INFO) << "ackhandling: prepared an ack message to " << ackMessage.destination();
                     msgQueue.push(ackMessage);
                  }
                  guard.unlock();
                  // And when data has been received, notify the observer.
                  observer.receivedData();
               } catch (const std::exception & e) {
                  observer.errorInData(e.what());
               }
            }
         } else {
            LOG(WARNING) << TAG << "Async recv finished but NO data";
         }
         if (running)
         {
            // Make another read request to the socket.
            readSocket();
         }
      }
   }
   
   /** Stops the reader by setting the running flag to false, effectively ending the thread
    loop in the threadFunc(). */
   void NetworkReader::stop() {
      if (running) {
         LOG(INFO) << TAG << "Stop the reader...";
         running = false;
         LOG(INFO) << TAG << "Shutting down the socket.";
         socket.cancel();
         socket.close();
      }
   }
   
   /** Read the package object from the queue, received from the network.
    This method should be called by the NetworkReaderObserver only when it has been notified
    that data has arrived.
    @return The Package containing the data received from the previous ProcessorNode. If queue was empty returns an empty package.
    */
   Package NetworkReader::read() {
      LOG(INFO) << TAG << "Reading results from reader";
      guard.lock();
      Package result;
      if (!msgQueue.empty()) {
         LOG(INFO) << "METRICS packages in incoming queue: " << msgQueue.size();
         result = std::move(msgQueue.front());
         msgQueue.pop();
      }
      guard.unlock();
      return result;
   }
   
   
} //namespace
