//
//  ConfigurationHandler.cpp
//  PipesAndFiltersNode
//
//  Created by Antti Juustila on 15.12.2019.
//  Copyright (c) 2019 Antti Juustila. All rights reserved.
//

#include <g3log/g3log.hpp>

#include <OHARBaseLayer/ConfigurationHandler.h>
#include <OHARBaseLayer/ProcessorNode.h>
#include <OHARBaseLayer/Package.h>
#include <OHARBaseLayer/NodeConfiguration.h>

namespace OHARBase {

const std::string ConfigurationHandler::TAG{"ConfigHandler "};

static const std::string ConfigReadOperation{"read"};
static const std::string ConfigInfoOperation{"info"};
static const std::string ConfigSetOperation{"set"};


/**
 Constructs a ConfigurationHandler object. Configuration handler must now about the ProcessorNode
 in order to communicate with the Configurator app by replying to config messages.
 @param myNode The processor node to use to reply to config messages.
 */
ConfigurationHandler::ConfigurationHandler(ProcessorNode & myNode)
: node(myNode)
{
}

ConfigurationHandler::~ConfigurationHandler() {
   
}

/**
 Implementation of handling the config messages. Config message can be either
 <ul>
 <li>read message, where the remote configuration app wants to know the configuration of this node.</li>
 <li>set message, where the remote configuration app wants to change the configuration of this node</li>
 </ul>.
 <p>When this node receives the read message, it will send an info message back to the sender.</p>
 <p>For details about the JSON message structure related to the configuration message packages, see the project readme.</p>
 @param data Handler checks if the data contains a configuration message and if yes, handles it.
 @return If the message was a configuration message, returns true to indicate the ProcessorNode that
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
         LOG(INFO) << "Config requested";
         nlohmann::json configuration = node.getConfiguration();
         LOG(INFO) << "Step 1 " << configuration.dump();
         configuration["operation"] = ConfigInfoOperation;
         LOG(INFO) << "Step 2 " << configuration.dump();
         Package package;
         package.setType(Package::Configuration);
         package.setPayload(configuration.dump());
         package.setDestination(data.origin());
         node.sendData(package);
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
      return true; // Returns true, so package is not passed to additional handlers.
   }
   return false; // Package was not handled, pass to other handlers.
}

} //namespace


