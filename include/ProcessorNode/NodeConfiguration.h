//
//  NodeConfiguration.hpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 25.2.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#pragma once

#include <vector>

#include <ProcessorNode/ConfigurationDataItem.h>
#include <ProcessorNode/DataReaderObserver.h>

namespace OHARBase {

   /**
    This class holds the configuration data for a Node. Configuration
    data is held in ConfigurationDataItem objects, which have a name-value -pairs
    of configuration data.
    */
	class NodeConfiguration : public DataReaderObserver {
		
	public:
		
		NodeConfiguration() = default;
		virtual ~NodeConfiguration();
		
      virtual void handleNewItem(std::unique_ptr<DataItem> item) override;
		
		std::string getValue(const std::string & configName) const;
		
		NodeConfiguration(const NodeConfiguration &) = delete;
		NodeConfiguration * operator =(const NodeConfiguration &) = delete;

      friend void to_json(nlohmann::json & j, const NodeConfiguration & config);
      friend void from_json(const nlohmann::json & j, NodeConfiguration & config);

   private:
      void addOrReplace(const ConfigurationDataItem & item);
      
	private:
        /** Configuration items of the node. */
		std::vector<ConfigurationDataItem> configItems;
      
	};

   void to_json(nlohmann::json & j, const NodeConfiguration & config);
   void from_json(const nlohmann::json & j, NodeConfiguration & config);


}
