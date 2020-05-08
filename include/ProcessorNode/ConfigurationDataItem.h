//
//  ConfigurationDataItem.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 25.2.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#pragma once

#include <ProcessorNode/DataItem.h>

#include <nlohmann/json.hpp>

namespace OHARBase {

/**
 This class defines the configuration data items a ProcessorNode uses to
 configure itself. Configuration data items have a name and a value defining
 the configuration.
 */
class ConfigurationDataItem : public DataItem {
   
public:
   ConfigurationDataItem(const ConfigurationDataItem & item);
   ConfigurationDataItem() = default;
   
   static const std::string CONF_INPUTADDR;
   static const std::string CONF_CONFINADDR;
   static const std::string CONF_CONFOUTADDR;
   static const std::string CONF_OUTPUTADDR;
   static const std::string CONF_INPUTFILE;
   static const std::string CONF_OUTPUTFILE;
   static const std::string CONF_NODENAME;
   static const std::string CONF_ENCRYPT;
   static const std::string CONF_USE_ACK;
   
   void setItemName(const std::string &item);
   void setItemValue(const std::string &value);
   const std::string & getItemName() const;
   const std::string & getItemValue() const;
   
   virtual bool parse(const std::string & fromString, const std::string & contentType) override;
   virtual bool addFrom(const DataItem & another) override;
   virtual std::unique_ptr<DataItem> clone() const override;
   
   ConfigurationDataItem & operator = (const ConfigurationDataItem & item);
   bool operator == (const ConfigurationDataItem & item) const;
   
private:
   /** The name of the configuration item. */
   std::string itemName;
   /** The value of the configuration item. */
   std::string itemValue;
   
   static const std::string TAG;
};

void to_json(nlohmann::json & j, const ConfigurationDataItem & configItem);
void from_json(const nlohmann::json & j, ConfigurationDataItem & configItem);

}

// EOF
