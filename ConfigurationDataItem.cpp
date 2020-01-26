//
//  ConfigurationDataItem.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 25.2.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

#include <g3log/g3log.hpp>

#include <ProcessorNode/ConfigurationDataItem.h>

namespace OHARBase {

/**
 Copy constructor for config item.
 @param item The item to get values from.
 */
ConfigurationDataItem::ConfigurationDataItem(const ConfigurationDataItem & item)
: itemName(item.itemName), itemValue(item.itemValue)
{
}

const std::string ConfigurationDataItem::TAG{"ConfigReader"};
/** Configuration data item name for ProcessorNode incoming address (port). */
const std::string ConfigurationDataItem::CONF_INPUTADDR{"input"};
/** Configuration data item name for ProcessorNode configuration address (port). */
const std::string ConfigurationDataItem::CONF_CONFINADDR{"config-in"};
/** Configuration data item name for ProcessorNode configuration outgoing address (value is yes or no or is missing). */
const std::string ConfigurationDataItem::CONF_CONFOUTADDR{"config-out"};
/** Configuration data item name for ProcessorNode outgoing address. */
const std::string ConfigurationDataItem::CONF_OUTPUTADDR{"output"};
/** Configuration data item name for ProcessorNode input data file. */
const std::string ConfigurationDataItem::CONF_INPUTFILE{"filein"};
/** Configuration data item name for ProcessorNode output data file. */
const std::string ConfigurationDataItem::CONF_OUTPUTFILE{"fileout"};
/** Configuration data item name for ProcessorNode's name. */
const std::string ConfigurationDataItem::CONF_NODENAME{"name"};

/**
 Sets the configuration data item name.
 @param item The configuration item name.
 */
void ConfigurationDataItem::setItemName(const std::string &item) {
   itemName = item;
}

/**
 Sets the configuration data item value.
 @param value The configuration item value.
 */
void ConfigurationDataItem::setItemValue(const std::string &value) {
   itemValue = value;
}

/**
 Gets the configuration data item name.
 @returns The configuration item name.
 */
const std::string & ConfigurationDataItem::getItemName() const {
   return itemName;
}

/**
 Gets the configuration data item value.
 @returns The configuration item value.
 */
const std::string & ConfigurationDataItem::getItemValue() const {
   return itemValue;
}

/**
 Sets the value of the config item from another's value.
 @param item The item to get the value from.
 @return Reference to this configuration item.
 */
ConfigurationDataItem & ConfigurationDataItem::operator = (const ConfigurationDataItem & item) {
   if (this != &item) {
      itemValue = item.itemValue;
   }
   return *this;
}

/** Equals operator, returns true if item names match.
 @param item Item to compare to.
 @return Returns true if item names match.
 */
bool ConfigurationDataItem::operator == (const ConfigurationDataItem & item) const {
   return itemName == item.itemName;
}


/**
 Parses the configuration data item name and value from the provided string.
 The data is expected to be two strings separated by a tab character.
 @param fromString The string containing the configuration item data name and value.
 @param contentType The content type, must be "nodeconfiguration".
 @returns Returns true if parsing succeeds and content type is correct, otherwise returns false.
 */
bool ConfigurationDataItem::parse(const std::string & fromString, const std::string & contentType) {
   std::vector<std::string> strings;
   boost::split(strings, fromString, boost::is_any_of("\t"));
   if (contentType == "nodeconfiguration" && strings.size() == 2) {
      setItemName(strings.at(0));
      setItemValue(strings.at(1));
      return true;
   }
   return false;
}

/** Adds configuration data value from another configuration data item, if the
 iten name matches.
 @param another The item to copy configuration data from.
 @return Returns true if the item names match and configuration data was copied from another.
 */
bool ConfigurationDataItem::addFrom(const DataItem & another) {
   const ConfigurationDataItem *item = dynamic_cast<const ConfigurationDataItem*>(&another);
   if (item) {
      if (itemName == item->getItemName()) {
         itemValue = item->getItemValue();
         return true;
      }
   }
   return false;
}

/*
 Creates a new copy of this configuration data item.
 @returns a new copy of this configuration data item.
 */
std::unique_ptr<DataItem> ConfigurationDataItem::clone() const {
   return std::make_unique<ConfigurationDataItem>(*this);
}

/**
 Exports a configuration data item object as JSON.
 @param j JSON containing configuration data item.
 @param configItem The configuration data item to export.
 */
void to_json(nlohmann::json & j, const ConfigurationDataItem & configItem) {
   j = nlohmann::json{{configItem.getItemName(), configItem.getItemValue()}};
}

/**
 Imports, from JSON, a configuration data object.
 @param j JSON containing the configuration key-value pair.
 @param configItem The configuration data item from the JSON.
 */
void from_json(const nlohmann::json & j, ConfigurationDataItem & configItem) {
   if (j.is_object()) {
      std::pair<std::string, std::string> keyvalue(j);
      configItem.setItemName(keyvalue.first);
      configItem.setItemValue(keyvalue.second);
   }
}

}
