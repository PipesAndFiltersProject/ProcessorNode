//
//  NodeConfiguration.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 25.2.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#include <g3log/g3log.hpp>

#include <OHARBaseLayer/NodeConfiguration.h>

namespace OHARBase {

/**
 Destructor destroys the configuration data it holds.
 */
NodeConfiguration::~NodeConfiguration() {
   configItems.clear();
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
      addOrReplace(*d);
      item.release();
   }
}

/**
 Gets the value of the named configuration item.
 @param configName The name of the configuration to read.
 @returns The value of the named configuration item, empty string if not found.
 */
std::string NodeConfiguration::getValue(const std::string &configName) const {
   for (const ConfigurationDataItem & configItem : configItems) {
      if (configItem.getItemName() == configName) {
         return configItem.getItemValue();
      }
   }
   return "";
}

void NodeConfiguration::addOrReplace(const ConfigurationDataItem & item) {
   auto element = std::find(configItems.begin(), configItems.end(), item);
   if (element != configItems.end()) {
      *element = item;
   } else {
      configItems.push_back(item);
   }
   LOG(INFO) << "Has config: " << item.getItemName() << " " << item.getItemValue();
}

void to_json(nlohmann::json & j, const NodeConfiguration & config) {
   j["configitems"] = config.configItems;
}

void from_json(const nlohmann::json & j, NodeConfiguration & config) {
   if (j.find("configitems") != j.end()) {
      auto items = j["configitems"];
      for (auto & element : items) {
         ConfigurationDataItem item = element.get<ConfigurationDataItem>();
         config.addOrReplace(item);
      }
   }   
}

} // namespace
