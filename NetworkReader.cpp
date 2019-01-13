//
//  NetworkReader.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 20.9.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <g3log/g3log.hpp>

#include <nlohmann/json.hpp>

#include <OHARBaseLayer/NetworkReader.h>
#include <OHARBaseLayer/NetworkReaderObserver.h>


namespace OHARBase {
    
    /**
     Constructor to create the reader with host name. See the
     constructor of Networker class about handling the parameters.
     @param hostName the host to reading data, including port number. In practise,
     this should always be local host for reader, i.e. 127.0.0.1 with port, eg. 127:0.0.1:2222.
     @param obs The observer who gets notified of network events.
     @param io_s The boost asio io service.
     */
    NetworkReader::NetworkReader(const std::string & hostName,
                                 NetworkReaderObserver & obs,
                                 boost::asio::io_service & io_s)
    :		Networker(hostName, io_s),
    observer(obs),
    TAG("NetReader ")
    {
        using namespace boost::asio::ip;
        remote_endpoint = std::unique_ptr<udp::endpoint>(new boost::asio::ip::udp::endpoint(udp::v4(), port));
        socket = std::unique_ptr<udp::socket>(new udp::socket(io_s, *remote_endpoint));
    }
    
    /**
     Constructor to create the reader with host name. See the
     constructor of Networker class about handling the parameters.
     @param hostName the host to reading data. In practise,
     this should always be local host for reader, i.e. 127.0.0.1.
     @param portNumber The port to read data from.
     @param obs The observer who gets notified of network events.
     @param io_s The boost asio io service.
     */
    NetworkReader::NetworkReader(const std::string & hostName,
                                 int portNumber,
                                 NetworkReaderObserver & obs,
                                 boost::asio::io_service & io_s)
    :		Networker(hostName, portNumber, io_s),
    observer(obs),
    TAG("NetworkReader")
    {
        using namespace boost::asio::ip;
        remote_endpoint = std::unique_ptr<udp::endpoint>(new udp::endpoint(udp::v4(), port));
        socket = std::unique_ptr<udp::socket>(new udp::socket(io_s, *remote_endpoint));
    }
    
    NetworkReader::~NetworkReader() {
    }
    
    
    
    /** Starts the reader. If necessary, connects to the endpoint and then does
     asynchronous read. */
    void NetworkReader::start() {
        LOG(INFO) << TAG << "Start reading for data...";
        running = true;
        if (!socket->is_open()) {
            LOG(INFO) << TAG << "Opening read socket";
            socket->connect(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));
        }
        buffer->fill(0);
        socket->async_receive_from(boost::asio::buffer(*buffer),
                                   *remote_endpoint,
                                   boost::bind(&NetworkReader::handleReceive, this,
                                               boost::asio::placeholders::error,
                                               boost::asio::placeholders::bytes_transferred));
        
        LOG(INFO) << TAG << "Async recv ongoing";
    }
    
    /** Handles the incoming data and possible errors. Places finally another read
     to the socket.
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
                LOG(INFO) << TAG << "Received " << bytes_transferred << " bytes of data: " << buf;
                if (buf.length()>0) {
                    nlohmann::json j = nlohmann::json::parse(buf);
                    Package p = j.get<OHARBase::Package>();
                    guard.lock();
                    msgQueue.push(p);
                    guard.unlock();
                    // And when data has been received, notify the observer.
                    observer.receivedData();
                }
            } else {
                LOG(WARNING) << TAG << "Async recv finished but NO data";
            }
            if (running)
            {
                start();
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
            socket->cancel();
            socket->close();
        }
    }
    
    /** Allows another object to read the package object received from the network.
     This method should be called by the NetworkReaderObserver only when it has been notified
     that data has arrived.
     @return The Package containing the data received from the previous ProcessorNode.
     */
    Package NetworkReader::read() {
        LOG(INFO) << TAG << "Reading results from reader";
        guard.lock();
        Package result;
        if (!msgQueue.empty()) {
            result = msgQueue.front();
            msgQueue.pop();
        }
        guard.unlock();
        return result;
    }
    
    
} //namespace
