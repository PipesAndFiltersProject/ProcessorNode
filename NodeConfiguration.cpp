//
//  NodeConfiguration.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 25.2.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#include <OHARBaseLayer/NodeConfiguration.h>

namespace OHARBase {

void NodeConfiguration::setName(const std::string & nodeName) {
   name = nodeName;
}

const std::string & NodeConfiguration::getName() const {
   return name;
}


/**
 Destructor destroys the configuration data it holds.
 */
NodeConfiguration::~NodeConfiguration() {
   while (configItems.size() > 0) {
      delete configItems[0];
      configItems.erase(configItems.begin());
   }
}

/**
 Handles a new configuration data item, usually as parsed from the
 configuration file. Adds it to the container of configuration items.
 @param item The new configuration item to add.
 @todo Check if an item with the same name exists already and replace it with this one.
 */
void NodeConfiguration::handleNewItem(std::unique_ptr<DataItem> item) {
   ConfigurationDataItem * d = dynamic_cast<ConfigurationDataItem*>(item.get());
   if (d) {
      configItems.push_back(d);
      item.release();
   }
}

/**
 Gets the value of the named configuration item.
 @param configName The name of the configuration to read.
 @returns The value of the named configuration item, empty string if not found.
 */
std::string NodeConfiguration::getValue(const std::string &configName) const {
   for (ConfigurationDataItem * configItem : configItems) {
      if (configItem->getItemName() == configName) {
         return configItem->getItemValue();
      }
   }
   return "";
}


void to_json(nlohmann::json & j, const NodeConfiguration & config) {
   j = nlohmann::json{{"nodename", config.getName()}};
   auto pusher = [&j] (const ConfigurationDataItem * item) { j.push_back(*item); };
   std::for_each(config.configItems.begin(), config.configItems.end(), pusher);
}

void from_json(const nlohmann::json & j, NodeConfiguration & config) {
}

} // namespace
