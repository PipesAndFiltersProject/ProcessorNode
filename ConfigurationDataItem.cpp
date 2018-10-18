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

#include <OHARBaseLayer/ConfigurationDataItem.h>

namespace OHARBase {

	const std::string ConfigurationDataItem::TAG{"ConfigReader"};
   /** Configuration data item name for ProcessorNode incoming address. */
	const std::string ConfigurationDataItem::CONF_INPUTADDR{"input"};
   /** Configuration data item name for ProcessorNode outgoing address. */
	const std::string ConfigurationDataItem::CONF_OUTPUTADDR{"output"};
   /** Configuration data item name for ProcessorNode input data file. */
	const std::string ConfigurationDataItem::CONF_INPUTFILE{"filein"};
   /** Configuration data item name for ProcessorNode output data file. */
	const std::string ConfigurationDataItem::CONF_OUTPUTFILE{"fileout"};
	
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
    Parses the configuration data item name and value from the provided string.
    The data is expected to be two strings separated by a tab character.
    @param fromString The string containing the configuration item data name and value.
    @param contentType The content type, must be "nodeconfiguration".
    @returns Returns true if parsing succeeds and content type is correct, otherwise returns false.
    */
	bool ConfigurationDataItem::parse(const std::string & fromString, const std::string & contentType) {
		std::vector<std::string> strings;
		boost::split(strings, fromString, boost::is_any_of("\t"));
      LOG(INFO) << TAG << "String item count: " << strings.size();
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
	DataItem * ConfigurationDataItem::copy() const {
		return new ConfigurationDataItem(*this);
	}
	
	
}
