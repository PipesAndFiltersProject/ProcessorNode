//
//  ConfigurationHandler.cpp
//  PipesAndFiltersNode
//
//  Created by Antti Juustila on 25.11.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include <g3log/g3log.hpp>

#include <OHARBaseLayer/ConfigurationHandler.h>
#include <OHARBaseLayer/ProcessorNode.h>
#include <OHARBaseLayer/Package.h>

namespace OHARBase {

const std::string ConfigurationHandler::TAG{"ConfigurationHandler "};

static const std::string ConfigReadOperation{"read"};
static const std::string ConfigInfoOperation{"info"};
static const std::string ConfigSetOperation{"set"};


/**
 Constructs a ConfigurationHandler object. Ping handler must now about the ProcessorNode
 in order to forward the ping message to the next ProcessorNode.
 @param myNode The processor node to use to forward the ping message.
 */
ConfigurationHandler::ConfigurationHandler(ProcessorNode & myNode)
: node(myNode)
{
}

ConfigurationHandler::~ConfigurationHandler() {
   
}

/**
 Implementation of handling the ping message. Basic functionality is to log the ping
 message arrival and then forward the message to the next ProcessorNode.
 @param data Handler checks if the data contains a ping message and if yes, handles it.
 @return If the message was ping message, returns true to indicate the ProcessorNode that
 no further processing of the message package is needed from other Handlers.
 */
bool ConfigurationHandler::consume(Package & data) {
   if (data.getType() == Package::Configuration) {
      LOG(INFO) << TAG << "***** Configuration package received! *****";
      // parse data to a student data object
      std::string payload = data.getPayloadString();
      if (payload == ConfigReadOperation) {
         // Create a JSON from current configuration.
         // Create a Package from the config (operation:info) and put origin address of Package
         // to destination address of Package.
         // Send the package ahead using node.SendData
         // Return true, no one else should handle the package.
         nlohmann::json info = R"({"operation" : "info"})"_json;
         info["nodename"] = node.getName();
         /*
          "payload" :
          {
          "operation" : "info",
          "nodename" : "node name",
          "items" : [
          {"config-item" : "item-value"},
          {"config-item2" : "item-value"}
          ]
          }
          */
      } else if (payload == ConfigSetOperation) {
         // Parse payload to see if operation is set.
         // Info is only received by the remote configurator app.
         // When operation is set, parse config save it to file and tell Node to read & set new config.
         /*
          "payload" :
          {
             "operation" : "set",
             "nodename" : "node name",
             "items" : [
                {"config-item" : "item-value"},
                {"config-item2" : "item-value"}
             ]
          }
          */
         nlohmann::json jsonData = nlohmann::json::parse(data.getPayloadString());
         if (jsonData.find("operation") != jsonData.end()) {
            std::string operation = jsonData["operation"].get<std::string>();
            if (operation == ConfigSetOperation) {
               
            }
         }
      } else {
         // Something should have not received. Ignore or handle error.
      }

      
      //TODO: If command is to set, reset configration in Node.
      //TODO: If command is to read, create config JSON and reply to sender.
      return true;
   }
   return false;
}

} //namespace


