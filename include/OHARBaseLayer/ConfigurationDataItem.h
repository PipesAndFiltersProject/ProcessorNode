//
//  ConfigurationDataItem.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 25.2.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#ifndef ConfigurationDataItem_h
#define ConfigurationDataItem_h


#include <OHARBaseLayer/DataItem.h>

namespace OHARBase {

   /**
    This class defines the configuration data items a ProcessorNode uses to
    configure itself. Configuration data items have a name and a value defining
    the configuration.
    */
	class ConfigurationDataItem : public DataItem {

	public:
		
		static const std::string CONF_INPUTADDR;
		static const std::string CONF_OUTPUTADDR;
		static const std::string CONF_INPUTFILE;
		static const std::string CONF_OUTPUTFILE;
		
		void setItemName(const std::string &item);
		void setItemValue(const std::string &value);
		const std::string & getItemName() const;
		const std::string & getItemValue() const;
		
		virtual bool parse(const std::string & fromString, const std::string & contentType) override;
		virtual bool addFrom(const DataItem & another) override;
		virtual DataItem * copy() const override;
		
	private:
      /** The name of the configuration item. */
		std::string itemName;
      /** The value of the configuration item. */
		std::string itemValue;
		
		static const std::string TAG;
	};
	
	
}

#endif /* ConfigurationDataItem_h */
