//
//  NodeConfiguration.hpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 25.2.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#ifndef NodeConfiguration_hpp
#define NodeConfiguration_hpp

#include <vector>

#include "ConfigurationDataItem.h"
#include "DataReaderObserver.h"

namespace OHARBase {

	class NodeConfiguration : public DataReaderObserver {
		
	public:
		
		NodeConfiguration() = default;
		~NodeConfiguration();
		
		virtual void handleNewItem(DataItem * item) override;
		
		std::string getValue(const std::string &configName) const;
		
		NodeConfiguration(const NodeConfiguration &) = delete;
		NodeConfiguration * operator =(const NodeConfiguration &) = delete;
		
	private:
		std::vector<ConfigurationDataItem*> configItems;
		
	};



}

#endif /* NodeConfiguration_hpp */
